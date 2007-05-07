#include "gva-actions.h"

#include "gva-main.h"
#include "gva-play-back.h"
#include "gva-ui.h"
#include "gva-util.h"
#include "gva-xmame.h"

/* About Dialog Information */
static const gchar *authors[] =
{
        "Matthew Barnes <mbarnes@redhat.com>",
        NULL
};
static const gchar *copyright = "Copyright \xC2\xA9 2007 Matthew Barnes";

static void
action_about_cb (GtkAction *action)
{
        gtk_show_about_dialog (
                NULL,  /* FIXME */
                "name", PACKAGE_NAME,
                "version", PACKAGE_VERSION,
                "comments", _("XMAME front-end"),
                "copyright", copyright,
                "authors", authors,
                "logo-icon-name", "gnome-joystick",
                NULL);
}

static void
action_contents_cb (GtkAction *action)
{
}

static void
action_play_back_cb (GtkAction *action)
{
        GError *error = NULL;

        if (!gva_play_back_run_dialog (&error))
        {
                g_warning ("%s", error->message);
                g_error_free (error);
        }
}

static void
action_properties_cb (GtkAction *action)
{
}

static void
action_quit_cb (GtkAction *action)
{
        gtk_main_quit ();
}

static void
action_record_cb (GtkAction *action)
{
        gchar *romname;
        gchar *inpname;
        GError *error = NULL;

        romname = gva_main_get_selected_game ();
        inpname = gva_choose_inpname (romname);
        gva_xmame_record_game (romname, inpname, &error);
        g_free (inpname);
        g_free (romname);

        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_error_free (error);
        }
}

static void
action_start_cb (GtkAction *action)
{
        gchar *romname;
        GError *error = NULL;

        romname = gva_main_get_selected_game ();
        gva_xmame_run_game (romname, &error);
        g_free (romname);

        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_error_free (error);
        }
}

static void
action_view_changed_cb (GtkRadioAction *action, GtkRadioAction *current)
{
        gva_main_set_view (gtk_radio_action_get_current_value (current));
}

static void
action_unmark_favorite_cb (GtkAction *action)
{
}

static GtkActionEntry entries[] =
{
        { "about",
          GTK_STOCK_ABOUT,
          N_("_About"),
          NULL,
          NULL,
          G_CALLBACK (action_about_cb) },

        { "contents",
          GTK_STOCK_HELP,
          N_("_Contents"),
          NULL,
          NULL,
          G_CALLBACK (action_contents_cb) },

        { "play-back",
          GTK_STOCK_MEDIA_PLAY,
          N_("Play _Back..."),
          NULL,
          N_("Play back a previously recorded game"),
          G_CALLBACK (action_play_back_cb) },

        { "properties",
          GTK_STOCK_PROPERTIES,
          N_("_Properties"),
          NULL,
          N_("Show properties about the selected game"),
          G_CALLBACK (action_properties_cb) },

        { "quit",
          GTK_STOCK_QUIT,
          N_("_Quit"),
          NULL,
          NULL,
          G_CALLBACK (action_quit_cb) },

        { "record",
          GTK_STOCK_MEDIA_RECORD,
          N_("_Record"),
          "<Control>r",
          N_("Start the selected game and record keypresses to a file"),
          G_CALLBACK (action_record_cb) },

        { "start",
          GTK_STOCK_EXECUTE,
          N_("_Start"),
          "<Control>s",
          N_("Start the selected game"),
          G_CALLBACK (action_start_cb) },

        { "game-menu",
          NULL,
          N_("_Game"),
          NULL,
          NULL,
          NULL },

        { "help-menu",
          NULL,
          N_("_Help"),
          NULL,
          NULL,
          NULL },

        { "view-menu",
          NULL,
          N_("_View"),
          NULL,
          NULL,
          NULL }
};

static GtkRadioActionEntry view_radio_entries[] =
{
        { "view-available",
          NULL,
          N_("_Available Games"),
          NULL,
          N_("Show all available games"),
          0 },

        { "view-favorites",
          NULL,
          N_("_Favorite Games"),
          NULL,
          N_("Only show my favorite games"),
          1 },

        { "view-results",
          NULL,
          N_("Search _Results"),
          NULL,
          N_("Show my search results"),
          2 }
};

GtkAction *
gva_get_action (const gchar *action_name)
{
        GtkActionGroup *action_group;

        g_return_if_fail (action_name != NULL);

        action_group = gva_get_action_group ();
        return gtk_action_group_get_action (action_group, action_name);
}

GtkActionGroup *
gva_get_action_group (void)
{
        static GtkActionGroup *action_group = NULL;

        if (G_UNLIKELY (action_group == NULL))
        {
                action_group = gtk_action_group_new ("main");
                gtk_action_group_add_actions (
                        action_group, entries, G_N_ELEMENTS (entries), NULL);
                gtk_action_group_add_radio_actions (
                        action_group, view_radio_entries,
                        G_N_ELEMENTS (view_radio_entries),
                        0, G_CALLBACK (action_view_changed_cb), NULL);
        }

        return action_group;
}
