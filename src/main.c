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
#include "gva-game-db.h"
#include "gva-history.h"
#include "gva-main.h"
#include "gva-play-back.h"
#include "gva-preferences.h"
#include "gva-properties.h"
#include "gva-ui.h"
#include "gva-xmame.h"

static gboolean
database_needs_rebuilt (void)
{
        gchar *build;
        gchar *version;
        GError *error = NULL;

        gva_db_get_build (&build, &error);
        gva_error_handle (&error);

        version = gva_xmame_get_version (&error);
        gva_error_handle (&error);

        return (build == NULL) || (version == NULL) ||
                (strstr (version, build) == NULL);
}

static void
show_xmame_version (void)
{
        gchar *xmame_version;
        guint context_id;
        GError *error = NULL;

        context_id = gva_main_statusbar_get_context_id (G_STRFUNC);

        xmame_version = gva_xmame_get_version (&error);
        if (xmame_version != NULL)
        {
                gva_main_statusbar_push (context_id, "%s", xmame_version);
                g_free (xmame_version);
        }
        else
        {
                gva_main_statusbar_push (context_id, "%s", error->message);
                g_clear_error (&error);
        }
}

gint
main (gint argc, gchar **argv)
{
        GError *error = NULL;

        /* initialize locale data */
        setlocale (LC_ALL, "");
        bindtextdomain (PACKAGE, LOCALEDIR);
        textdomain (PACKAGE);

        g_thread_init (NULL);

        gtk_init_with_args (
                &argc, &argv, NULL, NULL, GETTEXT_PACKAGE, &error);
        if (error != NULL)
                g_error ("%s", error->message);
        
        gtk_window_set_default_icon_name (PACKAGE);

        if (!gva_db_init (&error))
                g_error ("%s", error->message);

        if (!gva_game_db_init (&error))
                g_error ("%s", error->message);

#ifdef HISTORY_FILE
        gva_history_init (&error);
        gva_error_handle (&error);
#endif

        gva_main_init ();
        gva_play_back_init ();
        gva_preferences_init ();
        gva_properties_init ();

        show_xmame_version ();

        gtk_main ();

        return EXIT_SUCCESS;
}
