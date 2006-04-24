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

#ifndef PCL_CELL_H
#define PCL_CELL_H

#include "pcl-config.h"
#include "pcl-container.h"

G_BEGIN_DECLS

typedef struct _PclCell PclCell;
typedef struct _PclCellClass PclCellClass;

struct _PclCell {
        PclContainer parent;
        PclObject *reference;
};

struct _PclCellClass {
        PclContainerClass parent_class;
};

GType           pcl_cell_get_type               (void);
PclObject *     pcl_cell_get_type_object        (void);
PclObject *     pcl_cell_new                    (PclObject *reference);
PclObject *     pcl_cell_get                    (PclObject *self);
gboolean        pcl_cell_set                    (PclObject *self,
                                                 PclObject *reference);

/* Standard GObject macros */
#define PCL_TYPE_CELL \
        (pcl_cell_get_type ())
#define PCL_CELL(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_CELL, PclCell))
#define PCL_CELL_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_CELL, PclCellClass))
#define PCL_IS_CELL(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_CELL))
#define PCL_IS_CELL_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_CELL))
#define PCL_CELL_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_CELL, PclCellClass))

G_END_DECLS

#endif /* PCL_CELL_H */
