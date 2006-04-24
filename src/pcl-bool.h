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

#ifndef PCL_BOOL_H
#define PCL_BOOL_H

#include "pcl-config.h"
#include "pcl-int.h"

G_BEGIN_DECLS

typedef struct _PclBool PclBool;
typedef struct _PclBoolClass PclBoolClass;

struct _PclBool {
        PclInt parent;
};

struct _PclBoolClass {
        PclIntClass parent_class;
};

GType           pcl_bool_get_type               (void);
PclObject *     pcl_bool_get_type_object        (void);
PclObject *     pcl_bool_get_instance_true      (void);
PclObject *     pcl_bool_get_instance_false     (void);
PclObject *     pcl_bool_from_boolean           (gboolean v_boolean);

/* Use these instead of calling the functions directly. */
#define PCL_TRUE (pcl_bool_get_instance_true ())
#define PCL_FALSE (pcl_bool_get_instance_false ())

/* Standard GObject macros */
#define PCL_TYPE_BOOL \
        (pcl_bool_get_type ())
#define PCL_BOOL(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_BOOL, PclBool))
#define PCL_BOOL_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_BOOL, PclBoolClass))
#define PCL_IS_BOOL(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_BOOL))
#define PCL_IS_BOOL_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_BOOL))
#define PCL_BOOL_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_BOOL, PclBoolClass))

G_END_DECLS

#endif /* PCL_BOOL_H */
