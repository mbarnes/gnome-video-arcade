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

static gpointer iterator_parent_class = NULL;

static gint
iterator_contains (PclObject *object, PclObject *value)
{
        PclObject *next;
        gint is_equal;

        while ((next = pcl_iterator_next (object)) != NULL)
        {
                is_equal = pcl_object_rich_compare_bool (next, value, PCL_EQ);
                if (is_equal > 0)
                        return 1;
                if (is_equal < 0)
                        return -1;
        }

        if (pcl_error_occurred ())
                return -1;

        return 0;
}

static PclObject *
iterator_iterate (PclObject *object)
{
        return pcl_object_ref (object);
}

static PclObject *
iterator_method_next (PclIterator *self)
{
        PclObject *result;
        result = PCL_ITERATOR_GET_CLASS (self)->next (self);
        if (result == NULL && !pcl_error_occurred ())
                pcl_error_set_none (pcl_exception_stop_iteration ());
        return result;
}

static PclObject *
iterator_method_stop (PclIterator *self)
{
        if (!PCL_ITERATOR_GET_CLASS (self)->stop (self))
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

static void
iterator_class_init (PclIteratorClass *class)
{
        PclObjectClass *object_class;

        iterator_parent_class = g_type_class_peek_parent (class);

        object_class = PCL_OBJECT_CLASS (class);
        object_class->contains = iterator_contains;
        object_class->iterate = iterator_iterate;
}

#define WRAPS(func)     G_STRUCT_OFFSET (PclIteratorClass, func)

static PclMethodDef iterator_methods[] = {
        { "next",               (PclCFunction) iterator_method_next,
                                PCL_METHOD_FLAG_NOARGS,
                                "x.next() -> "
                                "the next value, or raise StopIteration",
                                WRAPS (next) },
        { "stop",               (PclCFunction) iterator_method_stop,
                                PCL_METHOD_FLAG_NOARGS,
                                "x.stop() -> "
                                "henceforth raise StopIteration on x.next()",
                                WRAPS (stop) },
        { NULL }
};

#undef WRAPS

GType
pcl_iterator_get_type (void)
{
        static GType type = 0;

        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclIteratorClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) iterator_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclIterator),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclIterator", &type_info,
                        G_TYPE_FLAG_ABSTRACT);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, iterator_methods);
        }

        return type;
}

/**
 * pcl_iterator_next:
 * @object: a #PclObject
 * 
 * If @object is an instance of #PclIterator, returns the next item from
 * the iteration.  If the end of the iteration has been reached, or has
 * been terminated with pcl_iterator_stop(), returns %NULL with no
 * exception set.
 *
 * Returns: a new reference to the next item in the iteration, or %NULL
 */
PclObject *
pcl_iterator_next (PclObject *object)
{
        PclIteratorClass *class;

        class = PCL_ITERATOR_GET_CLASS (object);
        if (class != NULL && class->next != NULL)
        {
                PclObject *result;

                result = class->next (PCL_ITERATOR (object));
                if (result == NULL && pcl_error_occurred () &&
                        pcl_error_exception_matches (
                                pcl_exception_stop_iteration ()))
                        pcl_error_clear ();
                return result;
        }

        pcl_error_bad_internal_call ();
        return NULL;
}

/**
 * pcl_iterator_stop:
 * @object: a #PclObject
 *
 * If @object is an instance of #PclIterator, prematurely terminates the
 * iteration such that subsequent calls to pcl_iterator_next() will always
 * return %NULL.
 *
 * Returns: %TRUE if successful, or %FALSE if an exception was raised
 */
gboolean
pcl_iterator_stop (PclObject *object)
{
        PclIteratorClass *class;

        class = PCL_ITERATOR_GET_CLASS (object);
        if (class != NULL && class->stop != NULL)
                return class->stop (PCL_ITERATOR (object));

        pcl_error_bad_internal_call ();
        return FALSE;
}
