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

#include "gva-util.h"

#include "gva-error.h"
#include "gva-mame.h"

#define GCONF_MONOSPACE_FONT_NAME_KEY \
        "/desktop/gnome/interface/monospace_font_name"

/* Command Line Options */
gboolean opt_build_database;
gboolean opt_which_emulator;

static gboolean
inpname_exists (const gchar *inppath, const gchar *inpname)
{
        gchar *inpfile;
        gchar *filename;
        gboolean exists;

        inpfile = g_strdup_printf ("%s.inp", inpname);
        filename = g_build_filename (inppath, inpfile, NULL);
        exists = g_file_test (filename, G_FILE_TEST_EXISTS);
        g_free (filename);
        g_free (inpfile);

        return exists;
}

gchar *
gva_choose_inpname (const gchar *romname)
{
        gchar *inpname;
        gchar *inppath;
        gint nn = 1;
        GError *error = NULL;

        g_return_val_if_fail (romname != NULL, NULL);

        inppath = gva_mame_get_config_value ("input_directory", &error);

        if (inppath == NULL || !inpname_exists (inppath, romname))
        {
                gva_error_handle (&error);
                inpname = g_strdup (romname);
        }
        else while (TRUE)
        {
                inpname = g_strdup_printf ("%s-%d", romname, nn++);
                if (!inpname_exists (inppath, inpname))
                        break;
                g_free (inpname);
        }

        g_free (inppath);

        return inpname;
}

gchar *
gva_find_data_file (const gchar *basename)
{
        const gchar * const *datadirs;
        gchar *filename;

        g_return_val_if_fail (basename != NULL, NULL);

        /* Support running directly from the source tree. */
        filename = g_build_filename ("..", "data", basename, NULL);
        if (g_file_test (filename, G_FILE_TEST_EXISTS))
                return filename;
        g_free (filename);

        datadirs = g_get_system_data_dirs ();
        while (*datadirs != NULL)
        {
                filename = g_build_filename (
                        *datadirs++, PACKAGE, basename, NULL);
                if (g_file_test (filename, G_FILE_TEST_EXISTS))
                        return filename;
                g_free (filename);
        }

        return NULL;
}

const gchar *
gva_get_last_version (void)
{
        static gchar *last_version = NULL;
        static gboolean first_time = TRUE;

        if (G_UNLIKELY (first_time))
        {
                GConfClient *client;
                GError *error = NULL;

                client = gconf_client_get_default ();

                last_version = gconf_client_get_string (
                        client, GVA_GCONF_VERSION_KEY, &error);
                gva_error_handle (&error);

                gconf_client_set_string (
                        client, GVA_GCONF_VERSION_KEY,
                        PACKAGE_VERSION, &error);
                gva_error_handle (&error);

                g_object_unref (client);

                first_time = FALSE;
        }

        return last_version;
}

gchar *
gva_get_monospace_font_name (void)
{
        GConfClient *client;
        PangoFontDescription *desc;
        gchar *font_name;
        GError *error = NULL;

        client = gconf_client_get_default ();
        font_name = gconf_client_get_string (
                client, GCONF_MONOSPACE_FONT_NAME_KEY, &error);
        gva_error_handle (&error);
        g_object_unref (client);

        return font_name;
}

void
gva_get_time_elapsed (GTimeVal *start_time,
                      GTimeVal *time_elapsed)
{
        g_return_if_fail (start_time != NULL);
        g_return_if_fail (time_elapsed != NULL);

        g_get_current_time (time_elapsed);
        time_elapsed->tv_sec -= start_time->tv_sec;
        g_time_val_add (time_elapsed, -start_time->tv_usec);
}
