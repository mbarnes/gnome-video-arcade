#include "gva-common.h"

#include <stdlib.h>
#include <libintl.h>

#include "gva-game-store.h"
#include "gva-ui.h"
#include "gva-xmame.h"

void
show_xmame_version (void)
{
        GtkStatusbar *statusbar;
        gchar *xmame_version;
        guint context_id;
        GError *error = NULL;

        statusbar = gva_ui_get_statusbar ();
        context_id = gtk_statusbar_get_context_id (statusbar, G_STRFUNC);

        xmame_version = gva_xmame_get_version (&error);
        if (xmame_version != NULL)
        {
                gtk_statusbar_push (statusbar, context_id, xmame_version);
                g_free (xmame_version);
        }
        else
        {
                gtk_statusbar_push (statusbar, context_id, error->message);
                g_error_free (error);
        }
}

gint
main (gint argc, gchar **argv)
{
        GvaGameStore *game_store;
        GError *error = NULL;

        /* initialize locale data */
        setlocale (LC_ALL, "");
        bindtextdomain (PACKAGE, LOCALEDIR);
        textdomain (PACKAGE);

        gtk_init (&argc, &argv);

        game_store = gva_game_store_new ();
        if (!gva_game_store_load_games (game_store, &error))
        {
                g_error ("%s", error->message);
                g_error_free (error);
        }

        gva_ui_init (game_store);

        show_xmame_version ();

        gtk_main ();

        g_object_unref (game_store);

        return EXIT_SUCCESS;
}
