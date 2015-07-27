/* Copyright 2007-2015 Matthew Barnes
 *
 * This file is part of GNOME Video Arcade.
 *
 * GNOME Video Arcade is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * GNOME Video Arcade is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gva-error.h"

GQuark
gva_error_quark (void)
{
        static GQuark quark = 0;

        if (G_UNLIKELY (quark == 0))
                quark = g_quark_from_static_string ("gva-error-quark");

        return quark;
}

GQuark
gva_sqlite_error_quark (void)
{
        static GQuark quark = 0;

        if (G_UNLIKELY (quark == 0))
                quark = g_quark_from_static_string ("gva-sqlite-error-quark");

        return quark;
}

/**
 * gva_error_handle:
 * @error: a #GError
 *
 * If @error has been set, issues a warning message to standard error.
 * Otherwise the function does nothing.
 **/
void
gva_error_handle (GError **error)
{
        g_return_if_fail (error != NULL);

        if (*error != NULL)
        {
                g_warning ("%s", (*error)->message);
                g_clear_error (error);
        }
}
