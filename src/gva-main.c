#include "gva-main.h"

#include "gva-tree-view.h"
#include "gva-ui.h"

static guint menu_tooltip_cid;

static void
main_menu_item_select_cb (GtkItem *item, GtkAction *action)
{
        GtkStatusbar *statusbar;
        gchar *tooltip;

        statusbar = GTK_STATUSBAR (GVA_WIDGET_MAIN_STATUSBAR);
        g_object_get (G_OBJECT (action), "tooltip", &tooltip, NULL);

        if (tooltip != NULL)
        {
                gtk_statusbar_push (statusbar, menu_tooltip_cid, tooltip);
                g_free (tooltip);
        }
}

static void
main_menu_item_deselect_cb (GtkItem *item)
{
        GtkStatusbar *statusbar;

        statusbar = GTK_STATUSBAR (GVA_WIDGET_MAIN_STATUSBAR);
        gtk_statusbar_pop (statusbar, menu_tooltip_cid);
}

static void
main_window_destroy_cb (GtkObject *object)
{
        gtk_action_activate (GVA_ACTION_QUIT);
}

void
gva_main_init (void)
{
        gva_tree_view_init ();

        gtk_box_pack_start (
                GTK_BOX (GVA_WIDGET_MAIN_VBOX),
                gva_ui_get_managed_widget ("/main-menu"),
                FALSE, FALSE, 0);

        gtk_action_connect_proxy (
                GVA_ACTION_VIEW_AVAILABLE,
                GVA_WIDGET_MAIN_VIEW_BUTTON_0);

        gtk_action_connect_proxy (
                GVA_ACTION_VIEW_FAVORITES,
                GVA_WIDGET_MAIN_VIEW_BUTTON_1);

        gtk_action_connect_proxy (
                GVA_ACTION_VIEW_RESULTS,
                GVA_WIDGET_MAIN_VIEW_BUTTON_2);

        gtk_action_connect_proxy (
                GVA_ACTION_PROPERTIES,
                GVA_WIDGET_MAIN_PROPERTIES_BUTTON);

        gtk_action_connect_proxy (
                GVA_ACTION_START,
                GVA_WIDGET_MAIN_START_GAME_BUTTON);

        g_signal_connect (
                GVA_WIDGET_MAIN_WINDOW, "destroy",
                G_CALLBACK (main_window_destroy_cb), NULL);

        gva_tree_view_set_selected_view (
                gva_tree_view_get_last_selected_view ());
}

void
gva_main_connect_proxy_cb (GtkUIManager *manager,
                           GtkAction *action,
                           GtkWidget *proxy)
{
        /* Show GtkMenuItem tooltips in the statusbar. */
        if (GTK_IS_MENU_ITEM (proxy))
        {
                GtkStatusbar *statusbar;

                statusbar = GTK_STATUSBAR (GVA_WIDGET_MAIN_STATUSBAR);

                g_signal_connect (
                        proxy, "select",
                        G_CALLBACK (main_menu_item_select_cb), action);
                g_signal_connect (
                        proxy, "deselect",
                        G_CALLBACK (main_menu_item_deselect_cb), NULL);

                menu_tooltip_cid = gtk_statusbar_get_context_id (
                        statusbar, G_STRFUNC);
        }
}
