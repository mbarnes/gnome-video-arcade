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

#ifndef PCL_COMPLEX_H
#define PCL_COMPLEX_H

#include "pcl-config.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclComplex PclComplex;
typedef struct _PclComplexClass PclComplexClass;
typedef struct _PclComplexValue PclComplexValue;

struct _PclComplexValue {
        gdouble real;   /* the real part */
        gdouble imag;   /* the imaginary part */
};

struct _PclComplex {
        PclObject parent;
        PclComplexValue v_complex;
};

struct _PclComplexClass {
        PclObjectClass parent_class;
};

GType           pcl_complex_get_type            (void);
PclObject *     pcl_complex_get_type_object     (void);
PclComplexValue pcl_complex_as_complex          (PclObject *object);
gdouble         pcl_complex_real_as_double      (PclObject *object);
gdouble         pcl_complex_imag_as_double      (PclObject *object);

PclObject *     pcl_complex_from_complex        (PclComplexValue v_complex);
PclObject *     pcl_complex_from_doubles        (gdouble real,
                                                 gdouble imag);
PclObject *     pcl_complex_from_string         (const gchar *v_string,
                                                 gchar **endptr);

/* Arithmetic Operations */
PclComplexValue pcl_complex_value_add           (PclComplexValue a,
                                                 PclComplexValue b);
PclComplexValue pcl_complex_value_sub           (PclComplexValue a,
                                                 PclComplexValue b);
PclComplexValue pcl_complex_value_mul           (PclComplexValue a,
                                                 PclComplexValue b);
PclComplexValue pcl_complex_value_div           (PclComplexValue a,
                                                 PclComplexValue b);
PclComplexValue pcl_complex_value_pow           (PclComplexValue a,
                                                 PclComplexValue b);
gdouble         pcl_complex_value_abs           (PclComplexValue a);

/* Standard GObject macros */
#define PCL_TYPE_COMPLEX \
        (pcl_complex_get_type ())
#define PCL_COMPLEX(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_COMPLEX, PclComplex))
#define PCL_COMPLEX_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_COMPLEX, PclComplexClass))
#define PCL_IS_COMPLEX(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_COMPLEX))
#define PCL_IS_COMPLEX_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_COMPLEX))
#define PCL_COMPLEX_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_COMPLEX, PclComplexClass))

/* Fast access macro (use carefully) */
#define PCL_COMPLEX_AS_COMPLEX(obj) \
        (PCL_COMPLEX (obj)->v_complex)
#define PCL_COMPLEX_REAL_AS_DOUBLE(obj) \
        (PCL_COMPLEX_AS_COMPLEX (obj).real)
#define PCL_COMPLEX_IMAG_AS_DOUBLE(obj) \
        (PCL_COMPLEX_AS_COMPLEX (obj).imag)

G_END_DECLS

#endif /* PCL_COMPLEX_H */
