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

#ifndef PCL_SET_H
#define PCL_SET_H

#include "pcl-config.h"
#include "pcl-container.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclBaseSet PclBaseSet;
typedef struct _PclBaseSetClass PclBaseSetClass;

struct _PclBaseSet {
        PclContainer parent;
        PclObject *data;
};

struct _PclBaseSetClass {
        PclContainerClass parent_class;
};

GType           pcl_base_set_get_type           (void);

/* Standard GObject macros */
#define PCL_TYPE_BASE_SET \
        (pcl_base_set_get_type ())
#define PCL_BASE_SET(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_BASE_SET, PclBaseSet))
#define PCL_BASE_SET_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((obj), PCL_TYPE_BASE_SET, PclBaseSetClass))
#define PCL_IS_BASE_SET(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_BASE_SET))
#define PCL_IS_BASE_SET_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((obj), PCL_TYPE_BASE_SET))
#define PCL_BASE_SET_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_BASE_SET, PclBaseSetClass))

/*****************************************************************************/

typedef struct _PclSet PclSet;
typedef struct _PclSetClass PclSetClass;

struct _PclSet {
        PclBaseSet parent;
};

struct _PclSetClass {
        PclBaseSetClass parent_class;
};

GType           pcl_set_get_type                (void);
PclObject *     pcl_set_get_type_object         (void);
PclObject *     pcl_set_new                     (PclObject *iterable);

/* Standard GObject macros */
#define PCL_TYPE_SET \
        (pcl_set_get_type ())
#define PCL_SET(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_SET, PclSet))
#define PCL_SET_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_SET, PclSetClass))
#define PCL_IS_SET(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_SET))
#define PCL_IS_SET_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_SET))
#define PCL_SET_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_SET, PclSetClass))

/*****************************************************************************/

typedef struct _PclFrozenSet PclFrozenSet;
typedef struct _PclFrozenSetClass PclFrozenSetClass;

struct _PclFrozenSet {
        PclBaseSet parent;
        guint hash;
};

struct _PclFrozenSetClass {
        PclBaseSetClass parent_class;
};

GType           pcl_frozen_set_get_type         (void);
PclObject *     pcl_frozen_set_get_type_object  (void);
PclObject *     pcl_frozen_set_new              (PclObject *iterable);

/* Standard GObject macros */
#define PCL_TYPE_FROZEN_SET \
        (pcl_frozen_set_get_type ())
#define PCL_FROZEN_SET(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_FROZEN_SET, PclFrozenSet))
#define PCL_FROZEN_SET_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_FROZEN_SET, PclFrozenSetClass))
#define PCL_IS_FROZEN_SET(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_FROZEN_SET))
#define PCL_IS_FROZEN_SET_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_FROZEN_SET))
#define PCL_FROZEN_SET_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_FROZEN_SET, PclFrozenSetClass))

G_END_DECLS

#endif /* PCL_SET_H */
