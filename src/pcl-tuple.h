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

#ifndef PCL_TUPLE_H
#define PCL_TUPLE_H

#include "pcl-config.h"
#include "pcl-container.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclTuple PclTuple;
typedef struct _PclTupleClass PclTupleClass;

struct _PclTuple {
        PclContainer parent;
        GPtrArray *array;
};

struct _PclTupleClass {
        PclContainerClass parent_class;
};

GType           pcl_tuple_get_type              (void);
PclObject *     pcl_tuple_get_type_object       (void);
PclObject *     pcl_tuple_new                   (glong length);
PclObject *     pcl_tuple_pack                  (glong length, ...);
PclObject *     pcl_tuple_get_item              (PclObject *self,
                                                 glong index);
PclObject *     pcl_tuple_get_slice             (PclObject *self,
                                                 glong start,
                                                 glong stop);
gboolean        pcl_tuple_set_item              (PclObject *self,
                                                 glong index,
                                                 PclObject *value);

/* Standard GObject macros */
#define PCL_TYPE_TUPLE \
        (pcl_tuple_get_type ())
#define PCL_TUPLE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_TUPLE, PclTuple))
#define PCL_TUPLE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_TUPLE, PclTupleClass))
#define PCL_IS_TUPLE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_TUPLE))
#define PCL_IS_TUPLE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_TUPLE))
#define PCL_TUPLE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_TUPLE, PclTupleClass))

/* Fast access macros (use carefully) */
#define PCL_TUPLE_GET_ITEM(obj, idx) \
        (PCL_TUPLE (obj)->array->pdata[idx])
#define PCL_TUPLE_GET_SIZE(obj) \
        ((glong) PCL_TUPLE (obj)->array->len)
#define PCL_TUPLE_SET_ITEM(obj, idx, val) \
        (PCL_TUPLE (obj)->array->pdata[idx] = (val))

G_END_DECLS

#endif /* PCL_TUPLE_H */
