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

static gpointer generator_parent_class = NULL;

static void
generator_dispose (GObject *g_object)
{
        PclGenerator *self = PCL_GENERATOR (g_object);

        PCL_CLEAR (self->frame);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (generator_parent_class)->dispose (g_object);
}

static gboolean
generator_traverse (PclContainer *container, PclTraverseFunc func,
                    gpointer user_data)
{
        PclGenerator *self = PCL_GENERATOR (container);

        if (self->frame != NULL)
                if (!func (self->frame, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (generator_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
generator_next (PclIterator *iterator)
{
        PclGenerator *self = PCL_GENERATOR (iterator);
        PclThreadState *ts = pcl_thread_state_get ();
        PclObject *result;

        if (self->running)
        {
                pcl_error_set_string (
                        pcl_exception_value_error (),
                        "generator already executing");
                return NULL;
        }

        if (self->frame->tos == NULL)
                return NULL;

        /* Generators always return to their most recent caller, not
         * necessarily their creator. */
        if (ts->frame != NULL)
                pcl_object_ref (ts->frame);
        g_assert (self->frame->previous == NULL);
        self->frame->previous = ts->frame;

        self->running = TRUE;
        result = pcl_eval_frame (self->frame);
        self->running = FALSE;

        /* Don't keep the reference to frame->previous any longer than
         * necessary.  It may keep a chain of frames alive or it could
         * create a reference cycle. */
        g_assert (self->frame->previous != NULL);
        PCL_CLEAR (self->frame->previous);

        /* If the generator just returned (as opposed to yielding), signal
         * that the generator is exhausted. */
        if (result == PCL_NONE && self->frame->tos == NULL)
        {
                pcl_object_unref (result);
                result = NULL;
        }

        return result;
}

static gboolean
generator_stop (PclIterator *iterator)
{
        PclGenerator *self = PCL_GENERATOR (iterator);
        self->frame->tos = NULL;
        return TRUE;
}

static void
generator_class_init (PclGeneratorClass *class)
{
        PclIteratorClass *iterator_class;
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        generator_parent_class = g_type_class_peek_parent (class);

        iterator_class = PCL_ITERATOR_CLASS (class);
        iterator_class->next = generator_next;
        iterator_class->stop = generator_stop;

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = generator_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_generator_get_type_object;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = generator_dispose;
}

GType
pcl_generator_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclGeneratorClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) generator_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclGenerator),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_ITERATOR, "PclGenerator", &type_info, 0);
        }
        return type;
}

/**
 * pcl_generator_get_type_object:
 *
 * Returns the type object for #PclGenerator.
 *
 * Returns: a borrowed reference to the type object for #PclGenerator
 */
PclObject *
pcl_generator_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_GENERATOR, "generator");
                pcl_register_singleton ("<type 'generator'>", &object);
        }
        return object;
}

PclObject *
pcl_generator_new (PclFrame *frame)
{
        PclGenerator *generator = pcl_object_new (PCL_TYPE_GENERATOR, NULL);
        if (frame == NULL)
                return NULL;
        generator->frame = pcl_object_ref (frame);
        return PCL_OBJECT (generator);
}
