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

/* Pseudocode for 'property' type:
 *
 *     class property(object):
 *
 *         def __init__(self, fget=None, fset=None, fdel=None, doc=None):
 *             self.__get = fget
 *             self.__set = fset
 *             self.__del = fdel
 *             self.__doc__ = doc
 *
 *         def __get__(self, inst, type=None):
 *             if inst is None:
 *                 return self
 *             if self.__get is None:
 *                 raise AttributeError, "unreadable attribute"
 *             return self.__get(inst)
 *
 *         def __set__(self, inst, value):
 *             if self.__set is None:
 *                 raise AttributeError, "can't set attribute"
 *             return self.__set(inst, value)
 *
 *         def __delete__(self, inst):
 *             if self.__del is None:
 *                 raise AttributeError, "can't delete attribute"
 *             return self.__del(inst)
 */

static gchar property_doc[] =
"property(fget=None, fset=None, fdel=None, doc=None) -> property attribute\n\
\n\
fget is a function to be used for getting an attribute value, and likewise\n\
fset is a function for setting, and fdel a function for del'ing, an\n\
attribute.  Typical use is to define a managed attribute x:\n\
class C(object):\n\
    def getx(self): return self.__x\n\
    def setx(self, value): self.__x = value\n\
    def delx(self): del self.__x\n\
    x = property(getx, setx, delx, \"I'm the 'x' property.\")";

static gpointer property_parent_class = NULL;

static void
property_dispose (GObject *g_object)
{
        PclProperty *self = PCL_PROPERTY (g_object);

        PCL_CLEAR (self->get);
        PCL_CLEAR (self->set);
        PCL_CLEAR (self->del);
        PCL_CLEAR (self->doc);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (property_parent_class)->dispose (g_object);
}

static PclObject *
property_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        const gchar *kwlist[] = { "fget", "fset", "fdel", "doc", NULL };
        PclObject *del = NULL;
        PclObject *doc = NULL;
        PclObject *get = NULL;
        PclObject *set = NULL;
        PclProperty *property;

        if (!pcl_arg_parse_tuple_and_keywords (
                        args, kwds, "|OOOO:property", kwlist,
                        &get, &set, &del, &doc))
                return NULL;

        if (get == PCL_NONE)
                get = NULL;
        if (set == PCL_NONE)
                set = NULL;
        if (del == PCL_NONE)
                del = NULL;

        if (get != NULL)
                pcl_object_ref (get);
        if (set != NULL)
                pcl_object_ref (set);
        if (del != NULL)
                pcl_object_ref (del);

        property = g_object_new (PCL_TYPE_PROPERTY, NULL);
        property->get = get;
        property->set = set;
        property->del = del;
        property->doc = doc;
        return PCL_OBJECT (property);
}

static gboolean
property_traverse (PclContainer *container, PclTraverseFunc func,
                   gpointer user_data)
{
        PclProperty *self = PCL_PROPERTY (container);

        if (self->get != NULL)
                if (!func (self->get, user_data))
                        return FALSE;
        if (self->set != NULL)
                if (!func (self->set, user_data))
                        return FALSE;
        if (self->del != NULL)
                if (!func (self->del, user_data))
                        return FALSE;
        if (self->doc != NULL)
                if (!func (self->doc, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (property_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
property_get (PclDescriptor *descriptor, PclObject *object, PclObject *type)
{
        PclProperty *self = PCL_PROPERTY (descriptor);

        if (object == NULL || object == PCL_NONE)
                return pcl_object_ref (self);

        if (self->get == NULL)
        {
                pcl_error_set_string (
                        pcl_exception_attribute_error (),
                        "unreadable attribute");
                return NULL;
        }

        return pcl_object_call_function (self->get, "(O)", object);
}

static gboolean
property_set (PclDescriptor *descriptor, PclObject *object, PclObject *value)
{
        PclProperty *self = PCL_PROPERTY (descriptor);
        PclObject *result;

        if (value == NULL)
        {
                if (self->del == NULL)
                {
                        pcl_error_set_string (
                                pcl_exception_attribute_error (),
                                "can't delete attribute");
                        return FALSE;
                }
                result = pcl_object_call_function (
                                self->del, "(O)", object);
        }
        else
        {
                if (self->set == NULL)
                {
                        pcl_error_set_string (
                                pcl_exception_attribute_error (),
                                "can't set attribute");
                        return FALSE;
                }
                result = pcl_object_call_function (
                                self->set, "(OO)", object, value);
        }

        if (result == NULL)
                return FALSE;
        pcl_object_unref (result);
        return TRUE;
}

static void
property_class_init (PclPropertyClass *class)
{
        PclDescriptorClass *descriptor_class;
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        property_parent_class = g_type_class_peek_parent (class);

        descriptor_class = PCL_DESCRIPTOR_CLASS (class);
        descriptor_class->get = property_get;
        descriptor_class->set = property_set;

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = property_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_property_get_type_object;
        object_class->new_instance = property_new_instance;
        object_class->doc = property_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = property_dispose;
}

static PclMemberDef property_members[] = {
        { "__doc__",            G_TYPE_OBJECT,
                                G_STRUCT_OFFSET (PclProperty, doc),
                                PCL_MEMBER_FLAG_READONLY },
        { NULL }
};

GType
pcl_property_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclPropertyClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) property_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclProperty),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_DESCRIPTOR, "PclProperty", &type_info, 0);
                g_type_set_qdata (
                        type, PCL_DATA_MEMBERS, property_members);
        }
        return type;
}

/**
 * pcl_property_get_type_object:
 *
 * Returns the type object for #PclProperty.  During runtime this is the
 * built-in object %property.
 *
 * Returns: a borrowed reference to the type object for #PclProperty
 */
PclObject *
pcl_property_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_PROPERTY, "property");
                pcl_register_singleton ("<type 'property'>", &object);
        }
        return object;
}
