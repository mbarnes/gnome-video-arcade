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

#ifndef PCL_UTIL_H
#define PCL_UTIL_H

#include "pcl-config.h"
#include "pcl-object.h"

G_BEGIN_DECLS

GQuark          pcl_data_getsets_quark          (void);
GQuark          pcl_data_members_quark          (void);
GQuark          pcl_data_methods_quark          (void);

gint            pcl_repr_enter                  (PclObject *object);
void            pcl_repr_leave                  (PclObject *object);

gboolean        pcl_flush_line                  (void);
guint           pcl_hash_double                 (gdouble v_double);
FILE *          pcl_open_source                 (const gchar *filename);
PclObject *     pcl_build_value                 (const gchar *format, ...);
PclObject *     pcl_va_build_value              (const gchar *format,
                                                 va_list va);

G_END_DECLS

#endif /* PCL_UTIL_H */
