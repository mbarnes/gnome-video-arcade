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

/* See large comment block below.  This must be >= 1. */
#define DICT_PERTURB_SHIFT 5

/* Major subtleties ahead!
 *
 * Most hash schemes depend on having a "good" hash function, in the sense of
 * simulating randomness.  PCL doesn't: its most important hash functions
 * (for strings and ints) are very regular in common cases:
 *
 * >>> [hash(x) for x in (0, 1, 2, 3)]
 * [0, 1, 2, 3]
 * >>> [hash(x) for x in ("namea", "nameb", "namec", "named")]
 * [-1658398457, -1658398460, -1658398459, -1658398462]
 *
 * This isn't necessarily bad!  To the contrary, in a table of size 2**i,
 * taking the low-order i bits as the initial table index is extremely fast,
 * and there are no collisions at all for dicts indexed by a contiguous range
 * of ints.  The same is approximately true when keys are "consecutive"
 * strings.  So this gives better-than-random behavior in common cases, and
 * that's very desirable.
 *
 * OTOH, when collisions occur, the tendency to fill contiguous slices of the
 * hash table makes a good collision resolution strategy crucial.  Taking only
 * the last i bits of the hash code is also vulnerable.  For example, consider
 * [i << 16 for i in range(20000)] as a set of keys.  Since ints are their own
 * hash codes, and this fits in a dict of size 2**15, the last 15 bits of
 * every hash code are all 0.  They *all* map to the same table index.
 *
 * But catering to unusual cases should not slow the usual ones, so we just
 * take the last i bits anyway.  It's up to collision resolution to do the
 * rest.  If we *usually* find the key we're looking for on the first try
 * (and, it turns out, we usually do -- the table load factor is kept under
 * 2/3, so the odds are solidly in our favor), then it makes sense to keep
 * the initial index computation dirt cheap.
 *
 * The first half of collision resolution is to visit table indices via this
 * recurrence:
 *
 *     j = ((5*j) + 1) mod 2**i
 *
 * For any initial j in range(2**i), repeating that 2**i times generates each
 * int in range(2**i) exactly onces (see any text on random-number generation
 * for proof).  By itself, this doesn't help much.  Like linear probing
 * (setting j += 1, or j -= 1, on each loop trip), it scans the table entries
 * in a fixed order.  This would be bad, except that's not the only thing we
 * do, and it's actually good in common cases where hash keys are consecutive.
 * In an example that's really too small to make this entirely clear, for a
 * table of size 2**3 the order of indices is:
 *
 *     0 -> 1 -> 6 -> 7 -> 4 -> 5 -> 2 -> 3 -> 0 [and here it's repeating]
 *
 * If two things come in at index 5, the first place we look after is index 2,
 * not 6, so if another comes in at index 6 the collision at 5 didn't hurt it.
 * Linear probing is deadly in this case because there the fixed probe order
 * is the *same* as the order consecutive keys are likely to arrive.  But it's
 * extremely unlikely hash codes will follow 5*j+1 recurrence by accident, and
 * certain that consecutive hash codes do not.
 *
 * The other half of the strategy is to get the other bits of the hash code
 * into play.  This is done by initializing a (unsigned) variable "perturb" to
 * the full hash code, and changing the recurrence to:
 *
 *     j = (5*j) + 1 + perturb;
 *     perturb >>= DICT_PERTURB_SHIFT;
 *     use j % 2**i as the next table index;
 *
 * Now the probe sequence depends (eventually) on every bit in the hash code,
 * and the pseudo-scrambling property of recurring on 5*j+1 is more valuable,
 * because it quickly magnifies small differences in the bits that didn't
 * affect the initial index.  Note that because perturb is unsigned, if the
 * recurrence is executed often enough perturb eventually becomes and remains
 * 0.  At that point (very rarely reached) the recurrence is on (just) 5*j+1
 * again, and that's certain to find an empty slot eventually (since it
 * generates every int in range(2**i), and we make sure there's always at
 * least one empty slot).
 *
 * Selecting a good value for DICT_PERTURB_SHIFT is a balancing act.  You want
 * it small so that the high bits of the hash code continue to affect the probe
 * sequence across iterations; but you want it large so that in really bad
 * cases the high-order hash bits have an effect on early iterations. */

/* Initialization macros */
#define DICT_INIT_NONZERO_SLOTS(dict) \
        G_STMT_START { \
        (dict)->table = (dict)->small_table; \
        (dict)->mask = PCL_DICT_MIN_SIZE - 1; \
        } G_STMT_END
#define DICT_EMPTY_TO_MINSIZE(dict) \
        G_STMT_START { \
        memset ((dict)->small_table, 0, sizeof ((dict)->small_table)); \
        (dict)->used = (dict)->fill = 0; \
        DICT_INIT_NONZERO_SLOTS (dict); \
        } G_STMT_END

static gchar dict_doc[] =
"dict() -> new empty dictionary.\n\
dict(mapping) -> new dictionary initialized from a mapping object's\n\
    (key, value) pairs.\n\
dict(seq) -> new dictionary initialized as if via:\n\
    d = {}\n\
    for k, v in seq:\n\
        d[k] = v\n\
dict(**kwargs) -> new dictionary initialized with the name=value pairs\n\
    in the keyword argument list.  For example:  dict(one=1, two=2)";

/* Object used as dummy key to fill deleted entries */
static PclObject *dummy = NULL;

static PclFactory dict_factory;

static gpointer dict_parent_class = NULL;

static gboolean
dict_string_eq (PclObject *object1, PclObject *object2)
{
        /* Helper function for dict_lookup_string() */
        GString *a = PCL_STRING (object1)->string;
        GString *b = PCL_STRING (object2)->string;
        return a->len == b->len && *a->str == *b->str &&
                memcmp (a->str, b->str, a->len) == 0;
}

static PclObject *
dict_characterize (PclDict *dict1, PclDict *dict2, PclObject **p_value)
{
        /* Helper function that returns the smallest key in dict1 for which
         * dict2's value is different or absent.  The value is returned too,
         * through the p_value argument.  Both are NULL if no key in dict1
         * is found for which dict2's status differs.  The reference counts
         * on (and only on) non-NULL *p_value and function return values
         * must be decremented by the caller (this increments them to ensure
         * that mutating comparison and pcl_dict_get_item() calls can't
         * delete them before the caller is done looking at them). */

        PclObject *delta = NULL;
        PclObject *value = NULL;
        gint cmp;
        glong pos;

        for (pos = 0; pos <= dict1->mask; pos++)
        {
                PclObject *this_key;            /* dict1's key */
                PclObject *this_value;          /* dict1's value */
                PclObject *other_value;         /* dict2's value */

                if (dict1->table[pos].value == NULL)
                        continue;

                this_key = pcl_object_ref (dict1->table[pos].key);
                if (delta != NULL)
                {
                        cmp = pcl_object_rich_compare_bool (
                                delta, this_key, PCL_LT);
                        if (cmp < 0)
                        {
                                pcl_object_unref (this_key);
                                goto fail;
                        }
                        if (cmp > 0 || pos > dict1->mask ||
                                dict1->table[pos].value == NULL)
                        {
                                /* Not the *smallest* dict1 key; or maybe it
                                 * is but the compare shrunk the dict so we
                                 * can't find its associated value anymore;
                                 * or maybe it is but the compare deleted
                                 * the dict1[this_key] entry. */
                                pcl_object_unref (this_key);
                                continue;
                        }
                }

                /* compare dict1[this_key] to dict2[this_key] */
                this_value = dict1->table[pos].value;
                g_assert (this_value != NULL);
                pcl_object_ref (this_value);
                other_value = pcl_dict_get_item (PCL_OBJECT (dict2), this_key);
                if (other_value == NULL)
                        cmp = 0;
                else
                {
                        /* both dicts have this_key; same values? */
                        cmp = pcl_object_rich_compare_bool (
                                this_value, other_value, PCL_EQ);
                        if (cmp < 0)
                        {
                                pcl_object_unref (this_key);
                                pcl_object_unref (this_value);
                                goto fail;
                        }
                }
                if (cmp == 0)
                {
                        /* new winner */
                        if (delta != NULL)
                                pcl_object_unref (delta);
                        if (value != NULL)
                                pcl_object_unref (value);
                        delta = this_key;
                        value = this_value;
                }
                else
                {
                        pcl_object_unref (this_key);
                        pcl_object_unref (this_value);
                }
        }

        *p_value = value;
        return delta;

fail:
        if (delta != NULL)
                pcl_object_unref (delta);
        if (value != NULL)
                pcl_object_unref (value);
        *p_value = NULL;
        return NULL;
}

static void
dict_insert (PclDict *self, PclObject *key, guint hash, PclObject *value)
{
        /* This steals the references to key and value. */

        register PclDictEntry *entry;

        g_assert (self->lookup != NULL);
        entry = self->lookup (self, key, hash);
        if (entry->value != NULL)
        {
                PclObject *old_value;
                old_value = entry->value;
                entry->value = value;
                pcl_object_unref (old_value);
                pcl_object_unref (key);
        }
        else
        {
                if (entry->key == NULL)
                        self->fill++;
                else
                        pcl_object_unref (entry->key);
                entry->key = key;
                entry->hash = hash;
                entry->value = value;
                self->used++;
        }
}

static PclDictEntry *
dict_lookup (PclDict *self, PclObject *key, register guint hash)
{
        /* The basic lookup function used by all operations.
         *
         * This is based on Algorithm D from Knuth Vol. 3, Sec. 6.4.
         * Open addressing is preferred over chaining since the link
         * overhead for chaining would be substantial (100% with typical
         * malloc overhead).
         *
         * This initial probe index is computed as hash mod the table size.
         * Subsequent probe indices are computed as explained earlier.
         *
         * All arithmetic on hash should ignore overflow.
         *
         * This function must never return NULL.  Failures are indicated
         * by returning a PclDictEntry* for which the value field is NULL.
         * Exceptions are never reported by this function, and outstanding
         * exceptions are maintained. */

        PclDictEntry *table = self->table;
        register guint mask = self->mask;
        register PclDictEntry *entry;
        register PclDictEntry *free_slot;
        register gboolean checked_error;
        register gboolean restore_error;
        register guint perturb;
        register gint cmp;
        register gint pos;
        PclObject *start_key;

        /* for errors */
        PclObject *type;
        PclObject *value;
        PclObject *traceback;

        pos = hash & mask;
        entry = &table[pos];
        if (entry->key == NULL || entry->key == key)
                return entry;

        checked_error = FALSE;
        restore_error = FALSE;
        if (entry->key == dummy)
                free_slot = entry;
        else
        {
                if (entry->hash == hash)
                {
                        checked_error = TRUE;
                        if (pcl_error_occurred ())
                        {
                                restore_error = TRUE;
                                pcl_error_fetch (&type, &value, &traceback);
                        }
                        start_key = entry->key;
                        cmp = pcl_object_rich_compare_bool (
                                start_key, key, PCL_EQ);
                        if (cmp < 0)
                                pcl_error_clear ();
                        if (table == self->table && entry->key == start_key)
                        {
                                if (cmp > 0)
                                        goto done;
                        }
                        else
                        {
                                /* The compare did major nasty stuff to the
                                 * dict.  Start over */
                                entry = dict_lookup (self, key, hash);
                                goto done;
                        }
                }
                free_slot = NULL;
        }

        /* In the loop, getting a dummy key field is by far (factor of 100s)
         * the least likely outcome, so test for that last. */
        for (perturb = hash; ; perturb >>= DICT_PERTURB_SHIFT)
        {
                pos = (pos << 2) + pos + perturb + 1;
                entry = &table[pos & mask];
                if (entry->key == NULL)
                {
                        if (free_slot != NULL)
                                entry = free_slot;
                        break;
                }
                if (entry->key == key)
                        break;
                if (entry->hash == hash && entry->key != dummy)
                {
                        if (!checked_error)
                        {
                                checked_error = TRUE;
                                if (pcl_error_occurred ())
                                {
                                        restore_error = TRUE;
                                        pcl_error_fetch (
                                                &type, &value, &traceback);
                                }
                        }
                        start_key = entry->key;
                        cmp = pcl_object_rich_compare_bool (
                                start_key, key, PCL_EQ);
                        if (cmp < 0)
                                pcl_error_clear ();
                        if (table == self->table && entry->key == start_key)
                        {
                                if (cmp > 0)
                                        break;
                        }
                        else
                        {
                                /* The compare did major nasty stuff to the
                                 * dict.  Start over. */
                                entry = dict_lookup (self, key, hash);
                                break;
                        }
                }
                else if (entry->key == dummy && free_slot == NULL)
                        free_slot = entry;
        }

done:
        if (restore_error)
                pcl_error_restore (type, value, traceback);
        return entry;
}

static PclDictEntry *
dict_lookup_string (PclDict *self, PclObject *key, register guint hash)
{
        /* Hacked up version of dict_lookup() which can assume keys are
         * always strings.  This assumption allows testing for errors
         * during object comparisons to be dropped; string-string
         * comparisons never raise exceptions.  This also means we don't
         * need to go through the rich comparison API; we can always use
         * dict_string_eq() directly.
         *
         * This is valuable because the general-case error handling in
         * dict_lookup() is expensive, and dicts with pure-string keys
         * are very common. */

        PclDictEntry *table = self->table;
        register guint mask = self->mask;
        register PclDictEntry *entry;
        register PclDictEntry *free_slot;
        register guint perturb;
        register gint pos;

        /* Make sure this function doesn't have to handle non-string keys,
         * including subclasses of str; e.g., one reason to subclass strings
         * is to override __eq__, and for speed we don't cater to that here. */
        if (G_OBJECT_TYPE (key) != PCL_TYPE_STRING)
        {
                self->lookup = dict_lookup;
                return dict_lookup (self, key, hash);
        }

        pos = hash & mask;
        entry = &table[pos];
        if (entry->key == NULL || entry->key == key)
                return entry;
        if (entry->key == dummy)
                free_slot = entry;
        else
        {
                if (entry->hash == hash && dict_string_eq (entry->key, key))
                        return entry;
                free_slot = NULL;
        }

        /* In the loop, getting a dummy key field is by far (factor of 100s)
         * the least likely outcome, so test for that last. */
        for (perturb = hash; ; perturb >>= DICT_PERTURB_SHIFT)
        {
                pos = (pos << 2) + pos + perturb + 1;
                entry = &table[pos & mask];
                if (entry->key == NULL)
                        return free_slot == NULL ? entry : free_slot;
                if (entry->key == key ||
                        (entry->hash == hash && entry->key != dummy &&
                        dict_string_eq (entry->key, key)))
                {
                        return entry;
                }
                if (entry->key == dummy && free_slot == NULL)
                        free_slot = entry;
        }
}

static gboolean
dict_resize (PclDict *self, glong min_used)
{
        /* Restructure the table by allocating a new table and reinserting all
         * items again.  When entries have been deleted, the new table may
         * actually be smaller than the old one. */

        PclDictEntry small_copy[PCL_DICT_MIN_SIZE];
        PclDictEntry *new_table;
        PclDictEntry *old_table;
        PclDictEntry *entry;
        gboolean old_table_is_large;
        glong new_size;
        glong pos;

        g_assert (min_used >= 0);

        /* Find the smallest table size > min_used. */
        for (new_size = PCL_DICT_MIN_SIZE;
                new_size <= min_used && new_size > 0;
                new_size <<= 1);
        g_assert (new_size > 0);  /* XXX */

        /* Get space for a new table. */
        old_table = self->table;
        g_assert (old_table != NULL);
        old_table_is_large = old_table != self->small_table;

        if (new_size > PCL_DICT_MIN_SIZE)
                new_table = g_new0 (PclDictEntry, new_size);
        else
        {
                /* A large table is shrinking, or we can't get any smaller. */
                new_table = self->small_table;
                if (new_table == old_table)
                {
                        if (self->fill == self->used)
                        {
                                /* No dummies, so no point doing anything. */
                                return TRUE;
                        }
                        /* We're not going to resize it, but rebuild the table
                         * anyway to purge old dummy entries.
                         *
                         * Subtle: This is *necessary* if fill == size, as
                         * dict_lookup() needs at least one virgin slot to
                         * terminate failing searches.  If fill < size, it's
                         * merely desirable, as dummies slow searches. */
                        g_assert (self->fill > self->used);
                        memcpy (small_copy, old_table, sizeof (small_copy));
                        old_table = small_copy;
                }
        }

        /* Make the dict empty, using the new table. */
        g_assert (new_table != old_table);
        self->table = new_table;
        self->mask = new_size - 1;
        memset (new_table, 0, sizeof (PclDictEntry) * new_size);
        self->used = 0;
        pos = self->fill;
        self->fill = 0;

        /* Copy the data over.  This is reference-count-neutral for active
         * entries (dummy entries aren't copied over, of course). */
        for (entry = old_table; pos > 0; entry++)
        {
                if (entry->value != NULL)       /* active entry */
                {
                        pos--;
                        dict_insert (self,
                                entry->key,
                                entry->hash,
                                entry->value);
                }
                else if (entry->key != NULL)    /* dummy entry */
                {
                        pos--;
                        g_assert (entry->key == dummy);
                        pcl_object_unref (entry->key);
                }
        }

        if (old_table_is_large)
                g_free (old_table);
        return TRUE;
}

static void
dict_dispose (GObject *g_object)
{
        PclDict *self = PCL_DICT (g_object);

        pcl_mapping_clear (PCL_OBJECT (g_object));
        self->lookup = dict_lookup_string;

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (dict_parent_class)->dispose (g_object);
}

static PclObject *
dict_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        PclObject *dict;
        PclObject *from = NULL;

        if (!pcl_arg_unpack_tuple (args, "dict", 0, 1, &from))
                return NULL;

        dict = pcl_dict_new ();
        if (dict == NULL)
                return NULL;

        if (from != NULL && !pcl_mapping_merge (dict, from, TRUE))
                goto fail;
        if (kwds != NULL && !pcl_mapping_merge (dict, kwds, TRUE))
                goto fail;

        return dict;

fail:
        pcl_object_unref (dict);
        return NULL;
}

static PclObject *
dict_clone (PclObject *object)
{
        PclThreadState *ts = pcl_thread_state_get ();
        PclObject *clone;
        PclObject *cloned_key = NULL;
        PclObject *cloned_value = NULL;
        PclObject *key;
        PclObject *value;
        glong pos = 0;

        clone = pcl_dict_new ();
        if (clone == NULL)
                return NULL;

        g_hash_table_insert (ts->cache,
                             pcl_object_ref (object),
                             pcl_object_ref (clone));

        while (pcl_dict_next (object, &pos, &key, &value))
        {
                cloned_key = pcl_object_clone (key);
                if (cloned_key == NULL)
                        goto fail;
                cloned_value = pcl_object_clone (value);
                if (cloned_value == NULL)
                        goto fail;
                if (!pcl_dict_set_item (clone, cloned_key, cloned_value))
                        goto fail;
                pcl_object_unref (cloned_value);
                pcl_object_unref (cloned_key);
        }
        return clone;

fail:
        if (cloned_key != NULL)
                pcl_object_unref (cloned_key);
        if (cloned_value != NULL)
                pcl_object_unref (cloned_value);
        pcl_object_unref (clone);
        return NULL;
}

static gboolean
dict_print (PclObject *object, FILE *stream, PclPrintFlags flags)
{
        PclDict *self = PCL_DICT (object);
        gchar *emit = "";
        gchar *separator = ", ";
        gint been_here;
        glong pos;

        been_here = pcl_repr_enter (object);
        if (been_here < 0)
                return FALSE;
        if (been_here > 0)
        {
                fputs ("{...}", stream);
                return TRUE;
        }

        fputs ("{", stream);
        for (pos = 0; pos <= self->mask; pos++)
        {
                PclDictEntry *entry = &self->table[pos];
                PclObject *value = entry->value;
                if (value != NULL)
                {
                        pcl_object_ref (value);
                        fputs (emit, stream);
                        emit = separator;
                        if (!pcl_object_print (entry->key, stream, 0))
                        {
                                pcl_object_unref (value);
                                pcl_repr_leave (object);
                                return FALSE;
                        }
                        fputs (": ", stream);
                        if (!pcl_object_print (value, stream, 0))
                        {
                                pcl_object_unref (value);
                                pcl_repr_leave (object);
                                return FALSE;
                        }
                        pcl_object_unref (value);
                }
        }
        fputs ("}", stream);

        pcl_repr_leave (object);
        return TRUE;
}

static PclObject *
dict_copy (PclObject *object)
{
        PclObject *copy;

        copy = pcl_dict_new ();
        if (copy == NULL)
                return NULL;
        if (pcl_mapping_merge (copy, object, TRUE))
                return copy;
        pcl_object_unref (copy);
        return NULL;
}

static PclObject *
dict_repr (PclObject *object)
{
        PclDict *dict = PCL_DICT (object);
        PclObject *colon = NULL;
        PclObject *key;
        PclObject *pieces = NULL;
        PclObject *result = NULL;
        PclObject *string;
        PclObject *temp;
        PclObject *value;
        gint been_here;
        glong pos = 0;

        been_here = pcl_repr_enter (object);
        if (been_here < 0)
                return NULL;
        if (been_here > 0)
                return pcl_string_from_string ("{...}");

        if (dict->used == 0)
        {
                result = pcl_string_from_string ("{}");
                goto done;
        }

        pieces = pcl_list_new (0);
        if (pieces == NULL)
                goto done;

        colon = pcl_string_from_string (": ");
        if (colon == NULL)
                goto done;

        /* Do repr() on each (key, value) pair, and insert ": " between them.
         * Note that repr() may mutate the dict. */
        while (pcl_dict_next (object, &pos, &key, &value))
        {
                gboolean success;

                pcl_object_ref (value);
                string = pcl_object_repr (key);
                pcl_string_concat (&string, colon);
                pcl_string_concat_and_del (&string, pcl_object_repr (value));
                pcl_object_unref (value);
                if (string == NULL)
                        goto done;
                success = pcl_list_append (pieces, string);
                pcl_object_unref (string);
                if (!success)
                        goto done;
        }

        /* Add "{}" decorations to the first and last items. */
        g_assert (PCL_LIST_GET_SIZE (pieces) > 0);
        string = pcl_string_from_string ("{");
        if (string == NULL)
                goto done;
        temp = PCL_LIST_GET_ITEM (pieces, 0);
        pcl_string_concat_and_del (&string, temp);
        PCL_LIST_SET_ITEM (pieces, 0, string);
        if (string == NULL)
                goto done;
        string = pcl_string_from_string ("}");
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
        if (colon != NULL)
                pcl_object_unref (colon);
        pcl_repr_leave (object);
        return result;
}

static gint
dict_compare (PclObject *object1, PclObject *object2)
{
        PclDict *dict1, *dict2;
        PclObject *delta1, *delta2;
        PclObject *value1, *value2;
        gint cmp;

        if (!PCL_IS_DICT (object1) || !PCL_IS_DICT (object2))
                return -2;

        dict1 = PCL_DICT (object1);
        dict2 = PCL_DICT (object2);

        /* compare lengths first */
        if (dict1->used < dict2->used)
                return -1;
        if (dict1->used > dict2->used)
                return 1;

        /* same length; check all keys */
        delta2 = value2 = NULL;
        delta1 = dict_characterize (dict1, dict2, &value1);
        if (delta1 == NULL)
        {
                g_assert (value1 == NULL);
                /* either an error, or dict1 is a subset with the same length
                 * so they must be equal */
                cmp = pcl_error_occurred () ? -1 : 0;
                goto done;
        }
        delta2 = dict_characterize (dict2, dict1, &value2);
        if (delta2 == NULL && pcl_error_occurred ())
        {
                g_assert (value2 == NULL);
                cmp = -1;
                goto done;
        }
        cmp = 0;
        if (delta2 != NULL)
        {
                /* delta2 == NULL "should be" impossible now, but perhaps
                 * the last comparison done by dict_characterize() on dict1
                 * had the side effect of making the dicts equal! */
                cmp = pcl_object_compare (delta1, delta2);
        }
        if (cmp == 0 && value2 != NULL)
                cmp = pcl_object_compare (value1, value2);

done:
        if (delta1 != NULL)
                pcl_object_unref (delta1);
        if (delta2 != NULL)
                pcl_object_unref (delta2);
        if (value1 != NULL)
                pcl_object_unref (value1);
        if (value2 != NULL)
                pcl_object_unref (value2);
        return cmp;
}

static PclObject *
dict_rich_compare (PclObject *object1, PclObject *object2,
                   PclRichCompareOps op)
{
        PclDict *dict1;
        PclDict *dict2;
        gboolean is_equal;

        if (!PCL_IS_DICT (object1) || !PCL_IS_DICT (object2))
                return pcl_object_ref (PCL_NOT_IMPLEMENTED);
        if (op != PCL_EQ || op != PCL_NE)
                return pcl_object_ref (PCL_NOT_IMPLEMENTED);

        dict1 = PCL_DICT (object1);
        dict2 = PCL_DICT (object2);

        if (dict1->used != dict2->used)
                is_equal = FALSE;
        else
        {
                glong pos;

                is_equal = TRUE;
                for (pos = 0; pos <= dict1->mask; pos++)
                {
                        PclObject *key;
                        PclObject *value1;
                        PclObject *value2;
                        gint cmp;

                        value1 = dict1->table[pos].value;
                        if (value1 == NULL)
                                continue;
                        pcl_object_ref (value1);
                        key = dict1->table[pos].key;
                        value2 = pcl_dict_get_item (object2, key);
                        if (value2 == NULL)
                        {
                                pcl_object_unref (value1);
                                is_equal = FALSE;
                                break;
                        }
                        cmp = pcl_object_rich_compare_bool (
                                value1, value2, PCL_EQ);
                        pcl_object_unref (value1);
                        if (cmp < 0)
                                return NULL;
                        if (cmp == 0)
                        {
                                is_equal = FALSE;
                                break;
                        }
                }
        }

        return pcl_bool_from_boolean ((op == PCL_EQ) == is_equal);
}

static gint
dict_contains (PclObject *object, PclObject *value)
{
        PclDict *self = PCL_DICT (object);
        guint hash;

        if (G_OBJECT_TYPE (value) != PCL_TYPE_STRING)
                hash = pcl_object_hash (value);
        else if ((hash = PCL_STRING (value)->hash) == PCL_HASH_INVALID)
                hash = pcl_object_hash (value);
        if (hash == PCL_HASH_INVALID)
                return -1;
        return (self->lookup (self, value, hash)->value != NULL);
}

static PclObject *
dict_iterate (PclObject *object)
{
        return pcl_dict_key_iterator_new (object);
}

static glong
dict_measure (PclObject *object)
{
        return PCL_DICT (object)->used;
}

static PclObject *
dict_get_item (PclObject *object, PclObject *subscript)
{
        PclDict *self = PCL_DICT (object);
        PclObject *result;
        guint hash;

        g_assert (self->table != NULL);

        if (G_OBJECT_TYPE (subscript) != PCL_TYPE_STRING)
                hash = pcl_object_hash (subscript);
        else if ((hash = PCL_STRING (subscript)->hash) == PCL_HASH_INVALID)
                hash = pcl_object_hash (subscript);
        if (hash == PCL_HASH_INVALID)
                return NULL;

        result = self->lookup (self, subscript, hash)->value;
        if (result == NULL)
                pcl_error_set_object (pcl_exception_key_error (), subscript);
        else
                pcl_object_ref (result);
        return result;
}

static gboolean
dict_set_item (PclObject *object, PclObject *subscript, PclObject *value)
{
        if (value == NULL)
                return pcl_dict_del_item (object, subscript);
        else
                return pcl_dict_set_item (object, subscript, value);
}

static gboolean
dict_traverse (PclContainer *container, PclTraverseFunc func,
               gpointer user_data)
{
        PclObject *dict;
        PclObject *key;
        PclObject *value;
        glong pos = 0;

        dict = PCL_OBJECT (container);
        while (pcl_dict_next (dict, &pos, &key, &value))
        {
                if (!func (key, user_data))
                        return FALSE;
                if (!func (value, user_data))
                        return FALSE;
        }

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (dict_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
dict_mapping_keys (PclDict *self)
{
        return pcl_dict_key_iterator_new (PCL_OBJECT (self));
}

static PclObject *
dict_mapping_values (PclDict *self)
{
        return pcl_dict_value_iterator_new (PCL_OBJECT (self));
}

static PclObject *
dict_mapping_items (PclDict *self)
{
        return pcl_dict_item_iterator_new (PCL_OBJECT (self));
}

static gboolean
dict_mapping_clear (PclDict *self)
{
        PclDictEntry small_copy[PCL_DICT_MIN_SIZE];
        PclDictEntry *entry;
        PclDictEntry *table;
        gboolean table_is_large;
        glong fill;

        table = self->table;
        g_assert (table != NULL);
        table_is_large = table != self->small_table;

        /* This is delicate.  During the process of clearing the dict,
         * unrefs can cause the dict to mutate.  To avoid fatal confusion,
         * we have to make the dict empty before clearing the slots, and
         * never refer to anything via dict->xxx while clearing. */
        fill = self->fill;
        if (table_is_large)
                DICT_EMPTY_TO_MINSIZE (self);
        else if (fill > 0)
        {
                /* It's a small table with something that needs to be cleared.
                 * The only safe way is to copy the dict entries into another
                 * small table first. */
                memcpy (small_copy, table, sizeof (small_copy));
                table = small_copy;
                DICT_EMPTY_TO_MINSIZE (self);
        }

        /* Now we can finally clear things.  If C had reference counts, we
         * could assert that the reference count is 1 now, i.e. that this
         * function has unique access to it, so unref side-effects can't
         * alter it. */
        for (entry = table; fill > 0; entry++)
        {
                if (entry->key != NULL)
                {
                        fill--;
                        pcl_object_unref (entry->key);
                        if (entry->value != NULL)
                                pcl_object_unref (entry->value);
                }
        }

        if (table_is_large)
                g_free (table);
        return TRUE;
}

static PclObject *
dict_mapping_pop_item (PclDict *self)
{
        PclDictEntry *entry;
        PclObject *result;
        glong ii = 0;

        /* Allocate the result tuple before checking the size.  Believe it
         * or not, this allocation could trigger a garbage collection which
         * could empty the dict, so if we checked the size first and that
         * happened, the result would be an infinite loop (searching for an
         * entry that no longer exists).  Note that the usual popitem()
         * idiom is "while d: k, v = d.popitem()", so needing to throw the
         * tuple away if the dict *is* empty isn't a significant
         * inefficiency -- possible, but unlikely in practice. */
        result = pcl_tuple_new (2);
        if (result == NULL)
                return NULL;
        if (self->used == 0)
        {
                pcl_object_unref (result);
                pcl_error_set_string (
                        pcl_exception_key_error (),
                        "popitem(): dictionary is empty");
                return NULL;
        }

        /* Set entry to "the first" dictionary entry with a value.  We abuse
         * the hash field of slot 0 to hold a search finger:
         *
         *     If slot 0 has a value, use slot 0.
         *     Else slot 0 is being used to hold a search finger,
         *     and we use its hash value as the first index to look.
         */
        entry = &self->table[0];
        if (entry->value == NULL)
        {
                /* The hash field may be a real hash value, or it may be a
                 * legit search finger, or it may be a once-legit search finger
                 * that's out of bounds now because it wrapped around or the
                 * table shrunk -- simply make sure it's in bounds now. */
                ii = entry->hash;
                if (ii > self->mask || ii < 1)
                        ii = 1;  /* skip slot 0 */
                while ((entry = &self->table[ii])->value == NULL)
                {
                        ii++;
                        if (ii > self->mask)
                                ii = 1;
                }
        }
        PCL_TUPLE_SET_ITEM (result, 0, entry->key);
        PCL_TUPLE_SET_ITEM (result, 1, entry->value);
        entry->key = pcl_object_ref (dummy);
        entry->value = NULL;
        self->used--;
        g_assert (self->table[0].value == NULL);
        self->table[0].hash = ii + 1;  /* next place to start */
        return result;
}

static void
dict_mapping_init (PclMappingIface *iface)
{
        iface->keys = (PclMappingKeys) dict_mapping_keys;
        iface->values = (PclMappingValues) dict_mapping_values;
        iface->items = (PclMappingItems) dict_mapping_items;
        iface->clear = (PclMappingClear) dict_mapping_clear;
        iface->pop_item = (PclMappingPopItem) dict_mapping_pop_item;
}

static void
dict_class_init (PclDictClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        dict_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = dict_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_dict_get_type_object;
        object_class->new_instance = dict_new_instance;
        object_class->clone = dict_clone;
        object_class->print = dict_print;
        object_class->copy = dict_copy;
        object_class->repr = dict_repr;
        object_class->compare = dict_compare;
        object_class->rich_compare = dict_rich_compare;
        object_class->contains = dict_contains;
        object_class->iterate = dict_iterate;
        object_class->measure = dict_measure;
        object_class->get_item = dict_get_item;
        object_class->set_item = dict_set_item;
        object_class->doc = dict_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = dict_dispose;
}

void
dict_init (PclDict *dict)
{
        DICT_INIT_NONZERO_SLOTS (dict);
        dict->lookup = dict_lookup_string;
}

GType
pcl_dict_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclDictClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) dict_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclDict),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) dict_init,
                        NULL   /* value_table */
                };

                static const GInterfaceInfo mapping_info = {
                        (GInterfaceInitFunc) dict_mapping_init,
                        (GInterfaceFinalizeFunc) NULL,
                        NULL   /* interface_data */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclDict", &type_info, 0);
                g_type_add_interface_static (
                        type, PCL_TYPE_MAPPING, &mapping_info);
        }
        return type;
}

/**
 * pcl_dict_get_type_object:
 *
 * Returns the type object for #PclDict.  During runtime this is the built-in
 * object %dict.
 *
 * Returns: a borrowed reference to the type object for #PclDict
 */
PclObject *
pcl_dict_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_DICT, "dict");
                pcl_register_singleton ("<type 'dict'>", &object);
        }
        return object;
}

PclObject *
pcl_dict_new (void)
{
        /* auto-initialize dummy */
        if (G_UNLIKELY (dummy == NULL))
        {
                dummy = pcl_string_from_string ("<dummy key>");
                if (dummy == NULL)
                        return NULL;
        }

        return pcl_factory_order (&dict_factory);
}

gint
pcl_dict_has_key (PclObject *object, PclObject *key)
{
        if (!PCL_IS_DICT (object) || key == NULL)
        {
                pcl_error_bad_internal_call ();
                return -1;
        }

        return pcl_object_contains (object, key);
}

gint
pcl_dict_has_key_string (PclObject *object, const gchar *key)
{
        PclObject *string;
        gint result;

        string = pcl_string_from_string (key);
        if (string == NULL)
                return -1;
        result = pcl_dict_has_key (object, string);
        pcl_object_unref (string);
        return result;
}

PclObject *
pcl_dict_get_item (PclObject *object, PclObject *key)
{
        PclDict *self;
        guint hash;

        if (!PCL_IS_DICT (object))
                return NULL;

        if (G_OBJECT_TYPE (key) != PCL_TYPE_STRING)
                hash = pcl_object_hash (key);
        else if ((hash = PCL_STRING (key)->hash) == PCL_HASH_INVALID)
                hash = pcl_object_hash (key);
        if (hash == PCL_HASH_INVALID)
        {
                pcl_error_clear ();
                return NULL;
        }

        self = PCL_DICT (object);
        return self->lookup (self, key, hash)->value;
}

PclObject *
pcl_dict_get_item_string (PclObject *object, const gchar *key)
{
        PclObject *string;
        PclObject *result;

        string = pcl_string_from_string (key);
        if (string == NULL)
                return NULL;
        result = pcl_dict_get_item (object, string);
        pcl_object_unref (string);
        return result;
}

gboolean
pcl_dict_set_item (PclObject *object, PclObject *key, PclObject *value)
{
        /* !!! CAUTION !!!
         * pcl_dict_set_item() must guarantee that it won't resize the
         * dictionary if it is merely replacing the value for an existing
         * key.  This means that it's safe to loop over a dictionary with
         * pcl_dict_next() and occasionally replace a value, but you can't
         * insert new keys or remove them. */

        register PclDict *self;
        register glong was_used;
        register guint hash;

        if (!PCL_IS_DICT (object))
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        if (G_OBJECT_TYPE (key) != PCL_TYPE_STRING)
                hash = pcl_object_hash (key);
        else if ((hash = PCL_STRING (key)->hash) == PCL_HASH_INVALID)
                hash = pcl_object_hash (key);
        if (hash == PCL_HASH_INVALID)
                return FALSE;

        self = PCL_DICT (object);
        g_assert (self->fill <= self->mask);
        was_used = self->used;
        pcl_object_ref (value);
        pcl_object_ref (key);
        dict_insert (self, key, hash, value);

        /* If we added a key, we can safely resize.  Otherwise just return!
         * If fill >= 2/3 size, adjust size.  Normally, this doubles or
         * quadruples the size, but it's also possible for the dict to shrink
         * (if fill is much larger than used, meaning a lot of dict keys have
         * been deleted).
         *
         * Quadrupling the size improves the average dictionary sparseness
         * (reducing collisions) at the cost of some memory and iteration
         * speed (which loops over every possible entry).  It also halves the
         * number of expensive resize operations in a growing dictionary.
         *
         * Very large dictionaries (over 50K items) use doubling instead.
         * This may help applications with severe memory constraints. */
        if (!(self->used > was_used && self->fill * 3 >= (self->mask + 1) * 2))
                return TRUE;
        return dict_resize (self, self->used * (self->used > 50000 ? 2 : 4));
}

gboolean
pcl_dict_set_item_string (PclObject *object, const gchar *key,
                          PclObject *value)
{
        PclObject *string;
        gboolean result;

        string = pcl_string_from_string (key);
        if (string == NULL)
                return FALSE;
        result = pcl_dict_set_item (object, string, value);
        pcl_object_unref (string);
        return result;
}

gboolean
pcl_dict_del_item (PclObject *object, PclObject *key)
{
        register PclDict *self;
        register PclDictEntry *entry;
        register guint hash;
        PclObject *old_key;
        PclObject *old_value;

        if (!PCL_IS_DICT (object))
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        if (G_OBJECT_TYPE (key) != PCL_TYPE_STRING)
                hash = pcl_object_hash (key);
        else if ((hash = PCL_STRING (key)->hash) == PCL_HASH_INVALID)
                hash = pcl_object_hash (key);
        if (hash == PCL_HASH_INVALID)
                return FALSE;

        self = PCL_DICT (object);
        entry = self->lookup (self, key, hash);
        if (entry->value == NULL)
        {
                pcl_error_set_object (pcl_exception_key_error (), key);
                return FALSE;
        }
        old_key = entry->key;
        entry->key = pcl_object_ref (dummy);
        old_value = entry->value;
        entry->value = NULL;
        self->used--;
        pcl_object_unref (old_value);
        pcl_object_unref (old_key);
        return TRUE;
}

gboolean
pcl_dict_del_item_string (PclObject *object, const gchar *key)
{
        PclObject *string;
        gboolean result;

        string = pcl_string_from_string (key);
        if (string == NULL)
                return FALSE;
        result = pcl_dict_del_item (object, string);
        pcl_object_unref (string);
        return result;
}

gboolean
pcl_dict_next (PclObject *object, glong *p_pos, PclObject **p_key,
               PclObject **p_value)
{
        register PclDictEntry *table;
        register glong pos, mask;

        if (!PCL_IS_DICT (object))
                return FALSE;
        pos = *p_pos;
        if (pos < 0)
                return FALSE;
        table = PCL_DICT (object)->table;
        mask = PCL_DICT (object)->mask;
        while (pos <= mask && table[pos].value == NULL)
                pos++;
        *p_pos = pos + 1;
        if (pos > mask)
                return FALSE;
        if (p_key != NULL)
                *p_key = table[pos].key;
        if (p_value != NULL)
                *p_value = table[pos].value;
        return TRUE;
}

void
_pcl_dict_init (void)
{
        dict_factory.type = PCL_TYPE_DICT;
        dict_factory.limit = 80;
}

void
_pcl_dict_fini (void)
{
        pcl_factory_close (&dict_factory);
        PCL_CLEAR (dummy);  /* clear the dummy key */
}

/*****************************************************************************/

static gpointer dict_iterator_parent_class = NULL;

static PclObject *
dict_iterator_new (GType g_type, PclObject *dict)
{
        PclDictIterator *dict_iterator;

        if (!PCL_IS_DICT (dict))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        dict_iterator = pcl_object_new (g_type, NULL);
        dict_iterator->dict = pcl_object_ref (dict);
        dict_iterator->length = PCL_DICT (dict)->used;
        dict_iterator->used = PCL_DICT (dict)->used;
        return PCL_OBJECT (dict_iterator);
}

static void
dict_iterator_dispose (GObject *g_object)
{
        PclDictIterator *self = PCL_DICT_ITERATOR (g_object);

        PCL_CLEAR (self->dict);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (dict_iterator_parent_class)->dispose (g_object);
}

static glong
dict_iterator_measure (PclObject *object)
{
        PclDictIterator *self = PCL_DICT_ITERATOR (object);

        if (self->dict != NULL && self->used == self->dict->used)
                return self->length;
        return 0;
}

static gboolean
dict_iterator_traverse (PclContainer *container, PclTraverseFunc func,
                        gpointer user_data)
{
        PclDictIterator *self = PCL_DICT_ITERATOR (container);

        if (self->dict != NULL)
                if (!func (self->dict, user_data))
                        return FALSE;
        return TRUE;
}

static gboolean
dict_iterator_stop (PclIterator *iterator)
{
        PclDictIterator *self = PCL_DICT_ITERATOR (iterator);

        if (self->dict != NULL)
        {
                pcl_object_unref (self->dict);
                self->dict = NULL;
        }
        return TRUE;
}

static void
dict_iterator_class_init (PclDictIteratorClass *class)
{
        PclIteratorClass *iterator_class;
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        dict_iterator_parent_class = g_type_class_peek_parent (class);

        iterator_class = PCL_ITERATOR_CLASS (class);
        iterator_class->stop = dict_iterator_stop;

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = dict_iterator_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->measure = dict_iterator_measure;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = dict_iterator_dispose;
}

GType
pcl_dict_iterator_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclDictIteratorClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) dict_iterator_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclDictIterator),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_ITERATOR, "PclDictIterator", &type_info,
                        G_TYPE_FLAG_ABSTRACT);
        }
        return type;
}

/*****************************************************************************/

static gpointer dict_key_iterator_parent_class = NULL;

static PclObject *
dict_key_iterator_next (PclIterator *iterator)
{
        PclDictIterator *dict_iterator = PCL_DICT_ITERATOR (iterator);
        PclDict *dict = dict_iterator->dict;
        register PclDictEntry *table;
        register glong mask;
        register glong pos;

        if (dict == NULL)
                return NULL;
        if (dict_iterator->used != dict->used)
        {
                pcl_error_set_string (
                        pcl_exception_runtime_error (),
                        "dictionary changed size during iteration");
                dict_iterator->used = -1;  /* make this state sticky */
                return NULL;
        }

        pos = dict_iterator->pos;
        if (pos < 0)
                goto fail;
        table = dict->table;
        mask = dict->mask;
        while (pos <= mask && table[pos].value == NULL)
                pos++;
        dict_iterator->pos = pos + 1;
        if (pos > mask)
                goto fail;
        dict_iterator->length--;
        return pcl_object_ref (table[pos].key);

fail:
        pcl_object_unref (dict);
        dict_iterator->dict = NULL;
        return NULL;
}

static void
dict_key_iterator_class_init (PclDictKeyIteratorClass *class)
{
        PclIteratorClass *iterator_class;
        PclObjectClass *object_class;

        dict_key_iterator_parent_class = g_type_class_peek_parent (class);

        iterator_class = PCL_ITERATOR_CLASS (class);
        iterator_class->next = dict_key_iterator_next;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_dict_key_iterator_get_type_object;
}

GType
pcl_dict_key_iterator_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclDictKeyIteratorClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) dict_key_iterator_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclDictKeyIterator),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_DICT_ITERATOR, "PclDictKeyIterator",
                        &type_info, 0);
        }
        return type;
}

PclObject *
pcl_dict_key_iterator_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (
                        PCL_TYPE_DICT_KEY_ITERATOR, "dict-key-iterator");
                pcl_register_singleton (
                        "<type 'dict-key-iterator'>", &object);
        }
        return object;
}

PclObject *
pcl_dict_key_iterator_new (PclObject *dict)
{
        return dict_iterator_new (PCL_TYPE_DICT_KEY_ITERATOR, dict);
}

/*****************************************************************************/

static gpointer dict_value_iterator_parent_class = NULL;

static PclObject *
dict_value_iterator_next (PclIterator *iterator)
{
        PclDictIterator *dict_iterator = PCL_DICT_ITERATOR (iterator);
        PclDict *dict = dict_iterator->dict;
        register PclDictEntry *table;
        register glong mask;
        register glong pos;

        if (dict == NULL)
                return NULL;
        if (dict_iterator->used != dict->used)
        {
                pcl_error_set_string (
                        pcl_exception_runtime_error (),
                        "dictionary changed size during iteration");
                dict_iterator->used = -1;  /* make this state sticky */
                return NULL;
        }

        pos = dict_iterator->pos;
        if (pos < 0)
                goto fail;
        table = dict->table;
        mask = dict->mask;
        while (pos <= mask && table[pos].value == NULL)
                pos++;
        dict_iterator->pos = pos + 1;
        if (pos > mask)
                goto fail;
        dict_iterator->length--;
        return pcl_object_ref (table[pos].value);

fail:
        pcl_object_unref (dict);
        dict_iterator->dict = NULL;
        return NULL;
}

static void
dict_value_iterator_class_init (PclDictValueIteratorClass *class)
{
        PclIteratorClass *iterator_class;
        PclObjectClass *object_class;

        dict_value_iterator_parent_class = g_type_class_peek_parent (class);

        iterator_class = PCL_ITERATOR_CLASS (class);
        iterator_class->next = dict_value_iterator_next;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_dict_value_iterator_get_type_object;
}

GType
pcl_dict_value_iterator_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclDictValueIteratorClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) dict_value_iterator_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclDictValueIterator),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_DICT_ITERATOR, "PclDictValueIterator",
                        &type_info, 0);
        }
        return type;
}

PclObject *
pcl_dict_value_iterator_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (
                        PCL_TYPE_DICT_VALUE_ITERATOR, "dict-value-iterator");
                pcl_register_singleton (
                        "<type 'dict-value-iterator'>", &object);
        }
        return object;
}

PclObject *
pcl_dict_value_iterator_new (PclObject *dict)
{
        return dict_iterator_new (PCL_TYPE_DICT_VALUE_ITERATOR, dict);
}

/*****************************************************************************/

static gpointer dict_item_iterator_parent_class = NULL;

static PclObject *
dict_item_iterator_next (PclIterator *iterator)
{
        PclDictIterator *dict_iterator = PCL_DICT_ITERATOR (iterator);
        PclDict *dict = dict_iterator->dict;
        register PclDictEntry *table;
        register glong mask;
        register glong pos;
        PclObject *result;

        if (dict == NULL)
                return NULL;
        if (dict_iterator->used != dict->used)
        {
                pcl_error_set_string (
                        pcl_exception_runtime_error (),
                        "dictionary changed size during iteration");
                dict_iterator->used = -1;  /* make this state sticky */
                return NULL;
        }

        pos = dict_iterator->pos;
        if (pos < 0)
                goto fail;
        table = dict->table;
        mask = dict->mask;
        while (pos <= mask && table[pos].value == NULL)
                pos++;
        dict_iterator->pos = pos + 1;
        if (pos > mask)
                goto fail;
        result = pcl_tuple_new (2);
        if (result == NULL)
                return NULL;
        dict_iterator->length--;
        PCL_TUPLE_SET_ITEM (result, 0, pcl_object_ref (table[pos].key));
        PCL_TUPLE_SET_ITEM (result, 1, pcl_object_ref (table[pos].value));
        return result;

fail:
        pcl_object_unref (dict);
        dict_iterator->dict = NULL;
        return NULL;
}

static void
dict_item_iterator_class_init (PclDictItemIteratorClass *class)
{
        PclIteratorClass *iterator_class;
        PclObjectClass *object_class;

        dict_item_iterator_parent_class = g_type_class_peek_parent (class);

        iterator_class = PCL_ITERATOR_CLASS (class);
        iterator_class->next = dict_item_iterator_next;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_dict_item_iterator_get_type_object;
}

GType
pcl_dict_item_iterator_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclDictItemIteratorClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) dict_item_iterator_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclDictItemIterator),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_DICT_ITERATOR, "PclDictItemIterator",
                        &type_info, 0);
        }
        return type;
}

PclObject *
pcl_dict_item_iterator_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (
                        PCL_TYPE_DICT_ITEM_ITERATOR, "dict-item-iterator");
                pcl_register_singleton (
                        "<type 'dict-item-iterator'>", &object);
        }
        return object;
}

PclObject *
pcl_dict_item_iterator_new (PclObject *dict)
{
        return dict_iterator_new (PCL_TYPE_DICT_ITEM_ITERATOR, dict);
}
