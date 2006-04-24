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

static gchar attr_getter_doc[] =
"attrgetter(attr) -> attrgetter object\n\
\n\
Return a callable object that fetches the given attribute from its operand.\n\
After f=attrgetter('name'), the call f(b) returns b.name.";

static gpointer attr_getter_parent_class = NULL;

static void
attr_getter_dispose (GObject *g_object)
{
        PclAttrGetter *self = PCL_ATTR_GETTER (g_object);

        PCL_CLEAR (self->attr);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (attr_getter_parent_class)->dispose (g_object);
}

static PclObject *
attr_getter_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        PclAttrGetter *attr_getter;
        PclObject *attr;

        if (!pcl_arg_no_keywords ("attrgetter()", kwds))
                return NULL;

        if (!pcl_arg_unpack_tuple (args, "attrgetter", 1, 1, &attr))
                return NULL;

        attr_getter = g_object_new (PCL_TYPE_ATTR_GETTER, NULL);
        attr_getter->attr = pcl_object_ref (attr);
        return PCL_OBJECT (attr_getter);
}

static PclObject *
attr_getter_call (PclObject *object, PclObject *args, PclObject *kwds)
{
        PclAttrGetter *self = PCL_ATTR_GETTER (object);
        PclObject *from;

        if (!pcl_arg_unpack_tuple (args, "attrgetter", 1, 1, &from))
                return NULL;
        return pcl_object_get_attr (from, self->attr);
}

static gboolean
attr_getter_traverse (PclContainer *container, PclTraverseFunc func,
                      gpointer user_data)
{
        PclAttrGetter *self = PCL_ATTR_GETTER (container);

        if (self->attr != NULL)
                if (!func (self->attr, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (attr_getter_parent_class)->
                traverse (container, func, user_data);
}

static void
attr_getter_class_init (PclAttrGetterClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        attr_getter_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = attr_getter_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_attr_getter_get_type_object;
        object_class->new_instance = attr_getter_new_instance;
        object_class->call = attr_getter_call;
        object_class->doc = attr_getter_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = attr_getter_dispose;
}

GType
pcl_attr_getter_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclAttrGetterClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) attr_getter_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclAttrGetter),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclAttrGetter", &type_info, 0);
        }
        return type;
}

/**
 * pcl_attr_getter_get_type_object:
 *
 * Returns the type object for #PclAttrGetter.  During runtime this is the
 * built-in object %operator.%attrgetter.
 *
 * Returns: a borrowed reference to the type object for #PclAttrGetter
 */
PclObject *
pcl_attr_getter_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (
                        PCL_TYPE_ATTR_GETTER, "operator.attrgetter");
                pcl_register_singleton (
                        "<type 'operator.attrgetter'>", &object);
        }
        return object;
}
