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

static gchar item_getter_doc[] =
"itemgetter(item) -> itemgetter object\n\
\n\
Return a callable object that fetches the given item from its operand.\n\
After f=itemgetter(2), the call f(b) returns b[2].";

static gpointer item_getter_parent_class = NULL;

static void
item_getter_dispose (GObject *g_object)
{
        PclItemGetter *self = PCL_ITEM_GETTER (g_object);

        PCL_CLEAR (self->item);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (item_getter_parent_class)->dispose (g_object);
}

static PclObject *
item_getter_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        PclItemGetter *item_getter;
        PclObject *item;

        if (!pcl_arg_no_keywords ("itemgetter()", kwds))
                return NULL;

        if (!pcl_arg_unpack_tuple (args, "itemgetter", 1, 1, &item))
                return NULL;

        item_getter = g_object_new (PCL_TYPE_ITEM_GETTER, NULL);
        item_getter->item = pcl_object_ref (item);
        return PCL_OBJECT (item_getter);
}

static PclObject *
item_getter_call (PclObject *object, PclObject *args, PclObject *kwds)
{
        PclItemGetter *self = PCL_ITEM_GETTER (object);
        PclObject *from;

        if (!pcl_arg_unpack_tuple (args, "itemgetter", 1, 1, &from))
                return NULL;
        return pcl_object_get_item (from, self->item);
}

static gboolean
item_getter_traverse (PclContainer *container, PclTraverseFunc func,
                      gpointer user_data)
{
        PclItemGetter *self = PCL_ITEM_GETTER (container);

        if (self->item != NULL)
                if (!func (self->item, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (item_getter_parent_class)->
                traverse (container, func, user_data);
}

static void
item_getter_class_init (PclItemGetterClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        item_getter_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = item_getter_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_item_getter_get_type_object;
        object_class->new_instance = item_getter_new_instance;
        object_class->call = item_getter_call;
        object_class->doc = item_getter_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = item_getter_dispose;
}

GType
pcl_item_getter_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclItemGetterClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) item_getter_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclItemGetter),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclItemGetter", &type_info, 0);
        }
        return type;
}

/**
 * pcl_item_getter_get_type_object:
 *
 * Returns the type object for #PclItemGetter.  During runtime this is the
 * built-in object %operator.%itemgetter.
 *
 * Returns: a borrowed reference to the type object for #PclItemGetter
 */
PclObject *
pcl_item_getter_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (
                        PCL_TYPE_ITEM_GETTER, "operator.itemgetter");
                pcl_register_singleton (
                        "<type 'operator.itemgetter'>", &object);
        }
        return object;
}
