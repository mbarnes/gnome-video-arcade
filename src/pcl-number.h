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

#ifndef PCL_NUMBER_H
#define PCL_NUMBER_H

#include "pcl-config.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclNumber PclNumber;
typedef struct _PclNumberIface PclNumberIface;

struct _PclNumberIface {
        GTypeInterface parent_iface;

        PclObject *     (*as_int)               (PclObject *object);
        PclObject *     (*as_float)             (PclObject *object);
        PclObject *     (*as_complex)           (PclObject *object);
        PclObject *     (*oct)                  (PclObject *object);
        PclObject *     (*hex)                  (PclObject *object);
        PclObject *     (*add)                  (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*subtract)             (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*multiply)             (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*divide)               (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*divmod)               (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*modulo)               (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*power)                (PclObject *object1,
                                                 PclObject *object2,
                                                 PclObject *object3);
        PclObject *     (*floordiv)             (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*negative)             (PclObject *object);
        PclObject *     (*positive)             (PclObject *object);
        PclObject *     (*absolute)             (PclObject *object);
        PclObject *     (*bitwise_or)           (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*bitwise_xor)          (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*bitwise_and)          (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*bitwise_lshift)       (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*bitwise_rshift)       (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*bitwise_invert)       (PclObject *object);
        gint            (*nonzero)              (PclObject *object);

        /* mutable type methods */
        PclObject *     (*in_place_add)         (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*in_place_subtract)    (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*in_place_multiply)    (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*in_place_divide)      (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*in_place_divmod)      (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*in_place_modulo)      (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*in_place_power)       (PclObject *object1,
                                                 PclObject *object2,
                                                 PclObject *object3);
        PclObject *     (*in_place_floordiv)    (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*in_place_or)          (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*in_place_xor)         (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*in_place_and)         (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*in_place_lshift)      (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*in_place_rshift)      (PclObject *object1,
                                                 PclObject *object2);
};

GType           pcl_number_get_type             (void);
PclObject *     pcl_number_as_int               (PclObject *object);
PclObject *     pcl_number_as_float             (PclObject *object);
PclObject *     pcl_number_as_complex           (PclObject *object);
PclObject *     pcl_number_oct                  (PclObject *object);
PclObject *     pcl_number_hex                  (PclObject *object);
PclObject *     pcl_number_add                  (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_subtract             (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_multiply             (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_divide               (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_divmod               (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_modulo               (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_power                (PclObject *object1,
                                                 PclObject *object2,
                                                 PclObject *object3);
PclObject *     pcl_number_floordiv             (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_negative             (PclObject *object);
PclObject *     pcl_number_positive             (PclObject *object);
PclObject *     pcl_number_absolute             (PclObject *object);
PclObject *     pcl_number_or                   (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_xor                  (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_and                  (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_lshift               (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_rshift               (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_invert               (PclObject *object);
gint            pcl_number_nonzero              (PclObject *object);
PclObject *     pcl_number_in_place_add         (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_in_place_subtract    (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_in_place_multiply    (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_in_place_divide      (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_in_place_modulo      (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_in_place_power       (PclObject *object1,
                                                 PclObject *object2,
                                                 PclObject *object3);
PclObject *     pcl_number_in_place_floordiv    (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_in_place_or          (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_in_place_xor         (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_in_place_and         (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_in_place_lshift      (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_number_in_place_rshift      (PclObject *object1,
                                                 PclObject *object2);

/* Standard GObject macros */
#define PCL_TYPE_NUMBER \
        (pcl_number_get_type ())
#define PCL_IS_NUMBER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_NUMBER))
#define PCL_NUMBER_GET_IFACE(obj) \
        (G_TYPE_INSTANCE_GET_INTERFACE \
        ((obj), PCL_TYPE_NUMBER, PclNumberIface))

G_END_DECLS

#endif /* PCL_NUMBER_H */
