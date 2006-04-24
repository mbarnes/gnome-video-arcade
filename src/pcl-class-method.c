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

static gchar class_method_doc[] =
"classmethod(function) -> method\n\
\n\
Convert a function to be a class method.\n\
\n\
A class method receives the class as an implicit first argument,\n\
just like an instance method receives the instance.\n\
To declare a class method, use this idiom:\n\
\n\
    class C:\n\
        @classmethod\n\
        def f(cls, arg1, arg2, ...): ...\n\
\n\
It can be called either on the class (e.g. C.f()) or on an instance\n\
(e.g. C().f()).  The instance is ignored except for its class.\n\
If a class method is called for a derived class, the derived class\n\
object is passed as the implied first argument.\n\
\n\
Class methods are different than C++ or Java static methods.\n\
If you want those, see the staticmethod builtin.";

static gpointer class_method_parent_class = NULL;

static void
class_method_dispose (GObject *g_object)
{
        PclClassMethod *self = PCL_CLASS_METHOD (g_object);

        PCL_CLEAR (self->callable);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (class_method_parent_class)->dispose (g_object);
}

static PclObject *
class_method_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        PclClassMethod *class_method;
        PclObject *callable;

        if (!pcl_arg_no_keywords ("classmethod()", kwds))
                return NULL;

        if (!pcl_arg_unpack_tuple (args, "classmethod", 1, 1, &callable))
                return NULL;
        if (!PCL_IS_CALLABLE (callable))
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "'%s' object is not callable",
                        PCL_GET_TYPE_NAME (callable));
                return NULL;
        }

        class_method = pcl_object_new (PCL_TYPE_CLASS_METHOD, NULL);
        class_method->callable = pcl_object_ref (callable);
        return PCL_OBJECT (class_method);
}

static gboolean
class_method_traverse (PclContainer *container, PclTraverseFunc func,
                       gpointer user_data)
{
        PclClassMethod *self = PCL_CLASS_METHOD (container);

        if (self->callable != NULL)
                if (!func (self->callable, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (class_method_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
class_method_get (PclDescriptor *descriptor,
                  PclObject *object, PclObject *type)
{
        PclClassMethod *self = PCL_CLASS_METHOD (descriptor);

        if (self->callable == NULL)
        {
                pcl_error_set_string (
                        pcl_exception_runtime_error (),
                        "uninitialized classmethod object");
                return NULL;
        }

        if (type == NULL)
                type = PCL_GET_TYPE_OBJECT (object);
        return pcl_instance_method_new (
                self->callable, type, PCL_GET_TYPE_OBJECT (type));
}

static void
class_method_class_init (PclClassMethodClass *class)
{
        PclDescriptorClass *descriptor_class;
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        class_method_parent_class = g_type_class_peek_parent (class);

        descriptor_class = PCL_DESCRIPTOR_CLASS (class);
        descriptor_class->get = class_method_get;

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = class_method_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_class_method_get_type_object;
        object_class->new_instance = class_method_new_instance;
        object_class->doc = class_method_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = class_method_dispose;
}

GType
pcl_class_method_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclClassMethodClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) class_method_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclClassMethod),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_DESCRIPTOR, "PclClassMethod", &type_info, 0);
        }
        return type;
}

/**
 * pcl_class_method_get_type_object:
 *
 * Returns the type object for #PclClassMethod.  During runtime this is the
 * built-in object %classmethod.
 *
 * Returns: a borrowed reference to the type object for #PclClassMethod
 */
PclObject *
pcl_class_method_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_CLASS_METHOD, "classmethod");
                pcl_register_singleton ("<type 'classmethod'>", &object);
        }
        return object;
}
