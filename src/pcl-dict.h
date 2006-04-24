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

#ifndef PCL_DICT_H
#define PCL_DICT_H

/* Dictionary object type -- mapping from hashable object to object
 *
 * There are three kinds of slots in the table:
 *
 * 1. Unused.  key == value == NULL
 *    Does not hold an active (key, value) pair now and never did.  Unused can
 *    transition to Active upon key insertion.  This is the only case in which
 *    key is NULL, and is each slot's initial state.
 *
 * 2. Active.  key != NULL and key != dummy and value != NULL
 *    Holds an active (key, value) pair.  Active can transition to Dummy upon
 *    key deletion.  This is the only case in which value != NULL.
 *
 * 3. Dummy.  key == dummy and value == NULL
 *    Previously held an active (key, value) pair, but that was deleted and an
 *    active pair has not yet overwritten the slot.  Dummy can transition to
 *    Active upon key insertion.  Dummy slots cannot be made Unused again
 *    (cannot have key set to NULL), else the probe sequence in case of
 *    collision would have no way to know they were once active.
 *
 * Note: .popitem() abuses the hash field of an Unused or Dummy slot to hold a
 *       search finger.  The hash field of Unused or Dummy slots has no meaning
 *       otherwise.
 */

#include "pcl-config.h"
#include "pcl-container.h"
#include "pcl-iterator.h"
#include "pcl-object.h"

G_BEGIN_DECLS

/* PCL_DICT_MIN_SIZE is the minimum size of a dictionary.  This many slots are
 * allocated directly in the dict object (in the small_table member).  It must
 * be a power of 2, and at least 4.  8 allows dicts with no more than 5 active
 * entries to live in small_table (and so avoid an additional malloc);
 * instrumentation suggested this suffices for the majority of dicts
 * (consisting mostly of usually-small instance dicts and usually-small dicts
 * created to pass keyword arguments). */
#define PCL_DICT_MIN_SIZE 8

typedef struct _PclDict PclDict;
typedef struct _PclDictClass PclDictClass;
typedef struct _PclDictEntry PclDictEntry;

struct _PclDictEntry {
        guint hash;             /* cached hash(key) */
        PclObject *key;
        PclObject *value;
};

struct _PclDict {
        PclContainer parent;

        glong fill;             /* # Active + # Dummy */
        glong used;             /* # Active */

        /* The table contains mask + 1 slots, and that's a power of 2.
         * We store the mask instead of the size because the mask is more
         * frequently needed. */
        glong mask;

        /* table points to small_table for small tables, else to additional
         * malloc'ed memory.  table is never NULL!  This rule saves repeated
         * runtime NULL tests in the workhorse getitem and setitem calls. */
        PclDictEntry *table;
        PclDictEntry *(*lookup) (PclDict *, PclObject *, guint);
        PclDictEntry small_table[PCL_DICT_MIN_SIZE];
};

struct _PclDictClass {
        PclContainerClass parent;
};

GType           pcl_dict_get_type               (void);
PclObject *     pcl_dict_get_type_object        (void);
PclObject *     pcl_dict_new                    (void);
gint            pcl_dict_has_key                (PclObject *object,
                                                 PclObject *key);
gint            pcl_dict_has_key_string         (PclObject *object,
                                                 const gchar *key);
PclObject *     pcl_dict_get_item               (PclObject *object,
                                                 PclObject *key);
PclObject *     pcl_dict_get_item_string        (PclObject *object,
                                                 const gchar *key);
gboolean        pcl_dict_set_item               (PclObject *object,
                                                 PclObject *key,
                                                 PclObject *value);
gboolean        pcl_dict_set_item_string        (PclObject *object,
                                                 const gchar *key,
                                                 PclObject *value);
gboolean        pcl_dict_del_item               (PclObject *object,
                                                 PclObject *key);
gboolean        pcl_dict_del_item_string        (PclObject *object,
                                                 const gchar *key);
gboolean        pcl_dict_next                   (PclObject *dict,
                                                 glong *p_pos,
                                                 PclObject **p_key,
                                                 PclObject **p_value);

/* Standard GObject macros */
#define PCL_TYPE_DICT \
        (pcl_dict_get_type ())
#define PCL_DICT(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_DICT, PclDict))
#define PCL_DICT_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_DICT, PclDictClass))
#define PCL_IS_DICT(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_DICT))
#define PCL_IS_DICT_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_DICT))
#define PCL_DICT_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_DICT, PclDictClass))

/*****************************************************************************/

/* Abstract base class for dictionary iterators */

typedef struct _PclDictIterator PclDictIterator;
typedef struct _PclDictIteratorClass PclDictIteratorClass;

struct _PclDictIterator {
        PclIterator parent;
        PclDict *dict;
        glong length;
        glong used;
        glong pos;
};

struct _PclDictIteratorClass {
        PclIteratorClass parent_class;
};

GType           pcl_dict_iterator_get_type      (void);

/* Standard GObject macros */
#define PCL_TYPE_DICT_ITERATOR \
        (pcl_dict_iterator_get_type ())
#define PCL_DICT_ITERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_DICT_ITERATOR, PclDictIterator))
#define PCL_DICT_ITERATOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_DICT_ITERATOR, PclDictIteratorClass))
#define PCL_IS_DICT_ITERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_DICT_ITERATOR))
#define PCL_IS_DICT_ITERATOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_DICT_ITERATOR))
#define PCL_DICT_ITERATOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_DICT_ITERATOR, PclDictIteratorClass))

/*****************************************************************************/

/* Iterates over a dictionary's keys */

typedef struct _PclDictKeyIterator PclDictKeyIterator;
typedef struct _PclDictKeyIteratorClass PclDictKeyIteratorClass;

struct _PclDictKeyIterator {
        PclDictIterator parent;
};

struct _PclDictKeyIteratorClass {
        PclDictIteratorClass parent_class;
};

GType           pcl_dict_key_iterator_get_type          (void);
PclObject *     pcl_dict_key_iterator_get_type_object   (void);
PclObject *     pcl_dict_key_iterator_new               (PclObject *dict);

/* Standard GObject macros */
#define PCL_TYPE_DICT_KEY_ITERATOR \
        (pcl_dict_key_iterator_get_type ())
#define PCL_DICT_KEY_ITERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_DICT_KEY_ITERATOR, PclDictKeyIterator))
#define PCL_DICT_KEY_ITERATOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_DICT_KEY_ITERATOR, PclDictKeyIteratorClass))
#define PCL_IS_DICT_KEY_ITERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_DICT_KEY_ITERATOR))
#define PCL_IS_DICT_KEY_ITERATOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_DICT_KEY_ITERATOR))
#define PCL_DICT_KEY_ITERATOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_DICT_KEY_ITERATOR, PclDictKeyIteratorClass))

/*****************************************************************************/

/* Iterates over a dictionary's values */

typedef struct _PclDictValueIterator PclDictValueIterator;
typedef struct _PclDictValueIteratorClass PclDictValueIteratorClass;

struct _PclDictValueIterator {
        PclDictIterator parent;
};

struct _PclDictValueIteratorClass {
        PclDictIteratorClass parent_class;
};

GType           pcl_dict_value_iterator_get_type        (void);
PclObject *     pcl_dict_value_iterator_get_type_object (void);
PclObject *     pcl_dict_value_iterator_new             (PclObject *dict);

/* Standard GObject macros */
#define PCL_TYPE_DICT_VALUE_ITERATOR \
        (pcl_dict_value_iterator_get_type ())
#define PCL_DICT_VALUE_ITERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_DICT_VALUE_ITERATOR, PclDictValueIterator))
#define PCL_DICT_VALUE_ITERATOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_DICT_VALUE_ITERATOR, PclDictValueIteratorClass))
#define PCL_IS_DICT_VALUE_ITERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_DICT_VALUE_ITERATOR))
#define PCL_IS_DICT_VALUE_ITERATOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_DICT_VALUE_ITERATOR))
#define PCL_DICT_VALUE_ITERATOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_DICT_VALUE_ITERATOR, PclDictValueIteratorClass))

/*****************************************************************************/

/* Iterates over a dictionary's (key, value) pairs */

typedef struct _PclDictItemIterator PclDictItemIterator;
typedef struct _PclDictItemIteratorClass PclDictItemIteratorClass;

struct _PclDictItemIterator {
        PclDictIterator parent;
};

struct _PclDictItemIteratorClass {
        PclDictIteratorClass parent_class;
};

GType           pcl_dict_item_iterator_get_type         (void);
PclObject *     pcl_dict_item_iterator_get_type_object  (void);
PclObject *     pcl_dict_item_iterator_new              (PclObject *dict);

/* Standard GObject macros */
#define PCL_TYPE_DICT_ITEM_ITERATOR \
        (pcl_dict_item_iterator_get_type ())
#define PCL_DICT_ITEM_ITERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_DICT_ITEM_ITERATOR, PclDictItemIterator))
#define PCL_DICT_ITEM_ITERATOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_DICT_ITEM_ITERATOR, PclDictItemIteratorClass))
#define PCL_IS_DICT_ITEM_ITERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_DICT_ITEM_ITERATOR))
#define PCL_IS_DICT_ITEM_ITERATOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_DICT_ITEM_ITERATOR))
#define PCL_DICT_ITEM_ITERATOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_DICT_ITEM_ITERATOR, PclDictItemIteratorClass))

G_END_DECLS

#endif /* PCL_DICT_H */
