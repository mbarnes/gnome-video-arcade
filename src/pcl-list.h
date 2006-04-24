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

#ifndef PCL_LIST_H
#define PCL_LIST_H

#include "pcl-config.h"
#include "pcl-container.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclList PclList;
typedef struct _PclListClass PclListClass;

struct _PclList {
        PclContainer parent;
        GPtrArray *array;
};

struct _PclListClass {
        PclContainerClass parent_class;
};

GType           pcl_list_get_type               (void);
PclObject *     pcl_list_get_type_object        (void);
PclObject *     pcl_list_new                    (glong length);
PclObject *     pcl_list_from_iterator          (PclObject *iterator);
gboolean        pcl_list_append                 (PclObject *self,
                                                 PclObject *item);
gboolean        pcl_list_insert                 (PclObject *self,
                                                 glong index,
                                                 PclObject *item);
gboolean        pcl_list_reverse                (PclObject *self);
gboolean        pcl_list_sort                   (PclObject *self);
PclObject *     pcl_list_get_item               (PclObject *self,
                                                 glong index);
gboolean        pcl_list_set_item               (PclObject *self,
                                                 glong index,
                                                 PclObject *value);
PclObject *     pcl_list_get_slice              (PclObject *self,
                                                 PclObject *slice);
gboolean        pcl_list_set_slice              (PclObject *self,
                                                 PclObject *slice,
                                                 PclObject *value);
PclObject *     pcl_list_as_tuple               (PclObject *self);

#define pcl_list_del_item(self, index) \
        pcl_list_set_item ((self), (index), NULL)

/* Standard GObject macros */
#define PCL_TYPE_LIST \
        (pcl_list_get_type ())
#define PCL_LIST(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_LIST, PclList))
#define PCL_LIST_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_LIST, PclListClass))
#define PCL_IS_LIST(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_LIST))
#define PCL_IS_LIST_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_LIST))
#define PCL_LIST_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_LIST, PclListClass))

/* Fast access macros (use carefully) */
#define PCL_LIST_GET_ITEM(obj, idx) \
        (PCL_LIST (obj)->array->pdata[idx])
#define PCL_LIST_GET_SIZE(obj) \
        ((glong) PCL_LIST (obj)->array->len)
#define PCL_LIST_SET_ITEM(obj, idx, val) \
        (PCL_LIST (obj)->array->pdata[idx] = (val))

G_END_DECLS

#endif /* PCL_LIST_H */
