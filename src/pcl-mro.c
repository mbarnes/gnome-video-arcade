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

/* Method resolution order algorithm C3 described in
 * "A Monotonic Superclass Linearization for Dylan",
 * by Kim Barrett, Bob Cassel, Paul Haahr, David A. Moon,
 * Keith Playford, and P. Tucker Withington.  (OOPSLA 1996)
 *
 * Some notes about the rules implied by C3:
 *
 * No duplicate bases.
 * It isn't legal to repeat a class in a list of base classes.
 *
 * The next three properties are the 3 constraints in "C3".
 *
 * Local precedence order.
 * If A precedes B in C's MRO, then A will precede B in the MRO of
 * all subclasses of C.
 *
 * Monotonicity.
 * The MRO of a class must be an extension without reordering of the
 * MRO of each of its superclasses.
 *
 * Extended Precedence Graph (EPG).
 * Linearization is consistent if there is a path in the EPG from
 * each class to all of its successors in the linearization.
 * See the paper for definition of EPG.
 */

static PclObject *
mro_class_name (PclObject *class)
{
        PclObject *name;

        name = pcl_object_get_attr_string (class, "__name__");
        if (name == NULL)
        {
                pcl_error_clear ();
                name = pcl_object_repr (class);
                if (name == NULL)
                        return NULL;
        }
        if (!PCL_IS_STRING (name))
        {
                pcl_object_unref (name);
                return NULL;
        }
        return name;
}

static gboolean
mro_check_duplicates (PclObject *list)
{
        PclObject *item;
        glong ii, jj, size;

        size = PCL_LIST_GET_SIZE (list);
        for (ii = 0; ii < size; ii++)
        {
                item = PCL_LIST_GET_ITEM (list, ii);
                for (jj = ii + 1; jj < size; jj++)
                {
                        if (PCL_LIST_GET_ITEM (list, jj) == item)
                        {
                                PclObject *name;
                                name = mro_class_name (item);
                                pcl_error_set_format (
                                        pcl_exception_type_error (),
                                        "duplicate base class %s",
                                        (name == NULL) ? "?" :
                                        PCL_STRING_AS_STRING (name));
                                if (name != NULL)
                                        pcl_object_unref (name);
                                return FALSE;
                        }
                }
        }
        return TRUE;
}

static gboolean
mro_tail_contains (PclObject *list, glong whence, PclObject *object)
{
        glong ii, size;

        size = PCL_LIST_GET_SIZE (list);
        for (ii = whence + 1; ii < size; ii++)
                if (PCL_LIST_GET_ITEM (list, ii) == object)
                        return TRUE;
        return FALSE;
}

static void
mro_error (PclObject *to_merge, glong *remain)
{
        PclObject *iterator;
        PclObject *next;
        PclObject *set;
        GString *buffer;
        glong ii, n_to_merge;
        gchar *emit = " ";
        gchar *separator = ", ";

        /* Raise a TypeError for an MRO order disagreement.
         *
         * It's hard to produce a good error message.  In the absence of
         * better insight into error reporting, report the classes that were
         * candidates to be put next into the MRO.  There is some conflict
         * between the order in which they should be put in the MRO, but
         * it's hard to diagnose what constraint can't be satisfied. */

        set = pcl_dict_new ();
        n_to_merge = PCL_LIST_GET_SIZE (to_merge);
        for (ii = 0; ii < n_to_merge; ii++)
        {
                PclObject *list;
                PclObject *class;
                list = PCL_LIST_GET_ITEM (to_merge, ii);
                if (remain[ii] >= PCL_LIST_GET_SIZE (list))
                        continue;
                class = PCL_LIST_GET_ITEM (list, remain[ii]);
                if (pcl_dict_set_item (set, class, PCL_NONE))
                {
                        pcl_object_unref (set);
                        return;
                }
        }

        iterator = pcl_object_iterate (set);
        if (iterator == NULL)
        {
                pcl_object_unref (set);
                return;
        }

        buffer = g_string_new ("Cannot create a consistent method resolution\n"
                               "order (MRO) for bases");

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                PclObject *name;
                name = mro_class_name (next);
                pcl_object_unref (next);
                if (name == NULL)
                        continue;
                g_string_append (buffer, emit);
                g_string_append (buffer, PCL_STRING_AS_STRING (name));
                emit = separator;
        }

        if (!pcl_error_occurred ())
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        buffer->str);

        g_string_free (buffer, TRUE);
        pcl_object_unref (iterator);
        pcl_object_unref (set);
}

static gboolean
mro_merge (PclObject *result, PclObject *to_merge)
{
        glong n_merged;
        glong n_to_merge;
        glong ii, jj;
        glong *remain;

        n_to_merge = PCL_LIST_GET_SIZE (to_merge);

        /* remain stores an index into each sublist of to_merge.
         * remain[i] is the index of the next base in to_merge[i]
         * that is not included in result. */
        remain = g_new0 (glong, n_to_merge);

again:
        n_merged = 0;
        for (ii = 0; ii < n_to_merge; ii++)
        {
                PclObject *candidate;
                PclObject *curr_list;

                curr_list = PCL_LIST_GET_ITEM (to_merge, ii);

                if (remain[ii] >= PCL_LIST_GET_SIZE (curr_list))
                {
                        n_merged++;
                        continue;
                }

                /* Choose next candidate for MRO.  The input sequences alone
                 * can determine the choice.  If not, choose the class which
                 * appears in the MRO of the earliest direct superclass of
                 * the new class. */

                candidate = PCL_LIST_GET_ITEM (curr_list, remain[ii]);
                for (jj = 0; jj < n_to_merge; jj++)
                {
                        PclObject *list;
                        list = PCL_LIST_GET_ITEM (to_merge, jj);
                        if (mro_tail_contains (list, remain[jj], candidate))
                                goto skip;  /* continue outer loop */
                }
                if (!pcl_list_append (result, candidate))
                {
                        g_free (remain);
                        return FALSE;
                }
                for (jj = 0; jj < n_to_merge; jj++)
                {
                        PclObject *list;
                        list = PCL_LIST_GET_ITEM (to_merge, jj);
                        if (remain[jj] >= PCL_LIST_GET_SIZE (list))
                                continue;
                        if (PCL_LIST_GET_ITEM (list, remain[jj]) == candidate)
                                remain[jj]++;
                }
                goto again;
skip:
                ;
        }

        if (n_merged != n_to_merge)
        {
                mro_error (to_merge, remain);
                g_free (remain);
                return FALSE;
        }

        g_free (remain);
        return TRUE;
}

PclObject *
mro_algorithm (PclType *type)
{
        PclObject *bases;
        PclObject *to_merge;
        PclObject *result;
        glong ii, n_bases;
        gboolean success;

        /* Find a superclass linearization that honors the constraints
         * of the explicit lists of bases and the constraints implied by
         * each base class.
         *
         * to_merge is a list of lists, where each list is a superclass
         * linearization implied by a base class.  The last element of
         * to_merge is the declared list of bases. */

        bases = type->bases;
        n_bases = PCL_TUPLE_GET_SIZE (type->bases);

        to_merge = pcl_list_new (n_bases + 1);
        if (to_merge == NULL)
                return NULL;

        for (ii = 0; ii < n_bases; ii++)
        {
                PclObject *base;
                PclObject *parent_mro;

                base = PCL_TUPLE_GET_ITEM (type->bases, ii);
                g_assert (PCL_IS_TYPE (base));
                parent_mro = pcl_sequence_as_list (PCL_TYPE (base)->mro);
                if (parent_mro == NULL)
                {
                        pcl_object_unref (to_merge);
                        return NULL;
                }
                PCL_LIST_SET_ITEM (to_merge, ii, parent_mro);
        }

        bases = pcl_sequence_as_list (type->bases);
        if (bases == NULL)
        {
                pcl_object_unref (to_merge);
                return NULL;
        }
        if (!mro_check_duplicates (bases))
        {
                pcl_object_unref (bases);
                pcl_object_unref (to_merge);
                return NULL;
        }
        PCL_LIST_SET_ITEM (to_merge, ii, bases);

        result = pcl_build_value ("[O]", PCL_OBJECT (type));
        if (result == NULL)
        {
                pcl_object_unref (to_merge);
                return NULL;
        }
        success = mro_merge (result, to_merge);
        pcl_object_unref (to_merge);
        if (!success)
        {
                pcl_object_unref (result);
                return NULL;
        }
        return result;
}

PclObject *
mro_lookup (PclType *type, const gchar *name)
{
        PclObject *base;
        PclObject *dict;
        PclObject *item;
        PclObject *mro;
        glong ii, size;

        mro = type->mro;
        g_assert (PCL_IS_TUPLE (mro));
        size = PCL_TUPLE_GET_SIZE (mro);
        for (ii = 0; ii < size; ii++)
        {
                base = PCL_TUPLE_GET_ITEM (mro, ii);
                dict = pcl_object_get_dict (base);
                g_assert (PCL_IS_DICT (dict));
                item = pcl_dict_get_item_string (dict, name);
                if (item != NULL)
                        return item;
        }

        return NULL;
}
