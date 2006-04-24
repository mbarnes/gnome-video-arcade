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

#ifndef PCL_RANGE_H
#define PCL_RANGE_H

#include "pcl-config.h"
#include "pcl-iterator.h"
#include "pcl-object.h"

/* XXX PclRange is a subclass of PclContainer.
 *     PclContainer is intended for object types that reference other objects,
 *     but PclRange doesn't reference any other objects.  So PCL_IS_CONTAINER()
 *     will report a misleading answer when given a 'range' object, but that
 *     shouldn't cause any negative side-effects other than being needlessly
 *     tracked by the garbage collector. */

G_BEGIN_DECLS

typedef struct _PclRange PclRange;
typedef struct _PclRangeClass PclRangeClass;

struct _PclRange {
        PclIterator parent;
        glong length;
        glong next;
        glong step;
};

struct _PclRangeClass {
        PclIteratorClass parent_class;
};

GType           pcl_range_get_type              (void);
PclObject *     pcl_range_get_type_object       (void);

/* Standard GObject macros */
#define PCL_TYPE_RANGE \
        (pcl_range_get_type ())
#define PCL_RANGE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_RANGE, PclRange))
#define PCL_RANGE_CLASS(obj) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_RANGE, PclRangeClass))
#define PCL_IS_RANGE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_RANGE))
#define PCL_IS_RANGE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_RANGE))
#define PCL_RANGE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_RANGE, PclRangeClass))

G_END_DECLS

#endif /* PCL_RANGE_H */
