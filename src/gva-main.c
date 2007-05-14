#include "gva-main.h"

#include <string.h>

#include "gva-cell-renderer-pixbuf.h"
#include "gva-favorites.h"
#include "gva-game-db.h"
#include "gva-game-store.h"
#include "gva-models.h"
#include "gva-ui.h"

#define GCONF_SELECTED_GAME_KEY         GVA_GCONF_PREFIX "/selected-game"
#define GCONF_SELECTED_VIEW_KEY         GVA_GCONF_PREFIX "/selected-view"

static void
main_window_destroy_cb (GtkObject *object)
{
        gtk_action_activate (GVA_ACTION_QUIT);
}

static gboolean
main_tree_view_popup_menu_cb (GtkTreeView *view, GdkEventButton *event)
{
        GtkWidget *menu;

        menu = gva_ui_get_managed_widget ("/game-popup");

        if (event != NULL)
                gtk_menu_popup (
                        GTK_MENU (menu), NULL, NULL, NULL, NULL,
                        event->button, event->time);
        else
                gtk_menu_popup (
                        GTK_MENU (menu), NULL, NULL, NULL, NULL,
                        0, gtk_get_current_event_time ());

        return TRUE;
}

static gboolean
main_tree_view_button_press_cb (GtkTreeView *view, GdkEventButton *event)
{
        if (event->button == 3 && event->type == GDK_BUTTON_PRESS)
        {
                GtkTreePath *path;
                gboolean path_valid;

                /* Select the row that was clicked. */
                path_valid = gtk_tree_view_get_path_at_pos (
                        view, event->x, event->y, &path, NULL, NULL, NULL);
                g_assert (path_valid);
                gtk_tree_selection_select_path (
                        gtk_tree_view_get_selection (view), path);
                gtk_tree_path_free (path);

                return main_tree_view_popup_menu_cb (view, event);
        }

        return FALSE;
}

static void
main_tree_view_favorite_clicked_cb (GvaCellRendererPixbuf *renderer,
                                    GtkTreePath *path, GtkTreeView *view)
{
        GtkTreeModel *model;
        GtkTreeIter iter;
        gboolean favorite;
        gboolean iter_set;
        gchar *romname;

        model = gtk_tree_view_get_model (view);

        iter_set = gtk_tree_model_get_iter (model, &iter, path);
        g_assert (iter_set);
        gtk_tree_model_get (
                model, &iter, GVA_GAME_STORE_COLUMN_ROMNAME, &romname,
                GVA_GAME_STORE_COLUMN_FAVORITE, &favorite,-1);

        favorite = !favorite;  /* Toggle */

        /* Don't assume direct access to the GtkListStore. */
        model = gva_game_db_get_model ();
        path = gva_game_db_lookup (romname);
        iter_set = gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_path_free (path);
        g_assert (iter_set);

        gtk_list_store_set (
                GTK_LIST_STORE (model), &iter,
                GVA_GAME_STORE_COLUMN_FAVORITE, favorite, -1);
        if (favorite)
                gva_favorites_insert (romname);
        else
                gva_favorites_remove (romname);

        g_free (romname);
}

static void
main_tree_view_row_activated_cb (GtkTreeView *view,
                                 GtkTreePath *path,
                                 GtkTreeViewColumn *column)
{
        gtk_action_activate (GVA_ACTION_START);
}

static void
main_tree_selection_changed_cb (GtkTreeSelection *selection)
{
        gchar *romname;
        gboolean sensitive;

        romname = gva_main_get_selected_game ();
        if (romname != NULL)
        {
                GConfClient *client;
                GError *error = NULL;

                client = gconf_client_get_default ();

                gconf_client_set_string (
                        client, GCONF_SELECTED_GAME_KEY, romname, &error);
                if (error != NULL)
                {
                        g_warning ("%s", error->message);
                        g_clear_error (&error);
                }

                g_object_unref (client);
                g_free (romname);
        }

        sensitive = (gtk_tree_selection_count_selected_rows (selection) > 0);

        gtk_action_set_sensitive (GVA_ACTION_PROPERTIES, sensitive);
        gtk_action_set_sensitive (GVA_ACTION_START, sensitive);
}

static void
main_tree_select_default (GtkTreeView *view)
{
        gchar *romname;
        GConfClient *client;
        GtkTreePath *path = NULL;
        GError *error = NULL;

        client = gconf_client_get_default ();

        romname = gconf_client_get_string (
                client, GCONF_SELECTED_GAME_KEY, &error);

        if (romname != NULL)
        {
                path = gva_models_get_path (romname);
                g_free (romname);
        }
        else if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_clear_error (&error);
        }

        if (path == NULL)
                path = gtk_tree_path_new_first ();

        gva_main_tree_view_select_path (path);

        gtk_tree_path_free (path);

        g_object_unref (client);
}

static gboolean
main_tree_search_equal (GtkTreeModel *model, gint column,
                        const gchar *key, GtkTreeIter *iter)
{
        const gchar *valid;
        gchar **str_array;
        gchar *title;
        gboolean retval;

        /* Filter out spaces and punctuation so that, e.g. typing
         * "mspacman" will match "Ms. Pac-Man". */

        valid = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789";

        /* XXX The 'column' argument might be wrong depending on what
         *     model is being used.  I would expect it to always be
         *     GVA_GAME_STORE_COLUMN_TITLE.  Not sure if this is a bug
         *     in GTK+ or if my understanding is flawed (probably the
         *     latter).  In any case, override the column value just
         *     to be safe. */
        column = GVA_GAME_STORE_COLUMN_TITLE;
        gtk_tree_model_get (model, iter, column, &title, -1);
        g_assert (title != NULL);

        g_strcanon (title, valid, '?');
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
gva_main_init (void)
{
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        GtkTreeView *view;

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);

        gtk_box_pack_start (
                GTK_BOX (GVA_WIDGET_MAIN_VBOX),
                gva_ui_get_managed_widget ("/main-menu"),
                FALSE, FALSE, 0);

        gtk_tree_view_set_model (view, gva_game_db_get_model ());

        /* Favorite Column */
        renderer = gva_cell_renderer_pixbuf_new ();
        g_object_set (
                renderer, "icon-name", "emblem-favorite",
                "stock-size", GTK_ICON_SIZE_MENU, NULL);
        g_signal_connect (
                renderer, "clicked",
                G_CALLBACK (main_tree_view_favorite_clicked_cb), view);
        column = gtk_tree_view_column_new_with_attributes (
                _("Favorite"), renderer, "sensitive",
                GVA_GAME_STORE_COLUMN_FAVORITE, NULL);
        gtk_tree_view_append_column (view, column);

        /* Title Column */
        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes (
                _("Title"), renderer, "text",
                GVA_GAME_STORE_COLUMN_TITLE, NULL);
        gtk_tree_view_column_set_sort_column_id (
                column, GVA_GAME_STORE_COLUMN_TITLE);
        gtk_tree_view_append_column (view, column);

        gtk_tree_view_set_search_column (
                view, GVA_GAME_STORE_COLUMN_TITLE);
        gtk_tree_view_set_search_equal_func (
                view, (GtkTreeViewSearchEqualFunc)
                main_tree_search_equal, NULL, NULL);

        gtk_menu_attach_to_widget (
                GTK_MENU (gva_ui_get_managed_widget ("/game-popup")),
                GVA_WIDGET_MAIN_TREE_VIEW, NULL);

        gtk_action_connect_proxy (
                GVA_ACTION_VIEW_AVAILABLE,
                GVA_WIDGET_MAIN_AVAILABLE_BUTTON);

        gtk_action_connect_proxy (
                GVA_ACTION_VIEW_FAVORITES,
                GVA_WIDGET_MAIN_FAVORITES_BUTTON);

        gtk_action_connect_proxy (
                GVA_ACTION_VIEW_RESULTS,
                GVA_WIDGET_MAIN_RESULTS_BUTTON);

        gtk_action_connect_proxy (
                GVA_ACTION_PROPERTIES,
                GVA_WIDGET_MAIN_PROPERTIES_BUTTON);

        gtk_action_connect_proxy (
                GVA_ACTION_START,
                GVA_WIDGET_MAIN_START_GAME_BUTTON);

        g_signal_connect (
                GVA_WIDGET_MAIN_WINDOW, "destroy",
                G_CALLBACK (main_window_destroy_cb), NULL);

        g_signal_connect (
                GVA_WIDGET_MAIN_TREE_VIEW, "button-press-event",
                G_CALLBACK (main_tree_view_button_press_cb), NULL);

        g_signal_connect (
                GVA_WIDGET_MAIN_TREE_VIEW, "popup-menu",
                G_CALLBACK (main_tree_view_popup_menu_cb), NULL);

        g_signal_connect (
                GVA_WIDGET_MAIN_TREE_VIEW, "row-activated",
                G_CALLBACK (main_tree_view_row_activated_cb), NULL);

        g_signal_connect (
                gtk_tree_view_get_selection (view), "changed",
                G_CALLBACK (main_tree_selection_changed_cb), NULL);

        /* This sets a model for 'view'. */
        gtk_radio_action_set_current_value (
                GTK_RADIO_ACTION (GVA_ACTION_VIEW_AVAILABLE),
                gva_models_get_current_model ());

        main_tree_select_default (view);
}

gchar *
gva_main_get_selected_game (void)
{
        GtkTreeSelection *selection;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gchar *romname = NULL;

        selection = gtk_tree_view_get_selection (
                GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW));
        if (gtk_tree_selection_get_selected (selection, &model, &iter))
                gtk_tree_model_get (
                        model, &iter,
                        GVA_GAME_STORE_COLUMN_ROMNAME, &romname, -1);

        return romname;
}

void
gva_main_tree_view_select_path (GtkTreePath *path)
{
        GtkTreeSelection *selection;
        GtkTreeView *view;

        g_return_if_fail (path != NULL);

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);
        selection = gtk_tree_view_get_selection (view);
        gtk_tree_selection_select_path (selection, path);
        gtk_tree_view_scroll_to_cell (view, path, NULL, TRUE, 0.5, 0.0);
}
