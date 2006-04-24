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

#ifndef PCL_INSTANCE_METHOD_H
#define PCL_INSTANCE_METHOD_H

#include "pcl-config.h"
#include "pcl-descriptor.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclInstanceMethod PclInstanceMethod;
typedef struct _PclInstanceMethodClass PclInstanceMethodClass;

struct _PclInstanceMethod {
        PclDescriptor parent;
        PclObject *callable;
        PclObject *instance;
        PclObject *owner;
};

struct _PclInstanceMethodClass {
        PclDescriptorClass parent_class;
};

GType           pcl_instance_method_get_type            (void);
PclObject *     pcl_instance_method_get_type_object     (void);
PclObject *     pcl_instance_method_new                 (PclObject *callable,
                                                         PclObject *instance,
                                                         PclObject *owner);

/* Standard GObject macros */
#define PCL_TYPE_INSTANCE_METHOD \
        (pcl_instance_method_get_type ())
#define PCL_INSTANCE_METHOD(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_INSTANCE_METHOD, PclInstanceMethod))
#define PCL_INSTANCE_METHOD_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((obj), PCL_TYPE_INSTANCE_METHOD, PclInstanceMethodClass))
#define PCL_IS_INSTANCE_METHOD(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_INSTANCE_METHOD))
#define PCL_IS_INSTANCE_METHOD_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((obj), PCL_TYPE_INSTANCE_METHOD))
#define PCL_INSTANCE_METHOD_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_INSTANCE_METHOD, PclInstanceMethodClass))

/* Fast access macros (use carefully) */
#define PCL_INSTANCE_METHOD_GET_CALLABLE(obj) \
        (PCL_INSTANCE_METHOD (obj)->callable)
#define PCL_INSTANCE_METHOD_GET_INSTANCE(obj) \
        (PCL_INSTANCE_METHOD (obj)->instance)
#define PCL_INSTANCE_METHOD_GET_OWNER(obj) \
        (PCL_INSTANCE_METHOD (obj)->owner)

G_END_DECLS

#endif /* PCL_INSTANCE_METHOD_H */
