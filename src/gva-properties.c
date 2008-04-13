/* Copyright 2007, 2008 Matthew Barnes
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

#include <stdlib.h>

#include "gva-db.h"
#include "gva-error.h"
#include "gva-game-store.h"
#include "gva-history.h"
#include "gva-tree-view.h"
#include "gva-ui.h"
#include "gva-util.h"

#define SQL_SELECT_NAME \
        "SELECT * FROM available WHERE name = \"%s\""

#define SQL_SELECT_CPU \
        "SELECT COUNT(*), name, clock FROM chip " \
        "WHERE game = \"%s\" AND type = \"cpu\" " \
        "GROUP BY name, clock ORDER BY CAST (clock AS INT) DESC LIMIT %d"

#define SQL_SELECT_SOUND \
        "SELECT COUNT(*), name, clock FROM chip " \
        "WHERE game = \"%s\" AND type = \"audio\" " \
        "GROUP BY name, clock ORDER BY CAST (clock AS INT) DESC LIMIT %d"

#define SQL_SELECT_VIDEO \
        "SELECT type, rotate, width, height, refresh " \
        "FROM display WHERE game = \"%s\" LIMIT %d"

/* Keep this in sync with the Glade file. */
enum
{
        NOTEBOOK_PAGE_HISTORY,
        NOTEBOOK_PAGE_GALLERY,
        NOTEBOOK_PAGE_TECHNICAL
};

static const gchar *cpu_labels[] =
{
        "properties-cpu0-label",
        "properties-cpu1-label",
        "properties-cpu2-label",
        "properties-cpu3-label"
};

static const gchar *sound_labels[] =
{
        "properties-sound0-label",
        "properties-sound1-label",
        "properties-sound2-label",
        "properties-sound3-label"
};

static const gchar *video_labels[] =
{
        "properties-video0-label",
        "properties-video1-label",
        "properties-video2-label",
        "properties-video3-label"
};

static gchar *
properties_cpu_description (const gchar *name,
                            const gchar *clock,
                            gint count)
{
        GString *string;

        string = g_string_sized_new (128);

        if (count > 1)
                g_string_append_printf (string, "%d × ", count);
        g_string_append (string, name);

        if (clock != NULL)
        {
                gdouble hertz;

                hertz = g_ascii_strtod (clock, NULL);
                if (hertz >= 1000000.)
                        g_string_append_printf (
                                string, "  %.6f MHz", hertz / 1000000.);
                else
                        g_string_append_printf (
                                string, "  %.3f KHz", hertz / 1000.);
        }

        return g_string_free (string, FALSE);
}

static void
properties_update_cpu (const gchar *name)
{
        GtkWidget *label;
        gchar *sql;
        gchar **result = NULL;
        gint rows, columns, ii;
        GError *error = NULL;

        enum { CPU_COUNT, CPU_NAME, CPU_CLOCK };

        sql = g_strdup_printf (
                SQL_SELECT_CPU, name, G_N_ELEMENTS (cpu_labels));

        if (!gva_db_get_table (sql, &result, &rows, &columns, &error)) {
                gva_error_handle (&error);
                rows = columns = 0;
        }

        g_assert (rows <= G_N_ELEMENTS (cpu_labels));

        for (ii = 0; ii < rows; ii++)
        {
                const gchar *name;
                const gchar *clock;
                gchar **values;
                gchar *text;
                gint count;

                values = result + ((ii + 1) * columns);

                name = values[CPU_NAME];
                clock = values[CPU_CLOCK];
                count = (gint) strtol (values[CPU_COUNT], NULL, 10);

                label = gva_ui_get_widget (cpu_labels[ii]);
                text = properties_cpu_description (name, clock, count);
                gtk_label_set_text (GTK_LABEL (label), text);
                gtk_widget_show (label);
                g_free (text);
        }

        while (ii < G_N_ELEMENTS (cpu_labels))
        {
                label = gva_ui_get_widget (cpu_labels[ii++]);
                gtk_widget_hide (label);
        }

        g_strfreev (result);
        g_free (sql);
}

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

        if (description == NULL || *description == '\0')
                description = g_strdup (_("(Game Description Unknown)"));

        if (manufacturer == NULL || *manufacturer == '\0')
                manufacturer = g_strdup (_("(Manufacturer Unknown)"));

        if (year == NULL || *year == '\0')
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
#ifdef HISTORY_FILE
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
#endif
}

static void
properties_update_sound (const gchar *name)
{
        GtkWidget *label;
        gchar *sql;
        gchar **result = NULL;
        gint rows, columns, ii;
        GError *error = NULL;

        enum { SOUND_COUNT, SOUND_NAME, SOUND_CLOCK };

        sql = g_strdup_printf (
                SQL_SELECT_SOUND, name, G_N_ELEMENTS (sound_labels));

        if (!gva_db_get_table (sql, &result, &rows, &columns, &error)) {
                gva_error_handle (&error);
                rows = columns = 0;
        }

        g_assert (rows <= G_N_ELEMENTS (sound_labels));

        for (ii = 0; ii < rows; ii++)
        {
                const gchar *name;
                const gchar *clock;
                gchar **values;
                gchar *text;
                gint count;

                values = result + ((ii + 1) * columns);

                name = values[SOUND_NAME];
                clock = values[SOUND_CLOCK];
                count = (gint) strtol (values[SOUND_COUNT], NULL, 10);

                label = gva_ui_get_widget (sound_labels[ii]);
                text = properties_cpu_description (name, clock, count);
                gtk_label_set_text (GTK_LABEL (label), text);
                gtk_widget_show (label);
                g_free (text);
        }

        while (ii < G_N_ELEMENTS (sound_labels))
        {
                label = gva_ui_get_widget (sound_labels[ii++]);
                gtk_widget_hide (label);
        }

        g_strfreev (result);
        g_free (sql);
}

static void
properties_update_video (const gchar *name)
{
        GtkWidget *label;
        gchar *sql;
        gchar **result = NULL;
        gint rows, columns, ii;
        GError *error = NULL;

        enum
        {
                VIDEO_TYPE, VIDEO_ROTATE, VIDEO_WIDTH,
                VIDEO_HEIGHT, VIDEO_REFRESH
        };

        sql = g_strdup_printf (
                SQL_SELECT_VIDEO, name, G_N_ELEMENTS (video_labels));

        if (!gva_db_get_table (sql, &result, &rows, &columns, &error)) {
                gva_error_handle (&error);
                rows = columns = 0;
        }

        g_assert (rows <= G_N_ELEMENTS (video_labels));

        for (ii = 0; ii < rows; ii++)
        {
                GString *string;
                const gchar *type;
                const gchar *refresh;
                const gchar *width;
                const gchar *height;
                gchar **values;
                gint rotate;

                values = result + ((ii + 1) * columns);

                type = values[VIDEO_TYPE];
                refresh = values[VIDEO_REFRESH];
                width = values[VIDEO_WIDTH];
                height = values[VIDEO_HEIGHT];
                rotate = (gint) strtol (values[VIDEO_ROTATE], NULL, 10);

                string = g_string_sized_new (128);

                if (g_str_equal (type, "vector"))
                        g_string_assign (string, "Vector");
                else
                {
                        g_string_append_printf (
                                string, "%s × %s  ", width, height);
                        g_string_append (
                                string, (rotate % 180 == 0)
                                ? "(Horizontal)" : "(Vertical)");
                        g_string_append_printf (
                                string, "  %.6f Hz",
                                g_ascii_strtod (refresh, NULL));
                }

                label = gva_ui_get_widget (video_labels[ii]);
                gtk_label_set_text (GTK_LABEL (label), string->str);
                gtk_widget_show (label);

                g_string_free (string, TRUE);
        }

        while (ii < G_N_ELEMENTS (video_labels))
        {
                label = gva_ui_get_widget (video_labels[ii++]);
                gtk_widget_hide (label);
        }

        g_strfreev (result);
        g_free (sql);
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

                properties_update_cpu (name);
                properties_update_header (model, &iter);
                properties_update_history (model, &iter);
                properties_update_sound (name);
                properties_update_video (name);

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

#ifndef HISTORY_FILE
        GtkNotebook *notebook;
        GtkWidget *page;
#endif

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

#ifndef HISTORY_FILE
        /* Hide the history page if we have no history file. */
        notebook = GTK_NOTEBOOK (GVA_WIDGET_PROPERTIES_NOTEBOOK);
        page = gtk_notebook_get_nth_page (notebook, NOTEBOOK_PAGE_HISTORY);
        gtk_widget_hide (page);
#endif
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
