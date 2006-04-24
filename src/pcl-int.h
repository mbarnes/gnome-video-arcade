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

#ifndef PCL_INT_H
#define PCL_INT_H

#include "pcl-config.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclInt PclInt;
typedef struct _PclIntClass PclIntClass;

struct _PclInt {
        PclObject parent;
        glong v_long;
};

struct _PclIntClass {
        PclObjectClass parent_class;
};

GType           pcl_int_get_type                (void);
PclObject *     pcl_int_get_type_object         (void);
glong           pcl_int_as_long                 (PclObject *object);
PclObject *     pcl_int_from_long               (glong v_long);
PclObject *     pcl_int_from_string             (const gchar *v_string,
                                                 gchar **endptr,
                                                 guint base);

/* Standard GObject macros */
#define PCL_TYPE_INT \
        (pcl_int_get_type ())
#define PCL_INT(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_INT, PclInt))
#define PCL_INT_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_INT, PclIntClass))
#define PCL_IS_INT(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_INT))
#define PCL_IS_INT_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_INT))
#define PCL_INT_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_INT, PclIntClass))

/* Fast access macro (use carefully) */
#define PCL_INT_AS_LONG(obj) \
        (PCL_INT (obj)->v_long)

G_END_DECLS

#endif /* PCL_INT_H */
