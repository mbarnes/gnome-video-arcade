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
#include <glib/gprintf.h>  /* for g_vfprintf() */

/* Try to determine platform from GLib definitions. */
#if defined G_OS_WIN32
#define PLATFORM "win32"
#elif defined G_OS_BEOS
#define PLATFORM "beos"
#elif defined G_OS_UNIX
#define PLATFORM "unix"
#else
#define PLATFORM "(unknown)"
#endif

static gchar sys_doc[] =
"This module provides access to some objects used or maintained by the\n\
interpreter and to functions that interact strongly with the interpreter.\n";

static PclObject *
sys_displayhook (PclObject *self, PclObject *object)
{
        PclThreadState *ts = pcl_thread_state_get ();
        PclObject *builtins, *sysout;

        builtins = pcl_dict_get_item_string (ts->is->modules, "__builtin__");
        sysout = pcl_sys_get_object ("stdout");

        if (builtins == NULL)
        {
                pcl_error_set_string (
                        pcl_exception_runtime_error (),
                        "lost __builtin__");
                return NULL;
        }

        if (sysout == NULL)
        {
                pcl_error_set_string (
                        pcl_exception_runtime_error (),
                        "lost sys.stdout");
                return NULL;
        }

        /* Pseudo-code:
         *     if object is None:
         *         return None
         *     __builtin__._ = None
         *     print object
         *     __builtin__._ = object
         *     return None
         */
        if (object == PCL_NONE)
                return pcl_object_ref (PCL_NONE);
        if (!pcl_object_set_attr_string (builtins, "_", PCL_NONE))
                return NULL;
        if (!pcl_flush_line ())
                return NULL;
        if (!pcl_file_write_object (sysout, object, 0))
                return NULL;
        pcl_file_soft_space (sysout, TRUE);
        if (!pcl_flush_line ())
                return NULL;
        if (!pcl_object_set_attr_string (builtins, "_", object))
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

static gchar sys_displayhook_doc[] =
"displayhook(object) -> None\n\
\n\
Print an object to sys.stdout and also save it in __builtin__._.";

static PclObject *
sys_exc_clear (PclObject *self)
{
        PclThreadState *ts = pcl_thread_state_get ();
        PclObject *tmp_type, *tmp_value, *tmp_traceback;
        tmp_type = ts->exc_type;
        tmp_value = ts->exc_value;
        tmp_traceback = ts->exc_traceback;
        ts->exc_type = NULL;
        ts->exc_value = NULL;
        ts->exc_traceback = NULL;
        if (tmp_type != NULL)
                pcl_object_unref (tmp_type);
        if (tmp_value != NULL)
                pcl_object_unref (tmp_value);
        if (tmp_traceback != NULL)
                pcl_object_unref (tmp_traceback);
        return pcl_object_ref (PCL_NONE);
}

static gchar sys_exc_clear_doc[] =
"exc_clear() -> None\n\
\n\
Clear global information on the current exception.  Subsequent calls to\n\
exc_info() will return (None,None,None) until another exception is raised\n\
in the current thread or the executed stack returns to a frame where\n\
another exception is being handled.";

static PclObject *
sys_exc_info (PclObject *self)
{
        PclThreadState *ts = pcl_thread_state_get ();
        return pcl_build_value ("(OOO)",
               (ts->exc_type != NULL) ? ts->exc_type : PCL_NONE,
               (ts->exc_value != NULL) ? ts->exc_value : PCL_NONE,
               (ts->exc_traceback != NULL) ? ts->exc_traceback : PCL_NONE);
}

static gchar sys_exc_info_doc[] =
"exc_info() -> (type, value, traceback)\n\
\n\
Return information about the most recent exception caught by an except\n\
clause in the current stack frame or in an older stack frame.";

static PclObject *
sys_excepthook (PclObject *self, PclObject *args)
{
        PclObject *exception, *value, *traceback;
        if (!pcl_arg_unpack_tuple (args, "excepthook", 3, 3,
                                   &exception, &value, &traceback))
                return NULL;
        pcl_error_display (exception, value, traceback);
        return pcl_object_ref (PCL_NONE);
}

static gchar sys_excepthook_doc[] =
"excepthook(exctype, value, traceback) -> None\n\
\n\
Handle an exception by displaying it with a traceback on sys.stderr.";

static PclObject *
sys_exit (PclObject *self, PclObject *args)
{
        PclObject *object = NULL;
        if (!pcl_arg_unpack_tuple (args, "exit", 0, 1, &object))
                return NULL;
        pcl_error_set_object (pcl_exception_system_exit (), object);
        return NULL;
}

static gchar sys_exit_doc[] =
"exit([status])\n\
\n\
Exit the interpreter by raising SystemExit(status).\n\
If the status is omitted or None, it defaults to zero (i.e., success).\n\
If the status is numeric, it will be used as the system exit status.\n\
If it is another kind of object, it will be printed and the system\n\
exit status will be one (i.e., failure).";

static PclObject *
sys_getrefcount (PclObject *self, PclObject *object)
{
        /* GObject does not provide a documented means of accessing the
         * reference count of an object, so this is prone to breakage
         * in future versions of GObject. */
        return pcl_int_from_long (G_OBJECT (object)->ref_count);
}

static gchar sys_getrefcount_doc[] =
"getrefcount(object) -> integer\n\
\n\
Return the reference count of object.  The count returned is generally\n\
one higher than you might expect, because it includes the (temporary)\n\
reference as an argument to getrefcount().";

static PclObject *
sys_getrecursionlimit (PclObject *self)
{
        return pcl_int_from_long (pcl_get_recursion_limit ());
}

static gchar sys_getrecursionlimit_doc[] =
"getrecursionlimit()\n\
\n\
Return the current value of the recursion limit, the maximum depth\n\
of the PCL interpreter stack.  This limit prevents infinite recursion\n\
from causing an overflow of the C stack and crashing PCL.";

static PclObject *
sys_id (PclObject *self, PclObject *object)
{
        return pcl_long_from_void_ptr (object);
}

static gchar sys_id_doc[] =
"id(object) -> integer\n\
\n\
Return the identity of an object.  This is guaranteed to be unique among\n\
simultaneously existing objects.  (Hint: it's the object's memory address.)";

static PclObject *
sys_intern (PclObject *self, PclObject *args)
{
        PclObject *string;
        if (!pcl_arg_parse_tuple (args, "S:intern", &string))
                return NULL;
        if (G_OBJECT_TYPE (string) != PCL_TYPE_STRING)
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "can't intern subclass of string");
                return NULL;
        }
        pcl_object_ref (string);
        pcl_string_intern_in_place (&string);
        return string;
}

static gchar sys_intern_doc[] =
"intern(string) -> string\n\
\n\
``Intern'' the given string.  This enters the string in the (global)\n\
table of interned strings whose purpose is to speed up dictionary lookups.\n\
Return the string itself or the previously interned string object with the\n\
same value.";

static PclObject *
sys_setrecursionlimit (PclObject *self, PclObject *args)
{
        gint limit;
        if (!pcl_arg_parse_tuple (args, "i:setrecursionlimit", &limit))
                return NULL;
        if (limit <= 0)
        {
                pcl_error_set_string (
                        pcl_exception_value_error (),
                        "recursion limit must be positive");
                return NULL;
        }
        pcl_set_recursion_limit (limit);
        return pcl_object_ref (PCL_NONE);
}

static gchar sys_setrecursionlimit_doc[] =
"setrecursionlimit(n)\n\
\n\
Set the maximum depth of the PCL interpreter stack to n.  This limit\n\
prevents infinite recursion from causing an overflow of the C stack and\n\
crashing PCL.  The highest possible limit is platform-dependent.";

PclObject *
pcl_sys_get_object (const gchar *name)
{
        PclThreadState *ts = pcl_thread_state_get ();
        PclObject *dict = ts->is->sysdict;
        return dict ? pcl_dict_get_item_string (dict, name) : NULL;
}

gboolean
pcl_sys_set_object (const gchar *name, PclObject *object)
{
        PclThreadState *ts = pcl_thread_state_get ();
        PclObject *dict = ts->is->sysdict;
        if (object == NULL)
                return pcl_dict_del_item_string (dict, name);
        return pcl_dict_set_item_string (dict, name, object);
}

void
pcl_sys_set_argv (gint argc, gchar **argv)
{
        PclObject *sys_argv = pcl_list_new (argc);
        glong ii;

        for (ii = 0; ii < argc; ++ii)
                pcl_list_set_item (sys_argv, ii,
                        pcl_string_from_string (argv[ii]));
        pcl_sys_set_object ("argv", sys_argv);
        pcl_object_unref (sys_argv);
}

void
pcl_sys_set_path (const gchar *path)
{
        PclObject *sys_path = pcl_list_new (0);
        PclObject *object;

        /* XXX This is a quick hack so that importing will always look in the
         *     current directory first.  It needs to find a better home. */
        object = pcl_string_from_string ("");
        pcl_list_append (sys_path, object);
        pcl_object_unref (object);

        if (path != NULL)
        {
                gchar **strings, **iterator;
                strings = g_strsplit (path, G_SEARCHPATH_SEPARATOR_S, 0);
                iterator = strings;
                while (*iterator != NULL)
                {
                        object = pcl_string_from_string (*iterator++);
                        pcl_list_append (sys_path, object);
                        pcl_object_unref (object);
                }
                g_strfreev (strings);
        }
        pcl_sys_set_object ("path", sys_path);
        pcl_object_unref (sys_path);
}

/* helper for pcl_sys_write_stdout() and pcl_sys_write_stderr() */
static void
sys_write (const gchar *name, FILE *stream, const gchar *format, va_list va)
{
        PclObject *error_type;
        PclObject *error_value;
        PclObject *error_traceback;
        PclObject *file;

        pcl_error_fetch (&error_type, &error_value, &error_traceback);
        file = pcl_sys_get_object (name);
        if (file == NULL || pcl_file_as_file (file) == stream)
                g_vfprintf (stream, format, va);
        else
        {
                gchar string[1000];
                const gint output_size = g_vsnprintf (
                        string, sizeof (string), format, va);
                if (!pcl_file_write_string (file, string))
                {
                        pcl_error_clear ();
                        fputs (string, stream);
                }
                if (output_size >= sizeof (string))
                {
                        const gchar *truncated = "... truncated";
                        if (!pcl_file_write_string (file, truncated))
                        {
                                pcl_error_clear ();
                                fputs (truncated, stream);
                        }
                }
        }
        pcl_error_restore (error_type, error_value, error_traceback);
}

void
pcl_sys_write_stdout (const gchar *format, ...)
{
        va_list va;

        va_start (va, format);
        sys_write ("stdout", stdout, format, va);
        va_end (va);
}

void
pcl_sys_write_stderr (const gchar *format, ...)
{
        va_list va;

        va_start (va, format);
        sys_write ("stderr", stderr, format, va);
        va_end (va);
}

#ifdef DYNAMIC_EXECUTION_PROFILE
/* Defined in pcl-eval.c because it uses static globals in that file. */
extern PclObject *pcl_get_dx_profile (PclObject *, PclObject *);
#endif

static PclMethodDef sys_methods[] = {
        { "displayhook",        (PclCFunction) sys_displayhook,
                                PCL_METHOD_FLAG_ONEARG,
                                sys_displayhook_doc },
        { "exc_clear",          (PclCFunction) sys_exc_clear,
                                PCL_METHOD_FLAG_NOARGS,
                                sys_exc_clear_doc },
        { "exc_info",           (PclCFunction) sys_exc_info,
                                PCL_METHOD_FLAG_NOARGS,
                                sys_exc_info_doc },
        { "excepthook",         (PclCFunction) sys_excepthook,
                                PCL_METHOD_FLAG_VARARGS,
                                sys_excepthook_doc },
        { "exit",               (PclCFunction) sys_exit,
                                PCL_METHOD_FLAG_VARARGS,
                                sys_exit_doc },
        { "getrefcount",        (PclCFunction) sys_getrefcount,
                                PCL_METHOD_FLAG_ONEARG,
                                sys_getrefcount_doc },
        { "getrecursionlimit",  (PclCFunction) sys_getrecursionlimit,
                                PCL_METHOD_FLAG_NOARGS,
                                sys_getrecursionlimit_doc },
        { "id",                 (PclCFunction) sys_id,
                                PCL_METHOD_FLAG_ONEARG,
                                sys_id_doc },
        { "intern",             (PclCFunction) sys_intern,
                                PCL_METHOD_FLAG_VARARGS,
                                sys_intern_doc },
        { "setrecursionlimit",  (PclCFunction) sys_setrecursionlimit,
                                PCL_METHOD_FLAG_VARARGS,
                                sys_setrecursionlimit_doc },
#ifdef DYNAMIC_EXECUTION_PROFILE
        { "getdxp",             (PclCFunction) pcl_get_dx_profile,
                                PCL_METHOD_FLAG_NOARGS },
#endif
        { NULL }
};

PclObject *
_pcl_sys_init (void)
{
        PclObject *module = pcl_module_init ("sys", sys_methods, sys_doc);
        PclObject *globals = pcl_module_get_dict (module);
        PclObject *sysin, *sysout, *syserr;

        sysin = pcl_file_from_file (stdin, "<stdin>", "r", NULL);
        sysout = pcl_file_from_file (stdout, "<stdout>", "w", NULL);
        syserr = pcl_file_from_file (stderr, "<stderr>", "w", NULL);

        pcl_dict_set_item_string (globals, "stdin", sysin);
        pcl_dict_set_item_string (globals, "stdout", sysout);
        pcl_dict_set_item_string (globals, "stderr", syserr);

        /* Backup Copies of Hook Objects */
        pcl_dict_set_item_string (globals, "__displayhook__",
                        pcl_dict_get_item_string (globals, "displayhook"));
        pcl_dict_set_item_string (globals, "__excepthook__",
                        pcl_dict_get_item_string (globals, "excepthook"));
        pcl_dict_set_item_string (globals, "__stdin__", sysin);
        pcl_dict_set_item_string (globals, "__stdout__", sysout);
        pcl_dict_set_item_string (globals, "__stderr__", syserr);

        pcl_object_unref (sysin);
        pcl_object_unref (sysout);
        pcl_object_unref (syserr);

        pcl_module_add_string (module, "exec_prefix", PCL_EXEC_PREFIX);
        pcl_module_add_int (module, "maxint", G_MAXLONG);
        pcl_module_add_string (module, "platform", PLATFORM);
        pcl_module_add_string (module, "prefix", PCL_PREFIX);
        pcl_module_add_string (module, "version", PACKAGE_VERSION);

        return module;
}
