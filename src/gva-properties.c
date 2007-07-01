#include "gva-properties.h"

#include "gva-game-store.h"
#include "gva-ui.h"

static void
properties_update_header (GtkTreeModel *model,
                          GtkTreeIter *iter)
{
        GtkLabel *label;
        gchar *title;
        gchar *manufacturer;
        gchar *year;
        gchar *markup;

        label = GTK_LABEL (GVA_WIDGET_PROPERTIES_HEADER);

        gtk_tree_model_get (
                model, iter,
                GVA_GAME_STORE_COLUMN_TITLE, &title,
                GVA_GAME_STORE_COLUMN_MANUFACTURER, &manufacturer,
                GVA_GAME_STORE_COLUMN_YEAR, &year, -1);

        if (title == NULL)
                title = g_strdup (_("(Game Description Unknown)"));

        if (manufacturer == NULL)
                manufacturer = g_strdup (_("(Manufacturer Unknown)"));

        if (year == NULL)
                year = g_strdup (_("(Year Unknown)"));

        markup = g_markup_printf_escaped (
                "<big><b>%s</b></big>\n%s, %s",
                title, manufacturer, year);
        gtk_label_set_markup (label, markup);
        g_free (markup);

        g_free (title);
        g_free (manufacturer);
        g_free (year);
}

static void
properties_selection_changed_cb (GtkTreeSelection *selection)
{
        GtkTreeModel *model;
        GtkTreeIter iter;

        /* Leave the window alone if a game is not selected. */
        if (!gtk_tree_selection_get_selected (selection, &model, &iter))
                return;

        properties_update_header (model, &iter);
}

void
gva_properties_init (void)
{
        GtkWindow *window;
        GtkTreeView *view;

        window = GTK_WINDOW (GVA_WIDGET_PROPERTIES_WINDOW);
        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);

        gtk_action_connect_proxy (
                GVA_ACTION_GO_BACK,
                GVA_WIDGET_PROPERTIES_BACK_BUTTON);

        gtk_action_connect_proxy (
                GVA_ACTION_GO_FORWARD,
                GVA_WIDGET_PROPERTIES_FORWARD_BUTTON);

        g_signal_connect (
                GVA_WIDGET_PROPERTIES_WINDOW, "delete_event",
                G_CALLBACK (gtk_widget_hide_on_delete), NULL);

        g_signal_connect_swapped (
                GVA_WIDGET_PROPERTIES_CLOSE_BUTTON, "clicked",
                G_CALLBACK (gtk_widget_hide), window);

        g_signal_connect (
                gtk_tree_view_get_selection (view), "changed",
                G_CALLBACK (properties_selection_changed_cb), NULL);
}
