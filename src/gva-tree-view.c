#include "gva-tree-view.h"

#include <string.h>

#include "gva-cell-renderer-pixbuf.h"
#include "gva-favorites.h"
#include "gva-game-db.h"
#include "gva-game-store.h"
#include "gva-ui.h"

static GSList *visible_favorites = NULL;

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
        /* TODO */
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
        GSList *element;
        gboolean visible = TRUE;

        gtk_tree_model_get (
                model, iter, GVA_GAME_STORE_COLUMN_ROMNAME, &romname, -1);

        switch (gva_tree_view_get_selected_view ())
        {
                case 0:  /* Available Games */
                        visible = TRUE;
                        break;

                case 1:  /* Favorite Games */
                        visible = 
                                g_slist_find (visible_favorites,
                                g_intern_string (romname)) != NULL;
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
                gtk_action_set_visible (GVA_ACTION_INSERT_FAVORITE, !favorite);
                gtk_action_set_visible (GVA_ACTION_REMOVE_FAVORITE, favorite);

                gva_tree_view_set_last_selected_game (romname);
        }
        else
        {
                gtk_action_set_visible (GVA_ACTION_INSERT_FAVORITE, FALSE);
                gtk_action_set_visible (GVA_ACTION_REMOVE_FAVORITE, FALSE);
        }

        gtk_action_set_sensitive (GVA_ACTION_PROPERTIES, game_is_selected);
        gtk_action_set_sensitive (GVA_ACTION_RECORD, game_is_selected);
        gtk_action_set_sensitive (GVA_ACTION_START, game_is_selected);
}

static void
tree_view_favorite_clicked_cb (GvaCellRendererPixbuf *renderer,
                               GtkTreePath *path,
                               GtkTreeView *view)
{
        const gchar *romname;

        /* The row that was clicked is not yet selected.  We need to
         * select it first so that gva_tree_view_get_selected_game()
         * returns the correct romname. */
        gtk_tree_view_set_cursor (view, path, NULL, FALSE);
        gtk_widget_grab_focus (GTK_WIDGET (view));

        romname = gva_tree_view_get_selected_game ();
        g_assert (romname != NULL);

        if (gva_favorites_contains (romname))
                gtk_action_activate (GVA_ACTION_REMOVE_FAVORITE);
        else
                gtk_action_activate (GVA_ACTION_INSERT_FAVORITE);
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

static GdkPixbuf *
tree_view_get_icon_name (const gchar *icon_name)
{
        GtkIconTheme *icon_theme;
        GdkPixbuf *pixbuf;
        gboolean valid;
        gint size;
        GError *error = NULL;

        icon_theme = gtk_icon_theme_get_default ();
        valid = gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &size, NULL);
        g_assert (valid);
        pixbuf = gtk_icon_theme_load_icon (
                icon_theme, icon_name, size, 0, &error);
        if (pixbuf != NULL)
        {
                GdkPixbuf *scaled;

                scaled = gdk_pixbuf_scale_simple (
                        pixbuf, size, size, GDK_INTERP_BILINEAR);
                g_object_unref (pixbuf);
                pixbuf = scaled;
        }
        else
        {
                g_assert (error != NULL);
                g_warning ("%s", error->message);
                g_error_free (error);
        }

        return pixbuf;
}

static GtkTreeViewColumn *
tree_view_column_new_favorite (GtkTreeView *view)
{
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        GdkPixbuf *pixbuf;

        pixbuf = tree_view_get_icon_name ("emblem-favorite");

        renderer = gva_cell_renderer_pixbuf_new ();
        g_object_set (renderer, "pixbuf", pixbuf, NULL);
        g_signal_connect (
                renderer, "clicked",
                G_CALLBACK (tree_view_favorite_clicked_cb), view);
        column = gtk_tree_view_column_new_with_attributes (
                _("Favorite"), renderer, "sensitive",
                GVA_GAME_STORE_COLUMN_FAVORITE, NULL);
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (
                column, GVA_GAME_STORE_COLUMN_FAVORITE);

        if (pixbuf != NULL)
                g_object_unref (pixbuf);

        return column;
}

static GtkTreeViewColumn *
tree_view_column_new_title (GtkTreeView *view)
{
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;

        renderer = gtk_cell_renderer_text_new ();
        g_object_set (renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
        column = gtk_tree_view_column_new_with_attributes (
                _("Title"), renderer, "text",
                GVA_GAME_STORE_COLUMN_TITLE, NULL);
        gtk_tree_view_set_search_column (
                view, GVA_GAME_STORE_COLUMN_TITLE);
        gtk_tree_view_set_search_equal_func (
                view, (GtkTreeViewSearchEqualFunc)
                tree_view_search_equal, NULL, NULL);
        gtk_tree_view_column_set_expand (column, TRUE);
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (
                column, GVA_GAME_STORE_COLUMN_TITLE);

        return column;
}

static GtkTreeViewColumn *
tree_view_column_new_samples (GtkTreeView *view)
{
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        GdkPixbuf *pixbuf;

        pixbuf = tree_view_get_icon_name ("emblem-sound");

        renderer = gtk_cell_renderer_pixbuf_new ();
        g_object_set (renderer, "pixbuf", pixbuf, NULL);
        column = gtk_tree_view_column_new_with_attributes (
                _("Samples"), renderer, "visible",
                GVA_GAME_STORE_COLUMN_USES_SAMPLES, "sensitive",
                GVA_GAME_STORE_COLUMN_HAVE_SAMPLES, NULL);
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (
                column, GVA_GAME_STORE_COLUMN_USES_SAMPLES);

        if (pixbuf != NULL)
                g_object_unref (pixbuf);

        return column;
}

void
gva_tree_view_init (void)
{
        GtkTreeViewColumn *column;
        GtkTreeView *view;
        GtkTreeModel *model;
        GtkMenu *menu;
        gchar *romname;

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);

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

        column = tree_view_column_new_favorite (view);
        gtk_tree_view_append_column (view, column);

        column = tree_view_column_new_title (view);
        gtk_tree_view_append_column (view, column);

        column = tree_view_column_new_samples (view);
        gtk_tree_view_append_column (view, column);

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

        gva_tree_view_update ();
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
                        GVA_GAME_STORE_COLUMN_ROMNAME, &romname, -1);
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
        {
                gtk_tree_view_scroll_to_cell (
                        view, sorted_path, NULL, TRUE, 0.5, 0.0);
                gtk_tree_path_free (sorted_path);
        }

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
        g_object_unref (client);

        if (romname != NULL)
        {
                retval = g_intern_string (romname);
                g_free (romname);
        }
        else if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_error_free (error);
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
        g_object_unref (client);

        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_error_free (error);
        }
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
        g_object_unref (client);

        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_error_free (error);
        }

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
        g_object_unref (client);

        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_error_free (error);
        }
}
