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

static gchar range_doc[] =
"range([start,] stop[, step]) -> iterator\n\
\n\
Return an iterator that expands to an arithmetic progression of integers.\n\
list(range(i, j)) returns [i, i+1, i+2, ..., j-1]; start (!) defaults to 0.\n\
When step is given, it specifies the increment (or decrement).\n\
For example, list(range(4)) returns [0, 1, 2, 3].  The end point is omitted!\n\
These are exactly the valid indices for a list of 4 elements.";

static gpointer range_parent_class = NULL;

static glong
range_calculate_length (glong lower, glong upper, glong step)
{
        glong length = 0;

        if (lower < upper)
        {
                gulong u_lower = (gulong) lower;
                gulong u_upper = (gulong) upper;
                gulong diff = u_upper - u_lower - 1;
                length = (glong) (diff / (gulong) step + 1);
        }
        return length;
}

static PclObject *
range_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        PclRange *range;
        glong lower = 0, upper = 0, step = 1;
        glong length;

        if (!pcl_arg_no_keywords ("range()", kwds))
                return NULL;

        if (PCL_TUPLE_GET_SIZE (args) <= 1)
        {
                if (!pcl_arg_parse_tuple (args,
                        "l;range() requires 1-3 int arguments", &upper))
                        return NULL;
        }
        else
        {
                if (!pcl_arg_parse_tuple (args,
                        "ll|l;range() requires 1-3 int arguments",
                        &lower, &upper, &step))
                        return NULL;
        }
        if (step == 0)
        {
                pcl_error_set_string (
                        pcl_exception_value_error (),
                        "range() step argument must not be zero");
                return NULL;
        }
        if (step > 0)
                length = range_calculate_length (lower, upper, step);
        else
                length = range_calculate_length (upper, lower, -step);
        if (length < 0)
        {
                pcl_error_set_string (
                        pcl_exception_overflow_error (),
                        "range() result has too many items");
                return NULL;
        }

        range = pcl_object_new (PCL_TYPE_RANGE, NULL);
        range->length = length;
        range->next = lower;
        range->step = step;
        return PCL_OBJECT (range);
}

static glong
range_measure (PclObject *object)
{
        return PCL_RANGE (object)->length;
}

static PclObject *
range_next (PclIterator *iterator)
{
        PclRange *self = PCL_RANGE (iterator);
        PclObject *result = NULL;

        if (self->length > 0)
        {
                result = pcl_int_from_long (self->next);
                self->next += self->step;
                self->length--;
        }
        return result;
}

static gboolean
range_stop (PclIterator *iterator)
{
        PclRange *self = PCL_RANGE (iterator);
        self->length = 0;
        return TRUE;
}

static void
range_class_init (PclRangeClass *class)
{
        PclIteratorClass *iterator_class;
        PclObjectClass *object_class;

        range_parent_class = g_type_class_peek_parent (class);

        iterator_class = PCL_ITERATOR_CLASS (class);
        iterator_class->next = range_next;
        iterator_class->stop = range_stop;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_range_get_type_object;
        object_class->new_instance = range_new_instance;
        object_class->measure = range_measure;
        object_class->doc = range_doc;
}

GType
pcl_range_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclRangeClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) range_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclRange),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_ITERATOR, "PclRange", &type_info, 0);
        }
        return type;
}

/**
 * pcl_range_get_type_object:
 *
 * Returns the type object for #PclRange.  During runtime this is the built-in
 * object %range.
 *
 * Returns: a borrowed reference to the type object for #PclRange
 */
PclObject *
pcl_range_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_RANGE, "range");
                pcl_register_singleton ("<type 'range'>", &object);
        }
        return object;
}
