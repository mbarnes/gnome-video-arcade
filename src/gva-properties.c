#include "gva-properties.h"

#include "gva-game-db.h"
#include "gva-game-store.h"
#include "gva-ui.h"
#include "gva-util.h"

static void
properties_update_header (GtkTreeModel *model,
                          GtkTreeIter *iter)
{
        GtkLabel *label;
        gchar *description;
        gchar *manufacturer;
        gchar *year;
        gchar *markup;

        label = GTK_LABEL (GVA_WIDGET_PROPERTIES_HEADER);

        gtk_tree_model_get (
                model, iter,
                GVA_GAME_STORE_COLUMN_DESCRIPTION, &description,
                GVA_GAME_STORE_COLUMN_MANUFACTURER, &manufacturer,
                GVA_GAME_STORE_COLUMN_YEAR, &year, -1);

        if (description == NULL)
                description = g_strdup (_("(Game Description Unknown)"));

        if (manufacturer == NULL)
                manufacturer = g_strdup (_("(Manufacturer Unknown)"));

        if (year == NULL)
                year = g_strdup (_("(Year Unknown)"));

        markup = g_markup_printf_escaped (
                "<big><b>%s</b></big>\n<small>%s, %s</small>",
                description, manufacturer, year);
        gtk_label_set_markup (label, markup);
        g_free (markup);

        g_free (description);
        g_free (manufacturer);
        g_free (year);
}

static void
properties_update_history (GtkTreeModel *model,
                           GtkTreeIter *iter)
{
        GtkTextView *view;
        GtkTextBuffer *buffer;
        gchar *history;
        gchar *name;
        gchar *cloneof;
        GError *error = NULL;

        view = GTK_TEXT_VIEW (GVA_WIDGET_PROPERTIES_HISTORY_TEXT_VIEW);
        buffer = gtk_text_view_get_buffer (view);

        gtk_tree_model_get (
                model, iter, GVA_GAME_STORE_COLUMN_NAME, &name,
                GVA_GAME_STORE_COLUMN_CLONEOF, &cloneof, -1);

        history = gva_game_db_get_history (name, &error);
        if (history == NULL && error == NULL && cloneof != NULL)
                history = gva_game_db_get_history (cloneof, &error);

        if (history == NULL)
        {
                if (error != NULL)
                {
                        history = g_strdup_printf (
                                _("Error while fetching history:\n%s"),
                                error->message);
                        g_clear_error (&error);
                }
                else
                        history = g_strdup (_("History not available"));
        }

        gtk_text_buffer_set_text (buffer, history, -1);

        g_free (history);
        g_free (cloneof);
        g_free (name);
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
        properties_update_history (model, &iter);
}

void
gva_properties_init (void)
{
        GtkWindow *window;
        GtkTreeView *view;
        GtkWidget *text_view;
        PangoFontDescription *desc;
        gchar *font_name;
        GError *error = NULL;

        window = GTK_WINDOW (GVA_WIDGET_PROPERTIES_WINDOW);
        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);
        text_view = GVA_WIDGET_PROPERTIES_HISTORY_TEXT_VIEW;

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

        font_name = gva_get_monospace_font_name ();
        desc = pango_font_description_from_string (font_name);
        gtk_widget_modify_font (text_view, desc);
        pango_font_description_free (desc);
        g_free (font_name);
}
