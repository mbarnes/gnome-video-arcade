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

static gpointer sequence_iterator_parent_class = NULL;

static void
sequence_iterator_dispose (GObject *g_object)
{
        PclSequenceIterator *self = PCL_SEQUENCE_ITERATOR (g_object);

        PCL_CLEAR (self->sequence);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (sequence_iterator_parent_class)->dispose (g_object);
}

static glong
sequence_iterator_measure (PclObject *object)
{
        PclSequenceIterator *self = PCL_SEQUENCE_ITERATOR (object);
        glong length = 0;

        if (self->sequence != NULL)
        {
                length = pcl_object_measure (self->sequence);
                if (self->index < length)
                        length -= self->index;
        }

        return length;
}

static gboolean
sequence_iterator_traverse (PclContainer *container, PclTraverseFunc func,
                            gpointer user_data)
{
        PclSequenceIterator *self = PCL_SEQUENCE_ITERATOR (container);

        if (self->sequence != NULL)
                if (!func (self->sequence, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (sequence_iterator_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
sequence_iterator_next (PclIterator *iterator)
{
        PclSequenceIterator *self = PCL_SEQUENCE_ITERATOR (iterator);
        PclObject *result;

        if (self->sequence == NULL)
                return NULL;
        result = pcl_sequence_get_item (self->sequence, self->index);
        if (result != NULL)
        {
                self->index++;
                return result;
        }
        if (pcl_error_exception_matches (pcl_exception_index_error ()) ||
                pcl_error_exception_matches (pcl_exception_stop_iteration ()))
        {
                pcl_error_clear ();
                pcl_object_unref (self->sequence);
                self->sequence = NULL;
        }
        return NULL;
}

static gboolean
sequence_iterator_stop (PclIterator *iterator)
{
        PclSequenceIterator *self = PCL_SEQUENCE_ITERATOR (iterator);

        if (self->sequence != NULL)
        {
                pcl_object_unref (self->sequence);
                self->sequence = NULL;
        }
        return TRUE;
}

static void
sequence_iterator_class_init (PclSequenceIteratorClass *class)
{
        PclIteratorClass *iterator_class;
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        sequence_iterator_parent_class = g_type_class_peek_parent (class);

        iterator_class = PCL_ITERATOR_CLASS (class);
        iterator_class->next = sequence_iterator_next;
        iterator_class->stop = sequence_iterator_stop;

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = sequence_iterator_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_sequence_iterator_get_type_object;
        object_class->measure = sequence_iterator_measure;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = sequence_iterator_dispose;
}

GType
pcl_sequence_iterator_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclSequenceIteratorClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) sequence_iterator_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclSequenceIterator),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_ITERATOR, "PclSequenceIterator",
                        &type_info, 0);
        }
        return type;
}

PclObject *
pcl_sequence_iterator_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (
                        PCL_TYPE_SEQUENCE_ITERATOR, "sequence-iterator");
                pcl_register_singleton (
                        "<type 'sequence-iterator'>", &object);
        }
        return object;
}

PclObject *
pcl_sequence_iterator_new (PclObject *sequence)
{
        PclSequenceIterator *sequence_iterator;

        if (!PCL_IS_SEQUENCE (sequence))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        sequence_iterator = pcl_object_new (PCL_TYPE_SEQUENCE_ITERATOR, NULL);
        if (sequence_iterator == NULL)
                return NULL;
        sequence_iterator->sequence = pcl_object_ref (sequence);
        return PCL_OBJECT (sequence_iterator);
}
