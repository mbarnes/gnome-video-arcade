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

#ifndef PCL_TYPE_H
#define PCL_TYPE_H

#include "pcl-config.h"
#include "pcl-container.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclType PclType;
typedef struct _PclTypeClass PclTypeClass;

struct _PclType {
        PclContainer parent;
        PclObject *base;
        PclObject *bases;
        PclObject *dict;
        PclObject *mro;
        PclObject *subclasses;
        const gchar *name;
        PclObjectClass *object_class;
};

struct _PclTypeClass {
        PclContainerClass parent_class;
};

GType           pcl_type_get_type               (void);
PclObject *     pcl_type_get_type_object        (void);
PclObject *     pcl_type_new                    (GType g_type,
                                                 const gchar *name);
PclObject *     pcl_type_get_attr               (PclObject *type,
                                                 const gchar *name);
gint            pcl_type_is_subtype             (PclObject *subtype,
                                                 PclObject *of);

/* Extract type name from object */
#define PCL_GET_TYPE_NAME(obj) \
        (PCL_TYPE (PCL_GET_TYPE_OBJECT (obj))->name)

/* Standard GObject macros */
#define PCL_TYPE_TYPE \
        (pcl_type_get_type ())
#define PCL_TYPE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_TYPE, PclType))
#define PCL_TYPE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_TYPE, PclTypeClass))
#define PCL_IS_TYPE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_TYPE))
#define PCL_IS_TYPE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_TYPE))
#define PCL_TYPE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_TYPE, PclTypeClass))

G_END_DECLS

#endif /* PCL_TYPE_H */
