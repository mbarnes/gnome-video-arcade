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

#ifndef PCL_ITERATOR_H
#define PCL_ITERATOR_H

#include "pcl-config.h"
#include "pcl-container.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclIterator PclIterator;
typedef struct _PclIteratorClass PclIteratorClass;

struct _PclIterator {
        PclContainer parent;
};

struct _PclIteratorClass {
        PclContainerClass parent_class;

        PclObject *     (*next)         (PclIterator *iterator);
        gboolean        (*stop)         (PclIterator *iterator);
};

GType           pcl_iterator_get_type           (void);
PclObject *     pcl_iterator_next               (PclObject *object);
gboolean        pcl_iterator_stop               (PclObject *object);

/* Standard GObject macros */
#define PCL_TYPE_ITERATOR \
        (pcl_iterator_get_type ())
#define PCL_ITERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_ITERATOR, PclIterator))
#define PCL_ITERATOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_ITERATOR, PclIteratorClass))
#define PCL_IS_ITERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_ITERATOR))
#define PCL_IS_ITERATOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_ITERATOR))
#define PCL_ITERATOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_ITERATOR, PclIteratorClass))

G_END_DECLS

#endif /* PCL_ITERATOR_H */
