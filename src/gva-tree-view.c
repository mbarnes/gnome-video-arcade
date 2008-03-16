/* Copyright 2007, 2008 Matthew Barnes
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
#include "gva-preferences.h"
#include "gva-search.h"
#include "gva-ui.h"

#define SQL_SELECT_GAMES \
        "SELECT %s FROM available"

static gboolean
tree_view_show_popup_menu (GdkEventButton *event)
{
        GtkMenu *menu;

        menu = GTK_MENU (gva_ui_get_managed_widget ("/game-popup"));

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

/**
 * gva_tree_view_init:
 *
 * Initializes the main tree view.
 *
 * This function should be called once when the application starts.
 **/
void
gva_tree_view_init (void)
{
        GtkTreeView *view;
        GtkMenu *menu;

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);

        menu = GTK_MENU (gva_ui_get_managed_widget ("/game-popup"));
        gtk_menu_attach_to_widget (menu, GTK_WIDGET (view), NULL);

        g_signal_connect (
                gtk_tree_view_get_selection (view), "changed",
                G_CALLBACK (tree_view_selection_changed_cb), NULL);

        gva_columns_load (view);

        gtk_tree_view_set_search_equal_func (
                view, (GtkTreeViewSearchEqualFunc)
                tree_view_search_equal, NULL, NULL);
}

/**
 * gva_tree_view_lookup:
 * @game: the name of a game
 *
 * Looks up @game in the tree view's store and returns a #GtkTreePath
 * to the corresponding row, or %NULL if @game was not found.
 *
 * Returns: a #GtkTreePath to the row corresponding to @game, or %NULL
 **/
GtkTreePath *
gva_tree_view_lookup (const gchar *game)
{
        GtkTreeModel *model;

        g_return_val_if_fail (game != NULL, NULL);

        model = gva_tree_view_get_model ();

        return gva_game_store_index_lookup (GVA_GAME_STORE (model), game);
}

/**
 * gva_tree_view_update:
 * @error: return location for a #GError, or %NULL
 *
 * Refreshes the contents of the tree view by querying the game database
 * using criteria appropriate for the currently selected game list view.
 * If an error occurs, it returns %FALSE and sets @error.
 *
 * Returns: %TRUE on success, %FALSE if an error occurred
 **/
gboolean
gva_tree_view_update (GError **error)
{
        GString *expr;
        const gchar *name;
        gboolean success;

        expr = g_string_sized_new (128);

        switch (gva_tree_view_get_selected_view ())
        {
                case 0:  /* Available Games */
                        break;

                case 1:  /* Favorite Games */
                        g_string_append (expr, "favorite == \"yes\"");
                        break;

                case 2:  /* Search Results */
                {
                        gchar *text;

                        text = gva_search_get_last_search ();
                        if (text != NULL && *text != '\0')
                                g_string_append_printf (
                                        expr, "(name LIKE '%s' OR "
                                        "category LIKE '%%%s%%' OR "
                                        "sourcefile LIKE '%s' OR "
                                        "description LIKE '%%%s%%' OR "
                                        "manufacturer LIKE '%%%s%%' OR "
                                        "year LIKE '%s')",
                                        text, text, text, text, text, text);
                        else
                                g_string_append (expr, "name ISNULL");
                        g_free (text);
                        break;
                }

                default:
                        g_assert_not_reached ();
        }

        if (!gva_preferences_get_show_clones ())
        {
                if (expr->len > 0)
                        g_string_append (expr, " AND ");
                g_string_append (expr, "cloneof ISNULL");
        }

        success = gva_tree_view_run_query (expr->str, error);
        g_string_free (expr, TRUE);

        if (!success)
                return FALSE;

        name = gva_tree_view_get_last_selected_game ();
        if (name != NULL)
                gva_tree_view_set_selected_game (name);

        return TRUE;
}

/**
 * gva_tree_view_run_query:
 * @expr: an SQL "where" expression
 * @error: return location for a #GError, or %NULL
 *
 * Similar to gva_tree_view_update() but applies custom criteria to the game
 * database query.  If an error occurs, it returns %FALSE and sets @error.
 *
 * Returns: %TRUE on success, %FALSE if an error occurred
 **/
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
        gboolean sensitive;
        gchar *columns;
        gint ii = 0;

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);

        /* Build a comma-separated list of column names. */
        list = gva_columns_get_names_full (view);
        strv = g_new0 (const gchar *, g_slist_length (list) + 1);
        while (list != NULL)
        {
                strv[ii++] = list->data;
                list = g_slist_delete_link (list, list);
        }
        columns = g_strjoinv (", ", (gchar **) strv);
        g_free (strv);

        string = g_string_new (NULL);
        g_string_printf (string, SQL_SELECT_GAMES, columns);
        g_free (columns);

        if (expr != NULL && *expr != '\0')
                g_string_append_printf (string, " WHERE %s", expr);

        window = gtk_widget_get_parent_window (GTK_WIDGET (view));
        display = gtk_widget_get_display (GTK_WIDGET (view));
        cursor = gdk_cursor_new_for_display (display, GDK_WATCH);

        gdk_window_set_cursor (window, cursor);
        gtk_widget_set_sensitive (GTK_WIDGET (view), FALSE);
        model = gva_game_store_new_from_query (string->str, error);
        sensitive = (gtk_tree_model_iter_n_children (model, NULL) > 0);
        gtk_widget_set_sensitive (GTK_WIDGET (view), sensitive);
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
        gtk_tree_view_columns_autosize (view);
        g_object_unref (model);

        /* Need to reset the search column after loading a new model,
         * since GtkTreeView apparenly forgets.  This is not mentioned
         * in the GTK+ documentation (GNOME bug #351910). */
        gtk_tree_view_set_search_column (
                view, GVA_GAME_STORE_COLUMN_DESCRIPTION);

        return TRUE;
}

/**
 * gva_tree_view_get_model:
 *
 * Thin wrapper for gtk_tree_view_get_model() that uses the main tree view.
 *
 * Returns the model the main tree view is based on.  Returns %NULL if the
 * model is unset.
 *
 * Returns: a #GtkTreeModel, or %NULL if the model is unset
 **/
GtkTreeModel *
gva_tree_view_get_model (void)
{
        GtkTreeView *view;

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);

        return gtk_tree_view_get_model (view);
}

/**
 * gva_tree_view_get_selected_game:
 *
 * Returns the name of the game corresponding to the currently selected row,
 * or %NULL if no row is selected.
 *
 * Returns: the name of the selected game, or %NULL if no game is selected
 **/
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
                gchar *game;

                gtk_tree_model_get (
                        model, &iter,
                        GVA_GAME_STORE_COLUMN_NAME, &game, -1);
                retval = g_intern_string (game);
                g_free (game);
        }

        return retval;
}

/**
 * gva_tree_view_set_selected_game:
 * @game: the name of a game
 *
 * Selects the row corresponding to @game.  If the row is invisible in the
 * current view, select the first row instead.  The function also calls
 * gva_tree_view_set_last_selected_game() so that the selected game will be
 * persistent across sessions.
 **/
void
gva_tree_view_set_selected_game (const gchar *game)
{
        GtkTreeModel *model;
        GtkTreeView *view;
        GtkTreePath *path;
        GtkTreeIter iter;

        g_return_if_fail (game != NULL);

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);
        path = gva_tree_view_lookup (game);

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

        gva_tree_view_set_last_selected_game (game);
}

/**
 * gva_tree_view_get_selected_view:
 *
 * Returns the index of the currently selected view.
 *
 * <itemizedlist>
 *   <listitem>0 = Available Games</listitem>
 *   <listitem>1 = Favorite Games</listitem>
 *   <listitem>2 = Search Results</listitem>
 * </itemizedlist>
 *
 * Returns: the index of the currently selected view
 **/
gint
gva_tree_view_get_selected_view (void)
{
        return gtk_radio_action_get_current_value (
                GTK_RADIO_ACTION (GVA_ACTION_VIEW_AVAILABLE));
}

/**
 * gva_tree_view_set_selected_view:
 * @view: index of the view to select
 *
 * Sets the game list view corresponding to index @view.  See
 * gva_tree_view_get_selected_view() for a list of valid indices.
 *
 * Setting the game list view triggers a tree view update.  See
 * gva_tree_view_update() for details.
 **/
void
gva_tree_view_set_selected_view (gint view)
{
        gtk_radio_action_set_current_value (
                GTK_RADIO_ACTION (GVA_ACTION_VIEW_AVAILABLE), view);
}

/**
 * gva_tree_view_get_last_selected_game:
 *
 * Returns the name of the most recently selected game in either the
 * current or the previous session of <emphasis>GNOME Video Arcade</emphasis>.
 * Note that it is still possible for this function to return %NULL.
 *
 * Returns: the name of the most recently selected game
 **/
const gchar *
gva_tree_view_get_last_selected_game (void)
{
        const gchar *retval = NULL;
        GConfClient *client;
        gchar *game;
        GError *error = NULL;

        client = gconf_client_get_default ();
        game = gconf_client_get_string (
                client, GVA_GCONF_SELECTED_GAME_KEY, &error);
        gva_error_handle (&error);
        g_object_unref (client);

        if (game != NULL)
        {
                retval = g_intern_string (game);
                g_free (game);
        }

        return retval;
}

/**
 * gva_tree_view_set_last_selected_game:
 * @game: the name of a game
 *
 * Writes @game to GConf key
 * <filename>/apps/gnome-video-arcade/selected-game</filename>.
 *
 * This is used to remember which game was selected in the previous
 * session of <emphasis>GNOME Video Arcade</emphasis> so that the same
 * game can be selected again automatically at startup.
 **/
void
gva_tree_view_set_last_selected_game (const gchar *game)
{
        GConfClient *client;
        GError *error = NULL;

        g_return_if_fail (game != NULL);

        client = gconf_client_get_default ();
        gconf_client_set_string (
                client, GVA_GCONF_SELECTED_GAME_KEY, game, &error);
        gva_error_handle (&error);
        g_object_unref (client);
}

/**
 * gva_tree_view_get_last_sort_column_id:
 * @column_id: return location for the column ID
 * @order: return location for the sort order
 *
 * Retrieves the most recently selected sort column ID and sort order in
 * either the current or the previous session of
 * <emphasis>GNOME Video Arcade</emphasis>.
 **/
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

/**
 * gva_tree_view_set_last_sort_column_id:
 * @column_id: sort column ID
 * @order: sort order
 *
 * Writes @column_id and @order to GConf key
 * <filename>/apps/gnome-video-arcade/sort-column</filename>.
 *
 * This is used to remember which how the game list view was sorted in
 * the previous session of <emphasis>GNOME Video Arcade</emphasis> so that
 * the same sorting can be applied again automatically at startup.
 **/
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

/**
 * gva_tree_view_button_press_event_cb:
 * @view: the main tree view
 * @event: a #GdkEventButton
 *
 * Handler for #GtkWidget::button-press-event signals to the main tree view.
 *
 * On right-click, selects the row that was clicked and shows a popup menu.
 *
 * Returns: %TRUE if the popup menu was shown, %FALSE otherwise
 **/
gboolean
gva_tree_view_button_press_event_cb (GtkTreeView *view,
                                     GdkEventButton *event)
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

                        return tree_view_show_popup_menu (event);
                }
        }

        return FALSE;
}

/**
 * gva_tree_view_popup_menu_cb:
 * @view: the main tree view
 *
 * Handler for #GtkWidget::popup-menu signals to the main tree view.
 *
 * Shows a popup menu.
 *
 * Returns: %TRUE
 **/
gboolean
gva_tree_view_popup_menu_cb (GtkTreeView *view)
{
        return tree_view_show_popup_menu (NULL);
}

/**
 * gva_tree_view_query_tooltip_cb:
 * @view: the main tree view
 * @x: the x coordinate of the cursor position where the request has
 *     been emitted
 * @y: the y coordinate of the cursor position where the request has
 *     been emitted
 * @keyboard_mode: %TRUE if the tooltip was triggered using the keyboard
 * @tooltip: a #GtkTooltip
 *
 * Handles tooltips for the main tree view.
 *
 * Returns: %TRUE if @tooltip should be shown right now, %FALSE otherwise
 **/
gboolean
gva_tree_view_query_tooltip_cb (GtkTreeView *view,
                                gint x,
                                gint y,
                                gboolean keyboard_mode,
                                GtkTooltip *tooltip)
{
        GtkTreeViewColumn *column;
        GtkTreePath *path;
        gboolean valid;
        gint bx, by;

        /* XXX Don't know how to handle keyboard tooltips yet. */
        if (keyboard_mode)
                return FALSE;

        gtk_tree_view_convert_widget_to_bin_window_coords (
                view, x, y, &bx, &by);
        valid = gtk_tree_view_get_path_at_pos (
                view, bx, by, &path, &column, NULL, NULL);
        if (!valid)
                return FALSE;

        gtk_tree_view_set_tooltip_cell (view, tooltip, path, column, NULL);

        return gva_columns_query_tooltip (column, path, tooltip);

}

/**
 * gva_tree_view_row_activated_cb:
 * @view: the main tree view
 * @path: the #GtkTreePath for the activated row
 * @column: the #GtkTreeViewColumn in which the activation occurred
 *
 * Handler for #GtkTreeView::row-activated signals to the main tree view.
 *
 * Activates the #GVA_ACTION_START action.
 **/
void
gva_tree_view_row_activated_cb (GtkTreeView *view,
                                GtkTreePath *path,
                                GtkTreeViewColumn *column)
{
        gtk_action_activate (GVA_ACTION_START);
}
