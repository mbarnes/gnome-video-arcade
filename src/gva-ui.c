#include "gva-ui.h"

#include <glade/glade.h>

#include "gva-actions.h"
#include "gva-tree-view.h"
#include "gva-util.h"

static GladeXML *xml = NULL;
static GtkUIManager *manager = NULL;

static void
main_window_destroy_cb (GtkObject *object)
{
        gtk_action_activate (gva_get_action ("quit"));
}

static gboolean
popup_menu_cb (GtkTreeView *tree_view, GdkEventButton *event)
{
        GtkWidget *menu;

        menu = gtk_ui_manager_get_widget (manager, "/game-popup");

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

gboolean
button_press_cb (GtkTreeView *tree_view, GdkEventButton *event)
{
        if (event->button == 3 && event->type == GDK_BUTTON_PRESS)
        {
                GtkTreePath *tree_path;
                gboolean path_valid;

                /* Select the row that was clicked. */
                path_valid = gtk_tree_view_get_path_at_pos (
                        tree_view, event->x, event->y,
                        &tree_path, NULL, NULL, NULL);
                g_assert (path_valid);
                gtk_tree_selection_select_path (
                        gtk_tree_view_get_selection (tree_view), tree_path);
                gtk_tree_path_free (tree_path);

                return popup_menu_cb (tree_view, event);
        }

        return FALSE;
}

static void
row_activated_cb (GtkTreeView *tree_view, GtkTreePath *tree_path,
                  GtkTreeViewColumn *tree_view_column)
{
        gtk_action_activate (gva_get_action ("start"));
}

void
gva_ui_init (GvaGameStore *game_store)
{
        GtkWidget *widget;
        gchar *filename;

        g_return_if_fail (game_store != NULL);

        /* Load the UI files. */

        filename = gva_find_data_file (PACKAGE ".glade");
        if (filename != NULL && xml == NULL)
                xml = glade_xml_new (filename, NULL, NULL);
        g_free (filename);

        filename = gva_find_data_file (PACKAGE ".ui");
        if (filename != NULL && manager == NULL)
        {
                manager = gtk_ui_manager_new ();
                gtk_ui_manager_add_ui_from_file (
                        manager, filename, NULL);
                gtk_ui_manager_insert_action_group (
                        manager, gva_get_action_group (), 0);
        }
        g_free (filename);

        if (xml == NULL || manager == NULL)
                g_error ("%s", _("Failed to initialize user interface"));

        /* Wire things up. */

        gtk_tree_sortable_set_sort_column_id (
                GTK_TREE_SORTABLE (game_store),
                GVA_GAME_STORE_COLUMN_TITLE,
                GTK_SORT_ASCENDING);

        g_signal_connect (
                glade_xml_get_widget (xml, "main_window"), "destroy",
                G_CALLBACK (main_window_destroy_cb), NULL);

        gtk_box_pack_start (
                GTK_BOX (glade_xml_get_widget (xml, "main_vbox")),
                gtk_ui_manager_get_widget (manager, "/main-menu"),
                FALSE, FALSE, 0);

        widget = glade_xml_get_widget (xml, "games_tree_view");
        gva_tree_view_init (GTK_TREE_VIEW (widget), game_store);
        gtk_menu_attach_to_widget (
                GTK_MENU (gtk_ui_manager_get_widget (
                manager, "/game-popup")), widget, NULL);
        g_signal_connect (
                widget, "button-press-event",
                G_CALLBACK (button_press_cb), NULL);
        g_signal_connect (
                widget, "popup-menu",
                G_CALLBACK (popup_menu_cb), NULL);
        g_signal_connect (
                widget, "row-activated",
                G_CALLBACK (row_activated_cb), NULL);

        gtk_action_connect_proxy (
                gtk_ui_manager_get_action (
                        manager, "/main-menu/game-menu/start"),
                glade_xml_get_widget (xml, "start_game_button"));

        gtk_action_connect_proxy (
                gtk_ui_manager_get_action (
                        manager, "/main-menu/game-menu/properties"),
                glade_xml_get_widget (xml, "properties_button"));
}

gchar *
gva_ui_get_selected_game (void)
{
        GtkTreeIter tree_iter;
        GtkTreeView *tree_view;
        GtkTreeModel *tree_model;
        gchar *romname = NULL;
        gboolean selected;

        g_return_val_if_fail (xml != NULL, NULL);

        tree_view = GTK_TREE_VIEW (
                glade_xml_get_widget (xml, "games_tree_view"));
        selected = gtk_tree_selection_get_selected (
                gtk_tree_view_get_selection (tree_view),
                &tree_model, &tree_iter);
        if (selected)
                gtk_tree_model_get (
                        tree_model, &tree_iter,
                        GVA_GAME_STORE_COLUMN_ROMNAME, &romname, -1);

        return romname;
}

GtkStatusbar *
gva_ui_get_statusbar (void)
{
        g_return_val_if_fail (xml != NULL, NULL);

        return GTK_STATUSBAR (glade_xml_get_widget (xml, "status_bar"));
}
