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

#ifndef PCL_TABLE_H
#define PCL_TABLE_H

#include "pcl-config.h"
#include "pcl-container.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclTable PclTable;
typedef struct _PclTableClass PclTableClass;

struct _PclTable {
        PclContainer parent;
        PclObject *key_fields;
        glong num_keys;
        PclObject *all_fields;
        glong num_fields;
        PclObject *records;
        PclObject *lookups;
};

struct _PclTableClass {
        PclContainerClass parent_class;
};

GType           pcl_table_get_type              (void);
PclObject *     pcl_table_get_type_object       (void);
PclObject *     pcl_table_new                   (PclObject *keys,
                                                 PclObject *data);

/* Standard GObject macros */
#define PCL_TYPE_TABLE \
        (pcl_table_get_type ())
#define PCL_TABLE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_TABLE, PclTable))
#define PCL_TABLE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_TABLE, PclTableClass))
#define PCL_IS_TABLE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_TABLE))
#define PCL_IS_TABLE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_TABLE))
#define PCL_TABLE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_TABLE, PclTableClass))

G_END_DECLS

#endif /* PCL_TABLE_H */
