#include "gva-main.h"

#include "gva-tree-view.h"
#include "gva-ui.h"

#define GCONF_SELECTED_VIEW_KEY         GVA_GCONF_PREFIX "/selected-view"

static void
main_window_destroy_cb (GtkObject *object)
{
        gtk_action_activate (GVA_ACTION_QUIT);
}

static void
main_notebook_switch_page_cb (GtkNotebook *notebook,
                              GtkNotebookPage *page,
                              guint page_num)
{
        gchar *romname;
        GConfClient *client;
        GError *error = NULL;

        gva_tree_view_refresh_favorites ();

        romname = gva_tree_view_get_selected_game ();
        if (romname != NULL)
        {
                gva_tree_view_set_selected_game (romname);
                g_free (romname);
        }

        client = gconf_client_get_default ();
        gconf_client_set_int (
                client, GCONF_SELECTED_VIEW_KEY, (gint) page_num, &error);
        g_object_unref (client);

        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_error_free (error);
        }
}

void
main_init_select_view (void)
{
        GConfClient *client;
        gint view;
        GError *error = NULL;

        client = gconf_client_get_default ();
        view = gconf_client_get_int (client, GCONF_SELECTED_VIEW_KEY, &error);
        g_object_unref (client);

        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_error_free (error);
        }

        gtk_radio_action_set_current_value (
                GTK_RADIO_ACTION (GVA_ACTION_VIEW_AVAILABLE), view);
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

        g_signal_connect (
                GVA_WIDGET_MAIN_NOTEBOOK, "switch-page",
                G_CALLBACK (main_notebook_switch_page_cb), NULL);

        main_init_select_view ();
}
