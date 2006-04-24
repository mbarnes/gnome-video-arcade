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

#ifndef PCL_WEAK_REF_H
#define PCL_WEAK_REF_H

#include "pcl-config.h"
#include "pcl-container.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclWeakRef PclWeakRef;
typedef struct _PclWeakRefClass PclWeakRefClass;

struct _PclWeakRef {
        PclContainer parent;
        PclObject *object;  /* weakref */
        PclObject *callback;
        guint hash;
};

struct _PclWeakRefClass {
        PclContainerClass parent_class;
};

GType           pcl_weak_ref_get_type           (void);
PclObject *     pcl_weak_ref_get_type_object    (void);
PclObject *     pcl_weak_ref_new                (PclObject *object,
                                                 PclObject *callback);

/* Standard GObject macros */
#define PCL_TYPE_WEAK_REF \
        (pcl_weak_ref_get_type ())
#define PCL_WEAK_REF(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_WEAK_REF, PclWeakRef))
#define PCL_WEAK_REF_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_WEAK_REF, PclWeakRefClass))
#define PCL_IS_WEAK_REF(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_WEAK_REF))
#define PCL_IS_WEAK_REF_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_WEAK_REF))
#define PCL_WEAK_REF_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_WEAK_REF, PclWeakRefClass))

/* Fast access macros (use carefully) */
#define PCL_WEAK_REF_GET_OBJECT(obj) \
        (PCL_WEAK_REF (obj)->object)
#define PCL_WEAK_REF_GET_CALLBACK(obj) \
        (PCL_WEAK_REF (obj)->callback)

G_END_DECLS

#endif /* PCL_WEAK_REF_H */
