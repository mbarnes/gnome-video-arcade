/* PCL - Predicate Constraint Language
 * Copyright (C) 2006 The Boeing Company
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "pcl.h"

#define DEFAULT_BUFFER_SIZE 256

extern PclObject *_pcl_gc_init (void);
extern PclObject *_pcl_math_init (void);
extern PclObject *_pcl_operator_init (void);
extern PclObject *_pcl_signal_init (void);

static PclFrozenModule import_frozen_modules[] = {
        { NULL }
};

static PclObject *silly_list = NULL;
static PclObject *builtins_str = NULL;

PclFrozenModule *PCL_IMPORT_FROZEN_MODULES = import_frozen_modules;

struct ImportInitTable {
        gchar *name;
        PclObject *(*initfunc) (void);
};

static struct ImportInitTable import_init_table[] = {
        { "gc", _pcl_gc_init },
        { "math", _pcl_math_init },
        { "operator", _pcl_operator_init },
        { "signal", _pcl_signal_init },
        { "__builtin__", NULL },
        { "__main__", NULL },
        { "sys", NULL },
        { NULL, NULL }
};

static PclFileDescription import_file_table[] = {
        { ".pcl", "r", PCL_IMPORT_SOURCE },
        { "." G_MODULE_SUFFIX, "rb", PCL_IMPORT_EXTENSION },
        { NULL }
};

/* Foward Declarations */
static PclCode *import_parse_source_module (
                const gchar *pathname, FILE *stream);
static PclObject *import_load_source_module (
                const gchar *name, const gchar *pathname, FILE *stream);
static PclObject *import_load_dynamic_module (
                const gchar *name, const gchar *pathname, FILE *stream);
static PclObject *import_load_package (
                const gchar *name, const gchar *pathname);
static PclObject *import_get_parent (
                PclObject *globals, GString *buffer);
static gboolean import_is_builtin (const gchar *name);
static PclFrozenModule *import_find_frozen (const gchar *name);
static gboolean import_find_init_module (GString *buffer);
static PclObject *import_get_path_importer (
                PclObject *path_importer_cache, PclObject *path_hooks,
                PclObject *path);
static gboolean import_init_builtin (const gchar *name);
static PclFileDescription *import_find_module (
                const gchar *fullname, const gchar *subname, PclObject *path,
                GString *buffer, FILE **p_stream, PclObject **p_loader);
static PclObject *import_load_module (
                const gchar *name, FILE *stream, GString *buffer,
                PclImportType type, PclObject *loader);
static gboolean import_add_submodule (
                PclObject *module, PclObject *submodule,
                const gchar *fullname, const gchar *subname);
static PclObject *import_submodule (
                PclObject *module, const gchar *subname,
                const gchar *fullname);
static gboolean import_ensure_fromlist (
                PclObject *module, PclObject *fromlist, GString *buffer,
                gboolean recursive);
static gboolean import_mark_miss (const gchar *name);
static PclObject *import_load_next (
                PclObject *module, PclObject *alt_module,
                const gchar **p_name, GString *buffer);
static PclObject *import_module_ex (
                const gchar *name, PclObject *globals,
                PclObject *locals, PclObject *fromlist);

static PclCode *
import_parse_source_module (const gchar *pathname, FILE *stream)
{
        PclParseInfo *parse_info;
        PclCode *code;

        parse_info = pcl_parse_input_file (stream, pathname, PCL_FILE_INPUT);
        if (parse_info == NULL)
                return NULL;
        code = pcl_compile (parse_info->parse_tree, pathname, NULL);
        pcl_parse_info_destroy (parse_info);
        return code;
}

static PclObject *
import_load_source_module (const gchar *name, const gchar *pathname,
                           FILE *stream)
{
        /* XXX Highly simplified from Python's version */
        PclObject *module;
        PclCode *code;

        code = import_parse_source_module (pathname, stream);
        if (code == NULL)
                return NULL;
        module = pcl_import_exec_code_module_ex (
                 name, PCL_OBJECT (code), pathname);
        pcl_object_unref (code);
        return module;
}

static PclObject *
import_load_dynamic_module (const gchar *name, const gchar *pathname,
                            FILE *stream)
{
        PclObject *module;
        GModule *g_module;

        g_assert (g_module_supported ());

        g_module = g_module_open (pathname, 0);
        if (g_module == NULL)
        {
                pcl_error_set_string (
                        pcl_exception_import_error (),
                        g_module_error ());
                return NULL;
        }

        module = pcl_dict_get_item_string (
                 pcl_import_get_module_dict (), name);

        /* ERROR: dynamic module not initialized properly */
        g_assert (module != NULL);

        PCL_MODULE (module)->g_module = g_module;
        pcl_module_add_string (module, "__file__", pathname);

        return pcl_object_ref (module);
}

static PclObject *
import_load_package (const gchar *name, const gchar *pathname)
{
        PclFileDescription *fdp;
        PclObject *dict, *module;
        PclObject *path = NULL;
        PclObject *file = NULL;
        GString *buffer = NULL;
        FILE *stream = NULL;

        if ((module = pcl_import_add_module (name)) == NULL)
                return NULL;
        dict = pcl_module_get_dict (module);
        if ((file = pcl_string_from_string (pathname)) == NULL)
                goto error;
        if ((path = pcl_build_value ("[O]", file)) == NULL)
                goto error;
        if (!pcl_dict_set_item_string (dict, "__file__", file) ||
                        !pcl_dict_set_item_string (dict, "__path__", path))
                goto error;
        buffer = g_string_sized_new (DEFAULT_BUFFER_SIZE);
        fdp = import_find_module (name, "__init__", path,
                                  buffer, &stream, NULL);
        if (fdp == NULL)
        {
                if (pcl_error_exception_matches (
                                pcl_exception_import_error ()))
                {
                        pcl_error_clear ();
                        pcl_object_ref (module);
                }
                else
                        module = NULL;
                goto cleanup;
        }
        module = import_load_module (name, stream, buffer, fdp->type, NULL);
        if (stream != NULL)
                fclose (stream);
        goto cleanup;

error:
        module = NULL;
cleanup:
        if (path != NULL)
                pcl_object_unref (path);
        if (file != NULL)
                pcl_object_unref (file);
        if (buffer != NULL)
                g_string_free (buffer, TRUE);
        return module;
}

static PclObject *
import_get_parent (PclObject *globals, GString *buffer)
{
        PclObject *module_name;
        PclObject *module_path;
        PclObject *sys_modules;
        PclObject *parent;

        if (!PCL_IS_DICT (globals))
                return PCL_NONE;

        /* Return the package that an import is being performed in.  If
         * globals comes from the module foo.bar.bat (not itself a package),
         * this returns the sys.modules entry for foo.bar.  If globals is
         * from a package's __init__.pcl, the package's entry in sys.modules
         * is returned.  If globals doesn't come from a package or a module
         * in a package, or a corresponding entry is not found in sys.modules,
         * None is returned.
         *
         *   if not globals.has_key('__name__'):
         *       return None
         *   if globals.has_key('__path__'):
         *        buffer = globals['__name__']
         *    else:
         *        lastdot = globals['__name__'].rfind('.')
         *        if lastdot < 0:
         *            return None
         *        buffer = globals['__name__'][:lastdot] 
         *   return sys.modules.get(buffer, None)
         */

        module_name = pcl_dict_get_item_string (globals, "__name__");
        if (!PCL_IS_STRING (module_name))
                return PCL_NONE;

        module_path = pcl_dict_get_item_string (globals, "__path__");
        if (PCL_IS_STRING (module_path))
                g_string_assign (buffer, pcl_string_as_string (module_name));
        else
        {
                gchar *start = pcl_string_as_string (module_name);
                gchar *lastdot = strrchr (start, '.');
                gssize len;
                if (lastdot == NULL)
                        return PCL_NONE;
                len = lastdot - start;
                g_string_append_len (buffer, start, len);
        }

        sys_modules = pcl_import_get_module_dict ();
        parent = pcl_dict_get_item_string (sys_modules, buffer->str);
        if (parent == NULL)
                parent = PCL_NONE;
        return parent;
}

static gboolean
import_is_builtin (const gchar *name)
{
        struct ImportInitTable *builtin;
        for (builtin = import_init_table; builtin->name != NULL; builtin++)
        {
                if (strcmp (name, builtin->name) == 0)
                        return TRUE;
        }
        return FALSE;
}

static PclFrozenModule *
import_find_frozen (const gchar *name)
{
        PclFrozenModule *fm = PCL_IMPORT_FROZEN_MODULES;
        while (fm != NULL)
        {
                if (fm->name == NULL)
                        return NULL;
                if (strcmp (fm->name, name) == 0)
                        break;
        }
        return fm;
}

static gboolean
import_find_init_module (GString *buffer)
{
        gsize saved_len = buffer->len;
        gboolean found;

        g_string_append_printf (buffer, "%c__init__.pcl", G_DIR_SEPARATOR);
        found = g_file_test (buffer->str, G_FILE_TEST_EXISTS);
        g_string_truncate (buffer, saved_len);
        return found;
}

static PclObject *
import_get_path_importer (PclObject *path_importer_cache,
                          PclObject *path_hooks, PclObject *path)
{
        /* Return an importer object for a sys.path/pkg.__path__ item 'path',
         * possibly by fetching it from the path_importer_cache dict.  If it
         * wasn't yet cached, traverse path_hooks until a hook is found that
         * can handle the path item.  Return None if no hook could; this tells
         * our caller it should fall back to the built-in import mechanism.
         * Cache the result in path_importer_cache.  Returns a borrowed
         * reference. */

        PclObject *importer;
        glong ii, path_length;

        /* These conditions are the caller's responsibility */
        g_assert (PCL_IS_LIST (path_hooks));
        g_assert (PCL_IS_DICT (path_importer_cache));

        importer = pcl_dict_get_item (path_importer_cache, path);
        if (importer != NULL)
                return importer;

        /* set path_importer_cache[path] to None to avoid recursion */
        if (!pcl_dict_set_item (path_importer_cache, path, PCL_NONE))
                return NULL;

        path_length = pcl_object_measure (path_hooks);
        for (ii = 0; ii < path_length; ii++)
        {
                PclObject *hook;
                hook = pcl_list_get_item (path_hooks, ii);
                if (hook == NULL)
                        return NULL;
                importer = pcl_object_call_function (hook, "O", path);
                if (importer != NULL)
                        break;
                if (!pcl_error_exception_matches (
                                pcl_exception_import_error ()))
                        return NULL;
                pcl_error_clear ();
        }

        if (importer == NULL)
                importer = PCL_NONE;
        else if (importer != PCL_NONE)
        {
                gboolean success;
                success = pcl_dict_set_item (
                          path_importer_cache, path, importer);
                pcl_object_unref (importer);
                if (!success)
                        return NULL;
        }
        return importer;
}

static gboolean
import_init_builtin (const gchar *name)
{
        struct ImportInitTable *builtin;
        for (builtin = import_init_table; builtin->name != NULL; builtin++)
        {
                if (strcmp (name, builtin->name) == 0)
                {
                        g_assert (builtin->initfunc != NULL);
                        (*builtin->initfunc) ();
                        return TRUE;
                }
        }
        return FALSE;
}

static PclFileDescription *
import_find_module (const gchar *fullname, const gchar *subname,
                    PclObject *path, GString *buffer, FILE **p_stream,
                    PclObject **p_loader)
{
        /* Search the path (default sys.path) for a module.  Return the
         * corresponding PclFileDescription struct, and (via return arguments)
         * the pathname and an open file.  Return NULL if the module is not
         * found. */

        static PclFileDescription fd_builtin = { "", "", PCL_IMPORT_BUILTIN };
        static PclFileDescription fd_package = { "", "", PCL_IMPORT_PACKAGE };
        static PclFileDescription fd_frozen = { "", "", PCL_IMPORT_FROZEN };
        static PclFileDescription fd_hook = { "", "", PCL_IMPORT_HOOK };

        PclFileDescription *fdp = NULL;
        FILE *stream = NULL;
        PclObject *path_hooks;
        PclObject *path_importer_cache;
        glong ii, path_length;
        gchar *name;

        if (p_loader != NULL)
                *p_loader = NULL;

        name = g_strdup (subname);

        /* sys.meta_path import hook */
        if (p_loader != NULL)
        {
                PclObject *meta_path;

                meta_path = pcl_sys_get_object ("meta_path");
                if (!PCL_IS_LIST (meta_path))
                {
                        pcl_error_set_string (
                                pcl_exception_import_error (),
                                "sys.meta_path must be a list of "
                                "import hooks");
                        return NULL;
                }

                pcl_object_ref (meta_path);
                path_length = pcl_object_measure (meta_path);
                for (ii = 0; ii < path_length; ii++)
                {
                        PclObject *hook, *loader;
                        hook = pcl_list_get_item (meta_path, ii);
                        loader = pcl_object_call_method (
                                 hook, "find_module", "sO", fullname,
                                 (path != NULL) ? path : PCL_NONE);
                        if (loader == NULL)
                        {
                                pcl_object_unref (meta_path);
                                return NULL;
                        }
                        if (loader != PCL_NONE)
                        {
                                *p_loader = loader;
                                pcl_object_unref (meta_path);
                                return &fd_hook;
                        }
                }
                pcl_object_unref (meta_path);
        }

        if (PCL_IS_STRING (path))
        {
                /* The only type of submodule allowed inside a "frozen"
                 * package are other frozen modules or packages. */
                g_string_printf (buffer, "%s.%s",
                        pcl_string_as_string (path), name);
                g_free (name);
                name = g_strdup (buffer->str);
                if (import_find_frozen (name) != NULL)
                {
                        g_string_assign (buffer, name);
                        return &fd_frozen;
                }
                pcl_error_set_format (
                        pcl_exception_import_error (),
                        "no frozen submodule named %s", name);
                return NULL;
        }

        if (path == NULL)
        {
                if (import_is_builtin (name))
                {
                        g_string_assign (buffer, name);
                        return &fd_builtin;
                }

                if (import_find_frozen (name) != NULL)
                {
                        g_string_assign (buffer, name);
                        return &fd_frozen;
                }

                path = pcl_sys_get_object ("path");
        }

        if (!PCL_IS_LIST (path))
        {
                pcl_error_set_string (
                        pcl_exception_import_error (),
                        "sys.path must be a list of directory names");
                return NULL;
        }

        /* XXX Leave this disabled for now... */
        /*     Need to implement _PyImportHooks_Init equivalent. */
        /* path_hooks = pcl_sys_get_object ("path_hooks");
        if (!PCL_IS_LIST (path_hooks))
        {
                pcl_error_set_string (
                        pcl_exception_import_error (),
                        "sys.path_hooks must be a list of import hooks");
                return NULL;
        }

        path_importer_cache = pcl_sys_get_object ("path_importer_cache");
        if (!PCL_IS_DICT (path_importer_cache))
        {
                pcl_error_set_string (
                        pcl_exception_import_error (),
                        "sys.path_importer_cache must be a dictionary");
                return NULL;
        }*/
        
        path_length = pcl_object_measure (path);
        for (ii = 0; ii < path_length; ++ii)
        {
                PclObject *item = pcl_list_get_item (path, ii);
                gsize saved_len;

                if (!PCL_IS_STRING (item))
                        continue;

                /* TODO: Check for '\0' in item */

                /* sys.path_hooks import hook */
                if (p_loader != NULL)
                {
                        PclObject *importer;

                        /* note: importer is a borrowed reference */
                        importer = import_get_path_importer (
                                   path_importer_cache, path_hooks, item);
                        if (importer == NULL)
                                return NULL;
                        if (importer != PCL_NONE)
                        {
                                PclObject *loader;

                                loader = pcl_object_call_method (importer,
                                         "find_module", "s", fullname);
                                if (loader == NULL)
                                        return NULL;
                                if (loader != PCL_NONE)
                                {
                                        *p_loader = loader;
                                        return &fd_hook;
                                }
                                pcl_object_unref (loader);
                        }
                        /* no hook was successful, use built-in import */
                }

                g_string_assign (buffer, pcl_string_as_string (item));

                /* Replace an empty path with a dot to indicate the current
                 * working directory. */
                if (buffer->len == 0)
                        g_string_append_c (buffer, '.');

                if (buffer->str[buffer->len - 1] != G_DIR_SEPARATOR)
                        g_string_append_c (buffer, G_DIR_SEPARATOR);
                g_string_append (buffer, name);

                if (g_file_test (buffer->str, G_FILE_TEST_IS_DIR) &&
                    import_find_init_module (buffer))
                        return &fd_package;

                saved_len = buffer->len;
                for (fdp = import_file_table; fdp->suffix != NULL; ++fdp)
                {
                        g_string_append (buffer, fdp->suffix);
                        stream = fopen (buffer->str, fdp->mode);
                        if (stream != NULL)
                                break;
                        g_string_truncate (buffer, saved_len);
                }

                if (stream != NULL)
                        break;
        }

        if (stream == NULL)
        {
                pcl_error_set_format (
                        pcl_exception_import_error (),
                        "no module named %s", name);

                return NULL;
        }

        *p_stream = stream;
        return fdp;
}

static PclObject *
import_load_module (const gchar *name, FILE *stream, GString *buffer,
                    PclImportType type, PclObject *loader)
{
        /* Load an external module using the default search path and return
         * its module object with incremented reference count. */

        PclObject *module = NULL;
        PclObject *sys_modules;
        gboolean success;

        /* First check that there's an open file (if we need one) */
        if (type == PCL_IMPORT_SOURCE || type == PCL_IMPORT_COMPILED)
        {
                /* XXX What about PCL_IMPORT_EXTENSION? */
                if (stream == NULL)
                {
                        pcl_error_set_format (
                                pcl_exception_value_error (),
                                "file object required for import "
                                "(type code %d)", type);
                        return NULL;
                }
        }

        switch (type)
        {
                case PCL_IMPORT_SOURCE:
                        module = import_load_source_module (
                                 name, buffer->str, stream);
                        break;

                /* case PCL_IMPORT_COMPILED:
                 *      module = import_load_compiled_module (
                 *               name, buffer->str, stream);
                 *      break; */

                case PCL_IMPORT_EXTENSION:
                        module = import_load_dynamic_module (
                                 name, buffer->str, stream);
                        break;

                case PCL_IMPORT_PACKAGE:
                        module = import_load_package (name, buffer->str);
                        break;
        
                case PCL_IMPORT_BUILTIN:
                case PCL_IMPORT_FROZEN:
                        if (buffer->len > 0)
                                name = buffer->str;
                        if (type == PCL_IMPORT_BUILTIN)
                                success = import_init_builtin (name);
                        else
                                /* XXX Not yet supported */
                                success = FALSE;
                        if (!success)
                        {
                                if (!pcl_error_occurred ())
                                        pcl_error_set_format (
                                                pcl_exception_import_error (),
                                                "purported %s import %s "
                                                "not found",
                                                (type == PCL_IMPORT_BUILTIN)
                                                ? "builtin" : "frozen", name);
                                return NULL;
                        }
                        sys_modules = pcl_import_get_module_dict ();
                        module = pcl_dict_get_item_string (sys_modules, name);
                        if (module == NULL)
                        {
                                pcl_error_set_format (
                                        pcl_exception_import_error (),
                                        "%s module %s not property "
                                        "initialized",
                                        (type == PCL_IMPORT_BUILTIN)
                                        ? "builtin" : "frozen", name);
                                return NULL;
                        }
                        pcl_object_ref (module);
                        break;

                case PCL_IMPORT_HOOK:
                        if (loader == NULL)
                        {
                                pcl_error_set_string (
                                        pcl_exception_import_error (),
                                        "import hook without loader");
                                return NULL;
                        }
                        module = pcl_object_call_method (
                                 loader, "load_module", "s", name);
                        break;

                default:
                        pcl_error_set_format (
                                pcl_exception_import_error (),
                                "don't know how to import %s (type code %d)",
                                name, type);
        }

        return module;
}

static gboolean
import_add_submodule (PclObject *module, PclObject *submodule,
                      const gchar *fullname, const gchar *subname)
{
        PclObject *sys_modules = pcl_import_get_module_dict ();

        if (PCL_IS_NONE (module))
                return TRUE;

        /* Irrespective of the success of this load, make a reference to it
         * in the parent package module.  A copy gets saved in the modules
         * dictionary under the full name, so get a reference from there, if
         * need be. */
        if (submodule == NULL)
                submodule = pcl_dict_get_item_string (sys_modules, fullname);

        if (submodule != NULL)
                return pcl_object_set_attr_string (module, subname, submodule);

        return TRUE;
}

static PclObject *
import_submodule (PclObject *module, const gchar *subname,
                  const gchar *fullname)
{
        PclObject *sys_modules = pcl_import_get_module_dict ();
        PclObject *submodule = NULL;

        /* Require:
         * if module == None: subname == fullname
         * else: module.__name__ + "." + subname == fullname */

        submodule = pcl_dict_get_item_string (sys_modules, fullname);
        if (submodule != NULL)
                pcl_object_ref (submodule);
        else
        {
                PclObject *path = NULL;
                PclObject *loader = NULL;
                GString *buffer;
                PclFileDescription *fdp;
                FILE *fp = NULL;

                if (module != PCL_NONE)
                {
                        path = pcl_object_get_attr_string (module, "__path__");
                        if (path == NULL)
                        {
                                pcl_error_clear ();
                                return pcl_object_ref (PCL_NONE);
                        }
                }

                buffer = g_string_sized_new (DEFAULT_BUFFER_SIZE);
                /* XXX Leave p_loader arg as NULL for now... */
                /*     Need to implement _PyImportHooks_Init equivalent. */
                fdp = import_find_module (fullname, subname, path,
                                          buffer, &fp, NULL /*&loader*/);
                if (path != NULL)
                        pcl_object_unref (path);
                if (fdp == NULL)
                {
                        if (!pcl_error_exception_matches (
                                        pcl_exception_import_error ()))
                                return NULL;
                        pcl_error_clear ();
                        return pcl_object_ref (PCL_NONE);
                }
                submodule = import_load_module (fullname, fp, buffer,
                                                fdp->type, loader);
                if (loader != NULL)
                        pcl_object_unref (loader);
                if (fp != NULL)
                        fclose (fp);
                if (!import_add_submodule (
                        module, submodule, fullname, subname))
                {
                        if (submodule != NULL)
                                pcl_object_unref (submodule);
                        submodule = NULL;
                }
                g_string_free (buffer, TRUE);
        }

        return submodule;
}

static gboolean
import_ensure_fromlist (PclObject *module, PclObject *fromlist,
                        GString *buffer, gboolean recursive)
{
        PclObject *iterator;
        PclObject *next;

        /* Pseudo-code (sans error-handling):
         *
         *   if hasattr(module, '__path__'):
         *       return True;
         *   for item in fromlist:
         *       if item[0] == '*':
         *           if recursive:
         *               continue
         *           if hasattr(module, '__all__'):
         *               import_ensure_fromlist(module, fromlist, buffer, True)
         *           continue
         *       if hasattr(module, item):
         *           buffer += '.' + item
         *           import_submodule(module, item, buffer)
         *   return True
         */

        if (!pcl_object_has_attr_string (module, "__path__"))
                return TRUE;

        iterator = pcl_object_iterate (fromlist);
        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                if (!PCL_IS_STRING (next))
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "item in ``from list'' not a string");
                        pcl_object_unref (iterator);
                        pcl_object_unref (next);
                        return FALSE;
                }
                if (pcl_string_as_string (next)[0] == '*')
                {
                        PclObject *all;
                        pcl_object_unref (next);
                        /* See if the package defines __all__ */
                        if (recursive)
                                continue;  /* avoid endless recursion */
                        all = pcl_object_get_attr_string (module, "__all__");
                        if (all == NULL)
                                pcl_error_clear ();
                        else
                        {
                                gboolean success;
                                success = import_ensure_fromlist (
                                          module, all, buffer, TRUE);
                                pcl_object_unref (all);
                                if (!success)
                                {
                                        pcl_object_unref (iterator);
                                        return FALSE;
                                }
                        }
                        continue;
                }

                if (pcl_object_has_attr (module, next))
                {
                        PclObject *submodule;
                        gchar *subname = pcl_string_as_string (next);
                        g_string_append_c (buffer, '.');
                        g_string_append (buffer, subname);
                        submodule = import_submodule (module, subname,
                                                      buffer->str);
                        if (submodule != NULL)
                                pcl_object_unref (submodule);
                        else
                        {
                                pcl_object_unref (iterator);
                                pcl_object_unref (next);
                                return FALSE;
                        }
                }

                pcl_object_unref (next);
        }
        pcl_object_unref (iterator);
        return TRUE;
}

static gboolean
import_mark_miss (const gchar *name)
{
        /* sys.modules[name] = None */
        PclObject *sys_modules = pcl_import_get_module_dict ();
        return pcl_dict_set_item_string (sys_modules, name, PCL_NONE);
}

static PclObject *
import_load_next (PclObject *module, PclObject *alt_module,
                  const gchar **p_name, GString *buffer)
{
        const gchar *name = *p_name;
        const gchar *dot = strchr (name, '.');
        gssize len, saved_len;
        PclObject *submodule;

        g_assert (PCL_IS_NONE (alt_module) || alt_module == module);

        /* Find the length of the segment up to (but not including) the first
         * dot in *p_name, then point *p_name to the character following the
         * first dot (or NULL if there are no dots). */
        if (dot == NULL)
        {
                *p_name = NULL;
                len = strlen (name);
        }
        else
        {
                *p_name = dot + 1;
                len = dot - name;
        }
        if (len == 0)
        {
                pcl_error_set_string (
                        pcl_exception_value_error (),
                        "empty module name");
                return NULL;
        }

        /* Append the segment to the buffer. */
        if (buffer->len > 0)
                g_string_append_c (buffer, '.');
        saved_len = buffer->len;
        g_string_append_len (buffer, name, len);

        /* XXX Not really sure what's going on here. */
        submodule = import_submodule (module, name, buffer->str);
        if (PCL_IS_NONE (submodule) && alt_module != module)
        {
                pcl_object_unref (submodule);
                /* Here, alt_module must be None and module must not be None */
                submodule = import_submodule (alt_module, name, name);
                if (submodule != NULL && !PCL_IS_NONE (submodule))
                {
                        if (!import_mark_miss (buffer->str))
                        {
                                pcl_object_unref (submodule);
                                return NULL;
                        }
                        g_string_truncate (buffer, saved_len);
                        g_string_append_len (buffer, name, len);
                }
        }

        if (submodule == NULL)
                return NULL;

        if (submodule == PCL_NONE)
        {
                pcl_object_unref (PCL_NONE);
                pcl_error_set_format (
                        pcl_exception_import_error (),
                        "no module named %s", name);
                return NULL;
        }

        return submodule;
}

static PclObject *
import_module_ex (const gchar *name, PclObject *globals,
                  PclObject *locals, PclObject *fromlist)
{
        GString *buffer = g_string_sized_new (DEFAULT_BUFFER_SIZE);
        PclObject *parent, *head, *next, *tail;

        parent = import_get_parent (globals, buffer);
        if (parent == NULL)
        {
                g_string_free (buffer, TRUE);
                return NULL;
        }

        head = import_load_next (parent, PCL_NONE, &name, buffer);
        if (head == NULL)
        {
                g_string_free (buffer, TRUE);
                return NULL;
        }

        tail = pcl_object_ref (head);
        while (name != NULL)
        {
                next = import_load_next (tail, tail, &name, buffer);
                pcl_object_unref (tail);
                if (next == NULL)
                {
                        pcl_object_unref (head);
                        g_string_free (buffer, TRUE);
                        return NULL;
                }
                tail = next;
        }

        if (fromlist != NULL && !pcl_object_is_true (fromlist))
                fromlist = NULL;

        if (fromlist == NULL)
        {
                pcl_object_unref (tail);
                g_string_free (buffer, TRUE);
                return head;
        }

        pcl_object_unref (head);
        if (!import_ensure_fromlist (tail, fromlist, buffer, FALSE))
        {
                pcl_object_unref (tail);
                g_string_free (buffer, TRUE);
                return NULL;
        }

        g_string_free (buffer, TRUE);
        return tail;
}

PclObject *
pcl_import_module (const gchar *name)
{
        PclObject *string;
        PclObject *result;

        string = pcl_string_from_string (name);
        if (string == NULL)
                return NULL;
        result = pcl_import (string);
        pcl_object_unref (string);
        return result;
}

PclObject *
pcl_import_module_ex (const gchar *name, PclObject *globals,
                      PclObject *locals, PclObject *fromlist)
{
        PclObject *result;
        /* XXX lock_import (); */
        result = import_module_ex (name, globals, locals, fromlist);
        /* XXX if (unlock_import () < 0)
         *     {
         *             pcl_object_unref (result);
         *             pcl_error_set_string (
         *                     pcl_exception_runtime_error (),
         *                     "not holding the import lock");
         *             return NULL;
         *     } */
        return result;
}

PclObject *
pcl_import (PclObject *module_name)
{
        PclObject *globals = NULL;
        PclObject *import = NULL;
        PclObject *builtins = NULL;
        PclObject *result = NULL;

        /* Initialize constant string objects */
        if (G_UNLIKELY (builtins_str == NULL))
        {
                builtins_str = pcl_string_intern_from_string ("__builtins__");
                if (builtins_str == NULL)
                        return NULL;
        }
        if (G_UNLIKELY (silly_list == NULL))
        {
                silly_list = pcl_build_value ("[s]", "__doc__");
                if (silly_list == NULL)
                        return NULL;
        }

        /* Get the builtins from current globals */
        globals = pcl_eval_get_globals ();
        if (globals != NULL)
        {
                pcl_object_ref (globals);
                builtins = pcl_object_get_item (globals, builtins_str);
                if (builtins == NULL)
                        goto error;
        }
        else
        {
                /* No globals -- use standard builtins, and fake globals */
                pcl_error_clear ();
                builtins = pcl_import_module_ex (
                           "__builtin__", NULL, NULL, NULL);
                if (builtins == NULL)
                        return NULL;
                globals = pcl_build_value ("{OO}", builtins_str, builtins);
                if (globals == NULL)
                        goto error;
        }

        /* Get the __import__ function from the builtins */
        if (PCL_IS_DICT (builtins))
                import = pcl_dict_get_item_string (builtins, "__import__");
        else
                import = pcl_object_get_attr_string (builtins, "__import__");
        if (import == NULL)
                goto error;

        /* Call the __import__ function with the proper argument list. */
        result = pcl_object_call_function (import, "OOOO",
                        module_name, globals, globals, silly_list);

error:

        if (globals != NULL)
                pcl_object_unref (globals);
        if (builtins != NULL)
                pcl_object_unref (builtins);
        if (import != NULL)
                pcl_object_unref (import);

        return result;
}

PclObject *
pcl_import_reload_module (PclObject *module)
{
        PclObject *sys_modules = pcl_import_get_module_dict ();
        PclObject *new_module = NULL;
        PclObject *path = NULL;
        gchar *name, *subname;

        /* Test the argument for validity */
        if (!PCL_IS_MODULE (module))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "reload() argument must be module");
                return NULL;
        }
        name = pcl_module_get_name (module);
        if (name == NULL)
                return NULL;
        if (module != pcl_dict_get_item_string (sys_modules, name))
        {
                pcl_error_set_format (
                        pcl_exception_import_error (),
                        "reload(): module %s not in sys.modules", name);
                return NULL;
        }

        subname = strrchr (name, '.');
        if (subname == NULL)
                subname = name;
        else
        {
                PclObject *parent_name, *parent;
                parent_name = pcl_string_from_string_and_size (
                              name, (subname - name));
                if (parent_name == NULL)
                        return NULL;
                parent = pcl_dict_get_item (sys_modules, parent_name);
                if (parent == NULL)
                {
                        pcl_error_set_format (
                                pcl_exception_import_error (),
                                "reload(): parent %s not in sys.modules",
                                PCL_STRING_AS_STRING (parent_name));
                        pcl_object_unref (parent_name);
                        return NULL;
                }
                pcl_object_unref (parent_name);
                subname++;  /* move past the dot */
                path = pcl_object_get_attr_string (parent, "__path__");
                if (path == NULL)
                        pcl_error_clear ();
        }

        /* TODO: find module, load module */

        return new_module;
}

PclObject *
pcl_import_add_module (const gchar *name)
{
        PclObject *sys_modules = pcl_import_get_module_dict ();
        PclObject *module = pcl_dict_get_item_string (sys_modules, name);

        if (PCL_IS_MODULE (module))
                return module;

        module = pcl_module_new (name);
        if (!pcl_dict_set_item_string (sys_modules, name, module))
        {
                pcl_object_unref (module);
                return NULL;
        }
        pcl_object_unref (module);

        /* Returning borrowed reference from sys.modules */
        return module;
}

PclObject *
pcl_import_exec_code_module (const gchar *name, PclObject *code)
{
        return pcl_import_exec_code_module_ex (name, code, NULL);
}

PclObject *
pcl_import_exec_code_module_ex (const gchar *name, PclObject *code,
                                const gchar *pathname)
{
        PclObject *sys_modules = pcl_import_get_module_dict ();
        PclObject *module, *globals, *locals, *object;

        g_assert (PCL_IS_CODE (code));

        module = pcl_import_add_module (name);
        if (module == NULL)
                return NULL;

        /* If the module is being reloaded, we get the old module back and
         * reuse its __dict__ to execute the new code. */
        globals = locals = pcl_module_get_dict (module);
        if (pcl_dict_get_item_string (globals, "__builtins__") == NULL)
        {
                PclObject *builtins = pcl_eval_get_builtins ();
                if (!pcl_dict_set_item_string (globals,
                                "__builtins__", builtins))
                        goto error;
        }

        /* Remember the filename as the __file__ attribute */
        object = NULL;
        if (pathname != NULL)
        {
                object = pcl_string_from_string (pathname);
                if (object == NULL)
                        pcl_error_clear ();
        }
        if (object == NULL)
                object = PCL_CODE (code)->filename;
        if (!pcl_dict_set_item_string (globals, "__file__", object))
                pcl_error_clear ();  /* Not important enough to report */
        pcl_object_unref (object);

        /* Evaluate the code and discard the result. */
        object = pcl_eval_code (PCL_CODE (code), globals, locals);
        if (object == NULL)
                goto error;
        pcl_object_unref (object);

        module = pcl_dict_get_item_string (sys_modules, name);
        if (module == NULL)
        {
                pcl_error_set_format (
                        pcl_exception_import_error (),
                        "loaded module %s not found in sys.modules", name);
                return NULL;
        }

        return pcl_object_ref (module);

error:

        /* Inlined _RemoveModule() */
        if (pcl_dict_has_key_string (sys_modules, name) == 1)
                if (!pcl_dict_del_item_string (sys_modules, name))
                        g_error ("import: deleting existing key in "
                                 "sys.modules failed");
        return NULL;
}

PclObject *
pcl_import_get_module_dict (void)
{
        PclThreadState *ts = pcl_thread_state_get ();
        if (ts->is->modules == NULL)
                g_error ("%s: No module dictionary", G_STRFUNC);
        return ts->is->modules;
}

void
pcl_import_cleanup (void)
{
        PclInterpreterState *is;
        PclObject *dict;
        PclObject *key;
        PclObject *modules;
        PclObject *value;
        gboolean done;
        gchar *name;
        glong pos;

        static gchar *sys_names[] = {
                "argv",
                "meta_path",
                "path",
                "path_hooks",
                "path_importer_cache",
                "ps1",
                "ps2",
                NULL
        };
        static gchar *sys_files[] = {
                "stdin",  "__stdin__",
                "stdout", "__stdout__",
                "stderr", "__stderr__",
                NULL
        };

        is = pcl_thread_state_get ()->is;
        modules = is->modules;

        if (modules == NULL)
                return;  /* already done */

        /* From Python:
         * Delete some special variables first.  These are common places where
         * user values hide and people complain when their destructors fail.
         * Since the modules containing them are deleted *last* of all, they
         * would come too late in the normal destruction order.  Sigh. */
        value = pcl_dict_get_item_string (modules, "__builtin__");
        if (PCL_IS_MODULE (value))
        {
                dict = pcl_module_get_dict (value);
                pcl_dict_set_item_string (dict, "_", PCL_NONE);
        }
        value = pcl_dict_get_item_string (modules, "sys");
        if (PCL_IS_MODULE (value))
        {
                gchar **p_name;
                dict = pcl_module_get_dict (value);
                for (p_name = sys_names; *p_name != NULL; p_name++)
                        pcl_dict_set_item_string (dict, *p_name, PCL_NONE);
                for (p_name = sys_files; *p_name != NULL; p_name += 2)
                {
                        PclObject *orig;
                        orig = pcl_dict_get_item_string (dict, *(p_name + 1));
                        if (orig == NULL)
                                orig = PCL_NONE;
                        pcl_dict_set_item_string (dict, *p_name, orig);
                }
        }

        /* Clear the __main__ module first. */
        value = pcl_dict_get_item_string (modules, "__main__");
        if (PCL_IS_MODULE (value))
        {
                _pcl_module_clear (value);
                pcl_dict_set_item_string (modules, "__main__", PCL_NONE);
        }

        /* Next, repeatedly look for modules with a reference count of one
         * (skipping __builtin__ and sys) and delete them.
         *
         * The special treatment of __builtin__ here is because even when
         * it's not referenced as a module, its dictionary is referenced by
         * almost every module's __builtins__.  Since deleting a module
         * clears its dictionary (even if there are references left to it),
         * we need to delete the __builtin__ module last.  Likewise, we don't
         * delete sys until the very end because it is implicitly referenced
         * (e.g. by print).
         *
         * Also note that we 'delete' modules by replacing their entry in
         * the modules dictionary with None, rather than really deleting
         * them; this avoids a rehash of the modules dictionary and also
         * marks them as "non-existent" so they won't be re-imported. */
        done = FALSE;
        while (!done)
        {
                pos = 0;
                done = TRUE;  /* assume we're done */
                while (pcl_dict_next (modules, &pos, &key, &value))
                {
                        /* XXX GLib does not provide a documented way to
                         *     access an object's reference count. */
                        if (G_OBJECT (modules)->ref_count != 1)
                                continue;
                        if (!PCL_IS_STRING (key) || !PCL_IS_MODULE (value))
                                continue;
                        name = PCL_STRING_AS_STRING (key);
                        if (strcmp (name, "__builtin__") == 0)
                                continue;
                        if (strcmp (name, "sys") == 0)
                                continue;
                        _pcl_module_clear (value);
                        pcl_dict_set_item (modules, key, PCL_NONE);
                        done = FALSE;
                }
        }

        /* Next, delete all modules (still skipping __builtin__ and sys). */
        pos = 0;
        while (pcl_dict_next (modules, &pos, &key, &value))
        {
                if (!PCL_IS_STRING (key) || !PCL_IS_MODULE (value))
                        continue;
                name = PCL_STRING_AS_STRING (key);
                if (strcmp (name, "__builtin__") == 0)
                        continue;
                if (strcmp (name, "sys") == 0)
                        continue;
                _pcl_module_clear (value);
                pcl_dict_set_item (modules, key, PCL_NONE);
        }

        /* Next, delete sys and __builtin__ (in that order). */
        value = pcl_dict_get_item_string (modules, "sys");
        if (PCL_IS_MODULE (value))
        {
                _pcl_module_clear (value);
                pcl_dict_set_item_string (modules, "sys", PCL_NONE);
        }
        value = pcl_dict_get_item_string (modules, "__builtin__");
        if (PCL_IS_MODULE (value))
        {
                _pcl_module_clear (value);
                pcl_dict_set_item_string (modules, "__builtin__", PCL_NONE);
        }

        /* Finally, clear and delete the modules dictionary. */
        pcl_mapping_clear (modules);
        is->modules = NULL;
        pcl_object_unref (modules);

        /* Clear some miscellaneous string objects. */
        PCL_CLEAR (silly_list);
        PCL_CLEAR (builtins_str);
}
