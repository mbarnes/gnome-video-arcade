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

static gchar tuple_doc[] =
"tuple() -> an empty tuple\n\
tuple(sequence) -> tuple initialized from sequence's items\n\
\n\
If the argument is a tuple, the return value is the same object.";

/* Note: tuple_factory[0] is not used */
static PclObject *tuple_empty;
static PclFactory tuple_factory[20];

static gpointer tuple_parent_class = NULL;

static PclObject *
tuple_get_slice (PclObject *self, glong start, glong stop)
{
        gpointer *rdata, *wdata;
        PclObject *result;
        glong ii, length;

        length = PCL_TUPLE_GET_SIZE (self);
        if (start < 0)
                start = 0;
        if (stop > length)
                stop = length;
        if (stop < start)
                stop = start;
        if (start == 0 && stop == length)
                return pcl_object_ref (self);
        length = stop - start;
        result = pcl_tuple_new (length);
        if (result == NULL)
                return NULL;
        rdata = PCL_TUPLE (self)->array->pdata + start;
        wdata = PCL_TUPLE (result)->array->pdata;
        for (ii = 0; ii < length; ii++)
                wdata[ii] = pcl_object_ref (rdata[ii]);
        return result;
}

static void
tuple_dispose (GObject *g_object)
{
        PclTuple *self = PCL_TUPLE (g_object);
        gpointer *pdata = self->array->pdata;
        gpointer *end = pdata + self->array->len;

        while (pdata != end)
        {
                PCL_CLEAR (*pdata);
                pdata++;
        }

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (tuple_parent_class)->dispose (g_object);
}

static void
tuple_finalize (GObject *g_object)
{
        PclTuple *self = PCL_TUPLE (g_object);

        g_ptr_array_free (self->array, TRUE);

        /* Chain up to parent's finalize method. */
        G_OBJECT_CLASS (tuple_parent_class)->finalize (g_object);
}

static PclObject *
tuple_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        const gchar *kwlist[] = { "sequence", NULL };
        PclObject *sequence = NULL;

        if (!pcl_arg_parse_tuple_and_keywords (
                        args, kwds, "|O:tuple", kwlist, &sequence))
                return NULL;
        if (sequence == NULL)
                return pcl_tuple_new (0);
        if (PCL_IS_SEQUENCE (sequence))
                return pcl_sequence_as_tuple (sequence);
        if (PCL_IS_ITERATOR (sequence))
        {
                GPtrArray *array;
                PclObject *next;
                PclObject *tuple;

                /* FIXME Need better error-handling here. */
                tuple = pcl_object_new (PCL_TYPE_TUPLE, NULL);
                array = PCL_TUPLE (tuple)->array;
                while ((next = pcl_iterator_next (sequence)) != NULL)
                        g_ptr_array_add (array, next);
                return tuple;
        }

        pcl_error_set_string (
                pcl_exception_type_error (),
                "tuple(): argument must be a sequence or iterator");
        return NULL;
}

static PclObject *
tuple_clone (PclObject *object)
{
        PclThreadState *ts = pcl_thread_state_get ();
        GPtrArray *array_self, *array_clone;
        gpointer *rdata, *wdata, *end;
        gboolean has_mutable_items = FALSE;
        PclObject *clone;

        array_self = PCL_TUPLE (object)->array;
        clone = pcl_tuple_new (array_self->len);
        array_clone = PCL_TUPLE (clone)->array;

        rdata = array_self->pdata;
        wdata = array_clone->pdata;
        end = rdata + array_self->len;

        while (rdata != end)
        {
                *wdata = pcl_object_clone (*rdata);
                if (*wdata != *rdata)
                        has_mutable_items = TRUE;
                *wdata++, *rdata++;
        }

        if (!has_mutable_items)
        {
                pcl_object_unref (clone);
                clone = pcl_object_ref (object);
        }

        g_hash_table_insert (ts->cache,
                             pcl_object_ref (object),
                             pcl_object_ref (clone));
        return clone;
}

static gboolean
tuple_print (PclObject *object, FILE *stream, PclPrintFlags flags)
{
        PclTuple *self = PCL_TUPLE (object);
        gpointer *pdata = self->array->pdata;
        gpointer *end = pdata + self->array->len;

        fputs ("(", stream);
        while (pdata != end)
        {
                if (pdata != self->array->pdata)
                        fputs (", ", stream);
                if (!pcl_object_print (*pdata++, stream, 0))
                        return FALSE;
        }
        if (self->array->len == 1)
                fputs (",", stream);
        fputs (")", stream);
        return TRUE;
}

static guint
tuple_hash (PclObject *object)
{
        /* Copied from Python's tuple hash algorithm. */
        PclTuple *self = PCL_TUPLE (object);
        gpointer *pdata = self->array->pdata;
        guint mult = 1000003L;
        guint ii, length;
        guint x, y;

        x = 0x345678L;
        length = self->array->len;
        for (ii = 0; ii < length; ii++)
        {
                PclObject *tmp = *pdata++;
                g_assert (tmp != NULL);
                y = pcl_object_hash (tmp);
                if (y == PCL_HASH_INVALID)
                        return PCL_HASH_INVALID;
                x = (x ^ y) * mult;
                mult += 82520L + ii + ii;
        }
        x += 97531L;
        return PCL_HASH_VALIDATE (x);
}

static PclObject *
tuple_repr (PclObject *object)
{
        PclTuple *self = PCL_TUPLE (object);
        gpointer *pdata = self->array->pdata;
        gpointer *end = pdata + self->array->len;
        PclObject *pieces = NULL;
        PclObject *result = NULL;
        PclObject *string;
        PclObject *temp;

        if (self->array->len == 0)
                return pcl_string_from_string ("()");

        pieces = pcl_list_new (0);
        if (pieces == NULL)
                return NULL;

        while (pdata != end)
        {
                gboolean success;

                string = pcl_object_repr (*pdata++);
                if (string == NULL)
                        goto done;
                success = pcl_list_append (pieces, string);
                pcl_object_unref (string);
                if (!success)
                        goto done;
        }

        /* Add "()" decorations to the first and last items. */
        g_assert (PCL_LIST_GET_SIZE (pieces) > 0);
        string = pcl_string_from_string ("(");
        if (string == NULL)
                goto done;
        temp = PCL_LIST_GET_ITEM (pieces, 0);
        pcl_string_concat_and_del (&string, temp);
        PCL_LIST_SET_ITEM (pieces, 0, string);
        if (string == NULL)
                goto done;
        if (PCL_LIST_GET_SIZE (pieces) == 1)
                string = pcl_string_from_string (",)");
        else
                string = pcl_string_from_string (")");
        if (string == NULL)
                goto done;
        temp = PCL_LIST_GET_ITEM (pieces, PCL_LIST_GET_SIZE (pieces) - 1);
        pcl_string_concat_and_del (&temp, string);
        PCL_LIST_SET_ITEM (pieces, PCL_LIST_GET_SIZE (pieces) - 1, temp);
        if (temp == NULL)
                goto done;

        /* Paste them all together with ", " between. */
        string = pcl_string_from_string (", ");
        if (string == NULL)
                goto done;
        result = pcl_string_join (string, pieces);
        pcl_object_unref (string);

done:
        if (pieces != NULL)
                pcl_object_unref (pieces);
        return result;
}

static PclObject *
tuple_rich_compare (PclObject *object1, PclObject *object2,
                    PclRichCompareOps op)
{
        PclObject *item1 = NULL;
        PclObject *item2 = NULL;
        glong ii, length1, length2;
        gint is_true = 0;

        if (!PCL_IS_TUPLE (object1) || !PCL_IS_TUPLE (object2))
                return pcl_object_ref (PCL_NOT_IMPLEMENTED);

        length1 = PCL_TUPLE_GET_SIZE (object1);
        length2 = PCL_TUPLE_GET_SIZE (object2);

        /* Search for the first index where items are different. */
        for (ii = 0; ii < length1 && ii < length2; ii++)
        {
                item1 = PCL_TUPLE_GET_ITEM (object1, ii);
                item2 = PCL_TUPLE_GET_ITEM (object2, ii);
                is_true = pcl_object_rich_compare_bool (item1, item2, PCL_EQ);
                if (is_true < 0)
                        return NULL;
                if (!is_true)
                        break;
        }

        if (ii >= length1 || ii >= length2)
        {
                /* No more items to compare, so compare lengths. */
                switch (op)
                {
                        case PCL_LT:
                                is_true = (length1 < length2);
                                break;
                        case PCL_LE:
                                is_true = (length1 <= length2);
                                break;
                        case PCL_EQ:
                                is_true = (length1 == length2);
                                break;
                        case PCL_NE:
                                is_true = (length1 != length2);
                                break;
                        case PCL_GT:
                                is_true = (length1 > length2);
                                break;
                        case PCL_GE:
                                is_true = (length1 >= length2);
                                break;
                }
                return pcl_bool_from_boolean (is_true);
        }

        /* We have an item that differs -- shortcuts for EQ/NE. */
        if (op == PCL_EQ)
                return pcl_object_ref (PCL_FALSE);
        if (op == PCL_NE)
                return pcl_object_ref (PCL_TRUE);

        /* Compare the final item again using the proper operator. */
        return pcl_object_rich_compare (item1, item2, op);
}

static gint
tuple_contains (PclObject *object, PclObject *value)
{
        PclTuple *self = PCL_TUPLE (object);
        gpointer *pdata = self->array->pdata;
        gpointer *end = pdata + self->array->len;
        gint is_equal;

        while (pdata != end)
        {
                is_equal = pcl_object_rich_compare_bool (
                        value, *pdata++, PCL_EQ);
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
tuple_iterate (PclObject *object)
{
        return pcl_sequence_iterator_new (object);
}

static glong
tuple_measure (PclObject *object)
{
        PclTuple *self = PCL_TUPLE (object);
        return (glong) self->array->len;
}

static PclObject *
tuple_get_item (PclObject *object, PclObject *subscript)
{
        if (PCL_IS_INT (subscript))
        {
                PclObject *result;
                glong index;

                index = PCL_INT_AS_LONG (subscript);
                if (index < 0)
                        index += PCL_TUPLE_GET_SIZE (object);
                result = pcl_tuple_get_item (object, index);
                if (result != NULL)
                        pcl_object_ref (result);
                return result;
        }
        else if (PCL_IS_SLICE (subscript))
        {
                glong start, stop, step, length;
                gpointer *src, *dst, *end;
                PclObject *result;

                if (!pcl_slice_get_indices (PCL_SLICE (subscript),
                                PCL_TUPLE_GET_SIZE (object),
                                &start, &stop, &step, &length))
                        return NULL;

                result = pcl_tuple_new (length);
                src = PCL_TUPLE (object)->array->pdata + start;
                dst = PCL_TUPLE (result)->array->pdata;
                end = dst + length;

                while (dst != end)
                {
                        *dst++ = pcl_object_ref (*src);
                        src += step;
                }

                return result;
        }

        pcl_error_set_string (
                pcl_exception_type_error (),
                "tuple indices must be integers");
        return NULL;
}

static gboolean
tuple_traverse (PclContainer *container, PclTraverseFunc func,
                gpointer user_data)
{
        PclTuple *self = PCL_TUPLE (container);
        gpointer *pdata = self->array->pdata;
        gpointer *end = pdata + self->array->len;

        while (pdata != end)
        {
                PclObject *object = *pdata++;
                if (object != NULL)
                        if (!func (object, user_data))
                                return FALSE;
        }

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (tuple_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
tuple_sequence_concat (PclObject *object1, PclObject *object2)
{
        GPtrArray *array1, *array2;
        gpointer *rdata, *wdata, *end;
        PclObject *result;

        if (!PCL_IS_TUPLE (object2))
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "can only concatenate tuple (not \"%s\" to tuple",
                        PCL_GET_TYPE_NAME (object2));
                return NULL;
        }

        array1 = PCL_TUPLE (object1)->array;
        array2 = PCL_TUPLE (object2)->array;

        result = pcl_tuple_new (array1->len + array2->len);
        wdata = PCL_TUPLE (result)->array->pdata;

        rdata = array1->pdata;
        end = rdata + array1->len;

        while (rdata != end)
                *wdata++ = pcl_object_ref (*rdata++);

        rdata = array2->pdata;
        end = rdata + array2->len;

        while (rdata != end)
                *wdata++ = pcl_object_ref (*rdata++);

        return result;
}

static PclObject *
tuple_sequence_repeat (PclObject *object, glong times)
{
        PclTuple *self = PCL_TUPLE (object);
        gpointer *rdata, *wdata, *end;
        PclObject *result;
        glong ii;

        times = MAX (0, times);
        result = pcl_tuple_new (self->array->len * times);
        wdata = PCL_TUPLE (result)->array->pdata;

        for (ii = 0; ii < times; ii++)
        {
                rdata = self->array->pdata;
                end = rdata + self->array->len;

                while (rdata != end)
                        *wdata++ = pcl_object_ref (*rdata++);
        }

        return result;
}

static PclObject *
tuple_sequence_as_list (PclObject *object)
{
        PclTuple *self = PCL_TUPLE (object);
        gpointer *pdata = self->array->pdata;
        glong ii, length = self->array->len;
        PclObject *list = pcl_list_new (length);

        for (ii = 0; ii < length; ii++)
                pcl_list_set_item (list, ii, pcl_object_ref (*pdata++));
        return list;
}

static PclObject *
tuple_sequence_as_tuple (PclObject *object)
{
        return pcl_object_ref (object);
}

static void
tuple_sequence_init (PclSequenceIface *iface)
{
        iface->concat = tuple_sequence_concat;
        iface->repeat = tuple_sequence_repeat;
        iface->as_list = tuple_sequence_as_list;
        iface->as_tuple = tuple_sequence_as_tuple;
}

static PclObject *
tuple_method_getnewargs (PclTuple *self)
{
        glong length = PCL_TUPLE_GET_SIZE (self);
        return pcl_build_value ("(N)", tuple_get_slice (
                        PCL_OBJECT (self), 0, length));
}

static void
tuple_class_init (PclTupleClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        tuple_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = tuple_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_tuple_get_type_object;
        object_class->new_instance = tuple_new_instance;
        object_class->clone = tuple_clone;
        object_class->print = tuple_print;
        object_class->hash = tuple_hash;
        object_class->repr = tuple_repr;
        object_class->rich_compare = tuple_rich_compare;
        object_class->contains = tuple_contains;
        object_class->iterate = tuple_iterate;
        object_class->measure = tuple_measure;
        object_class->get_item = tuple_get_item;
        object_class->doc = tuple_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = tuple_dispose;
        g_object_class->finalize = tuple_finalize;
}

static void
tuple_init (PclTuple *tuple)
{
        tuple->array = g_ptr_array_new ();
}

static PclMethodDef tuple_methods[] = {
        { "__getnewargs__",     (PclCFunction) tuple_method_getnewargs,
                                PCL_METHOD_FLAG_NOARGS },
        { NULL }
};

GType
pcl_tuple_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclTupleClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) tuple_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclTuple),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) tuple_init,
                        NULL   /* value_table */
                };

                static const GInterfaceInfo sequence_info = {
                        (GInterfaceInitFunc) tuple_sequence_init,
                        (GInterfaceFinalizeFunc) NULL,
                        NULL   /* interface_data */
                };

                /* ITERABLE, MEASURABLE, and SUBSCRIPTABLE are prerequisites
                 * for SEQUENCE */
                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclTuple", &type_info, 0);
                g_type_add_interface_static (
                        type, PCL_TYPE_SEQUENCE, &sequence_info);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, tuple_methods);
        }
        return type;
}

/**
 * pcl_tuple_get_type_object:
 *
 * Returns the type object for #PclTuple.  During runtime this is the built-in
 * object %tuple.
 *
 * Returns: a borrowed reference to the type object for #PclTuple
 */
PclObject *
pcl_tuple_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_TUPLE, "tuple");
                pcl_register_singleton ("<type 'tuple'>", &object);
        }
        return object;
}

PclObject *
pcl_tuple_new (glong size)
{
        PclTuple *tuple;

        if (size < 0 || size > G_MAXUINT)
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        if (size == 0)
                return pcl_object_ref (tuple_empty);
        if (size < G_N_ELEMENTS (tuple_factory))
                tuple = pcl_factory_order (&tuple_factory[size]);
        else
                tuple = pcl_object_new (PCL_TYPE_TUPLE, NULL);
        g_ptr_array_set_size (tuple->array, size);
        return PCL_OBJECT (tuple);
}

PclObject *
pcl_tuple_pack (glong length, ...)
{
        PclObject *result;
        gpointer *pdata, *end;
        va_list ap;

        result = pcl_tuple_new (length);
        pdata = PCL_TUPLE (result)->array->pdata;
        end = pdata + length;

        va_start (ap, length);
        while (pdata != end)
                *pdata++ = pcl_object_ref (va_arg (ap, PclObject *));
        va_end (ap);

        return result;
}

PclObject *
pcl_tuple_get_item (PclObject *self, glong index)
{
        GPtrArray *array;

        if (!PCL_IS_TUPLE (self))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        array = PCL_TUPLE (self)->array;
        if (index < 0 || index >= array->len)
        {
                pcl_error_set_string (
                        pcl_exception_index_error (),
                        "tuple index out of range");
                return NULL;
        }
        return array->pdata[index];
}

PclObject *
pcl_tuple_get_slice (PclObject *self, glong start, glong stop)
{
        if (!PCL_IS_TUPLE (self))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        return tuple_get_slice (self, start, stop);
}

gboolean
pcl_tuple_set_item (PclObject *self, glong index, PclObject *value)
{
        GPtrArray *array;

        if (!PCL_IS_TUPLE (self))
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        array = PCL_TUPLE (self)->array;
        if (index < 0 || index >= array->len)
        {
                if (value != NULL)
                        pcl_object_unref (value);
                pcl_error_set_string (
                        pcl_exception_index_error (),
                        "tuple assignment index out of range");
                return FALSE;
        }

        if (value == NULL)
        {
                if (array->pdata[index] != NULL)
                        pcl_object_unref (array->pdata[index]);
                g_ptr_array_remove_index (array, index);
        }
        else
        {
                gpointer *pdata = array->pdata + index;
                gpointer tmp = *pdata;

                *pdata = value;  /* steal the reference */
                if (tmp != NULL)
                        pcl_object_unref (tmp);
        }
        return TRUE;
}

void
_pcl_tuple_init (void)
{
        gint ii;

        for (ii = 0; ii < G_N_ELEMENTS (tuple_factory); ii++)
        {
                tuple_factory[ii].type = PCL_TYPE_TUPLE;
                tuple_factory[ii].limit = 2000;
        }

        tuple_empty = pcl_object_new (PCL_TYPE_TUPLE, NULL);
}

void
_pcl_tuple_fini (void)
{
        gint ii;

        for (ii = 0; ii < G_N_ELEMENTS (tuple_factory); ii++)
                pcl_factory_close (&tuple_factory[ii]);

        PCL_CLEAR (tuple_empty);
}
