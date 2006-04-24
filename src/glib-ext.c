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

#include "glib-ext.h"

/* helper for g_hash_table_remove_all and g_hash_table_steal_all */
gboolean
true_func (gpointer key, gpointer value, gpointer user_data)
{
        return TRUE;
}

/* g_hash_table_remove_all
 * (bugzilla.gnome.org bug #168538)
 * Note, this is a sub-optimal implementation, but we don't have access to
 * the necessary GHashTable internals. */
void
g_hash_table_remove_all (GHashTable *hash_table)
{
        g_hash_table_foreach_remove (hash_table, true_func, NULL);
}

/* g_hash_table_steal_all
 * (bugzilla.gnome.org bug #168538)
 * Note, this is a sub-optimal implementation, but we don't have access to
 * the necessary GHashTable internals. */
void
g_hash_table_steal_all (GHashTable *hash_table)
{
        g_hash_table_foreach_steal (hash_table, true_func, NULL);
}

#if GLIB_MINOR_VERSION < 10
/* g_intern_string
 * Slated for GLib 2.10 */
const gchar *
g_intern_string (const gchar *string)
{
        GQuark quark;

        if (string == NULL)
                return NULL;

        quark = g_quark_from_string (string);
        return g_quark_to_string (quark);
}

/* g_intern_static_string
 * Slated for GLib 2.10 */
const gchar *
g_intern_static_string (const gchar *string)
{
        GQuark quark;

        if (string == NULL)
                return NULL;

        quark = g_quark_from_static_string (string);
        return g_quark_to_string (quark);
}
#endif
