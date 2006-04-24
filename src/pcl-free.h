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

#ifndef PCL_FREE_H
#define PCL_FREE_H

#include "pcl-config.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclFree PclFree;
typedef struct _PclFreeClass PclFreeClass;

struct _PclFree {
        PclObject parent;
};

struct _PclFreeClass {
        PclObjectClass parent_class;
};

GType           pcl_free_get_type               (void);
PclObject *     pcl_free_get_type_object        (void);
PclObject *     pcl_free_get_instance           (void);

/* Use this instead of calling the function directly. */
#define PCL_FREE (pcl_free_get_instance ())

/* Standard GObject macros */
#define PCL_TYPE_FREE \
        (pcl_free_get_type ())
#define PCL_FREE_CAST(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_FREE, PclFree))
#define PCL_FREE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_FREE, PclFreeClass))
#define PCL_IS_FREE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_FREE))
#define PCL_IS_FREE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_FREE))
#define PCL_FREE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_FREE, PclFreeClass))

G_END_DECLS

#endif /* PCL_FREE_H */
