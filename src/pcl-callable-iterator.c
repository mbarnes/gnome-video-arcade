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

static gpointer callable_iterator_parent_class = NULL;

static void
callable_iterator_dispose (GObject *g_object)
{
        PclCallableIterator *self = PCL_CALLABLE_ITERATOR (g_object);

        PCL_CLEAR (self->callable);
        PCL_CLEAR (self->sentinel);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (callable_iterator_parent_class)->dispose (g_object);
}

static gboolean
callable_iterator_traverse (PclContainer *container, PclTraverseFunc func,
                            gpointer user_data)
{
        PclCallableIterator *self = PCL_CALLABLE_ITERATOR (container);

        if (self->callable != NULL)
                if (!func (self->callable, user_data))
                        return FALSE;
        if (self->sentinel != NULL)
                if (!func (self->sentinel, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (callable_iterator_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
callable_iterator_next (PclIterator *iterator)
{
        PclCallableIterator *self = PCL_CALLABLE_ITERATOR (iterator);
        PclObject *args;
        PclObject *result;

        if (self->callable == NULL)
                return NULL;

        args = pcl_tuple_new (0);
        if (args == NULL)
                return NULL;
        result = pcl_object_call (self->callable, args, NULL);
        pcl_object_unref (args);
        if (result != NULL)
        {
                gint is_equal;
                is_equal = pcl_object_rich_compare_bool (
                        result, self->sentinel, PCL_EQ);
                if (is_equal == 0)
                        return result;
                pcl_object_unref (result);
                if (is_equal > 0)
                {
                        PCL_CLEAR (self->callable);
                        PCL_CLEAR (self->sentinel);
                }
        }
        else if (pcl_error_exception_matches (
                        pcl_exception_stop_iteration ()))
        {
                pcl_error_clear ();
                PCL_CLEAR (self->callable);
                PCL_CLEAR (self->sentinel);
        }
        return NULL;
}

static gboolean
callable_iterator_stop (PclIterator *iterator)
{
        PclCallableIterator *self = PCL_CALLABLE_ITERATOR (iterator);

        PCL_CLEAR (self->callable);
        PCL_CLEAR (self->sentinel);

        return TRUE;
}

static void
callable_iterator_class_init (PclCallableIteratorClass *class)
{
        PclIteratorClass *iterator_class;
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        callable_iterator_parent_class = g_type_class_peek_parent (class);

        iterator_class = PCL_ITERATOR_CLASS (class);
        iterator_class->next = callable_iterator_next;
        iterator_class->stop = callable_iterator_stop;

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = callable_iterator_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_callable_iterator_get_type_object;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = callable_iterator_dispose;
}

GType
pcl_callable_iterator_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclCallableIteratorClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) callable_iterator_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclCallableIterator),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_ITERATOR, "PclCallableIterator",
                        &type_info, 0);
        }
        return type;
}

PclObject *
pcl_callable_iterator_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (
                        PCL_TYPE_CALLABLE_ITERATOR, "callable-iterator");
                pcl_register_singleton (
                        "<type 'callable-iterator'>", &object);
        }
        return object;
}

PclObject *
pcl_callable_iterator_new (PclObject *callable, PclObject *sentinel)
{
        PclCallableIterator *callable_iterator;
        callable_iterator = pcl_object_new (PCL_TYPE_CALLABLE_ITERATOR, NULL);
        if (callable != NULL)
                pcl_object_ref (callable);
        if (sentinel != NULL)
                pcl_object_ref (sentinel);
        callable_iterator->callable = callable;
        callable_iterator->sentinel = sentinel;
        return PCL_OBJECT (callable_iterator);
}
