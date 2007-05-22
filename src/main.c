#include "gva-common.h"

#include <locale.h>
#include <stdlib.h>
#include <libintl.h>

#include "gva-game-db.h"
#include "gva-main.h"
#include "gva-play-back.h"
#include "gva-ui.h"
#include "gva-xmame.h"

void
show_xmame_version (void)
{
        GtkStatusbar *statusbar;
        gchar *xmame_version;
        guint context_id;
        GError *error = NULL;

        statusbar = GTK_STATUSBAR (GVA_WIDGET_MAIN_STATUSBAR);
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

        gtk_init (&argc, &argv);

        if (!gva_game_db_init (&error))
                g_error ("%s", error->message);

        if (!gva_game_db_update_samples (&error)) {
                g_warning ("%s", error->message);
                g_error_free (error);
        }

        gva_main_init ();
        gva_play_back_init ();

        show_xmame_version ();

        gtk_main ();

        return EXIT_SUCCESS;
}
