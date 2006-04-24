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

#ifndef PCL_ARG_H
#define PCL_ARG_H

#include "pcl-config.h"
#include "pcl-object.h"

G_BEGIN_DECLS

gboolean pcl_arg_parse_tuple                    (PclObject *args,
                                                 const gchar *format,
                                                 ...);
gboolean pcl_arg_va_parse_tuple                 (PclObject *args,
                                                 const gchar *format,
                                                 va_list va);
gboolean pcl_arg_parse_tuple_and_keywords       (PclObject *args,
                                                 PclObject *kwds,
                                                 const gchar *format,
                                                 const gchar **kwlist,
                                                 ...);
gboolean pcl_arg_va_parse_tuple_and_keywords    (PclObject *args,
                                                 PclObject *kwds,
                                                 const gchar *format,
                                                 const gchar **kwlist,
                                                 va_list va);
gboolean pcl_arg_unpack_tuple                   (PclObject *args,
                                                 const gchar *name,
                                                 glong min,
                                                 glong max,
                                                 ...);
gboolean pcl_arg_no_keywords                    (const gchar *name,
                                                 PclObject *kwds);

G_END_DECLS

#endif /* PCL_ARG_H */
