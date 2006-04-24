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

#ifndef PCL_SLICE_H
#define PCL_SLICE_H

#include "pcl-config.h"
#include "pcl-container.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclSlice PclSlice;
typedef struct _PclSliceClass PclSliceClass;

struct _PclSlice {
        PclContainer parent;
        PclObject *start;
        PclObject *stop;
        PclObject *step;
};

struct _PclSliceClass {
        PclContainerClass parent_class;
};

GType           pcl_slice_get_type              (void);
PclObject *     pcl_slice_get_type_object       (void);
PclObject *     pcl_slice_new                   (PclObject *start,
                                                 PclObject *stop,
                                                 PclObject *step);
gboolean        pcl_slice_get_indices           (PclSlice *slice,
                                                 glong input_length,
                                                 glong *start,
                                                 glong *stop,
                                                 glong *step,
                                                 glong *length);

/* Standard GObject macros */
#define PCL_TYPE_SLICE \
        (pcl_slice_get_type ())
#define PCL_SLICE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_SLICE, PclSlice))
#define PCL_SLICE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_SLICE, PclSliceClass))
#define PCL_IS_SLICE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_SLICE))
#define PCL_IS_SLICE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_SLICE))
#define PCL_SLICE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_SLICE, PclSliceClass))

G_END_DECLS

#endif /* PCL_SLICE_H */
