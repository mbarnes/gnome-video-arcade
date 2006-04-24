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

static gchar static_method_doc[] =
"staticmethod(function) -> method\n\
\n\
Convert a function to be a static method.\n\
\n\
A static method does not receive an implicit first argument.\n\
To declare a static method, use this idiom:\n\
\n\
    class C:\n\
        @staticmethod\n\
        def f(arg1, arg2, ...): ...\n\
\n\
It can be called either on the class (e.g. C.f()) or on an instance\n\
(e.g. C().f()).  The instance is ignored except for its class.\n\
\n\
Static methods in PCL are similar to those found in Java or C++.\n\
For a more advanced concept, see the classmethod builtin.";

static gpointer static_method_parent_class = NULL;

static void
static_method_dispose (GObject *g_object)
{
        PclStaticMethod *self = PCL_STATIC_METHOD (g_object);

        PCL_CLEAR (self->callable);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (static_method_parent_class)->dispose (g_object);
}

static PclObject *
static_method_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        PclStaticMethod *static_method;
        PclObject *callable;

        if (!pcl_arg_no_keywords ("staticmethod()", kwds))
                return NULL;

        if (!pcl_arg_unpack_tuple (args, "staticmethod", 1, 1, &callable))
                return NULL;

        static_method = pcl_object_new (PCL_TYPE_STATIC_METHOD, NULL);
        static_method->callable = pcl_object_ref (callable);
        return PCL_OBJECT (static_method);
}

static gboolean
static_method_traverse (PclContainer *container, PclTraverseFunc func,
                        gpointer user_data)
{
        PclStaticMethod *self = PCL_STATIC_METHOD (container);

        if (self->callable != NULL)
                if (!func (self->callable, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (static_method_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
static_method_get (PclDescriptor *descriptor,
                   PclObject *object, PclObject *type)
{
        PclStaticMethod *self = PCL_STATIC_METHOD (descriptor);

        if (self->callable == NULL)
        {
                pcl_error_set_string (
                        pcl_exception_runtime_error (),
                        "uninitialized staticmethod object");
                return NULL;
        }

        return pcl_object_ref (self->callable);
}

static void
static_method_class_init (PclStaticMethodClass *class)
{
        PclDescriptorClass *descriptor_class;
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        static_method_parent_class = g_type_class_peek_parent (class);

        descriptor_class = PCL_DESCRIPTOR_CLASS (class);
        descriptor_class->get = static_method_get;

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = static_method_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_static_method_get_type_object;
        object_class->new_instance = static_method_new_instance;
        object_class->doc = static_method_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = static_method_dispose;
}

GType
pcl_static_method_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclStaticMethodClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) static_method_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclStaticMethod),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_DESCRIPTOR, "PclStaticMethod", &type_info, 0);
        }
        return type;
}

/**
 * pcl_static_method_get_type_object:
 *
 * Returns the type object for #PclStaticMethod.  During runtime this is the
 * built-in object %staticmethod.
 *
 * Returns: a borrowed reference to the type object for #PclStaticMethod
 */
PclObject *
pcl_static_method_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_STATIC_METHOD, "staticmethod");
                pcl_register_singleton ("<type 'staticmethod'>", &object);
        }
        return object;
}
