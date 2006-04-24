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

static gchar enumerate_doc[] =
"enumerate(iterable) -> iterator for index, value of iterable\n\
\n\
Return an enumerate object.  iterable must be another object that supports\n\
iteration.  The enumerate object yields pairs containing a count (from\n\
zero) and a value yielded by the iterable argument.  enumerate is useful\n\
for obtaining an indexed list: (0, seq[0]), (1, seq[1]), (2, seq[2]), ...";

static gpointer enumerate_parent_class = NULL;

static void
enumerate_dispose (GObject *g_object)
{
        PclEnumerate *self = PCL_ENUMERATE (g_object);

        PCL_CLEAR (self->iterator);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (enumerate_parent_class)->dispose (g_object);
}

static PclObject *
enumerate_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        const gchar *kwlist[] = { "iterable", NULL };
        PclObject *iterable = NULL;
        PclEnumerate *enumerate;

        if (!pcl_arg_parse_tuple_and_keywords (
                        args, kwds, "O:enumerate", kwlist, &iterable))
                return NULL;

        enumerate = pcl_object_new (PCL_TYPE_ENUMERATE, NULL);
        enumerate->iterator = pcl_object_iterate (iterable);
        if (enumerate->iterator == NULL)
        {
                pcl_object_unref (enumerate);
                return NULL;
        }
        return PCL_OBJECT (enumerate);
}

static gboolean
enumerate_traverse (PclContainer *container, PclTraverseFunc func,
                    gpointer user_data)
{
        PclEnumerate *self = PCL_ENUMERATE (container);

        if (self->iterator != NULL)
                if (!func (self->iterator, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (enumerate_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
enumerate_next (PclIterator *iterator)
{
        PclEnumerate *self = PCL_ENUMERATE (iterator);
        PclObject *next_index;
        PclObject *next_item;
        PclObject *result;

        next_item = pcl_iterator_next (self->iterator);
        if (next_item == NULL)
                return NULL;

        next_index = pcl_int_from_long (self->index);
        if (next_index == NULL)
        {
                pcl_object_unref (next_item);
                return NULL;
        }
        self->index++;

        result = pcl_tuple_new (2);
        if (result == NULL)
        {
                pcl_object_unref (next_index);
                pcl_object_unref (next_item);
                return NULL;
        }
        pcl_tuple_set_item (result, 0, next_index);
        pcl_tuple_set_item (result, 1, next_item);
        return result;
}

static gboolean
enumerate_stop (PclIterator *iterator)
{
        PclEnumerate *self = PCL_ENUMERATE (iterator);
        return pcl_iterator_stop (self->iterator);
}

static void
enumerate_class_init (PclEnumerateClass *class)
{
        PclIteratorClass *iterator_class;
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        enumerate_parent_class = g_type_class_peek_parent (class);

        iterator_class = PCL_ITERATOR_CLASS (class);
        iterator_class->next = enumerate_next;
        iterator_class->stop = enumerate_stop;

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = enumerate_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_enumerate_get_type_object;
        object_class->new_instance = enumerate_new_instance;
        object_class->doc = enumerate_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = enumerate_dispose;
}

GType
pcl_enumerate_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclEnumerateClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) enumerate_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclEnumerate),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_ITERATOR, "PclEnumerate", &type_info, 0);
        }
        return type;
}

/**
 * pcl_enumerate_get_type_object:
 *
 * Returns the type object for #PclEnumerate.  During runtime this is the
 * built-in object %enumerate.
 *
 * Returns: a borrowed reference to the type object for #PclEnumerate
 */
PclObject *
pcl_enumerate_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_ENUMERATE, "enumerate");
                pcl_register_singleton ("<type 'enumerate'>", &object);
        }
        return object;
}
