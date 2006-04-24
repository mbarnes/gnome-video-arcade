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

static PclObject *
sequence_method_add (PclObject *self, PclObject *other)
{
        return pcl_number_add (self, other);
}

static PclObject *
sequence_method_iadd (PclObject *self, PclObject *other)
{
        return pcl_number_in_place_add (self, other);
}

static PclObject *
sequence_method_imul (PclObject *self, PclObject *other)
{
        return pcl_number_in_place_multiply (self, other);
}

static PclObject *
sequence_method_mul (PclObject *self, PclObject *other)
{
        return pcl_number_multiply (self, other);
}

static PclObject *
sequence_method_rmul (PclObject *self, PclObject *other)
{
        return pcl_number_multiply (other, self);
}

#define WRAPS(func)     G_STRUCT_OFFSET (PclSequenceIface, func)

static PclMethodDef sequence_methods[] = {
        { "__add__",            (PclCFunction) sequence_method_add,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__add__(y) <==> x + y",
                                WRAPS (concat) },
        { "__iadd__",           (PclCFunction) sequence_method_iadd,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__iadd__(y) <==> x += y",
                                WRAPS (in_place_concat) },
        { "__imul__",           (PclCFunction) sequence_method_imul,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__imul__(y) <==> x *= y",
                                WRAPS (in_place_repeat) },
        { "__mul__",            (PclCFunction) sequence_method_mul,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__mul__(y) <==> x * y",
                                WRAPS (repeat) },
        { "__rmul__",           (PclCFunction) sequence_method_rmul,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__rmul__(y) <==> y * x",
                                WRAPS (repeat) },
        { NULL }
};

#undef WRAPS

GType
pcl_sequence_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclSequenceIface),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) NULL,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        0,     /* instance_size */
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        G_TYPE_INTERFACE, "PclSequence", &type_info, 0);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, sequence_methods);
        }
        return type;
}

PclObject *
pcl_sequence_concat (PclObject *object1, PclObject *object2)
{
        PclSequenceIface *iface;

        iface = PCL_SEQUENCE_GET_IFACE (object1);
        if (iface != NULL && iface->concat != NULL)
                return iface->concat (object1, object2);

        pcl_error_bad_internal_call ();
        return NULL;
}

glong
pcl_sequence_count (PclObject *object1, PclObject *object2)
{
        PclObject *iterator;
        PclObject *next;
        glong count = 0;
        gint is_equal;

        if (object1 == NULL || object2 == NULL)
        {
                if (!pcl_error_occurred ())
                        pcl_error_set_string (
                                pcl_exception_system_error (), G_STRLOC
                                ": null argument to internal function");
                return -1;
        }

        iterator = pcl_object_iterate (object1);
        if (iterator == NULL)
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "iterable argument required");
                return -1;
        }

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                is_equal = pcl_object_rich_compare_bool (
                        object2, next, PCL_EQ);
                pcl_object_unref (next);
                if (is_equal < 0)
                {
                        pcl_object_unref (iterator);
                        return -1;
                }
                if (is_equal > 0 && ++count <= 0)
                {
                        pcl_error_set_string (
                                pcl_exception_overflow_error (),
                                "count exceeds C long size");
                        pcl_object_unref (iterator);
                        return -1;
                }
        }

        pcl_object_unref (iterator);
        if (pcl_error_occurred ())
                return -1;
        return count;
}

glong
pcl_sequence_index (PclObject *object1, PclObject *object2)
{
        PclObject *iterator;
        PclObject *next;
        gboolean wrapped = FALSE;
        glong index = 0;
        gint is_equal;

        if (object1 == NULL || object2 == NULL)
        {
                if (!pcl_error_occurred ())
                        pcl_error_set_string (
                                pcl_exception_system_error (), G_STRLOC
                                ": null argument to internal function");
                return -1;
        }

        iterator = pcl_object_iterate (object1);
        if (iterator == NULL)
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "iterable argument required");
                return -1;
        }

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                is_equal = pcl_object_rich_compare_bool (
                        object2, next, PCL_EQ);
                pcl_object_unref (next);
                if (is_equal < 0)
                        goto fail;
                if (is_equal > 0)
                {
                        if (wrapped)
                        {
                                pcl_error_set_string (
                                        pcl_exception_overflow_error (),
                                        "index exceeds C long size");
                                goto fail;
                        }
                        goto done;
                }

                wrapped = (++index <= 0);
        }

        if (!pcl_error_occurred ())
                pcl_error_set_format (
                        pcl_exception_value_error (),
                        "%s.index(x): x not in %s",
                        PCL_GET_TYPE_NAME (object1),
                        PCL_GET_TYPE_NAME (object1));

fail:
        index = -1;

done:
        pcl_object_unref (iterator);
        return index;
}

PclObject *
pcl_sequence_repeat (PclObject *object, glong count)
{
        PclSequenceIface *iface;

        iface = PCL_SEQUENCE_GET_IFACE (object);
        if (iface != NULL && iface->repeat != NULL)
                return iface->repeat (object, count);

        pcl_error_bad_internal_call ();
        return NULL;
}

PclObject *
pcl_sequence_as_list (PclObject *object)
{
        PclSequenceIface *iface;

        iface = PCL_SEQUENCE_GET_IFACE (object);
        if (iface != NULL && iface->as_list != NULL)
                return iface->as_list (object);

        pcl_error_bad_internal_call ();
        return NULL;
}

PclObject *
pcl_sequence_as_tuple (PclObject *object)
{
        PclSequenceIface *iface;

        iface = PCL_SEQUENCE_GET_IFACE (object);
        if (iface != NULL && iface->as_tuple != NULL)
                return iface->as_tuple (object);

        pcl_error_bad_internal_call ();
        return NULL;
}

PclObject *
pcl_sequence_in_place_concat (PclObject *object1, PclObject *object2)
{
        PclSequenceIface *iface;

        iface = PCL_SEQUENCE_GET_IFACE (object1);
        if (iface != NULL && iface->in_place_concat != NULL)
                return iface->in_place_concat (object1, object2);
        if (iface != NULL && iface->concat != NULL)
                return iface->concat (object1, object2);

        pcl_error_bad_internal_call ();
        return NULL;
}

PclObject *
pcl_sequence_in_place_repeat (PclObject *object, glong count)
{
        PclSequenceIface *iface;

        iface = PCL_SEQUENCE_GET_IFACE (object);
        if (iface != NULL && iface->in_place_repeat != NULL)
                return iface->in_place_repeat (object, count);
        if (iface != NULL && iface->repeat != NULL)
                return iface->repeat (object, count);

        pcl_error_bad_internal_call ();
        return NULL;
}

PclObject *
pcl_sequence_get_item (PclObject *object, glong index)
{
        /* XXX Obviously this doesn't belong here anymore. */
        PclObjectClass *class;

        class = PCL_OBJECT_GET_CLASS (object);

        if (class->get_item != NULL)
        {
                PclObject *result;
                PclObject *subscript;
                subscript = pcl_int_from_long (index);
                result = class->get_item (object, subscript);
                pcl_object_unref (subscript);
                return result;
        }

        pcl_error_bad_internal_call ();
        return NULL;
}
