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

static gpointer base_set_parent_class = NULL;

static gboolean
base_set_update (PclBaseSet *self, PclObject *other)
{
        PclObject *iterator;
        PclObject *next;

        if (PCL_IS_BASE_SET (other))
        {
                PclBaseSet *from = PCL_BASE_SET (other);
                return pcl_mapping_merge (self->data, from->data, TRUE);
        }

        iterator = pcl_object_iterate (other);
        if (iterator == NULL)
                return FALSE;

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                if (!pcl_object_set_item (self->data, next, PCL_TRUE))
                {
                        pcl_object_unref (iterator);
                        pcl_object_unref (next);
                        return FALSE;
                }
                pcl_object_unref (next);
        }

        pcl_object_unref (iterator);
        if (pcl_error_occurred ())
                return FALSE;
        return TRUE;
}

static PclObject *
base_set_new (GType subtype, PclObject *iterable)
{
        PclBaseSet *base_set;
        PclObject *data;

        data = pcl_dict_new ();
        if (data == NULL)
                return NULL;

        base_set = g_object_new (subtype, NULL);
        base_set->data = data;

        if (iterable != NULL && !base_set_update (base_set, iterable))
        {
                pcl_object_unref (base_set);
                return NULL;
        }

        return PCL_OBJECT (base_set);
}

static PclObject *
base_set_copy (PclBaseSet *self)
{
        return base_set_new (G_OBJECT_TYPE (self), PCL_OBJECT (self));
}

static gboolean
base_set_clear (PclBaseSet *self)
{
        /* XXX Bad design here.
         *     PclBaseSet shouldn't know about PclFrozenSet.
         *     Maybe a container::clear() method would be more suitable. */
        if (!pcl_mapping_clear (self->data))
                return FALSE;
        if (PCL_IS_FROZEN_SET (self))
                PCL_FROZEN_SET (self)->hash = -1;
        return TRUE;
}

static gboolean
base_set_difference_update (PclBaseSet *self, PclObject *other)
{
        PclObject *iterator;
        PclObject *next;

        if (PCL_OBJECT (self) == other)
                return base_set_clear (self);

        iterator = pcl_object_iterate (other);
        if (iterator == NULL)
                return FALSE;

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                if (!pcl_object_del_item (self->data, next))
                {
                        if (pcl_error_exception_matches (
                                        pcl_exception_key_error ()))
                                pcl_error_clear ();
                        else
                        {
                                pcl_object_unref (iterator);
                                pcl_object_unref (next);
                                return FALSE;
                        }
                }
                pcl_object_unref (next);
        }

        pcl_object_unref (iterator);
        if (pcl_error_occurred ())
                return FALSE;
        return TRUE;
}

static PclObject *
base_set_difference (PclBaseSet *self, PclObject *other)
{
        PclObject *iterator;
        PclObject *next;
        PclObject *result;
        PclObject *source;
        PclObject *target;

        if (PCL_IS_DICT (other))
                source = other;
        else if (PCL_IS_BASE_SET (other))
                source = PCL_BASE_SET (other)->data;
        else
        {
                gboolean success;
                result = base_set_copy (self);
                if (result == NULL)
                        return NULL;
                success = base_set_difference_update (
                        PCL_BASE_SET (result), other);
                if (!success)
                {
                        pcl_object_unref (result);
                        return NULL;
                }
                return result;
        }

        result = base_set_new (G_OBJECT_TYPE (self), NULL);
        if (result == NULL)
                return NULL;
        target = PCL_BASE_SET (result)->data;

        iterator = pcl_object_iterate (self->data);
        if (iterator == NULL)
        {
                pcl_object_unref (result);
                return NULL;
        }

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                if (!pcl_object_contains (source, next))
                {
                        if (!pcl_object_set_item (target, next, PCL_TRUE))
                        {
                                pcl_object_unref (iterator);
                                pcl_object_unref (result);
                                pcl_object_unref (next);
                                return NULL;
                        }
                }
                pcl_object_unref (next);
        }

        pcl_object_unref (iterator);
        if (pcl_error_occurred ())
        {
                pcl_object_unref (result);
                return NULL;
        }
        return result;
}

static gboolean
base_set_intersection_update (PclBaseSet *self, PclObject *other)
{
        PclObject *dict;
        PclObject *iterator;
        PclObject *next;
        PclObject *temp;

        dict = pcl_dict_new ();
        if (dict == NULL)
                return FALSE;

        iterator = pcl_object_iterate (other);
        if (iterator == NULL)
        {
                pcl_object_unref (dict);
                return FALSE;
        }

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                if (pcl_object_contains (self->data, next))
                {
                        if (!pcl_object_set_item (dict, next, PCL_TRUE))
                        {
                                pcl_object_unref (iterator);
                                pcl_object_unref (next);
                                pcl_object_unref (dict);
                                return FALSE;
                        }
                }
                pcl_object_unref (next);
        }

        pcl_object_unref (iterator);
        if (pcl_error_occurred ())
        {
                pcl_object_unref (dict);
                return FALSE;
        }
        temp = self->data;
        self->data = dict;
        pcl_object_unref (temp);
        return TRUE;
}

static PclObject *
base_set_intersection (PclBaseSet *self, PclObject *other)
{
        PclObject *iterator;
        PclObject *next;
        PclObject *result;
        PclObject *source;
        PclObject *target;

        result = base_set_new (G_OBJECT_TYPE (self), NULL);
        if (result == NULL)
                return NULL;

        source = self->data;
        target = PCL_BASE_SET (result)->data;

        if (PCL_IS_BASE_SET (other))
                other = PCL_BASE_SET (other)->data;

        if (PCL_IS_DICT (other) && pcl_object_measure (other) >
                        pcl_object_measure (source))
        {
                PclObject *swap;
                swap = source;
                source = other;
                other = swap;
        }

        iterator = pcl_object_iterate (other);
        if (iterator == NULL)
        {
                pcl_object_unref (result);
                return NULL;
        }

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                if (pcl_object_contains (source, next))
                {
                        if (!pcl_object_set_item (target, next, PCL_TRUE))
                        {
                                pcl_object_unref (iterator);
                                pcl_object_unref (next);
                                pcl_object_unref (result);
                                return NULL;
                        }
                }
                pcl_object_unref (next);
        }

        pcl_object_unref (iterator);
        if (pcl_error_occurred ())
        {
                pcl_object_unref (result);
                return NULL;
        }
        return result;
}

static gint
base_set_is_subset (PclBaseSet *self, PclObject *other)
{
        PclObject *iterator;
        PclObject *next;
        gint is_subset;

        if (!PCL_IS_BASE_SET (other))
        {
                PclObject *set;
                set = pcl_set_new (other);
                if (set == NULL)
                        return -1;
                is_subset = base_set_is_subset (self, set);
                pcl_object_unref (set);
                return is_subset;
        }

        if (pcl_object_measure (self->data) > pcl_object_measure (other))
                return 0;

        iterator = pcl_object_iterate (self->data);
        if (iterator == NULL)
                return -1;

        is_subset = 1;
        while (is_subset && (next = pcl_iterator_next (iterator)) != NULL)
        {
                /* this does handle errors correctly */
                if (!pcl_object_contains (other, next))
                        is_subset = 0;
                pcl_object_unref (next);
        }

        pcl_object_unref (iterator);
        if (pcl_error_occurred ())
                return -1;
        return is_subset;
}

static gint
base_set_is_superset (PclBaseSet *self, PclObject *other)
{
        if (!PCL_IS_BASE_SET (other))
        {
                PclObject *set;
                gint is_superset;
                set = pcl_set_new (other);
                if (set == NULL)
                        return -1;
                is_superset = base_set_is_superset (self, set);
                pcl_object_unref (set);
                return is_superset;
        }

        return base_set_is_subset (PCL_BASE_SET (other), PCL_OBJECT (self));
}

static gboolean
base_set_symmetric_difference_update (PclBaseSet *self, PclObject *other)
{
        PclObject *iterator;
        PclObject *next;
        PclObject *self_data;
        PclObject *other_data;
        PclObject *other_set = NULL;

        /* special case: self == other */
        if (PCL_OBJECT (self) == other)
                return base_set_clear (self);

        self_data = self->data;
        if (PCL_IS_DICT (other))
                other_data = other;
        else if (PCL_IS_BASE_SET (other))
                other_data = PCL_BASE_SET (other)->data;
        else
        {
                other_set = base_set_new (G_OBJECT_TYPE (self), other);
                if (other_set == NULL)
                        return FALSE;
                other_data = PCL_BASE_SET (other_set)->data;
        }

        iterator = pcl_object_iterate (other_data);
        if (iterator == NULL)
                return FALSE;

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                if (pcl_object_contains (self_data, next))
                {
                        if (!pcl_object_del_item (self_data, next))
                        {
                                if (other_set != NULL)
                                        pcl_object_unref (other_set);
                                pcl_object_unref (next);
                                return FALSE;
                        }
                }
                else
                {
                        if (!pcl_object_set_item (self_data, next, PCL_TRUE))
                        {
                                if (other_set != NULL)
                                        pcl_object_unref (other_set);
                                pcl_object_unref (next);
                                return FALSE;
                        }
                }
                pcl_object_unref (next);
        }

        pcl_object_unref (iterator);
        if (other_set != NULL)
                pcl_object_unref (other_set);
        if (pcl_error_occurred ())
                return FALSE;
        return TRUE;
}

static PclObject *
base_set_symmetric_difference (PclBaseSet *self, PclObject *other)
{
        PclObject *iterator;
        PclObject *next;
        PclObject *other_data;
        PclObject *result;
        PclObject *source;
        PclObject *target;

        if (PCL_IS_DICT (other))
                other_data = other;
        else if (PCL_IS_BASE_SET (other))
                other_data = PCL_BASE_SET (other)->data;
        else
        {
                PclObject *set;
                gboolean success;
                set = base_set_new (G_OBJECT_TYPE (self), other);
                if (set == NULL)
                        return NULL;
                success = base_set_symmetric_difference_update (
                        PCL_BASE_SET (set), PCL_OBJECT (self));
                if (!success)
                {
                        pcl_object_unref (set);
                        return NULL;
                }
                return set;
        }

        result = base_set_new (G_OBJECT_TYPE (self), NULL);
        if (result == NULL)
                return NULL;

        source = self->data;
        target = PCL_BASE_SET (result)->data;

        iterator = pcl_object_iterate (other_data);
        if (iterator == NULL)
        {
                pcl_object_unref (result);
                return NULL;
        }

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                if (!pcl_object_contains (source, next))
                {
                        if (!pcl_object_set_item (target, next, PCL_TRUE))
                        {
                                pcl_object_unref (iterator);
                                pcl_object_unref (next);
                                pcl_object_unref (result);
                                return NULL;
                        }
                }
                pcl_object_unref (next);
        }

        pcl_object_unref (iterator);
        if (pcl_error_occurred ())
        {
                pcl_object_unref (result);
                return NULL;
        }

        iterator = pcl_object_iterate (source);
        if (iterator == NULL)
        {
                pcl_object_unref (result);
                return NULL;
        }

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                if (!pcl_object_contains (other_data, next))
                {
                        if (!pcl_object_set_item (target, next, PCL_TRUE))
                        {
                                pcl_object_unref (iterator);
                                pcl_object_unref (next);
                                pcl_object_unref (result);
                                return NULL;
                        }
                }
                pcl_object_unref (next);
        }

        pcl_object_unref (iterator);
        if (pcl_error_occurred ())
        {
                pcl_object_unref (result);
                return NULL;
        }
        return result;
}

static PclObject *
base_set_union (PclBaseSet *self, PclObject *other)
{
        PclObject *result;

        result = base_set_copy (self);
        if (result == NULL)
                return NULL;
        if (!base_set_update (PCL_BASE_SET (result), other))
        {
                pcl_object_unref (result);
                return NULL;
        }
        return result;
}

static void
base_set_dispose (GObject *g_object)
{
        PclBaseSet *self = PCL_BASE_SET (g_object);

        PCL_CLEAR (self->data);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (base_set_parent_class)->dispose (g_object);
}

static gboolean
base_set_print (PclObject *object, FILE *stream, PclPrintFlags flags)
{
        PclBaseSet *self = PCL_BASE_SET (object);
        PclObject *iterator;
        PclObject *next;
        gchar *comma = ", ";
        gchar *separator = "";

        iterator = pcl_object_iterate (self->data);
        if (iterator == NULL)
                return FALSE;

        fprintf (stream, "%s([", PCL_GET_TYPE_NAME (object));
        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                fputs (separator, stream);
                separator = comma;
                if (!pcl_object_print (next, stream, 0))
                {
                        pcl_object_unref (iterator);
                        pcl_object_unref (next);
                        return FALSE;
                }
                pcl_object_unref (next);
        }
        fputs ("])", stream);

        pcl_object_unref (iterator);
        if (pcl_error_occurred ())
                return FALSE;
        return TRUE;
}

static PclObject *
base_set_repr (PclObject *object)
{
        PclBaseSet *self = PCL_BASE_SET (object);
        PclObject *init;
        PclObject *keys;
        PclObject *result;

        keys = pcl_mapping_keys (self->data);
        if (keys == NULL)
                return NULL;
        init = pcl_object_repr (keys);
        pcl_object_unref (keys);
        if (init == NULL)
                return NULL;
        result = pcl_string_from_format ("%s(%s)",
                PCL_GET_TYPE_NAME (object),
                PCL_STRING_AS_STRING (init));
        pcl_object_unref (init);
        return result;
}

static PclObject *
base_set_rich_compare (PclObject *object1, PclObject *object2,
                       PclRichCompareOps op)
{
        glong length1, length2;
        gint is_it;

        if (!PCL_IS_BASE_SET (object2))
        {
                if (op == PCL_EQ)
                        return pcl_object_ref (PCL_FALSE);
                if (op == PCL_NE)
                        return pcl_object_ref (PCL_TRUE);
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "can only compare to a set");
                return NULL;
        }

        switch (op)
        {
                case PCL_EQ:
                case PCL_NE:
                        return pcl_object_rich_compare (
                                PCL_BASE_SET (object1)->data,
                                PCL_BASE_SET (object2)->data, op);
                case PCL_LT:
                        length1 = pcl_object_measure (object1);
                        length2 = pcl_object_measure (object2);
                        if (length1 >= length2)
                                return pcl_object_ref (PCL_FALSE);
                        /* fallthrough */
                case PCL_LE:
                        is_it = base_set_is_subset (
                                PCL_BASE_SET (object1), object2);
                        if (is_it < 0)
                                return NULL;
                        return pcl_bool_from_boolean ((gboolean) is_it);
                case PCL_GT:
                        length1 = pcl_object_measure (object1);
                        length2 = pcl_object_measure (object2);
                        if (length1 <= length2)
                                return pcl_object_ref (PCL_FALSE);
                        /* fallthrough */
                case PCL_GE:
                        is_it = base_set_is_superset (
                                PCL_BASE_SET (object1), object2);
                        if (is_it < 0)
                                return NULL;
                        return pcl_bool_from_boolean ((gboolean) is_it);
        }

        return pcl_object_ref (PCL_NOT_IMPLEMENTED);
}

static gint
base_set_contains (PclObject *object, PclObject *value)
{
        PclBaseSet *self = PCL_BASE_SET (object);
        return pcl_object_contains (self->data, value);
}

static PclObject *
base_set_iterate (PclObject *object)
{
        PclBaseSet *self = PCL_BASE_SET (object);
        return pcl_object_iterate (self->data);
}

static glong
base_set_measure (PclObject *object)
{
        PclBaseSet *self = PCL_BASE_SET (object);
        return pcl_object_measure (self->data);
}

static gboolean
base_set_traverse (PclContainer *container, PclTraverseFunc func,
                   gpointer user_data)
{
        PclBaseSet *self = PCL_BASE_SET (container);

        if (self->data != NULL)
                if (!func (self->data, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (base_set_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
base_set_method_add (PclBaseSet *self, PclObject *item)
{
        if (!pcl_object_set_item (self->data, item, PCL_TRUE))
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

static gchar base_set_method_add_doc[] =
"Add an element to a set.\n\
\n\
This has no effect if the element is already present.";

static PclObject *
base_set_method_clear (PclBaseSet *self)
{
        if (!base_set_clear (self))
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

static gchar base_set_method_clear_doc[] =
"Remove all elements from this set.";

static PclObject *
base_set_method_copy (PclObject *self)
{
        return pcl_object_copy (self);
}

static gchar base_set_method_copy_doc[] =
"Return a shallow copy of a set.";

static PclObject *
base_set_method_difference (PclBaseSet *self, PclObject *other)
{
        return base_set_difference (self, other);
}

static gchar base_set_method_difference_doc[] =
"Return the difference of two sets as a new set.\n\
\n\
(i.e. all elements that are in this set but not the other.)";

static PclObject *
base_set_method_difference_update (PclBaseSet *self, PclObject *other)
{
        if (!base_set_difference_update (self, other))
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

static gchar base_set_method_difference_update_doc[] =
"Update a set with the symmetric difference of itself and another.";

static PclObject *
base_set_method_discard (PclBaseSet *self, PclObject *item)
{
        /* XXX incomplete implementation? */
        if (pcl_object_del_item (self->data, item))
                return pcl_object_ref (PCL_NONE);
        if (pcl_error_exception_matches (pcl_exception_key_error ()))
        {
                pcl_error_clear ();
                return pcl_object_ref (PCL_NONE);
        }
        return NULL;
}

static gchar base_set_method_discard_doc[] =
"Remove an element from a set if it is a member.\n\
\n\
If the element is not a member, do nothing.";

static PclObject *
base_set_method_intersection (PclBaseSet *self, PclObject *other)
{
        return base_set_intersection (self, other);
}

static gchar base_set_method_intersection_doc[] =
"Return the intersection of two sets as a new set.\n\
\n\
(i.e. all elements that are in both sets.)";

static PclObject *
base_set_method_intersection_update (PclBaseSet *self, PclObject *other)
{
        if (!base_set_intersection_update (self, other))
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

static gchar base_set_method_intersection_update_doc[] =
"Update a set with the intersection of itself and another.";

static PclObject *
base_set_method_issubset (PclBaseSet *self, PclObject *other)
{
        gint is_subset;

        is_subset = base_set_is_subset (self, other);
        if (is_subset < 0)
                return NULL;
        return pcl_bool_from_boolean ((gboolean) is_subset);
}

static gchar base_set_method_issubset_doc[] =
"Report whether another set contains this set.";

static PclObject *
base_set_method_issuperset (PclBaseSet *self, PclObject *other)
{
        gint is_superset;

        is_superset = base_set_is_superset (self, other);
        if (is_superset < 0)
                return NULL;
        return pcl_bool_from_boolean ((gboolean) is_superset);
}

static gchar base_set_method_issuperset_doc[] =
"Report whether this set contains another set.";

static PclObject *
base_set_method_pop (PclBaseSet *self)
{
        PclObject *item;
        PclObject *key;

        if (pcl_object_measure (self->data) == 0)
        {
                pcl_error_set_string (
                        pcl_exception_key_error (),
                        "pop from an empty set");
                return NULL;
        }

        item = pcl_object_call_method (self->data, "popitem", NULL);
        if (item == NULL)
                return NULL;
        g_assert (PCL_IS_TUPLE (item));
        key = PCL_TUPLE_GET_ITEM (item, 0);
        pcl_object_ref (key);
        pcl_object_unref (item);
        return key;
}

static gchar base_set_method_pop_doc[] =
"Remove and return an arbitrary set element.";

static PclObject *
base_set_method_remove (PclBaseSet *self, PclObject *item)
{
        /* XXX Incomplete implementation? */
        if (!pcl_object_del_item (self->data, item))
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

static gchar base_set_method_remove_doc[] =
"Remove an element from a set; it must be a member.\n\
\n\
If the element is not a member, raise a KeyError.";

static PclObject *
base_set_method_symmetric_difference (PclBaseSet *self, PclObject *other)
{
        return base_set_symmetric_difference (self, other);
}

static gchar base_set_method_symmetric_difference_doc[] =
"Return the symmetric difference of two sets as a new set.\n\
\n\
(i.e. all elements that are in exactly one of the sets.)";

static PclObject *
base_set_method_symmetric_difference_update (PclBaseSet *self, PclObject *other)
{
        if (!base_set_symmetric_difference_update (self, other))
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

static gchar base_set_method_symmetric_difference_update_doc[] =
"Update a set with the symmetric difference of itself and another.";

static PclObject *
base_set_method_union (PclBaseSet *self, PclObject *other)
{
        return base_set_union (self, other);
}

static gchar base_set_method_union_doc[] =
"Return the union of two sets as a new set.\n\
\n\
(i.e. all elements that are in either set.)";

static PclObject *
base_set_method_update (PclBaseSet *self, PclObject *other)
{
        if (!base_set_update (self, other))
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

static gchar base_set_method_update_doc[] =
"Update a set with the union of itself and another.";

static void
base_set_class_init (PclBaseSetClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        base_set_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = base_set_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->print = base_set_print;
        object_class->repr = base_set_repr;
        object_class->rich_compare = base_set_rich_compare;
        object_class->contains = base_set_contains;
        object_class->iterate = base_set_iterate;
        object_class->measure = base_set_measure;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = base_set_dispose;
}

GType
pcl_base_set_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclBaseSetClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) base_set_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclBaseSet),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclBaseSet", &type_info,
                        G_TYPE_FLAG_ABSTRACT);
        }
        return type;
}

/*****************************************************************************/

static gchar set_doc[] =
"set(iterable) -> set object\n\
\n\
Build an unordered collection.";

static gpointer set_parent_class = NULL;

static PclObject *
set_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        PclObject *iterable = NULL;

        if (!pcl_arg_no_keywords ("set()", kwds))
                return NULL;

        if (!pcl_arg_unpack_tuple (args, "set", 0, 1, &iterable))
                return NULL;

        return pcl_set_new (iterable);
}

static PclObject *
set_copy (PclObject *object)
{
        return base_set_new (G_OBJECT_TYPE (object), object);
}

static PclObject *
set_number_subtract (PclObject *object1, PclObject *object2)
{
        if (!PCL_IS_BASE_SET (object1) || !PCL_IS_BASE_SET (object2))
                return pcl_object_ref (PCL_NOT_IMPLEMENTED);
        return base_set_difference (PCL_BASE_SET (object1), object2);
}

static PclObject *
set_number_bitwise_or (PclObject *object1, PclObject *object2)
{
        if (!PCL_IS_BASE_SET (object1) || !PCL_IS_BASE_SET (object2))
                return pcl_object_ref (PCL_NOT_IMPLEMENTED);
        return base_set_union (PCL_BASE_SET (object1), object2);
}

static PclObject *
set_number_bitwise_xor (PclObject *object1, PclObject *object2)
{
        if (!PCL_IS_BASE_SET (object1) || !PCL_IS_BASE_SET (object2))
                return pcl_object_ref (PCL_NOT_IMPLEMENTED);
        return base_set_symmetric_difference (PCL_BASE_SET (object1), object2);
}

static PclObject *
set_number_bitwise_and (PclObject *object1, PclObject *object2)
{
        if (!PCL_IS_BASE_SET (object1) || !PCL_IS_BASE_SET (object2))
                return pcl_object_ref (PCL_NOT_IMPLEMENTED);
        return base_set_intersection (PCL_BASE_SET (object1), object2);
}

static PclObject *
set_number_in_place_subtract (PclObject *object1, PclObject *object2)
{
        if (!PCL_IS_BASE_SET (object2))
                return pcl_object_ref (PCL_NOT_IMPLEMENTED);
        if (!base_set_difference_update (PCL_BASE_SET (object1), object2))
                return NULL;
        return pcl_object_ref (object1);
}

static PclObject *
set_number_in_place_or (PclObject *object1, PclObject *object2)
{
        if (!PCL_IS_BASE_SET (object2))
                return pcl_object_ref (PCL_NOT_IMPLEMENTED);
        if (!base_set_update (PCL_BASE_SET (object1), object2))
                return NULL;
        return pcl_object_ref (object1);
}

static PclObject *
set_number_in_place_xor (PclObject *object1, PclObject *object2)
{
        if (!PCL_IS_SET (object2))
                return pcl_object_ref (PCL_NOT_IMPLEMENTED);
        if (!base_set_symmetric_difference_update (
                        PCL_BASE_SET (object1), object2))
                return NULL;
        return pcl_object_ref (object1);
}

static PclObject *
set_number_in_place_and (PclObject *object1, PclObject *object2)
{
        if (!PCL_IS_SET (object2))
                return pcl_object_ref (PCL_NOT_IMPLEMENTED);
        if (!base_set_method_intersection_update (
                        PCL_BASE_SET (object1), object2))
                return NULL;
        return pcl_object_ref (object1);
}

static void
set_number_init (PclNumberIface *iface)
{
        iface->subtract = set_number_subtract;
        iface->bitwise_or = set_number_bitwise_or;
        iface->bitwise_xor = set_number_bitwise_xor;
        iface->bitwise_and = set_number_bitwise_and;
        iface->in_place_subtract = set_number_in_place_subtract;
        iface->in_place_or = set_number_in_place_or;
        iface->in_place_xor = set_number_in_place_xor;
        iface->in_place_and = set_number_in_place_and;
}

static void
set_class_init (PclSetClass *class)
{
        PclObjectClass *object_class;

        set_parent_class = g_type_class_peek_parent (class);

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_set_get_type_object;
        object_class->new_instance = set_new_instance;
        object_class->copy = set_copy;
        object_class->doc = set_doc;
}

static PclMethodDef set_methods[] = {
        { "add",                (PclCFunction) base_set_method_add,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_add_doc },
        { "clear",              (PclCFunction) base_set_method_clear,
                                PCL_METHOD_FLAG_NOARGS,
                                base_set_method_clear_doc },
        { "copy",               (PclCFunction) base_set_method_copy,
                                PCL_METHOD_FLAG_NOARGS,
                                base_set_method_copy_doc },
        { "discard",            (PclCFunction) base_set_method_discard,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_discard_doc },
        { "difference",         (PclCFunction) base_set_method_difference,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_difference_doc },
        { "difference_update",  (PclCFunction)
                                base_set_method_difference_update,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_difference_update_doc },
        { "intersection",       (PclCFunction) base_set_method_intersection,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_intersection_doc },
        { "intersection_update",
                                (PclCFunction)
                                base_set_method_intersection_update,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_intersection_update_doc },
        { "issubset",           (PclCFunction) base_set_method_issubset,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_issubset_doc },
        { "issuperset",         (PclCFunction) base_set_method_issuperset,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_issuperset_doc },
        { "pop",                (PclCFunction) base_set_method_pop,
                                PCL_METHOD_FLAG_NOARGS,
                                base_set_method_pop_doc },
        { "remove",             (PclCFunction) base_set_method_remove,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_remove_doc },
        { "symmetric_difference",
                                (PclCFunction)
                                base_set_method_symmetric_difference,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_symmetric_difference_doc },
        { "symmetric_difference_update",
                                (PclCFunction)
                                base_set_method_symmetric_difference_update,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_symmetric_difference_update_doc },
        { "union",              (PclCFunction) base_set_method_union,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_union_doc },
        { "update",             (PclCFunction) base_set_method_update,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_update_doc },
        { NULL }
};

GType
pcl_set_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclSetClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) set_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclSet),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                static const GInterfaceInfo number_info = {
                        (GInterfaceInitFunc) set_number_init,
                        (GInterfaceFinalizeFunc) NULL,
                        NULL   /* interface_data */
                };

                type = g_type_register_static (
                        PCL_TYPE_BASE_SET, "PclSet", &type_info, 0);
                g_type_add_interface_static (
                        type, PCL_TYPE_NUMBER, &number_info);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, set_methods);
        }
        return type;
}

/**
 * pcl_set_get_type_object:
 *
 * Returns the type object for #PclSet.  During runtime this is the built-in
 * object %set.
 *
 * Returns: a borrowed reference to the type object for #PclSet
 */
PclObject *
pcl_set_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_SET, "set");
                pcl_register_singleton ("<type 'set'>", &object);
        }
        return object;
}

PclObject *
pcl_set_new (PclObject *iterable)
{
        return base_set_new (PCL_TYPE_SET, iterable);
}

/*****************************************************************************/

static gchar frozen_set_doc[] =
"frozenset(iterable) -> frozenset object\n\
\n\
Build an immutable unordered collection.";

static gpointer frozen_set_parent_class = NULL;

static PclObject *
frozen_set_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        PclObject *iterable = NULL;

        if (!pcl_arg_no_keywords ("frozenset()", kwds))
                return NULL;

        if (!pcl_arg_unpack_tuple (args, "frozenset", 0, 1, &iterable))
                return NULL;

        if (PCL_IS_FROZEN_SET (iterable))
                return pcl_object_ref (iterable);

        return pcl_frozen_set_new (iterable);
}

static PclObject *
frozen_set_copy (PclObject *object)
{
        return pcl_object_ref (object);
}

static guint
frozen_set_hash (PclObject *object)
{
        PclFrozenSet *self = PCL_FROZEN_SET (object);
        PclObject *iterator;
        PclObject *next;
        guint hash = 1927868237L;
        guint tmp;

        if (self->hash != PCL_HASH_INVALID)
                return self->hash;

        iterator = pcl_object_iterate (object);
        if (iterator == NULL)
                return PCL_HASH_INVALID;

        hash *= (pcl_object_measure (object) + 1);

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                /* Work to increase the bit dispersion for closely spaced hash
                 * values.  This is important because some use cases have many
                 * combinations of a small number of elements with nearby
                 * hashes so that many distinct combinations collapse to only
                 * a handful of distinct hash values. */
                tmp = pcl_object_hash (next);
                hash ^= (tmp ^ (tmp << 16) ^ 89869747L) * 3644798167u;
                pcl_object_unref (next);
        }

        pcl_object_unref (iterator);
        if (pcl_error_occurred ())
                return PCL_HASH_INVALID;

        hash = hash * 69069L + 907133923L;
        if (hash == PCL_HASH_INVALID)
                hash = 590923713L;
        self->hash = hash;
        return hash;
}

static void
frozen_set_number_init (PclNumberIface *iface)
{
        /* binding to functions from set */
        iface->subtract = set_number_subtract;
        iface->bitwise_or = set_number_bitwise_or;
        iface->bitwise_xor = set_number_bitwise_xor;
        iface->bitwise_and = set_number_bitwise_and;
}

static void
frozen_set_class_init (PclFrozenSetClass *class)
{
        PclObjectClass *object_class;

        frozen_set_parent_class = g_type_class_peek_parent (class);

        object_class = PCL_OBJECT_CLASS (class);
        object_class->new_instance = frozen_set_new_instance;
        object_class->type = pcl_frozen_set_get_type_object;
        object_class->copy = frozen_set_copy;
        object_class->hash = frozen_set_hash;
        object_class->doc = frozen_set_doc;
}

static void
frozen_set_init (PclFrozenSet *frozen_set)
{
        frozen_set->hash = PCL_HASH_INVALID;
}

static PclMethodDef frozen_set_methods[] = {
        { "copy",               (PclCFunction) base_set_method_copy,
                                PCL_METHOD_FLAG_NOARGS,
                                base_set_method_copy_doc },
        { "difference",         (PclCFunction) base_set_method_difference,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_difference_doc },
        { "intersection",       (PclCFunction) base_set_method_intersection,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_intersection_doc },
        { "issubset",           (PclCFunction) base_set_method_issubset,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_issubset_doc },
        { "issuperset",         (PclCFunction) base_set_method_issuperset,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_issuperset_doc },
        { "symmetric_difference",
                                (PclCFunction)
                                base_set_method_symmetric_difference,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_symmetric_difference_doc },
        { "union",              (PclCFunction) base_set_method_union,
                                PCL_METHOD_FLAG_ONEARG,
                                base_set_method_union_doc },
        { NULL }
};

GType
pcl_frozen_set_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclFrozenSetClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) frozen_set_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclFrozenSet),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) frozen_set_init,
                        NULL   /* value_table */
                };

                static const GInterfaceInfo number_info = {
                        (GInterfaceInitFunc) frozen_set_number_init,
                        (GInterfaceFinalizeFunc) NULL,
                        NULL   /* interface_data */
                };

                type = g_type_register_static (
                        PCL_TYPE_BASE_SET, "PclFrozenSet", &type_info, 0);
                g_type_add_interface_static (
                        type, PCL_TYPE_NUMBER, &number_info);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, frozen_set_methods);
        }
        return type;
}

/**
 * pcl_frozen_set_get_type_object:
 *
 * Returns the type object for #PclFrozenSet.  During runtime this is the
 * built-in object %frozenset.
 *
 * Returns: a borrowed reference to the type object for #PclFrozenSet
 */
PclObject *
pcl_frozen_set_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_FROZEN_SET, "frozenset");
                pcl_register_singleton ("<type 'frozenset'>", &object);
        }
        return object;
}

PclObject *
pcl_frozen_set_new (PclObject *iterable)
{
        return base_set_new (PCL_TYPE_FROZEN_SET, iterable);
}
