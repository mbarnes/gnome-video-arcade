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

static gchar module_doc[] =
"module(name[, doc])\n\
\n\
Create a module object.\n\
The name must be a string; the optional doc argument can have any type.";

static gpointer module_parent_class = NULL;

static void
module_dispose (GObject *g_object)
{
        PclModule *self = PCL_MODULE (g_object);

        PCL_CLEAR (self->dict);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (module_parent_class)->dispose (g_object);
}

static void
module_finalize (GObject *g_object)
{
        PclModule *self = PCL_MODULE (g_object);

        if (self->g_module != NULL)
                g_module_close (self->g_module);

        /* Chain up to parent's finalize method. */
        G_OBJECT_CLASS (module_parent_class)->finalize (g_object);
}

static PclObject *
module_dict (PclObject *object)
{
        return PCL_MODULE (object)->dict;
}

static PclObject *
module_repr (PclObject *object)
{
        gchar *name;
        gchar *filename;

        name = pcl_module_get_name (object);
        if (name == NULL)
        {
                pcl_error_clear ();
                name = "?";
        }
        filename = pcl_module_get_filename (object);
        if (filename == NULL)
        {
                pcl_error_clear ();
                return pcl_string_from_format (
                        "<module '%s' (built-in)>", name);
        }
        return pcl_string_from_format (
                "<module '%s' from '%s'>", name, filename);
}

static gboolean
module_traverse (PclContainer *container, PclTraverseFunc func,
                 gpointer user_data)
{
        PclModule *self = PCL_MODULE (container);

        if (self->dict != NULL)
                if (!func (self->dict, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (module_parent_class)->
                traverse (container, func, user_data);
}

static PclMemberDef module_members[] = {
        { "__dict__",           G_TYPE_OBJECT,
                                G_STRUCT_OFFSET (PclModule, dict),
                                PCL_MEMBER_FLAG_READONLY },
        { NULL }
};

static void
module_class_init (PclModuleClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        module_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = module_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_module_get_type_object;
        object_class->dict = module_dict;
        object_class->repr = module_repr;
        object_class->doc = module_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = module_dispose;
        g_object_class->finalize = module_finalize;
}

GType
pcl_module_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclModuleClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) module_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclModule),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclModule", &type_info, 0);
                g_type_set_qdata (
                        type, PCL_DATA_MEMBERS, module_members);
        }
        return type;
}

/**
 * pcl_module_get_type_object:
 *
 * Returns the type object for #PclModule.
 *
 * Returns: a borrowed reference to the type object for #PclModule
 */
PclObject *
pcl_module_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_MODULE, "module");
                pcl_register_singleton ("<type 'module'>", &object);
        }
        return object;
}

PclObject *
pcl_module_new (const gchar *name)
{
        PclModule *module;
        PclObject *name_object;

        module = pcl_object_new (PCL_TYPE_MODULE, NULL);
        if (module == NULL)
                return NULL;
        module->dict = pcl_dict_new ();
        name_object = pcl_string_from_string (name);
        if (module->dict == NULL || name_object == NULL)
                goto fail;
        if (!pcl_dict_set_item_string (module->dict, "__name__", name_object))
                goto fail;
        if (!pcl_dict_set_item_string (module->dict, "__doc__", PCL_NONE))
                goto fail;
        pcl_object_unref (name_object);
        return PCL_OBJECT (module);

fail:
        if (name_object != NULL)
                pcl_object_unref (name_object);
        pcl_object_unref (module);
        return NULL;
}

PclObject *
pcl_module_get_dict (PclObject *module)
{
        if (!PCL_IS_MODULE (module))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        return PCL_MODULE (module)->dict;
}

gchar *
pcl_module_get_name (PclObject *module)
{
        PclObject *dict;
        PclObject *name;

        if (!PCL_IS_MODULE (module))
        {
                pcl_error_bad_argument ();
                return NULL;
        }

        dict = PCL_MODULE (module)->dict;
        name = pcl_dict_get_item_string (dict, "__name__");
        if (!PCL_IS_STRING (name))
        {
                pcl_error_set_string (
                        pcl_exception_system_error (),
                        "nameless module");
                return NULL;
        }
        return pcl_string_as_string (name);
}

gchar *
pcl_module_get_filename (PclObject *module)
{
        PclObject *dict;
        PclObject *file;

        if (!PCL_IS_MODULE (module))
        {
                pcl_error_bad_argument ();
                return NULL;
        }

        dict = PCL_MODULE (module)->dict;
        file = pcl_dict_get_item_string (dict, "__file__");
        if (!PCL_IS_STRING (file))
        {
                pcl_error_set_string (
                        pcl_exception_system_error (),
                        "module_filename missing");
                return NULL;
        }
        return pcl_string_as_string (file);
}

gboolean
pcl_module_add_int (PclObject *module, const gchar *name, glong value)
{
        PclObject *object = pcl_int_from_long (value);
        return pcl_module_add_object (module, name, object);
}

gboolean
pcl_module_add_float (PclObject *module, const gchar *name, gdouble value)
{
        PclObject *object = pcl_float_from_double (value);
        return pcl_module_add_object (module, name, object);
}

gboolean
pcl_module_add_string (PclObject *module, const gchar *name, const gchar *value)
{
        PclObject *object = pcl_string_from_string (value);
        return pcl_module_add_object (module, name, object);
}

gboolean
pcl_module_add_object (PclObject *module, const gchar *name, PclObject *object)
{
        PclObject *dict;
        gboolean success;

        if (!PCL_IS_MODULE (module) || object == NULL)
        {
                pcl_error_bad_internal_call ();
                if (object != NULL)
                        pcl_object_unref (object);
                return FALSE;
        }

        dict = PCL_MODULE (module)->dict;
        success = pcl_dict_set_item_string (dict, name, object);
        pcl_object_unref (object);  /* steal the reference */
        return success;
}

PclObject *
pcl_module_init (const gchar *name, PclMethodDef *methods, const gchar *doc)
{
        PclObject *dict;
        PclObject *module;

        module = pcl_import_add_module (name);
        dict = pcl_module_get_dict (module);
        if (methods != NULL)
        {
                PclObject *module_name = pcl_string_from_string (name);
                PclObject *method;
                PclMethodDef *def;

                for (def = methods; def->name != NULL; def++)
                {
                        method = pcl_method_new (def, NULL, module_name);
                        pcl_dict_set_item_string (dict, def->name, method);
                        pcl_object_unref (method);
                }
                pcl_object_unref (module_name);
        }
        if (doc != NULL)
        {
                PclObject *object;
                gboolean success;

                object = pcl_string_from_string (doc);
                if (object == NULL)
                        return NULL;
                success = pcl_dict_set_item_string (dict, "__doc__", object);
                pcl_object_unref (object);
                if (!success)
                        return NULL;
        }
        return module;
}

void
_pcl_module_clear (PclObject *module)
{
        /* To make execution order of destructors for global objects a bit
         * more predictable, we first zap all objects whose name starts with
         * a single underscore, before we clear the entire dictionary.  We
         * zap them by replacing them with None, rather than deleting them
         * from the dictionary, to avoid rehashing the dictionary (to some
         * extent). */

        PclObject *dict;
        PclObject *key;
        PclObject *value;
        glong pos;

        dict = PCL_MODULE (module)->dict;
        if (dict == NULL)
                return;

        /* First, clear only names starting with a single underscore. */
        pos = 0;
        while (pcl_dict_next (dict, &pos, &key, &value))
        {
                if (value != PCL_NONE && PCL_IS_STRING (key))
                {
                        gchar *name = pcl_string_as_string (key);
                        if (name[0] == '_' && name[1] != '_')
                                pcl_dict_set_item (dict, key, PCL_NONE);
                }
        }

        /* Next, clear all names except for __builtins__. */
        pos = 0;
        while (pcl_dict_next (dict, &pos, &key, &value))
        {
                if (value != PCL_NONE && PCL_IS_STRING (key))
                {
                        gchar *name = pcl_string_as_string (key);
                        if (name[0] != '_' && strcmp (name, "__builtins__"))
                                pcl_dict_set_item (dict, key, PCL_NONE);
                }
        }

        /* Note that we leave __builtins__ in place so that destructors of
         * non-global objects defined in this module can still use builtins,
         * in particular 'None'. */
}
