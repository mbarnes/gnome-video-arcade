/* Copyright 2007-2011 Matthew Barnes
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

#include "gva-audit.h"
#include "gva-column-manager.h"
#include "gva-db.h"
#include "gva-error.h"
#include "gva-favorites.h"
#include "gva-game-store.h"
#include "gva-main.h"
#include "gva-mame.h"
#include "gva-music-button.h"
#include "gva-mute-button.h"
#include "gva-play-back.h"
#include "gva-preferences.h"
#include "gva-process.h"
#include "gva-tree-view.h"
#include "gva-util.h"
#include "gva-wnck.h"

#ifdef HAVE_DBUS
#include "gva-dbus.h"
#endif

#define SQL_INSERT_LASTPLAYED \
        "INSERT INTO lastplayed VALUES ('%s', %" G_GINT64_FORMAT ")"

#define WEBSITE_URL \
        "http://mbarnes.github.com/gnome-video-arcade"

static GtkBuilder *builder = NULL;
static GtkUIManager *manager = NULL;
static gboolean initialized = FALSE;

/* About Dialog Information */
static const gchar *authors[] =
{
        "Matthew Barnes <mbarnes@redhat.com>",
        NULL
};
static const gchar *copyright =
"Copyright \xC2\xA9 2007-2011 Matthew Barnes";
static const gchar *license =
"GNOME Video Arcade is free software; you can redistribute it "
"and/or modify it under the terms of the GNU General Public License "
"as published by the Free Software Foundation; either version 3 of "
"the License, or (at your option) any later version.\n"
"\n"
"GNOME Video Arcade is distributed in the hope that it will be "
"useful, but WITHOUT ANY WARRANTY; without even the implied warranty "
"of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License "
"along with this program.  If not, see <http://www.gnu.org/licenses/>.";

/**
 * GVA_ACTION_ABOUT:
 *
 * Activation of this action displays the application's About dialog.
 *
 * Main menu item: Help -> About
 **/

/**
 * GVA_ACTION_AUTO_PLAY:
 *
 * This toggle action tracks the user's preference for whether to
 * automatically start playing a music clip from the selected game
 * (if available) when the Properties window is open.
 **/

/**
 * GVA_ACTION_AUTO_SAVE:
 *
 * This toggle action tracks the user's preference for whether to
 * restore the emulated machine's previous state when starting a game.
 **/

/**
 * GVA_ACTION_CONTENTS:
 *
 * Activation of this action opens the user manual for GNOME Video Arcade.
 *
 * Main menu item: Help -> Contents
 **/

/**
 * GVA_ACTION_FULL_SCREEN:
 *
 * This toggle action tracks the user's preference for whether to
 * start games in full screen mode.
 **/

/**
 * GVA_ACTION_INSERT_FAVORITE:
 *
 * Activation of this action adds the currently selected game to the
 * user's list of favorites.
 *
 * Main menu item: Game -> Add to Favorites
 **/

/**
 * GVA_ACTION_NEXT_GAME:
 *
 * Activation of this action selects the next game, rolling over to
 * the first game if necessary.
 **/

/**
 * GVA_ACTION_PLAY_BACK:
 *
 * Activation of this action plays back the selected game recording.
 **/

/**
 * GVA_ACTION_PREFERENCES:
 *
 * Activation of this action presents the "Preferences" window.
 *
 * Main menu item: Edit -> Preferences
 **/

/**
 * GVA_ACTION_PREVIOUS_GAME:
 *
 * Activation of this action selects the previous game, rolling over to
 * the last game if necessary.
 **/

/**
 * GVA_ACTION_PROPERTIES:
 *
 * Activation of this action presents the "Properties" window.
 *
 * Main menu item: View -> Properties
 **/

/**
 * GVA_ACTION_QUIT:
 *
 * Activation of this action initiates application shutdown.
 *
 * Main menu item: Game -> Quit
 **/

/**
 * GVA_ACTION_RECORD:
 *
 * Activation of this action starts the selected game with MAME recording
 * user inputs to a file.  The "Recorded Games" window will automatically
 * be displayed when the user exits the game.
 *
 * Main menu item: Game -> Record
 **/

/**
 * GVA_ACTION_REMOVE_COLUMN:
 *
 * Activation of this action removes the currently selected column from
 * the game list.
 **/

/**
 * GVA_ACTION_REMOVE_FAVORITE:
 *
 * Activation of this action removes the currently selected game from
 * the user's list of favorites.
 *
 * Main menu item: Game -> Remove from Favorites
 **/

/**
 * GVA_ACTION_SAVE_ERRORS:
 *
 * Activation of this action saves the errors from a ROM audit to a file.
 **/

/**
 * GVA_ACTION_SEARCH:
 *
 * Activation of this action presents the "Search for Games" window.
 *
 * Main menu item: Edit -> Search...
 **/

/**
 * GVA_ACTION_SHOW_CLONES:
 *
 * This toggle action tracks the user's preference for whether to
 * show cloned games in the main window's game list.
 **/

/**
 * GVA_ACTION_SHOW_PLAY_BACK:
 *
 * Activation of this action presents the "Recorded Games" window.
 *
 * Main menu item: Game -> Play Back...
 **/

/**
 * GVA_ACTION_START:
 *
 * Activation of this action starts the selected game.
 *
 * Main menu item: Game -> Start
 **/

/**
 * GVA_ACTION_VIEW_AVAILABLE:
 *
 * Activation of this action queries the database for all available games.
 *
 * Main menu item: View -> Available Games
 **/

/**
 * GVA_ACTION_VIEW_FAVORITES:
 *
 * Activation of this action queries the database for the user's favorite
 * games.
 *
 * Main menu item: View -> Favorite Games
 **/

/**
 * GVA_ACTION_VIEW_RESULTS:
 *
 * Activation of this action executes a custom database query.
 *
 * Main menu item: View -> Search Results
 **/

static void
log_lastplayed (GvaProcess *process,
                gint status,
                gchar *name)
{
        GtkTreePath *path;
        gchar *sql;
        time_t now;
        GError *error = NULL;

        if (process->error != NULL)
                return;

        time (&now);

        /* Record the time in the database. */
        sql = g_strdup_printf (SQL_INSERT_LASTPLAYED, name, (gint64) now);
        gva_db_execute (sql, &error);
        gva_error_handle (&error);
        g_free (sql);

        /* Record the time in the tree model. */
        path = gva_tree_view_lookup (name);
        if (path != NULL)
        {
                GtkTreeModel *model;
                GtkTreeIter iter;

                model = gva_tree_view_get_model ();
                gtk_tree_model_get_iter (model, &iter, path);

                gtk_tree_store_set (
                        GTK_TREE_STORE (model), &iter,
                        GVA_GAME_STORE_COLUMN_LAST_PLAYED, &now, -1);

                gtk_tree_path_free (path);
        }

        g_free (name);
}

static void
record_game_exited (GvaProcess *process,
                    gint status,
                    gchar *inpname)
{
        if (process->error == NULL)
                gva_play_back_show (inpname);

        g_free (inpname);
}

void
gva_action_about_cb (GtkAction *action)
{
        GdkPixbuf *logo;
        GError *error = NULL;

        logo = gtk_icon_theme_load_icon (
                gtk_icon_theme_get_default (),
                PACKAGE, 128, 0, &error);
        gva_error_handle (&error);

        gtk_show_about_dialog (
                GTK_WINDOW (GVA_WIDGET_MAIN_WINDOW),
                "program-name", PACKAGE_NAME,
                "version", PACKAGE_VERSION,
                "comments", _("M.A.M.E. Front-End"),
                "copyright", copyright,
                "license", license,
                "wrap-license", TRUE,
                "website", WEBSITE_URL,
                "website-label", _("Visit the Website"),
                "authors", authors,
                "translator-credits", _("translator-credits"),
                "logo", logo,
                NULL);

        if (logo != NULL)
                g_object_unref (logo);
}

static void
action_add_column_cb (GtkAction *action,
                      GtkTreeViewColumn *column)
{
        GtkTreeViewColumn *base_column;
        GtkTreeView *view;

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);
        base_column = g_object_get_data (G_OBJECT (view), "popup-menu-column");
        g_return_if_fail (column != base_column);

        gtk_tree_view_move_column_after (view, column, base_column);
        gtk_tree_view_column_set_visible (column, TRUE);
}

void
gva_action_contents_cb (GtkAction *action)
{
        gva_help_display (GTK_WINDOW (GVA_WIDGET_MAIN_WINDOW), NULL);
}

void
gva_action_insert_favorite_cb (GtkAction *action)
{
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;
        const gchar *name;
        gboolean valid;

        name = gva_tree_view_get_selected_game ();
        g_assert (name != NULL);

        model = gva_tree_view_get_model ();
        path = gva_tree_view_lookup (name);
        valid = gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_path_free (path);
        g_assert (valid);

        gtk_tree_store_set (
                GTK_TREE_STORE (model), &iter,
                GVA_GAME_STORE_COLUMN_FAVORITE, TRUE, -1);

        gva_favorites_insert (name);

        gtk_action_set_visible (GVA_ACTION_INSERT_FAVORITE, FALSE);
        gtk_action_set_visible (GVA_ACTION_REMOVE_FAVORITE, TRUE);
}

void
gva_action_next_game_cb (GtkAction *action)
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

void
gva_action_play_back_cb (GtkAction *action)
{
        GtkWidget *widget;
        GvaProcess *process;
        GtkTreeModel *model;
        GtkTreeView *view;
        GtkTreeIter iter;
        gchar *inpfile;
        gchar *inpname;
        gchar *name;
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
                GVA_GAME_STORE_COLUMN_NAME, &name, -1);
        inpname = g_strdelimit (g_path_get_basename (inpfile), ".", '\0');
        g_free (inpfile);

        widget = GVA_WIDGET_PROPERTIES_MUSIC_BUTTON;
        gva_music_button_pause (GVA_MUSIC_BUTTON (widget));

        process = gva_mame_playback_game (name, inpname, &error);
        gva_error_handle (&error);

#ifdef HAVE_DBUS
        if (process != NULL)
        {
                const gchar *reason;

                /* Translators: This is passed through D-Bus as the
                 * reason to inhibit GNOME screen saver. */
                reason = _("Watching a fullscreen game");
                gva_dbus_inhibit_screen_saver (process, reason, &error);
                gva_error_handle (&error);
        }
#endif

        if (process != NULL)
        {
                gva_wnck_listen_for_new_window (process, name);
                g_signal_connect_after (
                        process, "exited",
                        G_CALLBACK (log_lastplayed), g_strdup (name));
                g_signal_connect_after (
                        process, "exited",
                        G_CALLBACK (g_object_unref), NULL);
        }

        g_free (inpname);
        g_free (name);

        g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
        g_list_free (list);
}

void
gva_action_preferences_cb (GtkAction *action)
{
        gtk_window_present (GTK_WINDOW (GVA_WIDGET_PREFERENCES_WINDOW));
}

void
gva_action_previous_game_cb (GtkAction *action)
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

void
gva_action_properties_cb (GtkAction *action)
{
        gtk_window_present (GTK_WINDOW (GVA_WIDGET_PROPERTIES_WINDOW));
}

void
gva_action_quit_cb (GtkAction *action)
{
        gva_process_kill_all ();
        gtk_main_quit ();
}

void
gva_action_record_cb (GtkAction *action)
{
        GtkWidget *widget;
        GvaProcess *process;
        const gchar *name;
        gchar *inpname;
        GError *error = NULL;

        name = gva_tree_view_get_selected_game ();
        g_assert (name != NULL);

        inpname = gva_choose_inpname (name);

        widget = GVA_WIDGET_PROPERTIES_MUSIC_BUTTON;
        gva_music_button_pause (GVA_MUSIC_BUTTON (widget));

        process = gva_mame_record_game (name, inpname, &error);
        gva_error_handle (&error);

#ifdef HAVE_DBUS
        if (process != NULL)
        {
                const gchar *reason;

                /* Translators: This is passed through D-Bus as the
                 * reason to inhibit GNOME screen saver. */
                reason = _("Recording a fullscreen game");
                gva_dbus_inhibit_screen_saver (process, reason, &error);
                gva_error_handle (&error);
        }
#endif

        if (process != NULL)
        {
                gva_wnck_listen_for_new_window (process, name);
                g_signal_connect_after (
                        process, "exited",
                        G_CALLBACK (log_lastplayed), g_strdup (name));
                g_signal_connect_after (
                        process, "exited",
                        G_CALLBACK (record_game_exited), g_strdup (inpname));
                g_signal_connect_after (
                        process, "exited",
                        G_CALLBACK (g_object_unref), NULL);
        }

        g_free (inpname);
}

void
gva_action_remove_column_cb (GtkAction *action)
{
        GtkTreeViewColumn *column;
        GtkTreeView *view;

        /* The popup menu callback embeds the column for us. */
        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);
        column = g_object_get_data (G_OBJECT (view), "popup-menu-column");
        g_return_if_fail (column != NULL);

        gtk_tree_view_column_set_visible (column, FALSE);
}

void
gva_action_remove_favorite_cb (GtkAction *action)
{
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;
        const gchar *name;
        gboolean valid;

        name = gva_tree_view_get_selected_game ();
        g_assert (name != NULL);

        model = gva_tree_view_get_model ();
        path = gva_tree_view_lookup (name);
        valid = gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_path_free (path);
        g_assert (valid);

        gtk_tree_store_set (
                GTK_TREE_STORE (model), &iter,
                GVA_GAME_STORE_COLUMN_FAVORITE, FALSE, -1);

        gva_favorites_remove (name);

        gtk_action_set_visible (GVA_ACTION_INSERT_FAVORITE, TRUE);
        gtk_action_set_visible (GVA_ACTION_REMOVE_FAVORITE, FALSE);
}

void
gva_action_save_errors_cb (GtkAction *action)
{
        gva_audit_save_errors ();
}

void
gva_action_search_cb (GtkAction *action)
{
        GtkWidget *widget;

        gtk_widget_show (GVA_WIDGET_MAIN_SEARCH_HBOX);

        widget = GVA_WIDGET_MAIN_SEARCH_ENTRY;
        gtk_widget_grab_focus (widget);
        g_signal_emit_by_name (widget, "changed");
        gtk_cell_editable_start_editing (GTK_CELL_EDITABLE (widget), NULL);
}

void
gva_action_show_clones_cb (GtkAction *action)
{
        GError *error = NULL;

        /* This is a bit of a hack.  We want to instantly apply the
         * user preference, but not while loading the initial value
         * from GSettings.  So we desensitize the action until we're
         * up and running, and use that to decide what to do here. */
        if (gtk_action_is_sensitive (action))
        {
                gva_tree_view_update (&error);
                gva_error_handle (&error);
        }
}

void
gva_action_show_play_back_cb (GtkAction *action)
{
        gva_play_back_show (NULL);
}

void
gva_action_start_cb (GtkAction *action)
{
        GtkWidget *widget;
        GvaProcess *process;
        const gchar *name;
        GError *error = NULL;

        name = gva_tree_view_get_selected_game ();
        g_assert (name != NULL);

        if (!gva_preferences_get_auto_save ())
                gva_mame_delete_save_state (name);

        widget = GVA_WIDGET_PROPERTIES_MUSIC_BUTTON;
        gva_music_button_pause (GVA_MUSIC_BUTTON (widget));

        process = gva_mame_run_game (name, &error);
        gva_error_handle (&error);

#ifdef HAVE_DBUS
        if (process != NULL)
        {
                const gchar *reason;

                /* Translators: This is passed through D-Bus as the
                 * reason to inhibit GNOME screen saver. */
                reason = _("Playing a fullscreen game");
                gva_dbus_inhibit_screen_saver (process, reason, &error);
                gva_error_handle (&error);
        }
#endif

        if (process != NULL)
        {
                gva_wnck_listen_for_new_window (process, name);
                g_signal_connect_after (
                        process, "exited",
                        G_CALLBACK (log_lastplayed), g_strdup (name));
                g_signal_connect_after (
                        process, "exited",
                        G_CALLBACK (g_object_unref), NULL);
        }
}

void
gva_action_view_changed_cb (GtkRadioAction *action,
                            GtkRadioAction *current)
{
        GError *error = NULL;

        gva_tree_view_update (&error);
        gva_error_handle (&error);
}

static void
ui_column_notify_visible_cb (GtkTreeViewColumn *column,
                             GParamSpec *pspec,
                             GtkAction *action)
{
        gboolean visible;

        visible = gtk_tree_view_column_get_visible (column);
        gtk_action_set_visible (action, !visible);
}

static gint
ui_sort_columns (GtkTreeViewColumn *column1,
                 GtkTreeViewColumn *column2)
{
        const gchar *title1;
        const gchar *title2;

        title1 = gtk_tree_view_column_get_title (column1);
        title2 = gtk_tree_view_column_get_title (column2);

        return g_utf8_collate (title1, title2);
}

static void
ui_init (void)
{
        GObject *object;
        gchar *filename;
        GError *error = NULL;

        /* Set this immediately so we don't cause infinite recursion
         * when we use the GVA_WIDGET_MAIN_WINDOW macro below. */
        initialized = TRUE;

        manager = gtk_ui_manager_new ();

        filename = gva_find_data_file (PACKAGE ".builder");
        if (filename != NULL && error == NULL)
        {
                builder = gtk_builder_new ();
                gtk_builder_add_from_file (builder, filename, &error);
                gtk_builder_connect_signals (builder, NULL);
        }
        g_free (filename);

        filename = gva_find_data_file (PACKAGE ".ui");
        if (filename != NULL && error == NULL)
        {
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

        if (builder == NULL || manager == NULL)
                g_error ("%s", _("Failed to initialize user interface"));

        object = gtk_builder_get_object (builder, "standard-actions");
        gtk_ui_manager_insert_action_group (
                manager, GTK_ACTION_GROUP (object), 0);

        object = gtk_builder_get_object (builder, "lockable-actions");
        gtk_ui_manager_insert_action_group (
                manager, GTK_ACTION_GROUP (object), 0);
}

/**
 * gva_ui_get_action:
 * @action_name: the name of a #GtkAction
 *
 * Returns the #GtkAction named @action_name from the resident
 * #GtkUIManager.  Failure to find the action indicates a programming
 * error and causes the application to abort, so the return value is
 * guaranteed to be valid.
 *
 * Returns: the #GtkAction named @action_name
 **/
GtkAction *
gva_ui_get_action (const gchar *action_name)
{
        GtkAction *action;
        GList *list, *iter;

        g_return_val_if_fail (action_name != NULL, NULL);

        if (G_UNLIKELY (!initialized))
                ui_init ();

        list = gtk_ui_manager_get_action_groups (manager);
        for (iter = list; iter != NULL; iter = g_list_next (iter))
        {
                GtkActionGroup *group;

                group = GTK_ACTION_GROUP (iter->data);
                action = gtk_action_group_get_action (group, action_name);
                if (action != NULL)
                        return action;
        }

        g_error ("No action named '%s'", action_name);
}

/**
 * gva_ui_get_widget:
 * @widget_name: the name of a #GtkWidget
 *
 * Returns the #GtkWidget named @widget_name from the UI specification.
 * Failure to find the widget indicates a programming error and causes
 * the application to abort, so the return value is guaranteed to be
 * valid.
 *
 * Returns: the #GtkWidget named @widget_name
 **/
GtkWidget *
gva_ui_get_widget (const gchar *widget_name)
{
        GObject *object;

        g_return_val_if_fail (widget_name != NULL, NULL);

        if (G_UNLIKELY (!initialized))
                ui_init ();

        object = gtk_builder_get_object (builder, widget_name);
        g_assert (object != NULL);

        return GTK_WIDGET (object);
}

/**
 * gva_ui_get_managed_widget:
 * @widget_path: the path to the managed #GtkWidget
 *
 * Returns the #GtkWidget located at @widget_path from the resident
 * #GtkUIManager.  Failure to find the widget indicates a programming
 * error and causes the application to abort, so the return value is
 * guaranteed to be valid.
 *
 * Returns: the #GtkWidget located at @widget_path
 **/
GtkWidget *
gva_ui_get_managed_widget (const gchar *widget_path)
{
        GtkWidget *widget;

        g_return_val_if_fail (widget_path != NULL, NULL);

        if (G_UNLIKELY (!initialized))
                ui_init ();

        widget = gtk_ui_manager_get_widget (manager, widget_path);
        g_assert (widget != NULL);
        return widget;
}

/**
 * gva_ui_add_column_actions:
 * @view: a #GtkTreeView
 *
 * Creates "add-column" actions for each of the columns in @view.  These
 * appear as menu items in the tree view's context menu.  The columns'
 * visibility controls the menu items' visibility in that only invisible
 * columns are listed in the context menu.
 **/
void
gva_ui_add_column_actions (GtkTreeView *view)
{
        GObject *object;
        GtkActionGroup *action_group;
        GList *list;
        guint merge_id;

        g_return_if_fail (GTK_IS_TREE_VIEW (view));

        object = gtk_builder_get_object (builder, "standard-actions");
        action_group = GTK_ACTION_GROUP (object);

        list = gtk_tree_view_get_columns (view);
        list = g_list_sort (list, (GCompareFunc) ui_sort_columns);
        merge_id = gtk_ui_manager_new_merge_id (manager);

        while (list != NULL)
        {
                GtkTreeViewColumn *column = list->data;
                GtkAction *action;
                const gchar *column_name;
                const gchar *column_title;
                gchar *action_name;
                gchar *action_label;
                gchar *action_tooltip;
                gboolean visible;

                column_name = g_object_get_data (G_OBJECT (column), "name");
                column_title = gtk_tree_view_column_get_title (column);

                action_name = g_strconcat ("add-column-", column_name, NULL);
                action_label = g_strdup_printf (
                        _("Add %s Column"), column_title);
                action_tooltip = g_strdup_printf (
                        _("Add a \"%s\" column to the game list"),
                        column_title);

                action = gtk_action_new (
                        action_name, action_label, action_tooltip, NULL);
                gtk_action_group_add_action (action_group, action);

                visible = gtk_tree_view_column_get_visible (column);
                gtk_action_set_visible (action, !visible);

                g_signal_connect (
                        action, "activate",
                        G_CALLBACK (action_add_column_cb), column);
                g_signal_connect (
                        column, "notify::visible",
                        G_CALLBACK (ui_column_notify_visible_cb), action);

                gtk_ui_manager_add_ui (
                        manager, merge_id,
                        "/column-popup/add-column",
                        action_name, action_name,
                        GTK_UI_MANAGER_AUTO, FALSE);

                g_free (action_name);
                g_free (action_label);
                g_free (action_tooltip);

                list = g_list_delete_link (list, list);
        }
}

/**
 * gva_ui_lock:
 *
 * Locks down the user interface to prevent the user from interfereing
 * with a long-running task.  Once the long-running task has completed,
 * call gva_ui_unlock() to restore full interactivity.
 **/
void
gva_ui_lock (void)
{
        GObject *object;
        GtkActionGroup *action_group;

        object = gtk_builder_get_object (builder, "lockable-actions");
        action_group = GTK_ACTION_GROUP (object);

        /* Hide secondary windows that may interfere. */
        gtk_widget_hide (GVA_WIDGET_PLAY_BACK_WINDOW);
        gtk_widget_hide (GVA_WIDGET_PROPERTIES_WINDOW);

        gtk_action_group_set_sensitive (action_group, FALSE);
        gtk_widget_set_sensitive (GVA_WIDGET_MAIN_TREE_VIEW, FALSE);
}

/**
 * gva_ui_unlock:
 *
 * Undoes the effects of gva_ui_lock().
 **/
void
gva_ui_unlock (void)
{
        GObject *object;
        GtkActionGroup *action_group;

        object = gtk_builder_get_object (builder, "lockable-actions");
        action_group = GTK_ACTION_GROUP (object);

        gtk_action_group_set_sensitive (action_group, TRUE);
        gtk_widget_set_sensitive (GVA_WIDGET_MAIN_TREE_VIEW, TRUE);
}
