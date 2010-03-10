/* Copyright 2007 - 2010 Matthew Barnes
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

#include "gva-common.h"

#include <errno.h>
#include <libintl.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "gva-audit.h"
#include "gva-categories.h"
#include "gva-db.h"
#include "gva-error.h"
#include "gva-history.h"
#include "gva-main.h"
#include "gva-mame.h"
#include "gva-nplayers.h"
#include "gva-play-back.h"
#include "gva-preferences.h"
#include "gva-properties.h"
#include "gva-tree-view.h"
#include "gva-ui.h"
#include "gva-util.h"

#ifdef HAVE_DBUS
#include "gva-dbus.h"
#endif

#define SQL_COUNT_ROMS \
        "SELECT count(*) FROM game WHERE " \
        "romset NOTNULL AND romset != 'not found' " \
        "AND isbios != 'yes'"

static GOptionEntry entries[] =
{
        { "build-database", 'b', 0,
          G_OPTION_ARG_NONE, &opt_build_database,
          N_("Build the games database"), NULL },

        { "inspect", 'i', 0,
          G_OPTION_ARG_STRING, &opt_inspect,
          N_("Inspect an emulator setting"),
          N_("NAME") },

        { "version", 'v', 0,
          G_OPTION_ARG_NONE, &opt_version,
          N_("Show the application version"), NULL },

        { "which-emulator", 'w', 0,
          G_OPTION_ARG_NONE, &opt_which_emulator,
          N_("Show which emulator will be used"), NULL },

        { NULL }
};

static void
warn_if_no_roms (void)
{
        const gchar *sql = SQL_COUNT_ROMS;
        GtkWidget *dialog;
        gchar **result;
        glong n_roms;
        GError *error = NULL;

        if (!gva_db_get_table (sql, &result, NULL, NULL, &error))
        {
                gva_error_handle (&error);
                return;
        }

        errno = 0;
        g_return_if_fail (g_strv_length (result) > 1);
        n_roms = strtol (result[1], NULL, 10);
        g_strfreev (result);

        if (errno != 0)
        {
                g_warning ("%s", g_strerror (errno));
                return;
        }

        if (n_roms > 0)
                return;

        dialog = gtk_message_dialog_new_with_markup (
                GTK_WINDOW (GVA_WIDGET_MAIN_WINDOW),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE,
                "<big><b>%s</b></big>",
                _("No ROM files found"));

        gtk_message_dialog_format_secondary_markup (
                GTK_MESSAGE_DIALOG (dialog),
                _("GNOME Video Arcade was unable to locate any ROM files. "
                  "It could be that MAME is misconfigured or that no ROM "
                  "files are installed. Click <b>Help</b> for more details "
                  "and troubleshooting tips."));

        gtk_dialog_add_button (
                GTK_DIALOG (dialog), GTK_STOCK_HELP, GTK_RESPONSE_HELP);

        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_HELP)
                gva_help_display (GTK_WINDOW (dialog), "troubleshooting");

        gtk_widget_destroy (dialog);
}

static void
start (void)
{
        GError *error = NULL;

        if (gva_db_needs_rebuilt ())
        {
                gva_main_progress_bar_show ();

                if (!gva_main_build_database (&error))
                {
                        gva_error_handle (&error);
                        return;
                }

                gva_main_progress_bar_set_fraction (0.0);

                if (!gva_main_analyze_roms (&error))
                {
                        gva_error_handle (&error);
                        return;
                }

                gva_main_progress_bar_hide ();
        }
        else if (gva_audit_detect_changes ())
        {
                gva_main_progress_bar_show ();

                if (!gva_main_analyze_roms (&error))
                {
                        gva_error_handle (&error);
                        return;
                }

                gva_main_progress_bar_hide ();
        }

        /* Do this after ROMs are analyzed. */
        if (!gva_main_init_search_completion (&error))
        {
                gva_error_handle (&error);
                return;
        }

        gtk_action_set_sensitive (GVA_ACTION_SEARCH, TRUE);
        gtk_action_set_sensitive (GVA_ACTION_SHOW_CLONES, TRUE);
        gtk_action_set_sensitive (GVA_ACTION_VIEW_AVAILABLE, TRUE);
        gtk_action_set_sensitive (GVA_ACTION_VIEW_FAVORITES, TRUE);
        gtk_action_set_sensitive (GVA_ACTION_VIEW_RESULTS, TRUE);

        gconf_bridge_bind_property (
                gconf_bridge_get (), GVA_GCONF_SELECTED_VIEW_KEY,
                G_OBJECT (GVA_ACTION_VIEW_AVAILABLE), "current-value");

        /* Force a tree view update. */
        if (gva_tree_view_get_selected_view () == 0)
        {
                gva_tree_view_update (&error);
                gva_error_handle (&error);
        }

        /* Present a helpful dialog if no ROMs were found. */
        warn_if_no_roms ();
}

gint
main (gint argc, gchar **argv)
{
        gchar *path;
        GError *error = NULL;

        g_thread_init (NULL);

        bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        textdomain (GETTEXT_PACKAGE);

        gtk_init_with_args (
                &argc, &argv, NULL, entries, GETTEXT_PACKAGE, &error);
        if (error != NULL)
                g_error ("%s", error->message);

#ifdef HAVE_GSTREAMER
        if (!gst_init_check (&argc, &argv, &error))
                g_error ("%s", error->message);
#endif

        /* This installs handlers for our custom debug log levels. */
        gva_get_debug_flags ();

        /* Change the working directory to that of the MAME executable.
         * Why?  Because SDLMAME's default configuration uses relative
         * search paths such as "rompath = roms".  The paths are relative
         * to the directory containing the MAME executable, so we must run
         * from that directory in order for SDLMAME's default configuration
         * to work.  Annoying, but a common problem for users. */
        path = g_path_get_dirname (MAME_PROGRAM);
        g_chdir (path);
        g_free (path);

        if (opt_inspect != NULL)
        {
                gchar *value;

                value = gva_mame_get_config_value (opt_inspect, &error);
                if (value != NULL)
                {
                        g_print ("%s\n", value);
                        g_free (value);
                }
                else
                {
                        g_printerr ("%s\n", error->message);
                        g_clear_error (&error);
                }
                exit (0);
        }

        if (opt_version)
        {
                g_print ("%s\n", PACKAGE_STRING);
                exit (0);
        }

        if (opt_which_emulator)
        {
                g_print ("%s\n", MAME_PROGRAM);
                exit (0);
        }

        gtk_window_set_default_icon_name (PACKAGE);

        if (!gva_db_init (&error))
                g_error ("%s", error->message);

        gva_audit_init ();
        gva_main_init ();
        gva_play_back_init ();
        gva_preferences_init ();
        gva_properties_init ();

        gva_categories_init (&error);
        gva_error_handle (&error);

        gva_history_init (&error);
        gva_error_handle (&error);

        gva_nplayers_init (&error);
        gva_error_handle (&error);

#ifdef HAVE_DBUS
        gva_dbus_init (&error);
        gva_error_handle (&error);
#endif

        gtk_init_add ((GtkFunction) start, NULL);

        gtk_main ();

        return EXIT_SUCCESS;
}
