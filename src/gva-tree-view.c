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

#include "gva-tree-view.h"

#include <string.h>

#include "gva-db.h"
#include "gva-columns.h"
#include "gva-error.h"
#include "gva-favorites.h"
#include "gva-game-store.h"
#include "gva-main.h"
#include "gva-ui.h"

#define SQL_SELECT_GAMES \
        "SELECT %s FROM game WHERE (romset IN ('good', 'best available'))"

static gboolean
tree_view_popup_menu_cb (GtkTreeView *view,
                         GdkEventButton *event,
                         GtkMenu *menu)
{
        if (event != NULL)
                gtk_menu_popup (
                        menu, NULL, NULL, NULL, NULL,
                        event->button, event->time);
        else
                gtk_menu_popup (
                        menu, NULL, NULL, NULL, NULL,
                        0, gtk_get_current_event_time ());

        return TRUE;
}

static gboolean
tree_view_button_press_cb (GtkTreeView *view,
                           GdkEventButton *event,
                           GtkMenu *menu)
{
        if (event->button == 3 && event->type == GDK_BUTTON_PRESS)
        {
                GtkTreePath *path;
                gboolean valid;

                /* Select the row that was clicked. */
                valid = gtk_tree_view_get_path_at_pos (
                        view, event->x, event->y, &path, NULL, NULL, NULL);
                if (valid)
                {
                        gtk_tree_view_set_cursor (view, path, NULL, FALSE);
                        gtk_widget_grab_focus (GTK_WIDGET (view));
                        gtk_tree_path_free (path);

                        return tree_view_popup_menu_cb (view, event, menu);
                }
        }

        return FALSE;
}

static void
tree_view_columns_changed_cb (GtkTreeView *view)
{
        /* Stop the emission if the tree view is being destroyed. */
        if (GTK_OBJECT_FLAGS (view) & GTK_IN_DESTRUCTION)
                g_signal_stop_emission_by_name (view, "columns-changed");
        else
                gva_columns_save (view);
}

static void
tree_view_row_activated_cb (GtkTreeView *view,
                            GtkTreePath *path,
                            GtkTreeViewColumn *column)
{
        gtk_action_activate (GVA_ACTION_START);
}

static void
tree_view_selection_changed_cb (GtkTreeSelection *selection)
{
        const gchar *name;
        gboolean game_is_selected;

        name = gva_tree_view_get_selected_game ();
        game_is_selected = (name != NULL);

        if (game_is_selected)
        {
                gboolean favorite;

                favorite = gva_favorites_contains (name);
                gtk_action_set_sensitive (GVA_ACTION_NEXT_GAME, TRUE);
                gtk_action_set_sensitive (GVA_ACTION_PREVIOUS_GAME, TRUE);
                gtk_action_set_visible (GVA_ACTION_INSERT_FAVORITE, !favorite);
                gtk_action_set_visible (GVA_ACTION_REMOVE_FAVORITE, favorite);

                gva_tree_view_set_last_selected_game (name);
        }
        else
        {
                gtk_action_set_sensitive (GVA_ACTION_NEXT_GAME, FALSE);
                gtk_action_set_sensitive (GVA_ACTION_PREVIOUS_GAME, FALSE);
                gtk_action_set_visible (GVA_ACTION_INSERT_FAVORITE, FALSE);
                gtk_action_set_visible (GVA_ACTION_REMOVE_FAVORITE, FALSE);
        }

        gtk_action_set_sensitive (GVA_ACTION_PROPERTIES, game_is_selected);
        gtk_action_set_sensitive (GVA_ACTION_RECORD, game_is_selected);
        gtk_action_set_sensitive (GVA_ACTION_START, game_is_selected);
}

static void
tree_view_sort_column_changed_cb (GtkTreeSortable *sortable)
{
        GtkSortType order;
        gint column_id;

        gtk_tree_sortable_get_sort_column_id (sortable, &column_id, &order);
        gva_tree_view_set_last_sort_column_id (column_id, order);
}

static gboolean
tree_view_search_equal (GtkTreeModel *model,
                        gint column,
                        const gchar *key,
                        GtkTreeIter *iter)
{
        const gchar *valid_chars;
        gchar **str_array;
        gchar *title;
        gboolean retval;

        /* Filter out spaces and punctuation so that, e.g. typing
         * "mspacman" will match "Ms. Pac-Man". */

        valid_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "abcdefghijklmnopqrstuvwxyz"
                      "0123456789";

        g_assert (column == GVA_GAME_STORE_COLUMN_DESCRIPTION);
        gtk_tree_model_get (model, iter, column, &title, -1);
        g_assert (title != NULL);

        g_strcanon (title, valid_chars, '?');
        str_array = g_strsplit_set (title, "?", -1);
        g_free (title);
        title = g_strjoinv (NULL, str_array);
        g_strfreev (str_array);

        /* Return FALSE if the row matches. */
        retval = (g_ascii_strncasecmp (key, title, strlen (key)) != 0);

        g_free (title);

        return retval;
}

void
gva_tree_view_init (void)
{
        GtkTreeView *view;
        GtkMenu *menu;
        gchar *name;

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);

        /* Load columns before we connect to signals to avoid trapping
         * spurious "columns-changed" signals during loading. */
        gva_columns_load (view);

        menu = GTK_MENU (gva_ui_get_managed_widget ("/game-popup"));
        gtk_menu_attach_to_widget (menu, GTK_WIDGET (view), NULL);

        g_signal_connect (
                view, "button-press-event",
                G_CALLBACK (tree_view_button_press_cb), menu);

        g_signal_connect (
                view, "columns-changed",
                G_CALLBACK (tree_view_columns_changed_cb), NULL);

        g_signal_connect (
                view, "popup-menu",
                G_CALLBACK (tree_view_popup_menu_cb), NULL);

        g_signal_connect (
                view, "row-activated",
                G_CALLBACK (tree_view_row_activated_cb), NULL);

        g_signal_connect (
                gtk_tree_view_get_selection (view), "changed",
                G_CALLBACK (tree_view_selection_changed_cb), NULL);
}

GtkTreePath *
gva_tree_view_lookup (const gchar *name)
{
        GtkTreeModel *model;

        g_return_val_if_fail (name != NULL, NULL);

        model = gva_tree_view_get_model ();

        return gva_game_store_index_lookup (GVA_GAME_STORE (model), name);
}

gboolean
gva_tree_view_update (GError **error)
{
        const gchar *name;
        const gchar *expr;

        switch (gva_tree_view_get_selected_view ())
        {
                case 0:  /* Available Games */
                        expr = NULL;
                        break;

                case 1:  /* Favorite Games */
                        expr = "isfavorite(name) == \"yes\"";
                        break;

                case 2:  /* Search Results */
                        expr = "name == NULL";
                        break;

                default:
                        g_assert_not_reached ();
        }

        if (!gva_tree_view_run_query (expr, error))
                return FALSE;

        name = gva_tree_view_get_last_selected_game ();
        if (name != NULL)
                gva_tree_view_set_selected_game (name);

        return TRUE;
}

gboolean
gva_tree_view_run_query (const gchar *expr,
                         GError **error)
{
        GvaGameStoreColumn column_id;
        GtkSortType order;
        GtkTreeView *view;
        GtkTreeModel *model;
        GdkCursor *cursor;
        GdkDisplay *display;
        GdkWindow *window;
        GString *string;
        GSList *list;
        const gchar **strv;
        gchar *columns;
        gint ii = 0;

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);

        /* Build a comma-separated list of column names. */
        list = gva_columns_get_names (view, FALSE);
        strv = g_new0 (const gchar *, g_slist_length (list) + 1);
        while (list != NULL)
        {
                if (strcmp (list->data, "favorite") == 0)
                        strv[ii++] = "isfavorite(name) AS favorite";
                else
                        strv[ii++] = list->data;
                list = g_slist_delete_link (list, list);
        }
        columns = g_strjoinv (", ", (gchar **) strv);
        g_free (strv);

        string = g_string_new (NULL);
        g_string_printf (string, SQL_SELECT_GAMES, columns);
        g_free (columns);

        if (expr != NULL)
                g_string_append_printf (string, " AND (%s)", expr);

        window = gtk_widget_get_parent_window (GTK_WIDGET (view));
        display = gtk_widget_get_display (GTK_WIDGET (view));
        cursor = gdk_cursor_new_for_display (display, GDK_WATCH);

        gdk_window_set_cursor (window, cursor);
        gtk_widget_set_sensitive (GTK_WIDGET (view), FALSE);
        model = gva_game_store_new_from_query (string->str, error);
        gtk_widget_set_sensitive (GTK_WIDGET (view), TRUE);
        gdk_window_set_cursor (window, NULL);

        gdk_cursor_unref (cursor);
        g_string_free (string, TRUE);

        if (model == NULL)
                return FALSE;

        gva_tree_view_get_last_sort_column_id (&column_id, &order);

        gtk_tree_sortable_set_sort_column_id (
                GTK_TREE_SORTABLE (model), column_id, order);

        g_signal_connect (
                model, "sort-column-changed",
                G_CALLBACK (tree_view_sort_column_changed_cb), NULL);

        gtk_tree_view_set_model (view, model);
        g_object_unref (model);

        return TRUE;
}

GtkTreeModel *
gva_tree_view_get_model (void)
{
        GtkTreeView *view;

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);

        return gtk_tree_view_get_model (view);
}

const gchar *
gva_tree_view_get_selected_game (void)
{
        const gchar *retval = NULL;
        GtkTreeSelection *selection;
        GtkTreeModel *model;
        GtkTreeIter iter;

        selection = gtk_tree_view_get_selection (
                GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW));
        if (gtk_tree_selection_get_selected (selection, &model, &iter))
        {
                gchar *name;

                gtk_tree_model_get (
                        model, &iter,
                        GVA_GAME_STORE_COLUMN_NAME, &name, -1);
                retval = g_intern_string (name);
                g_free (name);
        }

        return retval;
}

void
gva_tree_view_set_selected_game (const gchar *name)
{
        GtkTreeModel *model;
        GtkTreeView *view;
        GtkTreePath *path;
        GtkTreeIter iter;

        g_return_if_fail (name != NULL);

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);
        path = gva_tree_view_lookup (name);

        /* If the game is visible in the current view, put the cursor on it.
         * Otherwise just select the root path. */
        if (path != NULL)
        {
                gtk_tree_view_set_cursor (view, path, NULL, FALSE);
                gtk_widget_grab_focus (GTK_WIDGET (view));
        }
        else
                path = gtk_tree_path_new_first ();

        /* If we have a path to a visible game, scroll to it.  Note that
         * we don't really need the GtkTreeIter here; we're just testing
         * whether the path is valid. */
        model = gtk_tree_view_get_model (view);
        if (gtk_tree_model_get_iter (model, &iter, path))
                gtk_tree_view_scroll_to_cell (
                        view, path, NULL, TRUE, 0.5, 0.0);

        gtk_tree_path_free (path);

        gva_tree_view_set_last_selected_game (name);
}

gint
gva_tree_view_get_selected_view (void)
{
        return gtk_radio_action_get_current_value (
                GTK_RADIO_ACTION (GVA_ACTION_VIEW_AVAILABLE));
}

void
gva_tree_view_set_selected_view (gint view)
{
        gtk_radio_action_set_current_value (
                GTK_RADIO_ACTION (GVA_ACTION_VIEW_AVAILABLE), view);

        gva_tree_view_set_last_selected_view (view);
}

const gchar *
gva_tree_view_get_last_selected_game (void)
{
        const gchar *retval = NULL;
        GConfClient *client;
        gchar *name;
        GError *error = NULL;

        client = gconf_client_get_default ();
        name = gconf_client_get_string (
                client, GVA_GCONF_SELECTED_GAME_KEY, &error);
        gva_error_handle (&error);
        g_object_unref (client);

        if (name != NULL)
        {
                retval = g_intern_string (name);
                g_free (name);
        }

        return retval;
}

void
gva_tree_view_set_last_selected_game (const gchar *name)
{
        GConfClient *client;
        GError *error = NULL;

        g_return_if_fail (name != NULL);

        client = gconf_client_get_default ();
        gconf_client_set_string (
                client, GVA_GCONF_SELECTED_GAME_KEY, name, &error);
        gva_error_handle (&error);
        g_object_unref (client);
}

gint
gva_tree_view_get_last_selected_view (void)
{
        GConfClient *client;
        gint view;
        GError *error = NULL;

        client = gconf_client_get_default ();
        view = gconf_client_get_int (
                client, GVA_GCONF_SELECTED_VIEW_KEY, &error);
        gva_error_handle (&error);
        g_object_unref (client);

        return view;
}

void
gva_tree_view_set_last_selected_view (gint view)
{
        GConfClient *client;
        GError *error = NULL;

        client = gconf_client_get_default ();
        gconf_client_set_int (
                client, GVA_GCONF_SELECTED_VIEW_KEY, view, &error);
        gva_error_handle (&error);
        g_object_unref (client);
}

void
gva_tree_view_get_last_sort_column_id (GvaGameStoreColumn *column_id,
                                       GtkSortType *order)
{
        GConfClient *client;
        gchar *column_name = NULL;
        gboolean descending;
        GError *error = NULL;

        g_return_if_fail (column_id != NULL);
        g_return_if_fail (order != NULL);

        client = gconf_client_get_default ();
        gconf_client_get_pair (
                client, GVA_GCONF_SORT_COLUMN_KEY,
                GCONF_VALUE_STRING, GCONF_VALUE_BOOL,
                &column_name, &descending, &error);
        gva_error_handle (&error);
        if (column_name == NULL)
                *column_id = GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID;
        else if (!gva_columns_lookup_id (column_name, column_id))
                *column_id = GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID;
        *order = descending ? GTK_SORT_DESCENDING : GTK_SORT_ASCENDING;
        g_object_unref (client);
}

void
gva_tree_view_set_last_sort_column_id (GvaGameStoreColumn column_id,
                                       GtkSortType order)
{
        GConfClient *client;
        const gchar *column_name;
        gboolean descending;
        GError *error = NULL;

        client = gconf_client_get_default ();
        column_name = gva_columns_lookup_name (column_id);
        if (column_name == NULL)
                column_name = "default";
        descending = (order == GTK_SORT_DESCENDING);
        gconf_client_set_pair (
                client, GVA_GCONF_SORT_COLUMN_KEY,
                GCONF_VALUE_STRING, GCONF_VALUE_BOOL,
                &column_name, &descending, &error);
        gva_error_handle (&error);
        g_object_unref (client);
}
