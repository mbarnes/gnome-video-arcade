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
#include "gva-game-db.h"
#include "gva-game-store.h"
#include "gva-main.h"
#include "gva-ui.h"

static GSList *visible_favorites = NULL;
static gboolean properties_loaded = FALSE;

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
        /* This signal gets emitted during shutdown as each column is
         * destroyed.  We don't want to save columns during this phase. */
        if (!(GTK_OBJECT_FLAGS (view) & GTK_IN_DESTRUCTION))
                gva_columns_save (view);
}

static void
tree_view_row_activated_cb (GtkTreeView *view,
                            GtkTreePath *path,
                            GtkTreeViewColumn *column)
{
        gtk_action_activate (GVA_ACTION_START);
}

static gboolean
tree_view_filter_visible (GtkTreeModel *model, GtkTreeIter *iter)
{
        gchar *romname;
        gboolean visible = TRUE;

        switch (gva_tree_view_get_selected_view ())
        {
                case 0:  /* Available Games */
                        visible = TRUE;
                        break;

                case 1:  /* Favorite Games */
                        gtk_tree_model_get (
                                model, iter, GVA_GAME_STORE_COLUMN_NAME,
                                &romname, -1);
                        visible = 
                                g_slist_find (visible_favorites,
                                g_intern_string (romname)) != NULL;
                        g_free (romname);
                        break;

                case 2:  /* Search Results */
                        visible = FALSE;  /* TODO */
                        break;

                default:
                        g_assert_not_reached ();
        }

        return visible;
}

static void
tree_view_selection_changed_cb (GtkTreeSelection *selection)
{
        const gchar *romname;
        gboolean game_is_selected;

        romname = gva_tree_view_get_selected_game ();
        game_is_selected = (romname != NULL);

        if (game_is_selected)
        {
                gboolean favorite;

                favorite = gva_favorites_contains (romname);
                gtk_action_set_sensitive (GVA_ACTION_GO_BACK, TRUE);
                gtk_action_set_sensitive (GVA_ACTION_GO_FORWARD, TRUE);
                gtk_action_set_visible (GVA_ACTION_INSERT_FAVORITE, !favorite);
                gtk_action_set_visible (GVA_ACTION_REMOVE_FAVORITE, favorite);

                gva_tree_view_set_last_selected_game (romname);
        }
        else
        {
                gtk_action_set_sensitive (GVA_ACTION_GO_BACK, FALSE);
                gtk_action_set_sensitive (GVA_ACTION_GO_FORWARD, FALSE);
                gtk_action_set_visible (GVA_ACTION_INSERT_FAVORITE, FALSE);
                gtk_action_set_visible (GVA_ACTION_REMOVE_FAVORITE, FALSE);
        }

        gtk_action_set_sensitive (
                GVA_ACTION_PROPERTIES,
                game_is_selected && properties_loaded);
        gtk_action_set_sensitive (GVA_ACTION_RECORD, game_is_selected);
        gtk_action_set_sensitive (GVA_ACTION_START, game_is_selected);
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

static void
tree_view_data_added (GvaProcess *process, gint status, gpointer user_data)
{
        GtkTreeView *view;
        GtkTreeModel *model;
        GtkTreeSelection *selection;

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);

        gva_main_statusbar_pop (GPOINTER_TO_UINT (user_data));

        properties_loaded = TRUE;

        selection = gtk_tree_view_get_selection (view);
        g_signal_emit_by_name (selection, "changed");

        g_object_unref (process);
}

static void
tree_view_samples_added (GvaProcess *process, gint status)
{
        g_object_unref (process);
}

static void
tree_view_titles_added (GvaProcess *process, gint status)
{
        GtkTreeModel *model;

        model = gva_game_db_get_model ();

        gtk_tree_sortable_set_sort_column_id (
                GTK_TREE_SORTABLE (model),
                GVA_GAME_STORE_COLUMN_DESCRIPTION,
                GTK_SORT_ASCENDING);

        gva_tree_view_update ();

        gtk_widget_set_sensitive (
                GVA_WIDGET_MAIN_TREE_VIEW, TRUE);

        g_object_unref (process);
}

static gboolean
tree_view_load_data (void)
{
        GvaProcess *process;
        guint context_id;
        GError *error = NULL;

        context_id = gva_main_statusbar_get_context_id (G_STRFUNC);

        gva_main_statusbar_push (
                context_id, "%s", _("Loading game properties..."));

        process = gva_game_db_update_samples (&error);
        gva_error_handle (&error);

        if (process != NULL)
                g_signal_connect (
                        process, "exited",
                        G_CALLBACK (tree_view_samples_added), NULL);

        process = gva_game_db_update_titles (&error);
        gva_error_handle (&error);

        if (process != NULL)
                g_signal_connect (
                        process, "exited",
                        G_CALLBACK (tree_view_titles_added), NULL);

        return FALSE;
}

void
gva_tree_view_init (void)
{
        GtkTreeView *view;
        GtkTreeModel *model;
        GtkMenu *menu;
        gchar *romname;

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

        /* Overlay a GtkTreeModelFilter on the GvaGameStore so we can
         * filter the model appropriately for the selected view. */
        model = gva_game_db_get_model ();
        model = gtk_tree_model_filter_new (model, NULL);
        gtk_tree_model_filter_set_visible_func (
                GTK_TREE_MODEL_FILTER (model),
                (GtkTreeModelFilterVisibleFunc)
                tree_view_filter_visible, NULL, NULL);

        /* But by doing so we lose the ability to sort.  So we have
         * to overlay a GtkTreeModelSort on the GtkTreeModelFilter
         * to get both capabilities.  So we wind up with three layers
         * of GtkTreeModels.  Bother. */
        model = gtk_tree_model_sort_new_with_model (model);

        gtk_tree_view_set_model (view, model);

        g_idle_add ((GSourceFunc) tree_view_load_data, NULL);
}

void
gva_tree_view_update (void)
{
        GtkTreeModel *model;
        const gchar *romname;

        g_slist_free (visible_favorites);
        visible_favorites = gva_favorites_copy ();
        model = gtk_tree_view_get_model (
                GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW));
        model = gtk_tree_model_sort_get_model (
                GTK_TREE_MODEL_SORT (model));
        gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (model));

        romname = gva_tree_view_get_last_selected_game ();
        if (romname != NULL)
                gva_tree_view_set_selected_game (romname);
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
                gchar *romname;

                gtk_tree_model_get (
                        model, &iter,
                        GVA_GAME_STORE_COLUMN_NAME, &romname, -1);
                retval = g_intern_string (romname);
                g_free (romname);
        }

        return retval;
}

void
gva_tree_view_set_selected_game (const gchar *romname)
{
        GtkTreeModel *filter_model;
        GtkTreeModel *sorted_model;
        GtkTreePath *gamedb_path;
        GtkTreePath *filter_path;
        GtkTreePath *sorted_path;
        GtkTreeView *view;
        GtkTreeIter iter;

        g_return_if_fail (romname != NULL);

        gamedb_path = gva_game_db_lookup (romname);
        g_return_if_fail (gamedb_path != NULL);

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);

        /* Dig up the GtkTreeModels we need. */
        sorted_model = gtk_tree_view_get_model (view);
        filter_model = gtk_tree_model_sort_get_model (
                GTK_TREE_MODEL_SORT (sorted_model));

        /* Convert the GvaGameStore path to a GtkTreeModelFilter path. */
        filter_path = gtk_tree_model_filter_convert_child_path_to_path (
                GTK_TREE_MODEL_FILTER (filter_model), gamedb_path);

        /* If the game is visible in the current view, convert the
         * GtkTreeModelFilter path to a GtkTreeModelSort path and put the
         * cursor on it.  Otherwise just select the root path. */
        if (filter_path != NULL)
        {
                sorted_path = gtk_tree_model_sort_convert_child_path_to_path (
                        GTK_TREE_MODEL_SORT (sorted_model), filter_path);
                g_assert (sorted_path != NULL);
                gtk_tree_view_set_cursor (view, sorted_path, NULL, FALSE);
                gtk_widget_grab_focus (GTK_WIDGET (view));
                gtk_tree_path_free (filter_path);
        }
        else
                sorted_path = gtk_tree_path_new_first ();

        /* If we have a path to a visible game, scroll to it.  Note that
         * we don't really need the GtkTreeIter here; we're just testing
         * whether the path is valid. */
        if (gtk_tree_model_get_iter (sorted_model, &iter, sorted_path))
                gtk_tree_view_scroll_to_cell (
                        view, sorted_path, NULL, TRUE, 0.5, 0.0);

        gtk_tree_path_free (sorted_path);
        gtk_tree_path_free (gamedb_path);

        gva_tree_view_set_last_selected_game (romname);
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
        gchar *romname;
        GError *error = NULL;

        client = gconf_client_get_default ();
        romname = gconf_client_get_string (
                client, GVA_GCONF_SELECTED_GAME_KEY, &error);
        gva_error_handle (&error);
        g_object_unref (client);

        if (romname != NULL)
        {
                retval = g_intern_string (romname);
                g_free (romname);
        }

        return retval;
}

void
gva_tree_view_set_last_selected_game (const gchar *romname)
{
        GConfClient *client;
        GError *error = NULL;

        g_return_if_fail (romname != NULL);

        client = gconf_client_get_default ();
        gconf_client_set_string (
                client, GVA_GCONF_SELECTED_GAME_KEY, romname, &error);
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
