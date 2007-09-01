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

#include "gva-common.h"

#include <libintl.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "gva-db.h"
#include "gva-history.h"
#include "gva-main.h"
#include "gva-mame.h"
#include "gva-play-back.h"
#include "gva-preferences.h"
#include "gva-properties.h"
#include "gva-ui.h"

static GOptionEntry entries[] =
{
        { "build-database", 'b', 0,
          G_OPTION_ARG_NONE, &opt_build_database,
          N_("Build the games database"), NULL },

        { "version", 'v', 0,
          G_OPTION_ARG_NONE, &opt_version,
          N_("Show the application version"), NULL },

        { "which-emulator", 'w', 0,
          G_OPTION_ARG_NONE, &opt_which_emulator,
          N_("Show which emulator will be used"), NULL },

        { NULL }
};

static void
start (void)
{
        GvaProcess *process = NULL;
        gchar *mame_version;
        guint context_id;
        gint last_view;
        GError *error = NULL;

        context_id = gva_main_statusbar_get_context_id (G_STRFUNC);
        mame_version = gva_mame_get_version (&error);
        gva_error_handle (&error);

        if (mame_version != NULL)
        {
                gva_main_statusbar_push (context_id, "%s", mame_version);
                g_free (mame_version);
        }

        if (gva_db_needs_rebuilt ())
        {
                process = gva_main_build_database (&error);
                gva_error_handle (&error);
        }

        if (process != NULL)
        {
                gboolean main_loop_quit;

                while (!gva_process_has_exited (process, NULL))
                        main_loop_quit = gtk_main_iteration ();
                g_object_unref (process);

                if (main_loop_quit)
                        return;
        }

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
}

gint
main (gint argc, gchar **argv)
{
        GError *error = NULL;

        bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        textdomain (GETTEXT_PACKAGE);

        g_thread_init (NULL);

        gtk_init_with_args (
                &argc, &argv, NULL, entries, GETTEXT_PACKAGE, &error);
        if (error != NULL)
                g_error ("%s", error->message);

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

        gva_main_init ();
        gva_play_back_init ();
        gva_preferences_init ();
        gva_properties_init ();

#ifdef HISTORY_FILE
        gva_history_init (&error);
        gva_error_handle (&error);
#endif

        gtk_init_add ((GtkFunction) start, NULL);

        gtk_main ();

        return EXIT_SUCCESS;
}
