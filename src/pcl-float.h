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

#ifndef PCL_FLOAT_H
#define PCL_FLOAT_H

#include "pcl-config.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclFloat PclFloat;
typedef struct _PclFloatClass PclFloatClass;

struct _PclFloat {
        PclObject parent;
        gdouble v_double;
};

struct _PclFloatClass {
        PclObjectClass parent_class;
};

GType           pcl_float_get_type              (void);
PclObject *     pcl_float_get_type_object       (void);
gdouble         pcl_float_as_double             (PclObject *object);
PclObject *     pcl_float_from_double           (gdouble v_double);
PclObject *     pcl_float_from_string           (const gchar *v_string,
                                                 gchar **endptr);

/* Standard GObject macros */
#define PCL_TYPE_FLOAT \
        (pcl_float_get_type ())
#define PCL_FLOAT(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_FLOAT, PclFloat))
#define PCL_FLOAT_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_FLOAT, PclFloatClass))
#define PCL_IS_FLOAT(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_FLOAT))
#define PCL_IS_FLOAT_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_FLOAT))
#define PCL_FLOAT_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_FLOAT, PclFloatClass))

/* Fast access macro (use carefully) */
#define PCL_FLOAT_AS_DOUBLE(obj)        (PCL_FLOAT (obj)->v_double)

G_END_DECLS

#endif /* PCL_FLOAT_H */
