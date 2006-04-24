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

#ifndef PCL_NONE_H
#define PCL_NONE_H

#include "pcl-config.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclNone PclNone;
typedef struct _PclNoneClass PclNoneClass;

struct _PclNone {
        PclObject parent;
};

struct _PclNoneClass {
        PclObjectClass parent_class;
};

GType           pcl_none_get_type               (void);
PclObject *     pcl_none_get_type_object        (void);
PclObject *     pcl_none_get_instance           (void);

/* Use this instead of calling the function directly. */
#define PCL_NONE (pcl_none_get_instance ())

/* Standard GObject macros */
#define PCL_TYPE_NONE \
        (pcl_none_get_type ())
#define PCL_NONE_CAST(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_NONE, PclNone))
#define PCL_NONE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_NONE, PclNoneClass))
#define PCL_IS_NONE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_NONE))
#define PCL_IS_NONE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_NONE))
#define PCL_NONE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_NONE, PclNoneClass))

G_END_DECLS

#endif /* PCL_NONE_H */
