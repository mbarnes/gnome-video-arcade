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

#ifndef PCL_SEQUENCE_H
#define PCL_SEQUENCE_H

#include "pcl-config.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclSequence PclSequence;
typedef struct _PclSequenceIface PclSequenceIface;

struct _PclSequenceIface {
        GTypeInterface parent_iface;

        PclObject *     (*concat)               (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*repeat)               (PclObject *object,
                                                 glong times);
        PclObject *     (*as_list)              (PclObject *object);
        PclObject *     (*as_tuple)             (PclObject *object);

        /* mutable type methods */
        PclObject *     (*in_place_concat)      (PclObject *object1,
                                                 PclObject *object2);
        PclObject *     (*in_place_repeat)      (PclObject *object1,
                                                 glong times);
};

GType           pcl_sequence_get_type           (void);
PclObject *     pcl_sequence_concat             (PclObject *object1,
                                                 PclObject *object2);
glong           pcl_sequence_count              (PclObject *object1,
                                                 PclObject *object2);
glong           pcl_sequence_index              (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_sequence_repeat             (PclObject *object,
                                                 glong times);
PclObject *     pcl_sequence_as_list            (PclObject *object);
PclObject *     pcl_sequence_as_tuple           (PclObject *object);
PclObject *     pcl_sequence_in_place_concat    (PclObject *object1,
                                                 PclObject *object2);
PclObject *     pcl_sequence_in_place_repeat    (PclObject *object,
                                                 glong times);
PclObject *     pcl_sequence_get_item           (PclObject *object,
                                                 glong index);

/* Standard GObject macros */
#define PCL_TYPE_SEQUENCE \
        (pcl_sequence_get_type ())
#define PCL_IS_SEQUENCE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_SEQUENCE))
#define PCL_SEQUENCE_GET_IFACE(obj) \
        (G_TYPE_INSTANCE_GET_INTERFACE \
        ((obj), PCL_TYPE_SEQUENCE, PclSequenceIface))

G_END_DECLS

#endif /* PCL_SEQUENCE_H */
