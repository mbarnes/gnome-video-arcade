/* Copyright 2007, 2008 Matthew Barnes
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

/* XMAME backend */

#include "gva-mame.h"

#include <string.h>

#include "gva-error.h"
#include "gva-mame-common.h"

const gchar *
gva_mame_get_path_sep (void)
{
        return ":";  /* XMAME uses UNIX-style search paths */
}

gchar *
gva_mame_get_version (GError **error)
{
        gchar *version = NULL;
        gchar **lines;

        /* Execute the command "${mame} -version". */
        if (gva_mame_command ("-version", &lines, NULL, error) != 0)
                return NULL;

        /* Output is as follows:
         *
         * xmame (backend) version 0.xxx (Mmm dd yyyy)
         */

        if (lines == NULL || lines[0] == NULL)
                goto exit;
        if (strstr (lines[0], "xmame") == NULL)
                goto exit;
        if (strstr (lines[0], "version") == NULL)
                goto exit;

        version = g_strdup (g_strstrip (lines[0]));

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
         * name      description
         * --------  -----------
         * puckman   "PuckMan (Japan set 1, Probably Bootleg)"
         * puckmana  "PuckMan (Japan set 2)"
         * puckmanf  "PuckMan (Japan set 1 with speedup hack)"
         * ...
         *
         *
         * Total Supported: xxxx
         */

        num_lines = (lines != NULL) ? g_strv_length (lines) : 0;
        g_strfreev (lines);

        /* Count the lines, excluding the header and footer. */
        return (num_lines > 4) ? num_lines - 4 : 0;
}
