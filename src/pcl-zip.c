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

static gchar zip_doc[] =
"zip(iterable1 [, iterable2 [...]]) -> iterator\n\
\n\
Return a zip object.  All of the arguments must support iteration.  The zip\n\
object yields tuples containing the i-th element from each of the iterable\n\
arguments.  Iteration stops when one of the component iterators stops.";

static gpointer zip_parent_class = NULL;

static void
zip_dispose (GObject *g_object)
{
        PclZip *self = PCL_ZIP (g_object);

        PCL_CLEAR (self->iterators);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (zip_parent_class)->dispose (g_object);
}

static PclObject *
zip_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        PclZip *zip;
        PclObject *iterator;
        PclObject *tuple;
        glong ii, length;

        if (!pcl_arg_no_keywords ("zip()", kwds))
                return NULL;

        length = PCL_TUPLE_GET_SIZE (args);

        tuple = pcl_tuple_new (length);
        if (tuple == NULL)
                return NULL;

        for (ii = 0; ii < length; ii++)
        {
                iterator = pcl_object_iterate (PCL_TUPLE_GET_ITEM (args, ii));
                if (iterator == NULL)
                {
                        if (pcl_error_exception_matches (
                                        pcl_exception_type_error ()))
                                pcl_error_set_format (
                                        pcl_exception_type_error (),
                                        "zip argument #%ld must support "
                                        "iteration", ii + 1);
                        pcl_object_unref (tuple);
                        return NULL;
                }
                PCL_TUPLE_SET_ITEM (tuple, ii, iterator);
        }

        zip = pcl_object_new (PCL_TYPE_ZIP, NULL);
        zip->iterators = tuple;
        return PCL_OBJECT (zip);
}

static gboolean
zip_traverse (PclContainer *container, PclTraverseFunc func,
              gpointer user_data)
{
        PclZip *self = PCL_ZIP (container);

        if (self->iterators != NULL)
                if (!func (self->iterators, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (zip_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
zip_next (PclIterator *iterator)
{
        PclZip *self = PCL_ZIP (iterator);
        PclObject *next;
        PclObject *result;
        glong ii, length;

        if (self->iterators == NULL)
                return NULL;

        length = PCL_TUPLE_GET_SIZE (self->iterators);

        result = pcl_tuple_new (length);
        if (result == NULL)
                return NULL;

        for (ii = 0; ii < length; ii++)
        {
                next = pcl_iterator_next (
                        PCL_TUPLE_GET_ITEM (self->iterators, ii));
                if (next == NULL)
                {
                        pcl_object_unref (result);
                        PCL_CLEAR (self->iterators);
                        return NULL;
                }
                PCL_TUPLE_SET_ITEM (result, ii, next);
        }

        return result;
}

static gboolean
zip_stop (PclIterator *iterator)
{
        PclZip *self = PCL_ZIP (iterator);
        PCL_CLEAR (self->iterators);
        return TRUE;
}

static void
zip_class_init (PclZipClass *class)
{
        PclIteratorClass *iterator_class;
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        zip_parent_class = g_type_class_peek_parent (class);

        iterator_class = PCL_ITERATOR_CLASS (class);
        iterator_class->next = zip_next;
        iterator_class->stop = zip_stop;

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = zip_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_zip_get_type_object;
        object_class->new_instance = zip_new_instance;
        object_class->doc = zip_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = zip_dispose;
}

GType
pcl_zip_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclZipClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) zip_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclZip),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_ITERATOR, "PclZip", &type_info, 0);
        }
        return type;
}

/**
 * pcl_zip_get_type_object:
 *
 * Returns the type object for #PclZip.  During runtime this is the built-in
 * object %zip.
 *
 * Returns: a borrowed reference to the type object for #PclZip
 */
PclObject *
pcl_zip_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_ZIP, "zip");
                pcl_register_singleton ("<type 'zip'>", &object);
        }
        return object;
}
