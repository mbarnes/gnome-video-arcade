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

static gchar reversed_doc[] =
"reversed(sequence) -> reverse iterator over values of the sequence\n\
\n\
Return a reverse iterator";

static gpointer reversed_parent_class = NULL;

static void
reversed_dispose (GObject *g_object)
{
        PclReversed *self = PCL_REVERSED (g_object);

        PCL_CLEAR (self->sequence);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (reversed_parent_class)->dispose (g_object);
}

static PclObject *
reversed_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        PclObject *sequence;
        PclReversed *reversed;

        /* Keyword arguments aren't used in Python either. */
        if (!pcl_arg_unpack_tuple (args, "reversed", 1, 1, &sequence))
                return NULL;

        if (pcl_object_has_attr_string (sequence, "__reversed__"))
                return pcl_object_call_method (
                                sequence, "__reversed__", NULL);

        if (!PCL_IS_SEQUENCE (sequence))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "argument to reversed() must be a sequence");
                return NULL;
        }

        reversed = pcl_object_new (PCL_TYPE_REVERSED, NULL);
        reversed->index = pcl_object_measure (sequence) - 1;
        reversed->sequence = pcl_object_ref (sequence);
        return PCL_OBJECT (reversed);
}

static glong
reversed_measure (PclObject *object)
{
        PclReversed *self = PCL_REVERSED (object);
        glong length, position;

        if (self->sequence == NULL)
                return 0;
        length = pcl_object_measure (self->sequence);
        position = self->index + 1;
        return (length < position) ? 0 : position;
}

static gboolean
reversed_traverse (PclContainer *container, PclTraverseFunc func,
                   gpointer user_data)
{
        PclReversed *self = PCL_REVERSED (container);

        if (self->sequence != NULL)
                if (!func (self->sequence, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (reversed_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
reversed_next (PclIterator *iterator)
{
        PclReversed *self = PCL_REVERSED (iterator);
        PclObject *item;

        if (self->index >= 0)
        {
                item = pcl_sequence_get_item (self->sequence, self->index);
                if (item != NULL)
                {
                        self->index--;
                        return item;
                }
                if (pcl_error_exception_matches (
                        pcl_exception_index_error ()) ||
                        pcl_error_exception_matches (
                        pcl_exception_stop_iteration ()))
                {
                        pcl_error_clear ();
                }
        }
        self->index = -1;
        PCL_CLEAR (self->sequence);
        return NULL;
}

static gboolean
reversed_stop (PclIterator *iterator)
{
        PclReversed *self = PCL_REVERSED (iterator);
        self->index = -1;
        return TRUE;
}

static void
reversed_class_init (PclReversedClass *class)
{
        PclIteratorClass *iterator_class;
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        reversed_parent_class = g_type_class_peek_parent (class);

        iterator_class = PCL_ITERATOR_CLASS (class);
        iterator_class->next = reversed_next;
        iterator_class->stop = reversed_stop;

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = reversed_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_reversed_get_type_object;
        object_class->new_instance = reversed_new_instance;
        object_class->measure = reversed_measure;
        object_class->doc = reversed_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = reversed_dispose;
}

GType
pcl_reversed_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclReversedClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) reversed_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclReversed),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_ITERATOR, "PclReversed", &type_info, 0);
        }
        return type;
}

/**
 * pcl_reversed_get_type_object:
 *
 * Returns the type object for #PclReversed.  During runtime this is the
 * built-in object %reversed.
 *
 * Returns: a borrowed reference to the type object for #PclReversed
 */
PclObject *
pcl_reversed_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_REVERSED, "reversed");
                pcl_register_singleton ("<type 'reversed'>", &object);
        }
        return object;
}
