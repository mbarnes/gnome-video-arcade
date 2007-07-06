#include "gva-common.h"

#include <locale.h>
#include <stdlib.h>
#include <libintl.h>

#include "gva-db.h"
#include "gva-game-db.h"
#include "gva-main.h"
#include "gva-play-back.h"
#include "gva-preferences.h"
#include "gva-properties.h"
#include "gva-ui.h"
#include "gva-xmame.h"

void
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

        gtk_init_with_args (
                &argc, &argv, NULL, NULL, GETTEXT_PACKAGE, &error);
        if (error != NULL)
                g_error ("%s", error->message);
        
        gtk_window_set_default_icon_name (PACKAGE);

        if (!gva_db_init (&error))
                g_error ("%s", error->message);

        if (!gva_game_db_init (&error))
                g_error ("%s", error->message);

        gva_main_init ();
        gva_play_back_init ();
        gva_preferences_init ();
        gva_properties_init ();

        show_xmame_version ();

        gtk_main ();

        return EXIT_SUCCESS;
}
