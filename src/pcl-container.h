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

#ifndef PCL_CONTAINER_H
#define PCL_CONTAINER_H

#include "pcl-config.h"
#include "pcl-mod-gc.h"
#include "pcl-object.h"

G_BEGIN_DECLS

/* A "container" in this context refers to a PCL object that contains
 * references to other PCL objects.  All subclasses should implement
 * GObject's dispose() method, as well as the traverse() method. */

typedef struct _PclContainer PclContainer;
typedef struct _PclContainerClass PclContainerClass;
typedef gboolean (*PclTraverseFunc) (gpointer, gpointer);

struct _PclContainer {
        PclObject parent;
        PclGCList gc;           /* Garbage collection hook */
};

struct _PclContainerClass {
        PclObjectClass parent_class;
        gboolean (*traverse) (PclContainer *container,
                              PclTraverseFunc func,
                              gpointer user_data);
};

GType           pcl_container_get_type          (void);
gboolean        pcl_object_traverse             (PclObject *object,
                                                 PclTraverseFunc func,
                                                 gpointer user_data);

/* Standard GObject macros */
#define PCL_TYPE_CONTAINER \
        (pcl_container_get_type ())
#define PCL_CONTAINER(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_CONTAINER, PclContainer))
#define PCL_CONTAINER_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_CONTAINER, PclContainerClass))
#define PCL_IS_CONTAINER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_CONTAINER))
#define PCL_IS_CONTAINER_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_CONTAINER))
#define PCL_CONTAINER_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_CONTAINER, PclContainerClass))

G_END_DECLS

#endif /* PCL_CONTAINER_H */
