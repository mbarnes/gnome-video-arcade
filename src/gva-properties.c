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

#include "gva-properties.h"

#include "gva-error.h"
#include "gva-game-store.h"
#include "gva-history.h"
#include "gva-tree-view.h"
#include "gva-ui.h"
#include "gva-util.h"

#define SQL_SELECT_NAME \
        "SELECT * FROM available WHERE name = \"%s\""

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
                "<big><b>%s</b></big>\n<small>%s %s</small>",
                description, year, manufacturer);
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

        history = gva_history_lookup (name, &error);
        if (history == NULL && error == NULL && cloneof != NULL)
                history = gva_history_lookup (cloneof, &error);

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
        const gchar *name;
        gchar *sql;
        GError *error = NULL;

        name = gva_tree_view_get_selected_game ();

        /* Leave the window alone if a game is not selected. */
        if (name == NULL)
                return;

        sql = g_strdup_printf (SQL_SELECT_NAME, name);
        model = gva_game_store_new_from_query (sql, &error);
        gva_error_handle (&error);
        g_free (sql);

        if (model != NULL)
        {
                GtkTreeIter iter;
                gboolean valid;

                valid = gtk_tree_model_get_iter_first (model, &iter);
                g_assert (valid);

                properties_update_header (model, &iter);
                properties_update_history (model, &iter);

                g_object_unref (model);
        }
}

/**
 * gva_properties_init:
 *
 * Initializes the Properties window.
 *
 * This function should be called once when the application starts.
 **/
void
gva_properties_init (void)
{
        GtkWindow *window;
        GtkTreeView *view;
        GtkWidget *text_view;
        PangoFontDescription *desc;
        gchar *font_name;

        window = GTK_WINDOW (GVA_WIDGET_PROPERTIES_WINDOW);
        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);
        text_view = GVA_WIDGET_PROPERTIES_HISTORY_TEXT_VIEW;

        gtk_action_connect_proxy (
                GVA_ACTION_PREVIOUS_GAME,
                GVA_WIDGET_PROPERTIES_BACK_BUTTON);

        gtk_action_connect_proxy (
                GVA_ACTION_NEXT_GAME,
                GVA_WIDGET_PROPERTIES_FORWARD_BUTTON);

        g_signal_connect (
                gtk_tree_view_get_selection (view), "changed",
                G_CALLBACK (properties_selection_changed_cb), NULL);

        font_name = gva_get_monospace_font_name ();
        desc = pango_font_description_from_string (font_name);
        gtk_widget_modify_font (text_view, desc);
        pango_font_description_free (desc);
        g_free (font_name);
}

/**
 * gva_properties_close_clicked_cb:
 * @window: the "Properties" window
 * @button: the "Close" button
 *
 * Handler for #GtkButton::clicked signals to the "Close" button.
 *
 * Hides @window.
 **/
void
gva_properties_close_clicked_cb (GtkWindow *window,
                                 GtkButton *button)
{
        gtk_widget_hide (GTK_WIDGET (window));
}
