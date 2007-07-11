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

#include "gva-ui.h"

#include <glade/glade.h>

#include "gva-error.h"
#include "gva-favorites.h"
#include "gva-game-db.h"
#include "gva-game-store.h"
#include "gva-main.h"
#include "gva-play-back.h"
#include "gva-preferences.h"
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
record_game_exited (GvaProcess *process, gint status, gchar *inpname)
{
        if (process->error == NULL)
                gva_play_back_show (inpname);

        g_object_unref (process);
        g_free (inpname);
}

static void
action_about_cb (GtkAction *action)
{
        GdkPixbuf *logo;
        GError *error = NULL;

        logo = gtk_icon_theme_load_icon (
                gtk_icon_theme_get_default (),
                PACKAGE, 128, 0, &error);
        gva_error_handle (&error);

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
action_auto_save_cb (GtkToggleAction *action)
{
        gboolean active;

        active = gtk_toggle_action_get_active (action);
        gva_preferences_set_auto_save (active);
}

static void
action_contents_cb (GtkAction *action)
{
}

static void
action_full_screen_cb (GtkToggleAction *action)
{
        gboolean active;

        active = gtk_toggle_action_get_active (action);
        gva_preferences_set_full_screen (active);
}

static void
action_go_back_cb (GtkAction *action)
{
        GtkTreeSelection *selection;
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeView *view;
        GtkTreeIter iter;
        gboolean valid;
        gint n_nodes;
        gint index;

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);
        selection = gtk_tree_view_get_selection (view);
        valid = gtk_tree_selection_get_selected (selection, &model, &iter);
        n_nodes = gtk_tree_model_iter_n_children (model, NULL);
        g_return_if_fail (valid);

        path = gtk_tree_model_get_path (model, &iter);
        index = gtk_tree_path_get_indices (path)[0];
        index = (index + n_nodes - 1) % n_nodes;
        gtk_tree_path_free (path);

        path = gtk_tree_path_new_from_indices (index, -1);
        gtk_tree_view_set_cursor (view, path, NULL, FALSE);
        gtk_tree_path_free (path);
}

static void
action_go_forward_cb (GtkAction *action)
{
        GtkTreeSelection *selection;
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeView *view;
        GtkTreeIter iter;
        gboolean valid;
        gint n_nodes;
        gint index;

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);
        selection = gtk_tree_view_get_selection (view);
        valid = gtk_tree_selection_get_selected (selection, &model, &iter);
        n_nodes = gtk_tree_model_iter_n_children (model, NULL);
        g_return_if_fail (valid);

        path = gtk_tree_model_get_path (model, &iter);
        index = gtk_tree_path_get_indices (path)[0];
        index = (index + n_nodes + 1) % n_nodes;
        gtk_tree_path_free (path);

        path = gtk_tree_path_new_from_indices (index, -1);
        gtk_tree_view_set_cursor (view, path, NULL, FALSE);
        gtk_tree_path_free (path);
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
        GvaProcess *process;
        GtkTreeModel *model;
        GtkTreeView *view;
        GtkTreeIter iter;
        gchar *inpfile;
        gchar *inpname;
        gchar *romname;
        GList *list;
        gboolean valid;
        GError *error = NULL;

        view = GTK_TREE_VIEW (GVA_WIDGET_PLAY_BACK_TREE_VIEW);

        /* We have to use gtk_tree_selection_get_selected_rows() instead of
         * gtk_tree_selection_get_selected() because the selection mode is
         * GTK_SELECTION_MULTIPLE, but only one row should be selected. */
        list = gtk_tree_selection_get_selected_rows (
                gtk_tree_view_get_selection (view), &model);
        g_assert (g_list_length (list) == 1);

        valid = gtk_tree_model_get_iter (model, &iter, list->data);
        g_assert (valid);
        gtk_tree_model_get (
                model, &iter, GVA_GAME_STORE_COLUMN_INPFILE, &inpfile,
                GVA_GAME_STORE_COLUMN_NAME, &romname, -1);
        inpname = g_strdelimit (g_path_get_basename (inpfile), ".", '\0');
        g_free (inpfile);

        process = gva_xmame_playback_game (romname, inpname, &error);
        gva_error_handle (&error);

        if (process != NULL)
                g_signal_connect_after (
                        process, "exited",
                        G_CALLBACK (g_object_unref), NULL);

        g_free (inpname);
        g_free (romname);

        g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
        g_list_free (list);
}

static void
action_preferences_cb (GtkAction *action)
{
        gtk_widget_show (GVA_WIDGET_PREFERENCES_WINDOW);
}

static void
action_properties_cb (GtkAction *action)
{
        gtk_widget_show (GVA_WIDGET_PROPERTIES_WINDOW);
}

static void
action_quit_cb (GtkAction *action)
{
        gtk_main_quit ();
}

static void
action_record_cb (GtkAction *action)
{
        GvaProcess *process;
        const gchar *romname;
        gchar *inpname;
        GError *error = NULL;

        romname = gva_tree_view_get_selected_game ();
        g_assert (romname != NULL);

        inpname = gva_choose_inpname (romname);

        process = gva_xmame_record_game (romname, inpname, &error);
        gva_error_handle (&error);

        if (process != NULL)
                g_signal_connect_after (
                        process, "exited",
                        G_CALLBACK (record_game_exited), inpname);
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
action_search_cb (GtkAction *action)
{
        /* TODO */
}

static void
action_show_play_back_cb (GtkAction *action)
{
        gva_play_back_show (NULL);
}

static void
action_start_cb (GtkAction *action)
{
        GvaProcess *process;
        const gchar *romname;
        GError *error = NULL;

        romname = gva_tree_view_get_selected_game ();
        g_assert (romname != NULL);

        if (!gva_preferences_get_auto_save ())
                if (!gva_xmame_clear_state (romname, &error))
                        gva_error_handle (&error);

        process = gva_xmame_run_game (romname, &error);
        gva_error_handle (&error);

        if (process != NULL)
                g_signal_connect_after (
                        process, "exited",
                        G_CALLBACK (g_object_unref), NULL);
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
          N_("Show information about the application"),
          G_CALLBACK (action_about_cb) },

        { "contents",
          GTK_STOCK_HELP,
          N_("_Contents"),
          NULL,
          NULL,
          G_CALLBACK (action_contents_cb) },

        { "go-back",
          GTK_STOCK_GO_BACK,
          N_("_Back"),
          "<Alt>leftarrow",
          N_("Show previous game"),
          G_CALLBACK (action_go_back_cb) },

        { "go-forward",
          GTK_STOCK_GO_FORWARD,
          N_("_Forward"),
          "<Alt>rightarrow",
          N_("Show next game"),
          G_CALLBACK (action_go_forward_cb) },

        { "insert-favorite",
          GTK_STOCK_ADD,
          N_("Add to _Favorites"),
          "<Control>plus",
          N_("Add the selected game to my list of favorites"),
          G_CALLBACK (action_insert_favorite_cb) },

        { "play-back",
          GTK_STOCK_MEDIA_PLAY,
          N_("Play _Back..."),
          NULL,
          N_("Play back the selected game recording"),
          G_CALLBACK (action_play_back_cb) },

        { "preferences",
          GTK_STOCK_PREFERENCES,
          N_("_Preferences"),
          NULL,
          N_("Configure the application"),
          G_CALLBACK (action_preferences_cb) },

        { "properties",
          GTK_STOCK_PROPERTIES,
          N_("_Properties"),
          NULL,
          N_("Show information about the selected game"),
          G_CALLBACK (action_properties_cb) },

        { "quit",
          GTK_STOCK_QUIT,
          N_("_Quit"),
          NULL,
          N_("Quit the application"),
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
          N_("Remove the selected game from my list of favorites"),
          G_CALLBACK (action_remove_favorite_cb) },

        { "search",
          GTK_STOCK_FIND,
          N_("S_earch..."),
          NULL,
          N_("Show a custom list of games"),
          G_CALLBACK (action_search_cb) },

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

        { "edit-menu",
          NULL,
          N_("_Edit"),
          NULL,
          NULL,
          NULL },

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

static GtkToggleActionEntry toggle_entries[] =
{
        { "auto-save",
          NULL,
          N_("_Restore previous state when starting a game"),
          NULL,
          NULL,
          G_CALLBACK (action_auto_save_cb),
          FALSE },  /* GConf overrides this */

        { "full-screen",
          NULL,
          N_("Start games in _fullscreen mode"),
          NULL,
          NULL,
          G_CALLBACK (action_full_screen_cb),
          FALSE }   /* GConf overrides this */
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
        GError *error = NULL;

        /* Set this immediately so we don't cause infinite recursion
         * when we use the GVA_WIDGET_MAIN_WINDOW macro below. */
        initialized = TRUE;

        action_group = gtk_action_group_new ("main");
        gtk_action_group_add_actions (
                action_group, entries,
                G_N_ELEMENTS (entries), NULL);
        gtk_action_group_add_toggle_actions (
                action_group, toggle_entries,
                G_N_ELEMENTS (toggle_entries), NULL);
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
                gtk_ui_manager_insert_action_group (manager, action_group, 0);

                gtk_window_add_accel_group (
                        GTK_WINDOW (GVA_WIDGET_MAIN_WINDOW),
                        gtk_ui_manager_get_accel_group (manager));

                g_signal_connect (
                        manager, "connect-proxy",
                        G_CALLBACK (gva_main_connect_proxy_cb), NULL);

                gtk_ui_manager_add_ui_from_file (manager, filename, &error);
        }
        g_free (filename);

        if (error != NULL)
                g_error ("%s", error->message);

        if (xml == NULL || manager == NULL)
                g_error ("%s", _("Failed to initialize user interface"));
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
