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

static gchar function_doc[] =
"function(code, globals[, name[, argdefs[, closure]]])\n\
\n\
Create a function object from a code object and a dictionary.\n\
The optional name string overrides the name from the code object.\n\
The optional argdefs tuple specifies the default argument values.\n\
The optional closure tuple supplies the bindings for free variables.";

static gpointer function_parent_class = NULL;

static void
function_dispose (GObject *g_object)
{
        PclFunction *self = PCL_FUNCTION (g_object);

        PCL_CLEAR (self->code);
        PCL_CLEAR (self->globals);
        PCL_CLEAR (self->defaults);
        PCL_CLEAR (self->closure);
        PCL_CLEAR (self->doc);
        PCL_CLEAR (self->name);
        PCL_CLEAR (self->dict);
        PCL_CLEAR (self->module);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (function_parent_class)->dispose (g_object);
}

static PclObject *
function_clone (PclObject *object)
{
        PclFunction *self = PCL_FUNCTION (object);
        PclFunction *clone = pcl_object_new (PCL_TYPE_FUNCTION, NULL);
        PclThreadState *ts = pcl_thread_state_get ();

        g_hash_table_insert (ts->cache,
                             pcl_object_ref (object),
                             pcl_object_ref (clone));

        if (self->code != NULL)
                clone->code = pcl_object_ref (self->code);
        if (self->globals != NULL)
                clone->globals = pcl_object_clone (self->globals);
        if (self->defaults != NULL)
                clone->defaults = pcl_object_clone (self->defaults);
        if (self->closure != NULL)
                clone->closure = pcl_object_clone (self->closure);
        if (self->doc != NULL)
                clone->doc = pcl_object_clone (self->doc);
        if (self->name != NULL)
                clone->name = pcl_object_clone (self->name);
        if (self->dict != NULL)
                clone->dict = pcl_object_clone (self->dict);

        return PCL_OBJECT (clone);
}

static PclObject *
function_call (PclObject *object, PclObject *args, PclObject *kwds)
{
        PclFunction *self = PCL_FUNCTION (object);
        PclObject *result;
        PclObject *defaults;
        PclObject **pda, **pka;
        glong nda, nka;

        defaults = PCL_FUNCTION_GET_DEFAULTS (self);
        if (PCL_IS_TUPLE (defaults))
        {
                pda = (PclObject **) &PCL_TUPLE_GET_ITEM (defaults, 0);
                nda = PCL_TUPLE_GET_SIZE (defaults);
        }
        else
        {
                pda = NULL;
                nda = 0;
        }

        if (PCL_IS_DICT (kwds))
        {
                glong ii = 0, pos = 0;
                nka = pcl_object_measure (kwds);
                pka = g_new0 (PclObject *, 2 * nka);
                while (pcl_dict_next (kwds, &pos, &pka[ii], &pka[ii + 1]))
                        ii += 2;
                nka = ii / 2;
        }
        else
        {
                pka = NULL;
                nka = 0;
        }

        result = pcl_eval_code_ex (
                        PCL_FUNCTION_GET_CODE (self),
                        PCL_FUNCTION_GET_GLOBALS (self),
                        NULL, (PclObject **)
                        &PCL_TUPLE_GET_ITEM (args, 0),
                        PCL_TUPLE_GET_SIZE (args),
                        pka, nka, pda, nda,
                        PCL_FUNCTION_GET_CLOSURE (self));

        g_free (pka);
        return result;
}

static PclObject *
function_dict (PclObject *object)
{
        PclFunction *self = PCL_FUNCTION (object);

        if (self->dict == NULL)
        {
                self->dict = pcl_dict_new ();
                if (self->dict == NULL)
                        return NULL;
        }
        return self->dict;
}

static PclObject *
function_repr (PclObject *object)
{
        PclFunction *self = PCL_FUNCTION (object);

        return pcl_string_from_format ("<function %s at %p>",
                pcl_string_as_string (self->name), (gpointer) object);
}

static gboolean
function_traverse (PclContainer *container, PclTraverseFunc func,
                   gpointer user_data)
{
        PclFunction *self = PCL_FUNCTION (container);

        if (self->code != NULL)
                if (!func (self->code, user_data))
                        return FALSE;
        if (self->globals != NULL)
                if (!func (self->globals, user_data))
                        return FALSE;
        if (self->defaults != NULL)
                if (!func (self->defaults, user_data))
                        return FALSE;
        if (self->closure != NULL)
                if (!func (self->closure, user_data))
                        return FALSE;
        if (self->doc != NULL)
                if (!func (self->doc, user_data))
                        return FALSE;
        if (self->name != NULL)
                if (!func (self->name, user_data))
                        return FALSE;
        if (self->dict != NULL)
                if (!func (self->dict, user_data))
                        return FALSE;
        if (self->module != NULL)
                if (!func (self->module, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (function_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
function_get_dict (PclFunction *self, gpointer context)
{
        if (self->dict == NULL)
        {
                self->dict = pcl_dict_new ();
                if (self->dict == NULL)
                        return NULL;
        }
        return pcl_object_ref (self->dict);
}

static gboolean
function_set_dict (PclFunction *self, PclObject *value, gpointer context)
{
        PclObject *tmp;

        if (value == NULL)
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "function's dictionary may not be deleted");
                return FALSE;
        }
        if (!PCL_IS_DICT (value))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "setting function's dictionary to a non-dict");
                return FALSE;
        }

        tmp = self->dict;
        self->dict = pcl_object_ref (value);
        if (tmp != NULL)
                pcl_object_unref (tmp);
        return TRUE;
}

static PclObject *
function_get_name (PclFunction *self, gpointer context)
{
        return pcl_object_ref (self->name);
}

static gboolean
function_set_name (PclFunction *self, PclObject *value, gpointer context)
{
        PclObject *tmp;

        if (!PCL_IS_STRING (value))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "__name__ must be set to a string object");
                return FALSE;
        }

        tmp = self->name;
        self->name = pcl_object_ref (value);
        pcl_object_unref (tmp);
        return TRUE;
}

static void
function_class_init (PclFunctionClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        function_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = function_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_function_get_type_object;
        object_class->clone = function_clone;
        object_class->call = function_call;
        object_class->dict = function_dict;
        object_class->repr = function_repr;
        object_class->doc = function_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = function_dispose;
}

static PclGetSetDef function_getsets[] = {
        { "__dict__",           (PclGetFunc) function_get_dict,
                                (PclSetFunc) function_set_dict },
        { "__name__",           (PclGetFunc) function_get_name,
                                (PclSetFunc) function_set_name },
        { NULL }
};

static PclMemberDef function_members[] = {
        { "__doc__",            G_TYPE_OBJECT,
                                G_STRUCT_OFFSET (PclFunction, doc) },
        { "__module__",         G_TYPE_OBJECT,
                                G_STRUCT_OFFSET (PclFunction, module) },
        { NULL }
};

GType
pcl_function_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclFunctionClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) function_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclFunction),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclFunction", &type_info, 0);
                g_type_set_qdata (
                        type, PCL_DATA_GETSETS, function_getsets);
                g_type_set_qdata (
                        type, PCL_DATA_MEMBERS, function_members);
        }
        return type;
}

/**
 * pcl_function_get_type_object:
 *
 * Returns the type object for #PclFunction.
 *
 * Returns: a borrowed reference to the type object for #PclFunction
 */
PclObject *
pcl_function_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_FUNCTION, "function");
                pcl_register_singleton ("<type 'function'>", &object);
        }
        return object;
}

PclObject *
pcl_function_new (PclCode *code, PclObject *globals)
{
        PclFunction *function;
        PclObject *doc;
        PclObject *module;
        function = pcl_object_new (PCL_TYPE_FUNCTION, NULL);
        function->code = pcl_object_ref (code);
        function->globals = pcl_object_ref (globals);
        function->name = pcl_object_ref (code->name);
        if (pcl_object_measure (code->constants) > 0)
        {
                doc = pcl_sequence_get_item (code->constants, 0);
                if (!PCL_IS_STRING (doc))
                        doc = PCL_NONE;
        }
        else
                doc = PCL_NONE;
        function->doc = pcl_object_ref (doc);
        module = pcl_dict_get_item_string (globals, "__name__");
        if (module != NULL)
                function->module = pcl_object_ref (module);
        return PCL_OBJECT (function);
}

PclCode *
pcl_function_get_code (PclObject *self)
{
        if (!PCL_IS_FUNCTION (self))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }
        return PCL_FUNCTION_GET_CODE (self);
}

PclObject *
pcl_function_get_globals (PclObject *self)
{
        if (!PCL_IS_FUNCTION (self))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }
        return PCL_FUNCTION_GET_GLOBALS (self);
}

PclObject *
pcl_function_get_defaults (PclObject *self)
{
        if (!PCL_IS_FUNCTION (self))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }
        return PCL_FUNCTION_GET_DEFAULTS (self);
}

gboolean
pcl_function_set_defaults (PclObject *self, PclObject *defaults)
{
        if (!PCL_IS_FUNCTION (self))
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }
        if (defaults == PCL_NONE)
                defaults = NULL;
        else if (PCL_IS_TUPLE (defaults))
                pcl_object_ref (defaults);
        else if (defaults != NULL)
        {
                pcl_error_set_string (
                        pcl_exception_system_error (),
                        "non-tuple default args");
                return FALSE;
        }
        if (PCL_FUNCTION (self)->defaults != NULL)
                pcl_object_unref (PCL_FUNCTION (self)->defaults);
        PCL_FUNCTION (self)->defaults = defaults;
        return TRUE;
}

PclObject *
pcl_function_get_closure (PclObject *self)
{
        if (!PCL_IS_FUNCTION (self))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }
        return PCL_FUNCTION (self)->closure;
}

gboolean
pcl_function_set_closure (PclObject *self, PclObject *closure)
{
        if (!PCL_IS_FUNCTION (self))
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }
        if (closure == PCL_NONE)
                closure = NULL;
        else if (PCL_IS_TUPLE (closure))
                pcl_object_ref (closure);
        else if (closure != NULL)
        {
                pcl_error_set_string (
                        pcl_exception_system_error (),
                        "non-tuple closure");
                return FALSE;
        }
        if (PCL_FUNCTION (self)->closure != NULL)
                pcl_object_unref (PCL_FUNCTION (self)->closure);
        PCL_FUNCTION (self)->closure = closure;
        return TRUE;
}

PclObject *
pcl_function_get_module (PclObject *self)
{
        if (!PCL_IS_FUNCTION (self))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }
        return PCL_FUNCTION (self)->module;
}
