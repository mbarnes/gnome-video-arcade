#include "gva-ui.h"

#include <glade/glade.h>

#include "gva-game-db.h"
#include "gva-favorites.h"
#include "gva-game-store.h"
#include "gva-play-back.h"
#include "gva-tree-view.h"
#include "gva-util.h"
#include "gva-xmame.h"

static GladeXML *xml = NULL;
static GtkUIManager *manager = NULL;
static GtkActionGroup *action_group = NULL;
static gboolean initialized = FALSE;

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
        GdkPixbuf *logo;
        GError *error = NULL;

        logo = gtk_icon_theme_load_icon (
                gtk_icon_theme_get_default (),
                PACKAGE, 128, 0, &error);
        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_error_free (error);
        }

        gtk_show_about_dialog (
                GTK_WINDOW (GVA_WIDGET_MAIN_WINDOW),
                "name", PACKAGE_NAME,
                "version", PACKAGE_VERSION,
                "comments", _("XMAME Front-End"),
                "copyright", copyright,
                "authors", authors,
                "translator-credits", _("translator-credits"),
                "logo", logo,
                NULL);

        if (logo != NULL)
                g_object_unref (logo);
}

static void
action_contents_cb (GtkAction *action)
{
}

static void
action_insert_favorite_cb (GtkAction *action)
{
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;
        const gchar *romname;
        gboolean valid;

        romname = gva_tree_view_get_selected_game ();
        g_assert (romname != NULL);

        model = gva_game_db_get_model ();
        path = gva_game_db_lookup (romname);
        valid = gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_path_free (path);
        g_assert (valid);

        gtk_list_store_set (
                GTK_LIST_STORE (model), &iter,
                GVA_GAME_STORE_COLUMN_FAVORITE, TRUE, -1);

        gva_favorites_insert (romname);

        gtk_action_set_visible (GVA_ACTION_INSERT_FAVORITE, FALSE);
        gtk_action_set_visible (GVA_ACTION_REMOVE_FAVORITE, TRUE);
}

static void
action_play_back_cb (GtkAction *action)
{
        GtkTreeModel *model;
        GtkTreeView *view;
        GtkTreeIter iter;
        gchar *inpname;
        gchar *inpfile;
        GList *list;
        gboolean iter_set;
        GError *error = NULL;

        view = GTK_TREE_VIEW (GVA_WIDGET_PLAY_BACK_TREE_VIEW);

        /* We have to use gtk_tree_selection_get_selected_rows() instead of
         * gtk_tree_selection_get_selected() because the selection mode is
         * GTK_SELECTION_MULTIPLE, but only one row should be selected. */
        list = gtk_tree_selection_get_selected_rows (
                gtk_tree_view_get_selection (view), &model);
        g_assert (g_list_length (list) == 1);

        iter_set = gtk_tree_model_get_iter (model, &iter, list->data);
        g_assert (iter_set);
        gtk_tree_model_get (
                model, &iter, GVA_GAME_STORE_COLUMN_INPFILE, &inpfile, -1);
        inpname = g_strdelimit (g_path_get_basename (inpfile), ".", '\0');
        g_free (inpfile);

        if (!gva_xmame_playback_game (inpname, &error))
        {
                g_warning ("%s", error->message);
                g_error_free (error);
        }

        g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
        g_list_free (list);
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
        const gchar *romname;
        gchar *inpname;
        GError *error = NULL;

        romname = gva_tree_view_get_selected_game ();
        g_assert (romname != NULL);

        inpname = gva_choose_inpname (romname);
        gva_xmame_record_game (romname, inpname, &error);
        g_free (inpname);

        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_clear_error (&error);
        }
}

static void
action_remove_favorite_cb (GtkAction *action)
{
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;
        const gchar *romname;
        gboolean valid;

        romname = gva_tree_view_get_selected_game ();
        g_assert (romname != NULL);

        model = gva_game_db_get_model ();
        path = gva_game_db_lookup (romname);
        valid = gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_path_free (path);
        g_assert (valid);

        gtk_list_store_set (
                GTK_LIST_STORE (model), &iter,
                GVA_GAME_STORE_COLUMN_FAVORITE, FALSE, -1);

        gva_favorites_remove (romname);

        gtk_action_set_visible (GVA_ACTION_INSERT_FAVORITE, TRUE);
        gtk_action_set_visible (GVA_ACTION_REMOVE_FAVORITE, FALSE);
}

static void
action_show_play_back_cb (GtkAction *action)
{
        gtk_widget_show (GVA_WIDGET_PLAY_BACK_WINDOW);
}

static void
action_start_cb (GtkAction *action)
{
        const gchar *romname;
        GError *error = NULL;

        romname = gva_tree_view_get_selected_game ();
        g_assert (romname != NULL);

        gva_xmame_run_game (romname, &error);

        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_clear_error (&error);
        }
}

static void
action_view_changed_cb (GtkRadioAction *action, GtkRadioAction *current)
{
        gva_tree_view_update ();
        gva_tree_view_set_last_selected_view (
                gva_tree_view_get_selected_view ());
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

        { "insert-favorite",
          GTK_STOCK_ADD,
          N_("Add to _Favorites"),
          "<Control>plus",
          N_("Add the selected game to your list of favorites"),
          G_CALLBACK (action_insert_favorite_cb) },

        { "play-back",
          GTK_STOCK_MEDIA_PLAY,
          N_("Play _Back..."),
          NULL,
          N_("Play back the selected game recording"),
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

        { "remove-favorite",
          GTK_STOCK_REMOVE,
          N_("Remove from _Favorites"),
          "<Control>minus",
          N_("Remove the selected game from your list of favorites"),
          G_CALLBACK (action_remove_favorite_cb) },

        { "show-play-back",
          GTK_STOCK_MEDIA_PLAY,
          N_("Play _Back..."),
          NULL,
          N_("Play back a previously recorded game"),
          G_CALLBACK (action_show_play_back_cb) },

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

static void
gva_ui_init (void)
{
        GtkWidget *widget;
        gchar *filename;

        action_group = gtk_action_group_new ("main");
        gtk_action_group_add_actions (
                action_group, entries, G_N_ELEMENTS (entries), NULL);
        gtk_action_group_add_radio_actions (
                action_group, view_radio_entries,
                G_N_ELEMENTS (view_radio_entries),
                0, G_CALLBACK (action_view_changed_cb), NULL);

        filename = gva_find_data_file (PACKAGE ".glade");
        if (filename != NULL)
                xml = glade_xml_new (filename, NULL, NULL);
        g_free (filename);

        filename = gva_find_data_file (PACKAGE ".ui");
        if (filename != NULL)
        {
                manager = gtk_ui_manager_new ();
                gtk_ui_manager_add_ui_from_file (manager, filename, NULL);
                gtk_ui_manager_insert_action_group (manager, action_group, 0);
        }
        g_free (filename);

        if (xml == NULL || manager == NULL)
                g_error ("%s", _("Failed to initialize user interface"));

        initialized = TRUE;
}

GtkAction *
gva_ui_get_action (const gchar *action_name)
{
        GtkAction *action;

        g_return_val_if_fail (action_name != NULL, NULL);

        if (G_UNLIKELY (!initialized))
                gva_ui_init ();

        action = gtk_action_group_get_action (action_group, action_name);
        g_assert (action != NULL);
        return action;
}

GtkWidget *
gva_ui_get_widget (const gchar *widget_name)
{
        GtkWidget *widget;

        g_return_val_if_fail (widget_name != NULL, NULL);

        if (G_UNLIKELY (!initialized))
                gva_ui_init ();

        widget = glade_xml_get_widget (xml, widget_name);
        g_assert (widget != NULL);
        return widget;
}

GtkWidget *
gva_ui_get_managed_widget (const gchar *widget_path)
{
        GtkWidget *widget;

        g_return_val_if_fail (widget_path != NULL, NULL);

        if (G_UNLIKELY (!initialized))
                gva_ui_init ();

        widget = gtk_ui_manager_get_widget (manager, widget_path);
        g_assert (widget != NULL);
        return widget;
}
