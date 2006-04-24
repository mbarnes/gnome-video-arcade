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

#ifndef PCL_SEQUENCE_ITERATOR_H
#define PCL_SEQUENCE_ITERATOR_H

#include "pcl-config.h"
#include "pcl-iterator.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclSequenceIterator PclSequenceIterator;
typedef struct _PclSequenceIteratorClass PclSequenceIteratorClass;

struct _PclSequenceIterator {
        PclIterator parent;
        glong index;
        PclObject *sequence;
};

struct _PclSequenceIteratorClass {
        PclIteratorClass parent_class;
};

GType           pcl_sequence_iterator_get_type          (void);
PclObject *     pcl_sequence_iterator_get_type_object   (void);
PclObject *     pcl_sequence_iterator_new               (PclObject *sequence);

/* Standard GObject macros */
#define PCL_TYPE_SEQUENCE_ITERATOR \
        (pcl_sequence_iterator_get_type ())
#define PCL_SEQUENCE_ITERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_SEQUENCE_ITERATOR, PclSequenceIterator))
#define PCL_SEQUENCE_ITERATOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_SEQUENCE_ITERATOR, PclSequenceIteratorClass))
#define PCL_IS_SEQUENCE_ITERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_SEQUENCE_ITERATOR))
#define PCL_IS_SEQUENCE_ITERATOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_SEQUENCE_ITERATOR))
#define PCL_SEQUENCE_ITERATOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_SEQUENCE_ITERATOR, PclSequenceIteratorClass))

G_END_DECLS

#endif /* PCL_SEQUENCE_ITERATOR_H */
