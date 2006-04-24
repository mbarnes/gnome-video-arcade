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

static PclFactory method_factory;

static gpointer method_parent_class = NULL;

static void
method_dispose (GObject *g_object)
{
        PclMethod *self = PCL_METHOD (g_object);

        PCL_CLEAR (self->self);
        PCL_CLEAR (self->module);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (method_parent_class)->dispose (g_object);
}

static PclObject *
method_call (PclObject *object, PclObject *args, PclObject *kwds)
{
        PclMethod *self = PCL_METHOD (object);
        PclObject *self_arg = PCL_METHOD_GET_SELF (self);
        PclCFunction func = PCL_METHOD_GET_FUNCTION (self);
        gint mask = ~PCL_METHOD_FLAG_CLASS;
        glong na = pcl_object_measure (args);

        if (!(PCL_METHOD_GET_FLAGS (self) & PCL_METHOD_FLAG_KEYWORDS) &&
                (kwds != NULL) && (pcl_object_measure (kwds) > 0))
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "%s() takes no keyword arguments",
                        self->def->name);
                return NULL;
        }

        switch (PCL_METHOD_GET_FLAGS (self) & mask)
        {
                case PCL_METHOD_FLAG_VARARGS:
                        return func (self_arg, args);
                case PCL_METHOD_FLAG_VARARGS | PCL_METHOD_FLAG_KEYWORDS:
                        return ((PclCFunctionWithKeywords) func)
                                (self_arg, args, kwds);
                case PCL_METHOD_FLAG_ONEARG:
                        if (na == 1)
                                return func (self_arg,
                                       PCL_TUPLE_GET_ITEM (args, 0));
                        pcl_error_set_format (
                                pcl_exception_type_error (),
                                "%s() takes exactly one argument (%d given)",
                                self->def->name, na);
                        break;
                case PCL_METHOD_FLAG_NOARGS:
                        if (na == 0)
                                return func (self_arg, NULL);
                        pcl_error_set_format (
                                pcl_exception_type_error (),
                                "%s takes no arguments (%d given)",
                                self->def->name, na);
                        break;
                default:
                        pcl_error_bad_internal_call ();
        }

        return NULL;
}

static guint
method_hash (PclObject *object)
{
        PclMethod *self = PCL_METHOD (object);
        /*guint x, y;*/
        guint x;

        if (self->self == NULL)
                x = 0;
        else
        {
                x = pcl_object_hash (self->self);
                if (x == PCL_HASH_INVALID)
                        return PCL_HASH_INVALID;
        }
        /* This part makes GCC 4.0 complain.
         * y = pcl_object_hash_pointer (
         *         (gpointer) PCL_METHOD_GET_FUNCTION (self));
         * if (y == PCL_HASH_INVALID)
         *         return PCL_HASH_INVALID;
         * x ^= y;*/
        return PCL_HASH_VALIDATE (x);
}

static PclObject *
method_repr (PclObject *object)
{
        PclMethod *self = PCL_METHOD (object);
        PclObject *self_arg = PCL_METHOD_GET_SELF (self);

        if (self_arg == NULL)
                return pcl_string_from_format (
                       "<built-in function %s>", self->def->name);
        else
                return pcl_string_from_format (
                       "<built-in method %s of %s object at %p>",
                       self->def->name, PCL_GET_TYPE_NAME (self_arg),
                       (gpointer) self_arg);
}

static gint
method_compare (PclObject *object1, PclObject *object2)
{
        PclMethod *method1;
        PclMethod *method2;

        if (!PCL_IS_METHOD (object1) || !PCL_IS_METHOD (object2))
                return -2;

        method1 = PCL_METHOD (object1);
        method2 = PCL_METHOD (object2);

        if (method1->self != method2->self)
                return (method1->self < method2->self) ? -1 : 1;
        if (method1->def->func == method2->def->func)
                return 0;
        if (strcmp (method1->def->name, method2->def->name) < 0)
                return -1;
        return 1;
}

static gboolean
method_traverse (PclContainer *container, PclTraverseFunc func,
                 gpointer user_data)
{
        PclMethod *self = PCL_METHOD (container);

        if (self->self != NULL)
                if (!func (self->self, user_data))
                        return FALSE;
        if (self->module != NULL)
                if (!func (self->module, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (method_parent_class)->
                traverse (container, func, user_data);
}

PclObject *
method_get_doc (PclMethod *self, gpointer closure)
{
        if (self->def->doc != NULL)
                return pcl_string_from_string (self->def->doc);
        return pcl_object_ref (PCL_NONE);
}

PclObject *
method_get_name (PclMethod *self, gpointer closure)
{
        return pcl_string_from_string (self->def->name);
}

PclObject *
method_get_self (PclMethod *self, gpointer closure)
{
        if (self->self != NULL)
                return pcl_object_ref (self->self);
        return pcl_object_ref (PCL_NONE);
}

static void
method_class_init (PclMethodClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        method_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = method_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_method_get_type_object;
        object_class->call = method_call;
        object_class->hash = method_hash;
        object_class->repr = method_repr;
        object_class->compare = method_compare;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = method_dispose;
}

static PclGetSetDef method_getsets[] = {
        { "__doc__",            (PclGetFunc) method_get_doc },
        { "__name__",           (PclGetFunc) method_get_name },
        { "__self__",           (PclGetFunc) method_get_self },
        { NULL }
};

static PclMemberDef method_members[] = {
        { "__module__",         G_TYPE_OBJECT,
                                G_STRUCT_OFFSET (PclMethod, module) },
        { NULL }
};

GType
pcl_method_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclMethodClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) method_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclMethod),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclMethod", &type_info, 0);
                g_type_set_qdata (
                        type, PCL_DATA_GETSETS, method_getsets);
                g_type_set_qdata (
                        type, PCL_DATA_MEMBERS, method_members);
        }
        return type;
}

/**
 * pcl_method_get_type_object:
 *
 * Returns the type object for #PclMethod.
 *
 * Returns: a borrowed reference to the type object for #PclMethod
 */
PclObject *
pcl_method_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (
                        PCL_TYPE_METHOD, "builtin_function_or_method");
                pcl_register_singleton (
                        "<type 'builtin_function_or_method'>", &object);
        }
        return object;
}

PclObject *
pcl_method_new (PclMethodDef *def, PclObject *self, PclObject *module)
{
        PclMethod *method;
        method = pcl_factory_order (&method_factory);
        method->def = def;
        if (self != NULL)
                pcl_object_ref (self);
        method->self = self;
        if (module != NULL)
                pcl_object_ref (module);
        method->module = module;
        return PCL_OBJECT (method);
}

void
_pcl_method_init (void)
{
        method_factory.type = PCL_TYPE_METHOD;
}

void
_pcl_method_fini (void)
{
        pcl_factory_close (&method_factory);
}
