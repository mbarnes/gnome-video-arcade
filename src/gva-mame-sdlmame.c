/* Copyright 2007 Matthew Barnes
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

/* SDLMAME backend */

#include "gva-mame.h"

#include <string.h>

#include "gva-error.h"
#include "gva-mame-common.h"

gchar *
gva_mame_get_version (GError **error)
{
        gchar *version = NULL;
        gchar **lines;
        gchar *cp;

        /* Execute the command "${mame} -help". */
        if (gva_mame_command ("-help", &lines, NULL, error) != 0)
                return NULL;

        /* Output is as follows:
         *
         * M.A.M.E. v0.xxx (Mmm dd yyyy) - Multiple Arcade Machine Emulator
         * Copyright (x) 1997-2007 by Nicola Salmoria and the MAME Team
         * ...
         */

        if (lines[0] == NULL)
                goto exit;

        cp = strstr (lines[0], " - Multiple Arcade Machine Emulator");
        if (cp != NULL)
        {
                *cp = '\0';
                version = g_strdup (lines[0]);
        }

exit:
        if (version == NULL)
                g_set_error (
                        error, GVA_ERROR, GVA_ERROR_MAME,
                        _("Could not determine emulator version"));

        g_strfreev (lines);

        return version;
}

guint
gva_mame_get_total_supported (GError **error)
{
        gchar **lines;
        guint num_lines;

        /* Execute the command "${mame} -listfull". */
        if (gva_mame_command ("-listfull", &lines, NULL, error) != 0)
                return 0;

        /* Output is as follows:
         *
         * Name:     Description:
         * puckman   "PuckMan (Japan set 1, Probably Bootleg)"
         * puckmana  "PuckMan (Japan set 2)"
         * puckmanf  "PuckMan (Japan set 1 with speedup hack)"
         * ...
         */

        num_lines = g_strv_length (lines);
        g_strfreev (lines);

        /* Count the lines, excluding the header. */
        return (num_lines > 1) ? num_lines - 1 : 0;
}
