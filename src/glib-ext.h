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

#ifndef GLIB_EXT_H
#define GLIB_EXT_H

#include <glib.h>

G_BEGIN_DECLS

void            g_hash_table_remove_all         (GHashTable *hash_table);
void            g_hash_table_steal_all          (GHashTable *hash_table);

#if GLIB_MINOR_VERSION < 10
const gchar *   g_intern_string                 (const gchar *string);
const gchar *   g_intern_static_string          (const gchar *string);
#endif

G_END_DECLS

#endif /* GLIB_EXT_H */
