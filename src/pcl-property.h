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

#ifndef PCL_PROPERTY_H
#define PCL_PROPERTY_H

#include "pcl-config.h"
#include "pcl-descriptor.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclProperty PclProperty;
typedef struct _PclPropertyClass PclPropertyClass;

struct _PclProperty {
        PclDescriptor parent;
        PclObject *get;
        PclObject *set;
        PclObject *del;
        PclObject *doc;
};

struct _PclPropertyClass {
        PclDescriptorClass parent_class;
};

GType           pcl_property_get_type           (void);
PclObject *     pcl_property_get_type_object    (void);

/* Standard GObject macros */
#define PCL_TYPE_PROPERTY \
        (pcl_property_get_type ())
#define PCL_PROPERTY(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_PROPERTY, PclProperty))
#define PCL_PROPERTY_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_PROPERTY, PclPropertyClass))
#define PCL_IS_PROPERTY(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_PROPERTY))
#define PCL_IS_PROPERTY_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_PROPERTY))
#define PCL_PROPERTY_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_PROPERTY, PclPropertyClass))

G_END_DECLS

#endif /* PCL_PROPERTY_H */
