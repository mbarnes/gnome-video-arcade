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

#ifndef PCL_GETSET_DESCRIPTOR_H
#define PCL_GETSET_DESCRIPTOR_H

#include "pcl-config.h"
#include "pcl-descriptor.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef PclObject *     (*PclGetFunc)           (PclObject *object,
                                                 gpointer context);
typedef gboolean        (*PclSetFunc)           (PclObject *object,
                                                 PclObject *value,
                                                 gpointer context);

typedef struct _PclGetSetDef PclGetSetDef;
typedef struct _PclGetSetDescriptor PclGetSetDescriptor;
typedef struct _PclGetSetDescriptorClass PclGetSetDescriptorClass;

struct _PclGetSetDef {
        gchar *name;
        PclGetFunc get;
        PclSetFunc set;
        gchar *doc;
        gpointer closure;
};

struct _PclGetSetDescriptor {
        PclDescriptor parent;
        PclGetSetDef *getset;
};

struct _PclGetSetDescriptorClass {
        PclDescriptorClass parent_class;
};

GType           pcl_getset_descriptor_get_type          (void);
PclObject *     pcl_getset_descriptor_get_type_object   (void);

/* Standard GObject macros */
#define PCL_TYPE_GETSET_DESCRIPTOR \
        (pcl_getset_descriptor_get_type ())
#define PCL_GETSET_DESCRIPTOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_GETSET_DESCRIPTOR, PclGetSetDescriptor))
#define PCL_GETSET_DESCRIPTOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_GETSET_DESCRIPTOR, PclGetSetDescriptorClass))
#define PCL_IS_GETSET_DESCRIPTOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_GETSET_DESCRIPTOR))
#define PCL_IS_GETSET_DESCRIPTOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_GETSET_DESCRIPTOR))
#define PCL_GETSET_DESCRIPTOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_GETSET_DESCRIPTOR, PclGetSetDescriptorClass))

G_END_DECLS

#endif /* PCL_GETSET_DESCRIPTOR_H */
