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

#ifndef PCL_STATIC_METHOD_H
#define PCL_STATIC_METHOD_H

#include "pcl-config.h"
#include "pcl-descriptor.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclStaticMethod PclStaticMethod;
typedef struct _PclStaticMethodClass PclStaticMethodClass;

struct _PclStaticMethod {
        PclDescriptor parent;
        PclObject *callable;
};

struct _PclStaticMethodClass {
        PclDescriptorClass parent_class;
};

GType           pcl_static_method_get_type              (void);
PclObject *     pcl_static_method_get_type_object       (void);

/* Standard GObject macros */
#define PCL_TYPE_STATIC_METHOD \
        (pcl_static_method_get_type ())
#define PCL_STATIC_METHOD(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_STATIC_METHOD, PclStaticMethod))
#define PCL_STATIC_METHOD_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((obj), PCL_TYPE_STATIC_METHOD, PclStaticMethodClass))
#define PCL_IS_STATIC_METHOD(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_STATIC_METHOD))
#define PCL_IS_STATIC_METHOD_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((obj), PCL_TYPE_STATIC_METHOD))
#define PCL_STATIC_METHOD_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_STATIC_METHOD, PclStaticMethodClass))

G_END_DECLS

#endif /* PCL_STATIC_METHOD_H */
