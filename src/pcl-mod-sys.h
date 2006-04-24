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

#ifndef PCL_MOD_SYS_H
#define PCL_MOD_SYS_H

#include "pcl-config.h"
#include "pcl-object.h"

G_BEGIN_DECLS

PclObject *     pcl_sys_get_object              (const gchar *name);
gboolean        pcl_sys_set_object              (const gchar *name,
                                                 PclObject *object);
void            pcl_sys_set_argv                (gint argc,
                                                 gchar **argv);
void            pcl_sys_set_path                (const gchar *path);
void            pcl_sys_write_stdout            (const gchar *format, ...);
void            pcl_sys_write_stderr            (const gchar *format, ...);

G_END_DECLS

#endif /* PCL_MOD_SYS_H */
