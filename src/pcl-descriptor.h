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

#ifndef PCL_DESCRIPTOR_H
#define PCL_DESCRIPTOR_H

#include "pcl-config.h"
#include "pcl-container.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclDescriptor PclDescriptor;
typedef struct _PclDescriptorClass PclDescriptorClass;

struct _PclDescriptor {
        PclContainer parent;
        PclObject *type;
        PclObject *name;
};

struct _PclDescriptorClass {
        PclContainerClass parent_class;
        PclObject *     (*get)                  (PclDescriptor *descriptor,
                                                 PclObject *object,
                                                 PclObject *type);
        gboolean        (*set)                  (PclDescriptor *descriptor,
                                                 PclObject *object,
                                                 PclObject *value);
};

GType           pcl_descriptor_get_type         (void);

#define PCL_DESCRIPTOR_IS_DATA(obj) \
        (PCL_DESCRIPTOR_GET_CLASS (obj)->set != NULL)

/* Standard GObject macros */
#define PCL_TYPE_DESCRIPTOR \
        (pcl_descriptor_get_type ())
#define PCL_DESCRIPTOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_DESCRIPTOR, PclDescriptor))
#define PCL_DESCRIPTOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_DESCRIPTOR, PclDescriptorClass))
#define PCL_IS_DESCRIPTOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_DESCRIPTOR))
#define PCL_IS_DESCRIPTOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_DESCRIPTOR))
#define PCL_DESCRIPTOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_DESCRIPTOR, PclDescriptorClass))

G_END_DECLS

#endif /* PCL_DESCRIPTOR_H */
