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

#include "gva-util.h"

#include <errno.h>

#include "gva-error.h"
#include "gva-mame.h"

#ifdef WITH_GNOME
#include <gnome.h>
#endif

#define GCONF_MONOSPACE_FONT_NAME_KEY \
        "/desktop/gnome/interface/monospace_font_name"

/* Command Line Options */
gboolean opt_build_database;
gchar *opt_inspect;
gboolean opt_version;
gboolean opt_which_emulator;

static gboolean
inpname_exists (const gchar *inppath, const gchar *inpname)
{
        gchar *inpfile;
        gchar *filename;
        gboolean exists;

        /* Last version of xmame creates input files named "game.inp",
         * but sdlmame names them "game".  Not sure when the extension
         * was dropped (tested versions 0.120 - 0.123).  In any case,
         * we need to check for both. */

        inpfile = g_strdup_printf ("%s", inpname);
        filename = g_build_filename (inppath, inpfile, NULL);
        exists = g_file_test (filename, G_FILE_TEST_EXISTS);
        g_free (filename);
        g_free (inpfile);

        if (exists)
                return TRUE;

        inpfile = g_strdup_printf ("%s.inp", inpname);
        filename = g_build_filename (inppath, inpfile, NULL);
        exists = g_file_test (filename, G_FILE_TEST_EXISTS);
        g_free (filename);
        g_free (inpfile);

        return exists;
}

/**
 * gva_choose_inpname:
 * @game: the name of a game
 *
 * Returns the name of a MAME input file for @game that does not already
 * exist.  For example, given the game "pacman" the function will return
 * the first filename that does not already exist: "pacman.inp",
 * "pacman-1.inp", "pacman-2.inp", etc.
 *
 * Returns: a newly-allocated input filename for @game
 **/
gchar *
gva_choose_inpname (const gchar *game)
{
        gchar *inpname;
        gchar *inppath;
        gint nn = 1;
        GError *error = NULL;

        g_return_val_if_fail (game != NULL, NULL);

        inppath = gva_mame_get_config_value ("input_directory", &error);

        if (inppath == NULL || !inpname_exists (inppath, game))
        {
                gva_error_handle (&error);
                inpname = g_strdup (game);
        }
        else while (TRUE)
        {
                inpname = g_strdup_printf ("%s-%d", game, nn++);
                if (!inpname_exists (inppath, inpname))
                        break;
                g_free (inpname);
        }

        g_free (inppath);

        return inpname;
}

/**
 * gva_find_data_file:
 * @basename: the base name of the file to search for
 *
 * Searches for a file named @basename in a number of standard system-wide
 * directories and returns a newly-allocated string containing the path to
 * the first match.  The string should be freed with g_free().  If no match
 * is found the function returns %NULL.
 *
 * Returns: the pathname of the first match, or %NULL if no match was
 *          found
 **/
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

/**
 * gva_get_debug_flags:
 *
 * Returns a set of bit flags indicating what kinds of debugging messages
 * are enabled through the GVA_DEBUG environment variable.
 *
 * Returns: bitwise combination of #GvaDebugFlags values
 **/
GvaDebugFlags
gva_get_debug_flags (void)
{
        static guint flags = G_MAXUINT;

        if (G_UNLIKELY (flags == G_MAXUINT))
        {
                static const GDebugKey debug_keys[] =
                {
                        { "mame",  GVA_DEBUG_MAME },
                        { "sql",   GVA_DEBUG_SQL }
                };

                const gchar *env = g_getenv ("GVA_DEBUG");

                flags = g_parse_debug_string (
                        (env != NULL) ? env : "", debug_keys,
                        G_N_ELEMENTS (debug_keys));
        }

        return (GvaDebugFlags) flags;
}

/**
 * gva_get_last_version:
 *
 * Returns the most recently run version of
 * <emphasis>GNOME Video Arcade</emphasis> prior to the current run.  This
 * is used to detect <emphasis>GNOME Video Arcade</emphasis> upgrades.
 *
 * Returns: the most recently run version of
 *          <emphasis>GNOME Video Arcade</emphasis>
 **/
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

/**
 * gva_get_monospace_font_name:
 *
 * Returns the user's preferred fixed-width font name.  The return value
 * is a newly-allocated string and should be freed with g_free().
 *
 * Returns: the name of a fixed-width font
 */
gchar *
gva_get_monospace_font_name (void)
{
        GConfClient *client;
        gchar *font_name;
        GError *error = NULL;

        client = gconf_client_get_default ();
        font_name = gconf_client_get_string (
                client, GCONF_MONOSPACE_FONT_NAME_KEY, &error);
        gva_error_handle (&error);
        g_object_unref (client);

        return font_name;
}

/**
 * gva_get_time_elapsed:
 * @start_time: a start time
 * @time_elapsed: location to put the time elasped
 *
 * Writes the time elapsed since @start_time to @time_elapsed.
 * Set the start time by calling g_get_current_time().
 **/
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

/**
 * gva_get_user_data_dir:
 *
 * Returns the directory where user-specific application data is stored.
 * The function also creates the directory the first time it is called.
 *
 * Returns: user-specific application data directory
 **/
const gchar *
gva_get_user_data_dir (void)
{
        static gchar *user_data_dir = NULL;

        if (G_UNLIKELY (user_data_dir == NULL))
        {
                user_data_dir = g_build_filename (
                        g_get_user_data_dir (),
                        "applications", PACKAGE, NULL);

                if (g_mkdir_with_parents (user_data_dir, 0777) < 0)
                        g_warning (
                                "Unable to create %s: %s",
                                user_data_dir, g_strerror (errno));
        }

        return user_data_dir;
}

/**
 * gva_help_display:
 * @parent: a parent #GtkWindow or %NULL
 * @link_id: help section to present or %NULL
 *
 * Opens the user documentation to the section given by @link_id, or to the
 * table of contents if @link_id is %NULL.  If the user documentation cannot
 * be opened, it presents a dialog describing the error.  The dialog is set
 * as transient to @parent if @parent is %NULL.
 **/
void
gva_help_display (GtkWindow *parent,
                  const gchar *link_id)
{
#ifdef WITH_GNOME
        GtkWidget *dialog;
        GError *error = NULL;

        if (gnome_help_display ("gnome-video-arcade.xml", link_id, &error))
                return;

        dialog = gtk_message_dialog_new_with_markup (
                parent, GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                "<big><b>%s</b></big>",
                _(PACKAGE_NAME " could not display the help contents."));

        gtk_message_dialog_format_secondary_text (
                GTK_MESSAGE_DIALOG (dialog), "%s", error->message);

        gtk_dialog_run (GTK_DIALOG (dialog));

        gtk_widget_destroy (dialog);
        g_error_free (error);
#endif
}
