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

#include "gva-preferences.h"

#include "gva-error.h"
#include "gva-mame.h"
#include "gva-ui.h"

enum
{
        COLUMN_TITLE,
        COLUMN_VISIBLE,
        COLUMN_OBJECT
};

static void
preferences_columns_selection_changed_cb (GtkTreeSelection *selection)
{
        GtkTreeViewColumn *column;
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;
        gint n_children;

        if (!gtk_tree_selection_get_selected (selection, &model, &iter))
                return;

        path = gtk_tree_model_get_path (model, &iter);
        gtk_tree_model_get (model, &iter, COLUMN_OBJECT, &column, -1);
        n_children = gtk_tree_model_iter_n_children (model, NULL);

        /* Update button sensitivity. */
        gtk_widget_set_sensitive (
                GTK_WIDGET (GVA_WIDGET_PREFERENCES_COLUMNS_MOVE_UP_BUTTON),
                gtk_tree_path_get_indices (path)[0] > 0);
        gtk_widget_set_sensitive (
                GTK_WIDGET (GVA_WIDGET_PREFERENCES_COLUMNS_MOVE_DOWN_BUTTON),
                gtk_tree_path_get_indices (path)[0] < n_children - 1);
        gtk_widget_set_sensitive (
                GTK_WIDGET (GVA_WIDGET_PREFERENCES_COLUMNS_SHOW_BUTTON),
                !gtk_tree_view_column_get_visible (column));
        gtk_widget_set_sensitive (
                GTK_WIDGET (GVA_WIDGET_PREFERENCES_COLUMNS_HIDE_BUTTON),
                gtk_tree_view_column_get_visible (column));

        gtk_tree_path_free (path);
        g_object_unref (column);
}

static void
preferences_columns_move_up_button_clicked_cb (GtkButton *button,
                                               GtkTreeView *view)
{
        GtkTreeSelection *selection;
        GtkTreeViewColumn *column;
        GtkTreeView *main_view;
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;
        GList *list;
        gint index;

        main_view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);

        selection = gtk_tree_view_get_selection (view);
        if (!gtk_tree_selection_get_selected (selection, &model, &iter))
                return;

        gtk_tree_model_get (model, &iter, COLUMN_OBJECT, &column, -1);
        list = gtk_tree_view_get_columns (main_view);

        index = g_list_index (list, column);
        g_assert (index > 0);

        gtk_tree_view_move_column_after (
                main_view, column, (index == 1) ? NULL :
                g_list_nth_data (list, index - 2));

        path = gtk_tree_path_new_from_indices (index - 1, -1);
        gtk_tree_view_set_cursor (view, path, NULL, FALSE);
        gtk_tree_path_free (path);

        g_object_unref (column);
        g_list_free (list);
}

static void
preferences_columns_move_down_button_clicked_cb (GtkButton *button,
                                                 GtkTreeView *view)
{
        GtkTreeSelection *selection;
        GtkTreeViewColumn *column;
        GtkTreeView *main_view;
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;
        GList *list;
        gint index;

        main_view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);

        selection = gtk_tree_view_get_selection (view);
        if (!gtk_tree_selection_get_selected (selection, &model, &iter))
                return;

        gtk_tree_model_get (model, &iter, COLUMN_OBJECT, &column, -1);
        list = gtk_tree_view_get_columns (main_view);

        index = g_list_index (list, column);
        g_assert (index >= 0);

        gtk_tree_view_move_column_after (
                main_view, column, g_list_nth_data (list, index + 1));

        path = gtk_tree_path_new_from_indices (index + 1, -1);
        gtk_tree_view_set_cursor (view, path, NULL, FALSE);
        gtk_tree_path_free (path);

        g_object_unref (column);
        g_list_free (list);
}

static void
preferences_columns_show_button_clicked_cb (GtkButton *button,
                                            GtkTreeView *view)
{
        GtkTreeSelection *selection;
        GtkTreeViewColumn *column;
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;

        selection = gtk_tree_view_get_selection (view);
        if (!gtk_tree_selection_get_selected (selection, &model, &iter))
                return;

        gtk_tree_model_get (model, &iter, COLUMN_OBJECT, &column, -1);
        gtk_tree_view_column_set_visible (column, TRUE);
        g_object_unref (column);

        gtk_widget_set_sensitive (
                GVA_WIDGET_PREFERENCES_COLUMNS_SHOW_BUTTON, FALSE);
        gtk_widget_set_sensitive (
                GVA_WIDGET_PREFERENCES_COLUMNS_HIDE_BUTTON, TRUE);

        path = gtk_tree_model_get_path (model, &iter);
        gtk_tree_model_row_changed (model, path, &iter);
        gtk_tree_path_free (path);
}

static void
preferences_columns_hide_button_clicked_cb (GtkButton *button,
                                            GtkTreeView *view)
{
        GtkTreeSelection *selection;
        GtkTreeViewColumn *column;
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;

        selection = gtk_tree_view_get_selection (view);
        if (!gtk_tree_selection_get_selected (selection, &model, &iter))
                return;

        gtk_tree_model_get (model, &iter, COLUMN_OBJECT, &column, -1);
        gtk_tree_view_column_set_visible (column, FALSE);
        g_object_unref (column);

        gtk_widget_set_sensitive (
                GVA_WIDGET_PREFERENCES_COLUMNS_SHOW_BUTTON, TRUE);
        gtk_widget_set_sensitive (
                GVA_WIDGET_PREFERENCES_COLUMNS_HIDE_BUTTON, FALSE);

        path = gtk_tree_model_get_path (model, &iter);
        gtk_tree_model_row_changed (model, path, &iter);
        gtk_tree_path_free (path);
}

static void
preferences_columns_toggled_cb (GtkCellRendererToggle *renderer,
                                gchar *path_string,
                                GtkTreeView *view)
{
        GtkTreeViewColumn *column;
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;
        gboolean valid;
        gboolean visible;

        model = gtk_tree_view_get_model (view);
        path = gtk_tree_path_new_from_string (path_string);
        valid = gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_path_free (path);
        g_assert (valid);

        gtk_tree_model_get (model, &iter, COLUMN_OBJECT, &column, -1);
        visible = gtk_cell_renderer_toggle_get_active (renderer);
        gtk_list_store_set (
                GTK_LIST_STORE (model), &iter, COLUMN_VISIBLE, !visible, -1);
        gtk_tree_view_column_set_visible (column, !visible);
        g_object_unref (column);

        gtk_widget_set_sensitive (
                GVA_WIDGET_PREFERENCES_COLUMNS_SHOW_BUTTON, !visible);
        gtk_widget_set_sensitive (
                GVA_WIDGET_PREFERENCES_COLUMNS_HIDE_BUTTON, visible);
}

static void
preferences_columns_update_view (GtkTreeView *view)
{
        GtkListStore *list_store;
        GtkTreeView *target_view;
        GList *list;

        list_store = gtk_list_store_new (
                3, G_TYPE_STRING, G_TYPE_BOOLEAN, GTK_TYPE_TREE_VIEW_COLUMN);
        target_view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);
        list = gtk_tree_view_get_columns (target_view);

        while (list != NULL)
        {
                GtkTreeViewColumn *column = list->data;
                GtkTreeIter iter;
                const gchar *title;
                gboolean visible;

                title = gtk_tree_view_column_get_title (column);
                visible = gtk_tree_view_column_get_visible (column);

                gtk_list_store_append (list_store, &iter);
                gtk_list_store_set (
                        list_store, &iter, COLUMN_TITLE, title,
                        COLUMN_VISIBLE, visible, COLUMN_OBJECT, column, -1);
                list = g_list_delete_link (list, list);
        }

        gtk_tree_view_set_model (view, GTK_TREE_MODEL (list_store));
        g_object_unref (list_store);
}

static void
preferences_init_general (void)
{
        /* Auto Save */

        gtk_action_connect_proxy (
                GVA_ACTION_AUTO_SAVE,
                GVA_WIDGET_PREFERENCES_AUTO_SAVE);

        gtk_action_set_sensitive (
                GVA_ACTION_AUTO_SAVE,
                gva_mame_supports_auto_save ());

        gtk_toggle_action_set_active (
                GTK_TOGGLE_ACTION (GVA_ACTION_AUTO_SAVE),
                gva_preferences_get_auto_save ());

        /* Full Screen */

        gtk_action_connect_proxy (
                GVA_ACTION_FULL_SCREEN,
                GVA_WIDGET_PREFERENCES_FULL_SCREEN);

        gtk_action_set_sensitive (
                GVA_ACTION_FULL_SCREEN,
                gva_mame_supports_full_screen () ||
                gva_mame_supports_window ());

        gtk_toggle_action_set_active (
                GTK_TOGGLE_ACTION (GVA_ACTION_FULL_SCREEN),
                gva_preferences_get_full_screen ());
}

static void
preferences_init_columns (void)
{
        GtkTreeView *view;
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;

        view = GTK_TREE_VIEW (GVA_WIDGET_PREFERENCES_COLUMNS_TREE_VIEW);

        column = gtk_tree_view_column_new ();

        renderer = gtk_cell_renderer_toggle_new ();
        g_object_set (
                renderer, "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE, NULL);
        g_signal_connect (
                renderer, "toggled",
                G_CALLBACK (preferences_columns_toggled_cb), view);
        column = gtk_tree_view_column_new_with_attributes (
                _("Visible"), renderer, "active", COLUMN_VISIBLE, NULL);
        gtk_tree_view_append_column (view, column);

        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes (
                _("Title"), renderer, "text", COLUMN_TITLE, NULL);
        gtk_tree_view_append_column (view, column);

        g_signal_connect_swapped (
                GVA_WIDGET_MAIN_TREE_VIEW, "columns-changed",
                G_CALLBACK (preferences_columns_update_view), view);

        g_signal_connect (
                gtk_tree_view_get_selection (view), "changed",
                G_CALLBACK (preferences_columns_selection_changed_cb), NULL);

        g_signal_connect (
                GVA_WIDGET_PREFERENCES_COLUMNS_MOVE_UP_BUTTON, "clicked",
                G_CALLBACK (preferences_columns_move_up_button_clicked_cb),
                view);

        g_signal_connect (
                GVA_WIDGET_PREFERENCES_COLUMNS_MOVE_DOWN_BUTTON, "clicked",
                G_CALLBACK (preferences_columns_move_down_button_clicked_cb),
                view);

        g_signal_connect (
                GVA_WIDGET_PREFERENCES_COLUMNS_SHOW_BUTTON, "clicked",
                G_CALLBACK (preferences_columns_show_button_clicked_cb),
                view);

        g_signal_connect (
                GVA_WIDGET_PREFERENCES_COLUMNS_HIDE_BUTTON, "clicked",
                G_CALLBACK (preferences_columns_hide_button_clicked_cb),
                view);

        preferences_columns_update_view (view);
}

void
gva_preferences_init (void)
{
        GtkWindow *window;
        gboolean active;

        window = GTK_WINDOW (GVA_WIDGET_PREFERENCES_WINDOW);

        g_signal_connect (
                GVA_WIDGET_PREFERENCES_WINDOW, "delete-event",
                G_CALLBACK (gtk_widget_hide_on_delete), NULL);

        g_signal_connect_swapped (
                GVA_WIDGET_PREFERENCES_CLOSE_BUTTON, "clicked",
                G_CALLBACK (gtk_widget_hide), window);

        preferences_init_general ();
        preferences_init_columns ();
}

gboolean
gva_preferences_get_auto_save (void)
{
        GConfClient *client;
        gboolean auto_save;
        GError *error = NULL;

        client = gconf_client_get_default ();
        auto_save = gconf_client_get_bool (
                client, GVA_GCONF_AUTO_SAVE_KEY, &error);
        gva_error_handle (&error);
        g_object_unref (client);

        return auto_save;
}

void
gva_preferences_set_auto_save (gboolean auto_save)
{
        GConfClient *client;
        GError *error = NULL;

        client = gconf_client_get_default ();
        gconf_client_set_bool (
                client, GVA_GCONF_AUTO_SAVE_KEY, auto_save, &error);
        gva_error_handle (&error);
        g_object_unref (client);
}

gboolean
gva_preferences_get_full_screen (void)
{
        GConfClient *client;
        gboolean full_screen;
        GError *error = NULL;

        client = gconf_client_get_default ();
        full_screen = gconf_client_get_bool (
                client, GVA_GCONF_FULL_SCREEN_KEY, &error);
        gva_error_handle (&error);
        g_object_unref (client);

        return full_screen;
}

void
gva_preferences_set_full_screen (gboolean full_screen)
{
        GConfClient *client;
        GError *error = NULL;

        client = gconf_client_get_default ();
        gconf_client_set_bool (
                client, GVA_GCONF_FULL_SCREEN_KEY, full_screen, &error);
        gva_error_handle (&error);
        g_object_unref (client);
}

