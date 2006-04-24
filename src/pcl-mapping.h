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

#ifndef PCL_MAPPING_H
#define PCL_MAPPING_H

#include "pcl-config.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclMapping PclMapping;
typedef struct _PclMappingIface PclMappingIface;

typedef PclObject *     (*PclMappingKeys)       (PclMapping *mapping);
typedef PclObject *     (*PclMappingValues)     (PclMapping *mapping);
typedef PclObject *     (*PclMappingItems)      (PclMapping *mapping);
typedef gboolean        (*PclMappingClear)      (PclMapping *mapping);
typedef PclObject *     (*PclMappingPopItem)    (PclMapping *mapping);

struct _PclMappingIface {
        GTypeInterface parent_iface;
        PclMappingKeys keys;
        PclMappingValues values;
        PclMappingItems items;
        PclMappingClear clear;
        PclMappingPopItem pop_item;
};

GType           pcl_mapping_get_type            (void);
PclObject *     pcl_mapping_keys                (PclObject *object);
PclObject *     pcl_mapping_values              (PclObject *object);
PclObject *     pcl_mapping_items               (PclObject *object);
gboolean        pcl_mapping_clear               (PclObject *object);
PclObject *     pcl_mapping_pop_item            (PclObject *object);
gboolean        pcl_mapping_merge               (PclObject *object,
                                                 PclObject *from,
                                                 gboolean override);

#define pcl_mapping_update(object, from) \
        (pcl_mapping_merge ((object), (from), TRUE))

/* Standard GObject macros */
#define PCL_TYPE_MAPPING \
        (pcl_mapping_get_type ())
#define PCL_IS_MAPPING(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_MAPPING))
#define PCL_MAPPING_GET_IFACE(obj) \
        (G_TYPE_INSTANCE_GET_INTERFACE \
        ((obj), PCL_TYPE_MAPPING, PclMappingIface))

G_END_DECLS

#endif /* PCL_MAPPING_H */
