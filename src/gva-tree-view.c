#include "gva-tree-view.h"

#include <string.h>

#include "gva-cell-renderer-pixbuf.h"
#include "gva-favorites.h"
#include "gva-game-db.h"
#include "gva-game-store.h"
#include "gva-ui.h"

#define GCONF_SELECTED_GAME_KEY         GVA_GCONF_PREFIX "/selected-game"

#define NUM_TREE_VIEWS 3

static GtkTreeView *views[NUM_TREE_VIEWS];

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
                g_assert (valid);
                gtk_tree_view_set_cursor (view, path, NULL, FALSE);
                gtk_widget_grab_focus (GTK_WIDGET (view));
                gtk_tree_path_free (path);

                return tree_view_popup_menu_cb (view, event, menu);
        }

        return FALSE;
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
        gchar *romname;
        gboolean sensitive;

        romname = gva_tree_view_get_selected_game ();
        if (romname != NULL)
        {
                GConfClient *client;
                GError *error = NULL;

                client = gconf_client_get_default ();
                gconf_client_set_string (
                        client, GCONF_SELECTED_GAME_KEY, romname, &error);
                g_object_unref (client);

                if (error != NULL)
                {
                        g_warning ("%s", error->message);
                        g_clear_error (&error);
                }

                g_free (romname);
        }

        sensitive = (gtk_tree_selection_count_selected_rows (selection) > 0);

        gtk_action_set_sensitive (GVA_ACTION_PROPERTIES, sensitive);
        gtk_action_set_sensitive (GVA_ACTION_START, sensitive);
}

static void
tree_view_favorite_clicked_cb (GvaCellRendererPixbuf *renderer,
                               GtkTreePath *path,
                               GtkTreeView *view)
{
        GtkTreeModel *model;
        GtkTreeIter iter;
        gboolean favorite;
        gboolean valid;
        gchar *romname;

        model = gtk_tree_view_get_model (view);
        valid = gtk_tree_model_get_iter (model, &iter, path);
        g_assert (valid);
        gtk_tree_model_get (
                model, &iter, GVA_GAME_STORE_COLUMN_ROMNAME, &romname,
                GVA_GAME_STORE_COLUMN_FAVORITE, &favorite, -1);

        favorite = !favorite;  /* toggle */

        /* Don't assume direct access to the GtkListStore. */
        model = gva_game_db_get_model ();
        path = gva_game_db_lookup (romname);
        valid = gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_path_free (path);
        g_assert (valid);

        gtk_list_store_set (
                GTK_LIST_STORE (model), &iter,
                GVA_GAME_STORE_COLUMN_FAVORITE, favorite, -1);
        if (favorite)
                gva_favorites_insert (romname);
        else
                gva_favorites_remove (romname);

        g_free (romname);
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

        g_assert (column == GVA_GAME_STORE_COLUMN_TITLE);
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

static GtkTreeViewColumn *
tree_view_column_new_favorite (GtkTreeView *view)
{
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;

        renderer = gva_cell_renderer_pixbuf_new ();
        g_object_set (
                renderer, "icon-name", "emblem-favorite",
                "stock-size", GTK_ICON_SIZE_MENU, NULL);
        g_signal_connect (
                renderer, "clicked",
                G_CALLBACK (tree_view_favorite_clicked_cb), view);
        column = gtk_tree_view_column_new_with_attributes (
                _("Favorite"), renderer, "sensitive",
                GVA_GAME_STORE_COLUMN_FAVORITE, NULL);

        return column;
}

static GtkTreeViewColumn *
tree_view_column_new_title (GtkTreeView *view)
{
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;

        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes (
                _("Title"), renderer, "text",
                GVA_GAME_STORE_COLUMN_TITLE, NULL);
        gtk_tree_view_column_set_sort_column_id (
                column, GVA_GAME_STORE_COLUMN_TITLE);
        gtk_tree_view_set_search_column (
                view, GVA_GAME_STORE_COLUMN_TITLE);
        gtk_tree_view_set_search_equal_func (
                view, (GtkTreeViewSearchEqualFunc)
                tree_view_search_equal, NULL, NULL);

        return column;
}

static void
tree_view_init_common (GtkTreeView *view, const gchar *popup_path)
{
        GtkMenu *menu;

        menu = GTK_MENU (gva_ui_get_managed_widget (popup_path));
        gtk_menu_attach_to_widget (menu, GTK_WIDGET (view), NULL);

        g_signal_connect (
                view, "button-press-event",
                G_CALLBACK (tree_view_button_press_cb), menu);

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

static void
tree_view_init_0 (void)  /* Available Games */
{
        GtkTreeViewColumn *column;
        GtkTreeView *view = views[0];
        GtkTreeModel *model;

        tree_view_init_common (view, "/game-popup-0");

        column = tree_view_column_new_favorite (view);
        gtk_tree_view_append_column (view, column);

        column = tree_view_column_new_title (view);
        gtk_tree_view_append_column (view, column);

        model = gva_game_db_get_model ();
        gtk_tree_view_set_model (view, model);
}

static void
tree_view_init_1 (void)  /* Favorite Games */
{
        GtkTreeViewColumn *column;
        GtkTreeView *view = views[1];
        GtkTreeModel *model;

        tree_view_init_common (view, "/game-popup-1");

        column = tree_view_column_new_favorite (view);
        gtk_tree_view_append_column (view, column);

        column = tree_view_column_new_title (view);
        gtk_tree_view_append_column (view, column);

        model = gva_game_db_get_model ();
        model = gtk_tree_model_filter_new (model, NULL);
        gtk_tree_model_filter_set_visible_column (
                GTK_TREE_MODEL_FILTER (model),
                GVA_GAME_STORE_COLUMN_FAVORITE);
        gtk_tree_view_set_model (view, model);
}

static void
tree_view_init_2 (void)  /* Search Results */
{
        GtkTreeViewColumn *column;
        GtkTreeView *view = views[2];
        GtkTreeModel *model;

        tree_view_init_common (view, "/game-popup-2");

        column = tree_view_column_new_favorite (view);
        gtk_tree_view_append_column (view, column);

        column = tree_view_column_new_title (view);
        gtk_tree_view_append_column (view, column);

        model = NULL; /* TODO */
        gtk_tree_view_set_model (view, model);
}

static void
tree_view_init_select_game (void)
{
        GConfClient *client;
        gchar *romname;
        GError *error = NULL;

        client = gconf_client_get_default ();
        romname = gconf_client_get_string (
                client, GCONF_SELECTED_GAME_KEY, &error);
        g_object_unref (client);

        if (romname != NULL)
        {
                gva_tree_view_set_selected_game (romname);
                g_free (romname);
        }
        else
        {
                g_assert (error != NULL);
                g_warning ("%s", error->message);
                g_error_free (error);
        }
}

static void
tree_view_select_path (GtkTreeView *view, GtkTreePath *path)
{
        g_assert (path != NULL);
        gtk_tree_view_set_cursor (view, path, NULL, FALSE);
        gtk_widget_grab_focus (GTK_WIDGET (view));
        gtk_tree_view_scroll_to_cell (view, path, NULL, TRUE, 0.5, 0.0);
}

void
gva_tree_view_init (void)
{
        GConfClient *client;
        gchar *romname;
        GError *error = NULL;

        views[0] = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW_0);
        views[1] = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW_1);
        views[2] = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW_2);

        tree_view_init_0 ();
        tree_view_init_1 ();
        tree_view_init_2 ();

        tree_view_init_select_game ();
}

gchar *
gva_tree_view_get_selected_game (void)
{
        GtkTreeSelection *selection;
        GtkNotebook *notebook;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gchar *romname = NULL;
        gint page;

        notebook = GTK_NOTEBOOK (GVA_WIDGET_MAIN_NOTEBOOK);
        page = gtk_notebook_get_current_page (notebook);
        selection = gtk_tree_view_get_selection (views[page]);
        if (gtk_tree_selection_get_selected (selection, &model, &iter))
                gtk_tree_model_get (
                        model, &iter,
                        GVA_GAME_STORE_COLUMN_ROMNAME, &romname, -1);

        return romname;
}

void
gva_tree_view_set_selected_game (const gchar *romname)
{
        GtkTreeModel *model;
        GtkTreePath *child_path;
        GtkTreePath *path;

        g_return_if_fail (romname != NULL);

        child_path = gva_game_db_lookup (romname);
        g_return_if_fail (child_path != NULL);

        tree_view_select_path (views[0], child_path);

        model = gtk_tree_view_get_model (views[1]);
        path = gtk_tree_model_filter_convert_child_path_to_path (
                GTK_TREE_MODEL_FILTER (model), child_path);
        if (path != NULL)
        {
                tree_view_select_path (views[1], path);
                gtk_tree_path_free (path);
        }

        /* TODO: Search results view */
}
