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

static gchar slice_doc[] =
"slice([start,] stop[, step)\n\
\n\
Create a slice object.  This is used for extended slicing (e.g. a[0:10:2]).";

static gpointer slice_parent_class = NULL;

static gboolean
slice_index (PclObject *object, glong *index)
{
        if (PCL_IS_INT (object))
                *index = pcl_int_as_long (object);
        else
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "slice indices must be integers");
                return FALSE;
        }

        return TRUE;
}

static void
slice_dispose (GObject *g_object)
{
        PclSlice *self = PCL_SLICE (g_object);

        PCL_CLEAR (self->start);
        PCL_CLEAR (self->stop);
        PCL_CLEAR (self->step);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (slice_parent_class)->dispose (g_object);
}

static PclObject *
slice_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        PclObject *start = NULL;
        PclObject *stop = NULL;
        PclObject *step = NULL;

        if (!pcl_arg_no_keywords ("slice()", kwds))
                return NULL;

        if (!pcl_arg_unpack_tuple (args, "slice", 1, 3, &start, &stop, &step))
                return NULL;

        /* maintain similarity with range() */
        if (stop == NULL)
        {
                stop = start;
                start = NULL;
        }

        return pcl_slice_new (start, stop, step);
}

static PclObject *
slice_copy (PclObject *object)
{
        return pcl_object_ref (object);
}

static PclObject *
slice_repr (PclObject *object)
{
        PclSlice *self = PCL_SLICE (object);
        PclObject *comma = pcl_string_from_string (", ");
        PclObject *repr;

        repr = pcl_string_from_string ("slice(");
        pcl_string_concat_and_del (&repr, pcl_object_repr (self->start));
        pcl_string_concat (&repr, comma);
        pcl_string_concat_and_del (&repr, pcl_object_repr (self->stop));
        pcl_string_concat (&repr, comma);
        pcl_string_concat_and_del (&repr, pcl_object_repr (self->step));
        pcl_string_concat_and_del (&repr, pcl_string_from_string (")"));
        pcl_object_unref (comma);
        return repr;
}

static gint
slice_compare (PclObject *object1, PclObject *object2)
{
        PclSlice *slice1;
        PclSlice *slice2;
        gint cmp;

        if (!PCL_IS_SLICE (object1) || !PCL_IS_SLICE (object2))
                return -2;

        slice1 = PCL_SLICE (object1);
        slice2 = PCL_SLICE (object2);

        cmp = pcl_object_compare (slice1->start, slice2->start);
        if (cmp != 0)
                return cmp;
        cmp = pcl_object_compare (slice1->stop, slice2->stop);
        if (cmp != 0)
                return cmp;
        cmp = pcl_object_compare (slice1->step, slice2->step);
        return cmp;
}

static gboolean
slice_traverse (PclContainer *container, PclTraverseFunc func,
                gpointer user_data)
{
        PclSlice *self = PCL_SLICE (container);

        if (self->start != NULL)
                if (!func (self->start, user_data))
                        return FALSE;
        if (self->stop != NULL)
                if (!func (self->stop, user_data))
                        return FALSE;
        if (self->step != NULL)
                if (!func (self->step, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (slice_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
slice_method_indices (PclSlice *self, PclObject *object)
{
        glong input_length, start, stop, step, length;

        input_length = pcl_int_as_long (object);
        if (input_length == -1 && pcl_error_occurred ())
                return NULL;

        if (!pcl_slice_get_indices (self, input_length, &start,
                                    &stop, &step, &length))
                return NULL;

        return pcl_build_value ("(iii)", start, stop, step);
}

static gchar slice_method_indices_doc[] =
"S.indices(len) -> (start, stop, stride)\n\
\n\
Assuming a sequence of length len, calculate the start and stop\n\
indices, and the stride length of the extended slice described by S.\n\
Out of bounds indices are clipped in a manner consistent with the\n\
handling of normal slices.";

static void
slice_class_init (PclSliceClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        slice_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = slice_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_slice_get_type_object;
        object_class->new_instance = slice_new_instance;
        object_class->copy = slice_copy;
        object_class->repr = slice_repr;
        object_class->compare = slice_compare;
        object_class->doc = slice_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = slice_dispose;
}

static PclMemberDef slice_members[] = {
        { "start",              G_TYPE_OBJECT,
                                G_STRUCT_OFFSET (PclSlice, start),
                                PCL_MEMBER_FLAG_READONLY },
        { "stop",               G_TYPE_OBJECT,
                                G_STRUCT_OFFSET (PclSlice, stop),
                                PCL_MEMBER_FLAG_READONLY },
        { "step",               G_TYPE_OBJECT,
                                G_STRUCT_OFFSET (PclSlice, step),
                                PCL_MEMBER_FLAG_READONLY },
        { NULL }
};

static PclMethodDef slice_methods[] = {
        { "indices",            (PclCFunction) slice_method_indices,
                                PCL_METHOD_FLAG_ONEARG,
                                slice_method_indices_doc },
        { NULL }
};

GType
pcl_slice_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclSliceClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) slice_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclSlice),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclSlice", &type_info, 0);
                g_type_set_qdata (
                        type, PCL_DATA_MEMBERS, slice_members);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, slice_methods);
        }
        return type;
}

/**
 * pcl_slice_get_type_object:
 *
 * Returns the type object for #PclSlice.  During runtime this is the built-in
 * object %slice.
 *
 * Returns: a borrowed reference to the type object for #PclSlice
 */
PclObject *
pcl_slice_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_SLICE, "slice");
                pcl_register_singleton ("<type 'slice'>", &object);
        }
        return object;
}

PclObject *
pcl_slice_new (PclObject *start, PclObject *stop, PclObject *step)
{
        PclSlice *slice = pcl_object_new (PCL_TYPE_SLICE, NULL);

        if (start == NULL)
                start = PCL_NONE;
        if (stop == NULL)
                stop = PCL_NONE;
        if (step == NULL)
                step = PCL_NONE;

        slice->start = pcl_object_ref (start);
        slice->stop = pcl_object_ref (stop);
        slice->step = pcl_object_ref (step);

        return PCL_OBJECT (slice);
}

gboolean
pcl_slice_get_indices (PclSlice *slice, glong input_length, glong *start,
                       glong *stop, glong *step, glong *length)
{
        glong default_start, default_stop;

        if (PCL_IS_NONE (slice->step))
                *step = 1;
        else
        {
                if (!slice_index (slice->step, step))
                        return FALSE;
                if (*step == 0)
                        /* err: slice step cannot be zero */
                        return FALSE;
        }

        default_start = (*step < 0) ? input_length - 1 : 0;
        default_stop = (*step < 0) ? -1 : input_length;

        if (PCL_IS_NONE (slice->start))
                *start = default_start;
        else
        {
                if (!slice_index (slice->start, start))
                        return FALSE;
                if (*start < 0)
                        *start += input_length;
                if (*start < 0)
                        *start = (*step < 0) ? -1 : 0;
                if (*start >= input_length)
                        *start = (*step < 0) ? input_length - 1 : input_length;
        }

        if (PCL_IS_NONE (slice->stop))
                *stop = default_stop;
        else
        {
                if (!slice_index (slice->stop, stop))
                        return FALSE;
                if (*stop < 0)
                        *stop += input_length;
                if (*stop < 0)
                        *stop = -1;
                if (*stop > input_length)
                        *stop = input_length;
        }

        if ((*step < 0 && *stop >= *start) || (*step > 0 && *start >= *stop))
                *length = 0;
        else if (*step < 0)
                *length = (*stop - *start + 1) / (*step) + 1;
        else
                *length = (*stop - *start - 1) / (*step) + 1;

        return TRUE;
}
