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

#ifndef PCL_OBJECT_H
#define PCL_OBJECT_H

#include "pcl-config.h"

G_BEGIN_DECLS

typedef struct _PclObject PclObject;
typedef struct _PclObjectClass PclObjectClass;

/**
 * PclObjectFlags:
 * @PCL_OBJECT_FLAG_SINGLETON:
 *      This is the only instance of its type. 
 * @PCL_OBJECT_FLAG_RECYCLABLE:
 *      A #PclFactory will try to recycle this object once it is no longer
 *      being used.
 *
 * Used to check or determine characteristics of a #PclObject.
 */
typedef enum {
        PCL_OBJECT_FLAG_SINGLETON       = 1 << 0,
        PCL_OBJECT_FLAG_RECYCLABLE      = 1 << 1
} PclObjectFlags;

/**
 * PclPrintFlags:
 * @PCL_PRINT_FLAG_RAW:
 *      Write the str() of the object instead of the repr().
 *
 * Used to specify options for pcl_object_print().
 */
typedef enum {
        PCL_PRINT_FLAG_RAW      = 1 << 0
} PclPrintFlags;

/**
 * PclRichCompareOps:
 * @PCL_LT:  Less than
 * @PCL_LE:  Less than or equal
 * @PCL_EQ:  Equal
 * @PCL_NE:  Not equal
 * @PCL_GT:  Greater than
 * @PCL_GE:  Greater than or equal
 *
 * Used to specifiy the desired rich-comparison operation.
 */
typedef enum {
        PCL_LT,
        PCL_LE,
        PCL_EQ,
        PCL_NE,
        PCL_GT,
        PCL_GE
} PclRichCompareOps;

struct _PclObject {
        GObject parent;
        PclObjectFlags flags;
};

struct _PclObjectClass {
        GObjectClass parent_class;

        PclObject *     (*type)         (void);
        PclObject *     (*new_instance) (PclObject *type,
                                         PclObject *args,
                                         PclObject *kwds);
        gboolean        (*init_instance)(PclObject *object,
                                         PclObject *args,
                                         PclObject *kwds);
        PclObject *     (*clone)        (PclObject *object);
        gboolean        (*print)        (PclObject *object,
                                         FILE *stream,
                                         PclPrintFlags flags);
        PclObject *     (*call)         (PclObject *object,
                                         PclObject *args,
                                         PclObject *kwds);
        PclObject *     (*copy)         (PclObject *object);
        PclObject *     (*dict)         (PclObject *object);
        guint           (*hash)         (PclObject *object);
        PclObject *     (*repr)         (PclObject *object);
        PclObject *     (*str)          (PclObject *object);
        gint            (*compare)      (PclObject *object1,
                                         PclObject *object2);
        PclObject *     (*rich_compare) (PclObject *object1,
                                         PclObject *object2,
                                         PclRichCompareOps op);
        gint            (*contains)     (PclObject *object,
                                         PclObject *value);
        PclObject *     (*iterate)      (PclObject *object);
        glong           (*measure)      (PclObject *object);
        PclObject *     (*get_attr)     (PclObject *object,
                                         const gchar *name);
        gboolean        (*set_attr)     (PclObject *object,
                                         const gchar *name,
                                         PclObject *value);
        PclObject *     (*get_item)     (PclObject *object,
                                         PclObject *subscript);
        gboolean        (*set_item)     (PclObject *object,
                                         PclObject *subscript,
                                         PclObject *value);

        const gchar *doc;
};

GType           pcl_object_get_type             (void);
PclObject *     pcl_object_get_type_object      (void);
PclObject *     pcl_object_type                 (PclObject *object);
PclObject *     pcl_object_clone                (PclObject *object);
gboolean        pcl_object_print                (PclObject *object,
                                                 FILE *stream,
                                                 PclPrintFlags flags);
PclObject *     pcl_object_call                 (PclObject *object,
                                                 PclObject *args,
                                                 PclObject *kwds);
PclObject *     pcl_object_call_function        (PclObject *object,
                                                 const gchar *format, ...);
PclObject *     pcl_object_call_method          (PclObject *object,
                                                 const gchar *name,
                                                 const gchar *format, ...);
PclObject *     pcl_object_call_object          (PclObject *object,
                                                 PclObject *args);
PclObject *     pcl_object_copy                 (PclObject *object);
PclObject *     pcl_object_get_dict             (PclObject *object);
guint           pcl_object_hash                 (PclObject *object);
PclObject *     pcl_object_repr                 (PclObject *object);
PclObject *     pcl_object_str                  (PclObject *object);
gint            pcl_object_compare              (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_object_rich_compare         (PclObject *object1,
                                                 PclObject *object2,
                                                 PclRichCompareOps op);
gint            pcl_object_rich_compare_bool    (PclObject *object1,
                                                 PclObject *object2,
                                                 PclRichCompareOps op);
gint            pcl_object_contains             (PclObject *object,
                                                 PclObject *value);
PclObject *     pcl_object_iterate              (PclObject *object);
guint           pcl_object_hash_pointer         (gpointer pointer);
glong           pcl_object_measure              (PclObject *object);
PclObject *     pcl_object_get_attr             (PclObject *object,
                                                 PclObject *name);
gboolean        pcl_object_has_attr             (PclObject *object,
                                                 PclObject *name);
gboolean        pcl_object_set_attr             (PclObject *object,
                                                 PclObject *name,
                                                 PclObject *value);
PclObject *     pcl_object_get_attr_string      (PclObject *object,
                                                 const gchar *name);
gboolean        pcl_object_has_attr_string      (PclObject *object,
                                                 const gchar *name);
gboolean        pcl_object_set_attr_string      (PclObject *object,
                                                 const gchar *name,
                                                 PclObject *value);
PclObject *     pcl_object_get_item             (PclObject *object,
                                                 PclObject *subscript);
gboolean        pcl_object_set_item             (PclObject *object,
                                                 PclObject *subscript,
                                                 PclObject *value);
gboolean        pcl_object_del_item             (PclObject *object,
                                                 PclObject *subscript);

PclObject *     pcl_object_dir                  (PclObject *object);
gint            pcl_object_is_instance          (PclObject *instance,
                                                 PclObject *of);
gint            pcl_object_is_subclass          (PclObject *subclass,
                                                 PclObject *of);
gint            pcl_object_is_true              (PclObject *object);
gint            pcl_object_not                  (PclObject *object);

/* For GType data storage */
#define PCL_DATA_GETSETS        (pcl_data_getsets_quark ())
#define PCL_DATA_MEMBERS        (pcl_data_members_quark ())
#define PCL_DATA_METHODS        (pcl_data_methods_quark ())

/* Extract type object from object */
#define PCL_GET_TYPE_OBJECT(obj) \
        (PCL_OBJECT_GET_CLASS (obj)->type ())

/* Test object characteristics */
#define PCL_IS_CALLABLE(obj) \
        ((obj) != NULL && PCL_OBJECT_GET_CLASS (obj)->call != NULL)
#define PCL_IS_ITERABLE(obj) \
        ((obj) != NULL && PCL_OBJECT_GET_CLASS (obj)->iterate != NULL)
#define PCL_IS_MEASURABLE(obj) \
        ((obj) != NULL && PCL_OBJECT_GET_CLASS (obj)->measure != NULL)

/* Reserved hash values
 * PCL_HASH_INVALID : either a hash function failed or a cached hash value
 *                    has not yet been set
 * PCL_HASH_DEFAULT : safe value to use if the hash algorithm happens to
 *                    produce PCL_HASH_INVALID */
#define PCL_HASH_INVALID (G_MAXUINT)
#define PCL_HASH_DEFAULT (G_MAXUINT - 1)

#define PCL_HASH_VALIDATE(hash) \
        ((hash) != PCL_HASH_INVALID ? (hash) : PCL_HASH_DEFAULT)

/* Standard GObject macros */
#define PCL_TYPE_OBJECT \
        (pcl_object_get_type ())
#define PCL_OBJECT(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_OBJECT, PclObject))
#define PCL_OBJECT_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_OBJECT, PclObjectClass))
#define PCL_IS_OBJECT(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_OBJECT))
#define PCL_IS_OBJECT_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_OBJECT))
#define PCL_OBJECT_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_OBJECT, PclObjectClass))

/* Fast access macro (use carefully) */
#define PCL_OBJECT_GET_FLAGS(obj) \
        (PCL_OBJECT (obj)->flags)

G_END_DECLS

#endif /* PCL_OBJECT_H */
