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

#ifndef PCL_METHOD_DESCRIPTOR_H
#define PCL_METHOD_DESCRIPTOR_H

#include "pcl-config.h"
#include "pcl-descriptor.h"
#include "pcl-method.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclMethodDescriptor PclMethodDescriptor;
typedef struct _PclMethodDescriptorClass PclMethodDescriptorClass;

struct _PclMethodDescriptor {
        PclDescriptor parent;
        PclMethodDef *method;
};

struct _PclMethodDescriptorClass {
        PclDescriptorClass parent_class;
};

GType           pcl_method_descriptor_get_type          (void);
PclObject *     pcl_method_descriptor_get_type_object   (void);

/* Standard GObject macros */
#define PCL_TYPE_METHOD_DESCRIPTOR \
        (pcl_method_descriptor_get_type ())
#define PCL_METHOD_DESCRIPTOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_METHOD_DESCRIPTOR, PclMethodDescriptor))
#define PCL_METHOD_DESCRIPTOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_METHOD_DESCRIPTOR, PclMethodDescriptorClass))
#define PCL_IS_METHOD_DESCRIPTOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_METHOD_DESCRIPTOR))
#define PCL_IS_METHOD_DESCRIPTOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_METHOD_DESCRIPTOR))
#define PCL_METHOD_DESCRIPTOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_METHOD_DESCRIPTOR, PclMethodDescriptorClass))

G_END_DECLS

#endif /* PCL_METHOD_DESCRIPTOR_H */
