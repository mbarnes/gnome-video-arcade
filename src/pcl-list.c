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

static gchar list_doc[] =
"list() -> new list\n\
list(sequence) -> new list initialized from sequence's items";

static PclFactory list_factory;

static gpointer list_parent_class = NULL;

/* Forward Declarations */
static PclObject *list_method_extend (PclList *self, PclObject *other);

/* This is a GCompareFunc for use with g_ptr_array_sort() */
static gint
list_sort_compare (PclObject **p_object1, PclObject **p_object2)
{
        return pcl_object_compare (*p_object1, *p_object2);
}

static void
list_dispose (GObject *g_object)
{
        PclList *self = PCL_LIST (g_object);
        gpointer *pdata = self->array->pdata;
        gpointer *end = pdata + self->array->len;

        while (pdata != end)
        {
                PCL_CLEAR (*pdata);
                pdata++;
        }

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (list_parent_class)->dispose (g_object);
}

static void
list_finalize (GObject *g_object)
{
        PclList *self = PCL_LIST (g_object);

        g_ptr_array_free (self->array, TRUE);

        /* Chain up to parent's finalize method. */
        G_OBJECT_CLASS (list_parent_class)->finalize (g_object);
}

static PclObject *
list_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        const gchar *kwlist[] = { "sequence", NULL };
        PclObject *sequence = NULL;

        if (!pcl_arg_parse_tuple_and_keywords (
                        args, kwds, "|O:list", kwlist, &sequence))
                return NULL;
        if (sequence == NULL)
                return pcl_list_new (0);
        if (PCL_IS_SEQUENCE (sequence))
                return pcl_sequence_as_list (sequence);
        if (PCL_IS_ITERATOR (sequence))
        {
                GPtrArray *array;
                PclObject *item;
                PclObject *list;

                /* XXX Need better error-handling here. */
                list = pcl_object_new (PCL_TYPE_LIST, NULL);
                array = PCL_LIST (list)->array;
                while ((item = pcl_iterator_next (sequence)) != NULL)
                        g_ptr_array_add (array, item);
                return list;
        }

        pcl_error_set_string (
                pcl_exception_type_error (),
                "list(): argument must be a sequence or iterator");
        return NULL;
}

static PclObject *
list_clone (PclObject *object)
{
        PclThreadState *ts = pcl_thread_state_get ();
        GPtrArray *array_self, *array_clone;
        gpointer *rdata, *wdata, *end;
        PclObject *clone;

        array_self = PCL_LIST (object)->array;
        clone = pcl_list_new (array_self->len);
        array_clone = PCL_LIST (clone)->array;

        rdata = array_self->pdata;
        wdata = array_clone->pdata;
        end = rdata + array_self->len;

        g_hash_table_insert (ts->cache,
                             pcl_object_ref (object),
                             pcl_object_ref (clone));

        while (rdata != end)
                /* XXX Should check for NULL return. */
                *wdata++ = pcl_object_clone (*rdata++);

        return clone;
}

static gboolean
list_print (PclObject *object, FILE *stream, PclPrintFlags flags)
{
        GPtrArray *array = PCL_LIST (object)->array;
        gpointer *pdata = array->pdata;
        gpointer *end = pdata + array->len;
        gint been_here;

        /* Prevent infinite recursion if a list refers to itself. */
        been_here = pcl_repr_enter (object);
        if (been_here < 0)
                return FALSE;
        if (been_here > 0)
                return fputs ("[...]", stream);

        fputs ("[", stream);
        while (pdata != end)
        {
                if (pdata != array->pdata)
                        fputs (", ", stream);
                if (!pcl_object_print (*pdata++, stream, 0))
                {
                        pcl_repr_leave (object);
                        return FALSE;
                }
        }
        fputs ("]", stream);

        pcl_repr_leave (object);
        return TRUE;
}

static PclObject *
list_copy (PclObject *object)
{
        GPtrArray *array_self, *array_copy;
        gpointer *rdata, *wdata, *end;
        PclObject *copy;

        array_self = PCL_LIST (object)->array;
        copy = pcl_list_new (array_self->len);
        array_copy = PCL_LIST (copy)->array;

        rdata = array_self->pdata;
        wdata = array_copy->pdata;
        end = rdata + array_self->len;

        /* shallow copy */
        while (rdata != end)
                *wdata++ = pcl_object_ref (*rdata++);
        return copy;
}

static PclObject *
list_repr (PclObject *object)
{
        PclList *self = PCL_LIST (object);
        gpointer *pdata = self->array->pdata;
        gpointer *end = pdata + self->array->len;
        PclObject *pieces = NULL;
        PclObject *result = NULL;
        PclObject *string;
        PclObject *temp;
        gint been_here;

        /* Prevent infinite recursion if a list refers to itself. */
        been_here = pcl_repr_enter (object);
        if (been_here < 0)
                return NULL;
        if (been_here > 0)
                return pcl_string_from_string ("[...]");

        if (pdata == end)
                return pcl_string_from_string ("[]");

        pieces = pcl_list_new (0);
        if (pieces == NULL)
                goto done;

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

        /* Add "[]" decorations to the first and last items. */
        g_assert (PCL_LIST_GET_SIZE (pieces) > 0);
        string = pcl_string_from_string ("[");
        if (string == NULL)
                goto done;
        temp = PCL_LIST_GET_ITEM (pieces, 0);
        pcl_string_concat_and_del (&string, temp);
        PCL_LIST_SET_ITEM (pieces, 0, string);
        if (string == NULL)
                goto done;
        string = pcl_string_from_string ("]");
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
        pcl_repr_leave (object);
        return result;
}

static PclObject *
list_rich_compare (PclObject *object1, PclObject *object2,
                   PclRichCompareOps op)
{
        PclObject *item1 = NULL;
        PclObject *item2 = NULL;
        glong ii, length1, length2;
        gint is_true = 0;

        if (!PCL_IS_LIST (object1) || !PCL_IS_LIST (object2))
                return pcl_object_ref (PCL_NOT_IMPLEMENTED);

        length1 = PCL_LIST_GET_SIZE (object1);
        length2 = PCL_LIST_GET_SIZE (object2);

        if (length1 != length2 && (op == PCL_EQ || op == PCL_NE))
        {
                /* If the lengths differ, the lists differ. */
                if (op == PCL_EQ)
                        return pcl_object_ref (PCL_FALSE);
                else
                        return pcl_object_ref (PCL_TRUE);
        }

        /* Search for the first index where items are different. */
        for (ii = 0; ii < length1 && ii < length2; ii++)
        {
                item1 = PCL_LIST_GET_ITEM (object1, ii);
                item2 = PCL_LIST_GET_ITEM (object2, ii);
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
list_contains (PclObject *object, PclObject *value)
{
        PclList *self = PCL_LIST (object);
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
        return 0;
}

static PclObject *
list_iterate (PclObject *object)
{
        return pcl_sequence_iterator_new (object);
}

static glong
list_measure (PclObject *object)
{
        return PCL_LIST_GET_SIZE (object);
}

static PclObject *
list_get_item (PclObject *object, PclObject *subscript)
{
        if (PCL_IS_INT (subscript))
        {
                PclObject *result;
                glong index;

                index = pcl_int_as_long (subscript);
                if (index < 0)
                        index += pcl_object_measure (object);
                result = pcl_list_get_item (object, index);
                if (result != NULL)
                        pcl_object_ref (result);
                return result;
        }
        else if (PCL_IS_SLICE (subscript))
                return pcl_list_get_slice (object, subscript);

        pcl_error_set_string (
                pcl_exception_type_error (),
                "list indices must be integers");
        return NULL;
}

static gboolean
list_set_item (PclObject *object, PclObject *subscript, PclObject *value)
{
        if (PCL_IS_INT (subscript))
        {
                glong index;

                index = pcl_int_as_long (subscript);
                if (index < 0)
                        index += PCL_LIST_GET_SIZE (object);
                if (value != NULL)
                        pcl_object_ref (value);
                /* pcl_list_set_item() steals the reference */
                return pcl_list_set_item (object, index, value);
        }
        else if (PCL_IS_SLICE (subscript))
                return pcl_list_set_slice (object, subscript, value);

        pcl_error_set_string (
                pcl_exception_type_error (),
                "list indices must be integers");
        return FALSE;
}

static gboolean
list_traverse (PclContainer *container, PclTraverseFunc func,
               gpointer user_data)
{
        PclList *self = PCL_LIST (container);
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
        return PCL_CONTAINER_CLASS (list_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
list_sequence_concat (PclObject *object1, PclObject *object2)
{
        GPtrArray *array1, *array2;
        gpointer *rdata, *wdata, *end;
        PclObject *result;

        if (!PCL_IS_LIST (object2))
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "can only concatenate list (not \"%s\") to list",
                        PCL_GET_TYPE_NAME (object2));
                return NULL;
        }

        array1 = PCL_LIST (object1)->array;
        array2 = PCL_LIST (object2)->array;

        result = pcl_list_new (array1->len + array2->len);
        wdata = PCL_LIST (result)->array->pdata;

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
list_sequence_repeat (PclObject *object, glong times)
{
        PclList *self = PCL_LIST (object);
        gpointer *rdata, *wdata, *end;
        PclObject *result;
        glong ii;

        times = MAX (0, times);
        result = pcl_list_new (self->array->len * times);
        wdata = PCL_LIST (result)->array->pdata;

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
list_sequence_as_list (PclObject *object)
{
        return pcl_object_copy (object);
}

static PclObject *
list_sequence_as_tuple (PclObject *object)
{
        return pcl_list_as_tuple (object);
}

static PclObject *
list_sequence_in_place_concat (PclObject *object1, PclObject *object2)
{
        PclList *self = PCL_LIST (object1);
        PclObject *result;

        result = list_method_extend (self, object2);
        if (result == NULL)
                return NULL;
        pcl_object_unref (result);
        return pcl_object_ref (object1);
}

static PclObject *
list_sequence_in_place_repeat (PclObject *object, glong times)
{
        PclList *self = PCL_LIST (object);
        gpointer *rdata, *wdata, *end;
        guint len;

        if (self->array->len == 0)
                return pcl_object_ref (object);
        if (times < 1)
        {
                wdata = self->array->pdata;
                end = wdata + self->array->len;
                while (wdata != end)
                        pcl_object_unref (*wdata++);
                g_ptr_array_set_size (self->array, 0);
                return pcl_object_ref (object);
        }

        len = self->array->len;
        g_ptr_array_set_size (self->array, len * times);

        rdata = self->array->pdata;
        wdata = rdata + len;
        end = rdata + self->array->len;
        while (wdata != end)
                *wdata++ = pcl_object_ref (*rdata++);

        return pcl_object_ref (object);
}

static void
list_sequence_init (PclSequenceIface *iface)
{
        iface->concat = list_sequence_concat;
        iface->repeat = list_sequence_repeat;
        iface->as_list = list_sequence_as_list;
        iface->as_tuple = list_sequence_as_tuple;
        iface->in_place_concat = list_sequence_in_place_concat;
        iface->in_place_repeat = list_sequence_in_place_repeat;
}

static PclObject *
list_method_append (PclList *self, PclObject *object)
{
        GPtrArray *array = PCL_LIST (self)->array;
        g_ptr_array_add (array, pcl_object_ref (object));
        return pcl_object_ref (PCL_NONE);
}

static PclObject *
list_method_count (PclList *self, PclObject *object)
{
        gpointer *pdata = self->array->pdata;
        gpointer *end = pdata + self->array->len;
        glong count = 0;
        gint is_equal;
 
        while (pdata != end)
        {
                is_equal = pcl_object_rich_compare_bool (
                        object, *pdata++, PCL_EQ);
                if (is_equal < 0)
                        return NULL;
                if (is_equal > 0)
                        count++;
        }
        return pcl_int_from_long (count);
}

static PclObject *
list_method_extend (PclList *self, PclObject *object)
{
        PclObject *iterator, *next;
        GPtrArray *array;
 
        if (!PCL_IS_ITERABLE (object))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "argument must be iterable");
                return NULL;
        }

        if ((PclObject *) self == object)
                object = pcl_object_copy (object);
        else
                pcl_object_ref (object);
               
        array = PCL_LIST (self)->array;
        iterator = pcl_object_iterate (object);
        while ((next = pcl_iterator_next (iterator)) != NULL)
                g_ptr_array_add (array, next);
 
        pcl_object_unref (iterator);
        pcl_object_unref (object);

        return pcl_object_ref (PCL_NONE);
}

static PclObject *
list_method_index (PclList *self, PclObject *args)
{
        GPtrArray *array = self->array;
        PclObject *item;
        gint is_equal;
        glong start = 0;
        glong stop = array->len;
        glong ii;
 
        if (!pcl_arg_parse_tuple (args, "O|ll:index", &item, &start, &stop))
                return NULL;
 
        if (start < 0)
        {
                start += array->len;
                if (start < 0)
                        start = 0;
        }
        if (stop <= 0 && stop < start)
        {
                stop += array->len;
                if (stop <= 0)
                        stop = 0;
        }

        for (ii = start; ii < stop && ii < array->len; ii++)
        {
                is_equal = pcl_object_rich_compare_bool (
                        item, array->pdata[ii], PCL_EQ);
                if (is_equal > 0)
                        return pcl_int_from_long (ii);
                if (is_equal < 0)
                        return NULL;
        }
 
        pcl_error_set_string (
                pcl_exception_value_error (),
                "list.index(x): x not in list");
        return NULL;
}

static PclObject *
list_method_insert (PclList *self, PclObject *args)
{
        PclObject *object;
        glong index;
        
        if (!pcl_arg_parse_tuple (args, "lO:insert", &index, &object))
                return NULL;
 
        if (!pcl_list_insert (PCL_OBJECT (self), index, object))
                return NULL;

        return pcl_object_ref (PCL_NONE);
}

static PclObject *
list_method_pop (PclList *self, PclObject *args)
{
        GPtrArray *array = PCL_LIST (self)->array;
        PclObject *value;
        glong index = -1;
        
        if (array->len == 0)
        {
                pcl_error_set_string (
                        pcl_exception_index_error (),
                        "pop from empty list");
                return NULL;
        }
        
        if (!pcl_arg_parse_tuple (args, "|l:pop", &index))
                return NULL;
 
        if (index < 0)
                index += array->len;
        if (index < 0 || index >= array->len)
        {
                pcl_error_set_string (
                        pcl_exception_index_error (),
                        "pop index out of range");
                return NULL;
        }
        
        /* Stealing the reference from the array */
        value = array->pdata[index];
        /* This function does not unref the removed value */
        g_ptr_array_remove_index (array, index);

        return value;
}

static PclObject *
list_method_remove (PclList *self, PclObject *object)
{
        GPtrArray *array = self->array;
        guint ii, length;
        gint is_equal;

        length = array->len;
        for (ii = 0; ii < length; ii++)
        {
                is_equal = pcl_object_rich_compare_bool (
                        object, array->pdata[ii], PCL_EQ);
                if (is_equal > 0)
                {
                        gpointer data;
                        data = g_ptr_array_remove_index (array, ii);
                        pcl_object_unref (data);
                        return pcl_object_ref (PCL_NONE);
                }
                if (is_equal < 0)
                        return NULL;
        }

        pcl_error_set_string (
                pcl_exception_value_error (),
                "list.remove(x): x not in list");
        return NULL;
}

static PclObject *
list_method_reverse (PclList *self)
{
        if (!pcl_list_reverse (PCL_OBJECT (self)))
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

/*********** Utility functions to support the quicksort algorithm **********/

static gint
compare (PclObject *left, PclObject *right, PclObject *cmp_func, gint order)
{
        PclObject *args, *value;
        gint retval = 0;
        
        if (cmp_func == NULL)
                return pcl_object_compare (left, right) * order;
                
        args = pcl_build_value ("(OO)", left, right);
        value = pcl_object_call (cmp_func, args, NULL);
        if (!value)
                return -2;
        if (!PCL_IS_INT (value))
        {
                pcl_error_set_string (pcl_exception_type_error (),
                                      "comparison function must return int");
                return -2;
        }
        retval = pcl_int_as_long (value) * order;
        pcl_object_unref (value);
        if (args)
                pcl_object_unref (args);
                
        return retval;
}

static gint
partition (gpointer *pdata, gint i, gint j, PclObject *cmp_func,
           GHashTable *ht, gint order)
{
        gint comp;
        PclObject *val, *swapper;
        PclObject *x = g_hash_table_lookup (ht, pdata[i]);
        gint a = i - 1;
        gint b = j + 1;
        
        while (a < b)
        {
                do
                {
                        a++;
                        val = g_hash_table_lookup (ht, pdata[a]);
                        if ((comp = compare (val, x, cmp_func, order)) == -2)
                                if (pcl_error_occurred ())
                                        return -1;
                } while (comp < 0 && a < j - 1);
                
                do
                {
                        b--;
                        val = g_hash_table_lookup (ht, pdata[b]);
                        if ((comp = compare (val, x, cmp_func, order)) == -2)
                                if (pcl_error_occurred ())
                                        return -1;
                } while (comp > 0 && i < b);
                
                if (a < b)
                {
                        swapper = pdata[a];
                        pdata[a] = pdata[b];
                        pdata[b] = swapper;
                }
        }
        return b;
}

static gint
quicksort (gpointer pdata, gint first, gint last, PclObject *cmp_func,
           GHashTable *ht, gint order)
{
        gint mid, retVal = 1;
        if (first < last)
        {
                mid = partition (pdata, first, last, cmp_func, 
                                 ht, order);
                if (mid < 0)
                        return 0;
                retVal &= quicksort (pdata, first, mid, cmp_func,
                                     ht, order);
                retVal &= quicksort (pdata, mid + 1, last, cmp_func,
                                     ht, order);
        }
        return retVal;
}

static PclObject *
list_method_sort (PclList *self, PclObject *args, PclObject *kwds)
{
        /* XXX Using quicksort for simplicity, but IT IS NOT STABLE */
        
        const gchar *kwlist[] = { "cmp", "key", "reverse", NULL };
        PclObject *cmp_func = NULL;
        PclObject *key_func = NULL;
        PclObject *reverse = NULL;
        PclObject *argtuple = NULL;
        PclObject *value = NULL;
        GHashTable *ht;
        GPtrArray *array;
        gint ii, retval, order = 1;
        
        if (!pcl_arg_parse_tuple_and_keywords (args, kwds, "|OOO:sort",
                        kwlist, &cmp_func, &key_func, &reverse))
                return NULL;
        

        if (cmp_func == PCL_NONE)
                cmp_func = NULL;
        if (cmp_func && !PCL_IS_CALLABLE (cmp_func))
        {
                pcl_error_set_string (pcl_exception_type_error (),
                        "user-defined cmp() method is uncallable");
                return NULL;
        }
        if (key_func && !PCL_IS_CALLABLE (key_func))
        {
                pcl_error_set_string (pcl_exception_type_error (),
                        "user-defined key() method is uncallable");
                return NULL;
        }
        
        if (reverse && pcl_object_is_true (reverse))
                order = -1;
                
        array = PCL_LIST (self)->array;
        if (array->len < 2)
                return pcl_object_ref (PCL_NONE);
        
        ht = g_hash_table_new_full (
                g_direct_hash, g_direct_equal, NULL, pcl_object_unref);
        
        /* Building up precomputed keys */
        for (ii = 0; ii < array->len; ii++)
        {
                if (key_func)
                {
                        argtuple = pcl_build_value ("(O)", array->pdata[ii]);
                        value = pcl_object_call (key_func, argtuple, NULL);
                        if (argtuple)
                                pcl_object_unref (argtuple);
                }
                else
                {
                        value = pcl_object_ref (array->pdata[ii]);
                }
                if (value == NULL)
                        return NULL;
                        
                g_hash_table_insert (ht, array->pdata[ii], value);
        }
        
        retval = quicksort (array->pdata, 0, array->len - 1, 
                            cmp_func, ht, order);
        g_hash_table_destroy (ht);
        
        if (retval)
                return pcl_object_ref (PCL_NONE);
        else
                return NULL;
}

static void
list_class_init (PclListClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        list_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = list_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_list_get_type_object;
        object_class->new_instance = list_new_instance;
        object_class->clone = list_clone;
        object_class->print = list_print;
        object_class->copy = list_copy;
        object_class->repr = list_repr;
        object_class->rich_compare = list_rich_compare;
        object_class->contains = list_contains;
        object_class->iterate = list_iterate;
        object_class->measure = list_measure;
        object_class->get_item = list_get_item;
        object_class->set_item = list_set_item;
        object_class->doc = list_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = list_dispose;
        g_object_class->finalize = list_finalize;
}

static void
list_init (PclList *list)
{
        list->array = g_ptr_array_new ();
}

static PclMethodDef list_methods[] = {
        { "append",             (PclCFunction) list_method_append,
                                PCL_METHOD_FLAG_ONEARG,
                                "L.append(object) -- append object to end" },
        { "count",              (PclCFunction) list_method_count,
                                PCL_METHOD_FLAG_ONEARG,
                                "L.count(value) -> integer -- "
                                "return number of occurrences of value" },
        { "extend",             (PclCFunction) list_method_extend,
                                PCL_METHOD_FLAG_ONEARG,
                                "L.extend(iterable) -- "
                                "extend list by appending elements from "
                                "the iterable" },
        { "index",              (PclCFunction) list_method_index,
                                PCL_METHOD_FLAG_VARARGS,
                                "L.index(value, [start, [stop]]) -> "
                                "integer -- return first index of value" },
        { "insert",             (PclCFunction) list_method_insert,
                                PCL_METHOD_FLAG_VARARGS,
                                "L.insert(index, object) -- "
                                "insert object before index" },
        { "pop",                (PclCFunction) list_method_pop,
                                PCL_METHOD_FLAG_VARARGS,
                                "L.pop([index]) -> item -- "
                                "remove and return item at index "
                                "(default last)" },
        { "remove",             (PclCFunction) list_method_remove,
                                PCL_METHOD_FLAG_ONEARG,
                                "L.remove(value) -- "
                                "remove first occurrence of value" },
        { "reverse",            (PclCFunction) list_method_reverse,
                                PCL_METHOD_FLAG_NOARGS,
                                "L.reverse() -- reverse *IN PLACE*" },
        { "sort",               (PclCFunction) list_method_sort,
                                PCL_METHOD_FLAG_VARARGS |
                                PCL_METHOD_FLAG_KEYWORDS,
                                "L.sort(cmp=None, key=None, reverse=False) -- "
                                "stable sort *IN PLACE*;\n"
                                "cmp(x, y) -> -1, 0, 1" },
        { NULL }
};

GType
pcl_list_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclListClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) list_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclList),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) list_init,
                        NULL   /* value_table */
                };

                static const GInterfaceInfo sequence_info = {
                        (GInterfaceInitFunc) list_sequence_init,
                        (GInterfaceFinalizeFunc) NULL,
                        NULL   /* interface_data */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclList", &type_info, 0);
                g_type_add_interface_static (
                        type, PCL_TYPE_SEQUENCE, &sequence_info);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, list_methods);
        }
        return type;
}

/**
 * pcl_list_get_type_object:
 *
 * Returns the type object for #PclList.  During runtime this is the built-in
 * object %list.
 *
 * Returns: a borrowed reference to the type object for #PclList
 */
PclObject *
pcl_list_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_LIST, "list");
                pcl_register_singleton ("<type 'list'>", &object);
        }
        return object;
}

PclObject *
pcl_list_new (glong size)
{
        PclList *list;

        if (size < 0 || size > G_MAXUINT)
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        list = pcl_factory_order (&list_factory);
        g_ptr_array_set_size (list->array, size);
        return PCL_OBJECT (list);
}

PclObject *
pcl_list_from_iterator (PclObject *iterator)
{
        PclObject *list;
        PclObject *next;

        if (!PCL_IS_ITERATOR (iterator))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }
 
        if (PCL_IS_MEASURABLE (iterator))
        {
                glong index = 0;
                glong length;

                length = pcl_object_measure (iterator);
                list = pcl_list_new (length);
                if (list == NULL)
                        return NULL;

                while ((next = pcl_iterator_next (iterator)) != NULL)
                        PCL_LIST_SET_ITEM (list, index++, next);

                g_assert (index == length);
        }
        else
        {
                list = pcl_list_new (0);
                if (list == NULL)
                        return NULL;

                while ((next = pcl_iterator_next (iterator)) != NULL)
                {
                        if (!pcl_list_append (list, next))
                        {
                                pcl_object_unref (next);
                                pcl_object_unref (list);
                                return NULL;
                        }
                }
        }

        if (pcl_error_occurred ())
        {
                pcl_object_unref (list);
                return NULL;
        }
        return list;
}

gboolean
pcl_list_append (PclObject *self, PclObject *item)
{
        GPtrArray *array;

        if (!PCL_IS_LIST (self) || (item == NULL))
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        array = PCL_LIST (self)->array;
        g_ptr_array_add (array, pcl_object_ref (item));
        return TRUE;
}

gboolean
pcl_list_insert (PclObject *self, glong index, PclObject *item)
{
        GPtrArray *array;
        gpointer *pdata;
        glong ii, length;

        if (!PCL_IS_LIST (self) || (item == NULL))
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        array = PCL_LIST (self)->array;
        length = (glong) array->len;

        g_ptr_array_set_size (array, length + 1);
        pdata = array->pdata;

        if (index < 0)
        {
                index += length;
                if (index < 0)
                        index = 0;
        }
        if (index > length)
                index = length;

        for (ii = length; ii > index; ii--)
                pdata[ii] = pdata[ii - 1];
        pdata[index] = pcl_object_ref (item);
        return TRUE;
}

gboolean
pcl_list_reverse (PclObject *self)
{
        GPtrArray *array;

        if (!PCL_IS_LIST (self))
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        array = PCL_LIST (self)->array;
        if (array->len > 1)
        {
                gpointer *fwd = array->pdata;
                gpointer *bwd = fwd + array->len - 1;

                while (fwd < bwd)
                {
                        gpointer tmp = *fwd;
                        *fwd++ = *bwd;
                        *bwd-- = tmp;
                }
        }
        return TRUE;
}

gboolean
pcl_list_sort (PclObject *self)
{
        GPtrArray *array;

        if (!PCL_IS_LIST (self))
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        array = PCL_LIST (self)->array;
        g_ptr_array_sort (array, (GCompareFunc) list_sort_compare);
        return TRUE;
}

PclObject *
pcl_list_get_item (PclObject *self, glong index)
{
        GPtrArray *array;

        if (!PCL_IS_LIST (self))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        array = PCL_LIST (self)->array;
        if (index < 0 || index >= array->len)
        {
                pcl_error_set_string (
                        pcl_exception_index_error (),
                        "list index out of range");
                return NULL;
        }
        return array->pdata[index];
}

gboolean
pcl_list_set_item (PclObject *self, glong index, PclObject *value)
{
        GPtrArray *array;

        if (!PCL_IS_LIST (self))
        {
                if (value != NULL)
                        pcl_object_unref (value);
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        array = PCL_LIST (self)->array;
        if (index < 0 || index >= array->len)
        {
                if (value != NULL)
                        pcl_object_unref (value);
                pcl_error_set_string (
                        pcl_exception_index_error (),
                        "list assignment index out of range");
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

PclObject *
pcl_list_get_slice (PclObject *self, PclObject *slice)
{
        glong start, stop, step, length;
        gpointer *rdata, *wdata, *end;
        PclObject *result;

        if (!PCL_IS_LIST (self))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        if (!pcl_slice_get_indices (PCL_SLICE (slice),
                                    pcl_object_measure (self),
                                    &start, &stop, &step, &length))
                return NULL;

        result = pcl_list_new (length);

        rdata = PCL_LIST (self)->array->pdata + start;
        wdata = PCL_LIST (result)->array->pdata;
        end = wdata + length;

        while (wdata != end)
        {
                *wdata++ = pcl_object_ref (*rdata);
                rdata += step;
        }

        return result;
}

gboolean
pcl_list_set_slice (PclObject *self, PclObject *slice, PclObject *value)
{
        GPtrArray *array;
        glong start, stop, step, length;

        if (!PCL_IS_LIST (self))
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        if (!pcl_slice_get_indices (PCL_SLICE (slice),
                                    pcl_object_measure (self),
                                    &start, &stop, &step, &length))
                return FALSE;

        array = PCL_LIST (self)->array;
        if (value == NULL)
        {
                /* Delete slice */
                gpointer *pdata = array->pdata;
                glong ii;

                if (length <= 0)
                        return TRUE;

                if (step < 0)
                {
                        stop = start + 1;
                        start = stop + step * (length - 1) - 1;
                        step = ABS (step);
                }

                for (ii = start; ii < stop; ii += step)
                {
                        if (pdata[ii] != NULL)
                                pcl_object_unref (pdata[ii]);
                        pdata[ii] = NULL;
                }

                while (g_ptr_array_remove (array, NULL));
        }
        else
        {
                /* Assign slice */
                PclObject *iterator, *next;
                gboolean normal_slice;

                normal_slice = PCL_IS_NONE (PCL_SLICE (slice)->step);

                if (!PCL_IS_SEQUENCE (value))
                {
                        pcl_error_set_format (
                                pcl_exception_type_error (),
                                "must assign sequence (not \"%s\") to %sslice",
                                PCL_GET_TYPE_NAME (value),
                                normal_slice ? "" : "extended ");
                        return FALSE;
                }

                if (normal_slice)
                {
                        /* Normal slice */
                        glong value_length;
                        gpointer *pdata, *end;

                        /* Special case: a[i:j] = a */
                        if (value == self)
                        {
                                gboolean success;
                                value = pcl_object_copy (self);
                                if (value == NULL)
                                        return FALSE;
                                success = pcl_list_set_slice (
                                        self, slice, value);
                                pcl_object_unref (value);
                                return success;
                        }

                        if (!pcl_list_set_slice (self, slice, NULL))
                                return FALSE;

                        value_length = pcl_object_measure (value);
                        g_ptr_array_set_size (array,
                                array->len + value_length);

                        pdata = array->pdata + array->len;
                        end = &array->pdata[start] + value_length - 1;
                        while (--pdata != end)
                                *pdata = *(pdata - value_length);

                        pdata = &array->pdata[start];
                        iterator = pcl_object_iterate (value);
                        while ((next = pcl_iterator_next (iterator)) != NULL)
                                *pdata++ = next;
                        pcl_object_unref (iterator);
                }
                else
                {
                        /* Extended slice */
                        glong value_length;
                        glong index = start;

                        value_length = pcl_object_measure (value);

                        if (value_length != length)
                        {
                                pcl_error_set_format (
                                        pcl_exception_value_error (),
                                        "attempt to assign sequence of size %d"
                                        " to extended slice of size %d",
                                        value_length, length);
                                return FALSE;
                        }

                        if (value_length == 0)
                                return TRUE;

                        iterator = pcl_object_iterate (value);
                        while ((next = pcl_iterator_next (iterator)) != NULL)
                        {
                                /* pcl_list_set_item() steals the reference */
                                pcl_list_set_item (self, index, next);
                                index += step;
                        }
                        pcl_object_unref (iterator);
                }
        }

        return TRUE;
}

PclObject *
pcl_list_as_tuple (PclObject *self)
{
        GPtrArray *array;
        PclObject *result;
        gpointer *pdata;
        glong ii, length;

        if (!PCL_IS_LIST (self))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        array = PCL_LIST (self)->array;
        pdata = array->pdata;
        length = (glong) array->len;
        result = pcl_tuple_new (length);

        for (ii = 0; ii < length; ii++)
                pcl_tuple_set_item (result, ii, pcl_object_ref (pdata[ii]));
        return result;
}

void
_pcl_list_init (void)
{
        list_factory.type = PCL_TYPE_LIST;
        list_factory.limit = 80;
}

void
_pcl_list_fini (void)
{
        pcl_factory_close (&list_factory);
}
