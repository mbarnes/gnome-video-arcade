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
#include <string.h>

#include "gva-db.h"
#include "gva-error.h"
#include "gva-game-store.h"
#include "gva-history.h"
#include "gva-tree-view.h"
#include "gva-ui.h"
#include "gva-util.h"

/* Update Delay (0.05 sec) */
#define UPDATE_TIMEOUT_MS 50

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

static guint update_timeout_source_id;

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
properties_update_bios (GtkTreeModel *model,
                        GtkTreeIter *iter)
{
        gchar *bios;

        gtk_tree_model_get (
                model, iter, GVA_GAME_STORE_COLUMN_BIOS, &bios, -1);

        if (bios != NULL && *bios != '\0')
        {
                GtkLabel *label;
                gchar *cp;
                gsize length;

                /* A lot of BIOS descriptions end with the word "BIOS".
                 * Strip it off, since the section is already titled "BIOS". */
                length = strlen (bios);
                cp = bios + length - 5;
                if (g_ascii_strcasecmp (cp, " BIOS") == 0)
                        *cp = '\0';

                label = GTK_LABEL (GVA_WIDGET_PROPERTIES_BIOS_LABEL);
                gtk_widget_show (GVA_WIDGET_PROPERTIES_BIOS_VBOX);
                gtk_label_set_text (label, bios);
        }
        else
                gtk_widget_hide (GVA_WIDGET_PROPERTIES_BIOS_VBOX);

        g_free (bios);
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

        if (rows > 0)
                gtk_widget_show (GVA_WIDGET_PROPERTIES_CPU_VBOX);
        else
                gtk_widget_hide (GVA_WIDGET_PROPERTIES_CPU_VBOX);

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

        if (rows > 0)
                gtk_widget_show (GVA_WIDGET_PROPERTIES_SOUND_VBOX);
        else
                gtk_widget_hide (GVA_WIDGET_PROPERTIES_SOUND_VBOX);

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
properties_update_status (GtkTreeModel *model,
                          GtkTreeIter *iter)
{
        GtkWidget *widget;
        const gchar *stock_id;
        gchar *driver_status;
        gchar *driver_emulation;
        gchar *driver_color;
        gchar *driver_sound;
        gchar *driver_graphic;
        gchar *driver_cocktail;
        gchar *driver_protection;
        gboolean visible;

        gtk_tree_model_get (
                model, iter,
                GVA_GAME_STORE_COLUMN_DRIVER_STATUS, &driver_status,
                GVA_GAME_STORE_COLUMN_DRIVER_EMULATION, &driver_emulation,
                GVA_GAME_STORE_COLUMN_DRIVER_COLOR, &driver_color,
                GVA_GAME_STORE_COLUMN_DRIVER_SOUND, &driver_sound,
                GVA_GAME_STORE_COLUMN_DRIVER_GRAPHIC, &driver_graphic,
                GVA_GAME_STORE_COLUMN_DRIVER_COCKTAIL, &driver_cocktail,
                GVA_GAME_STORE_COLUMN_DRIVER_PROTECTION, &driver_protection,
                -1);

        if (strcmp (driver_emulation, "preliminary") == 0)
                stock_id = GTK_STOCK_DIALOG_ERROR;
        else if (strcmp (driver_protection, "preliminary") == 0)
                stock_id = GTK_STOCK_DIALOG_ERROR;
        else
                stock_id = GTK_STOCK_DIALOG_WARNING;

        widget = GVA_WIDGET_PROPERTIES_STATUS_FRAME;
        visible = (strcmp (driver_status, "imperfect") == 0) ||
                (strcmp (driver_status, "preliminary") == 0);
        g_object_set (widget, "visible", visible, NULL);

        gtk_image_set_from_stock (
                GTK_IMAGE (GVA_WIDGET_PROPERTIES_STATUS_IMAGE),
                stock_id, GTK_ICON_SIZE_DND);
        g_object_set (widget, "visible", visible, NULL);

        widget = GVA_WIDGET_PROPERTIES_IMPERFECT_COLOR_LABEL;
        visible = (strcmp (driver_color, "imperfect") == 0);
        g_object_set (widget, "visible", visible, NULL);

        widget = GVA_WIDGET_PROPERTIES_IMPERFECT_GRAPHIC_LABEL;
        visible = (strcmp (driver_graphic, "imperfect") == 0);
        g_object_set (widget, "visible", visible, NULL);

        widget = GVA_WIDGET_PROPERTIES_IMPERFECT_SOUND_LABEL;
        visible = (strcmp (driver_sound, "imperfect") == 0);
        g_object_set (widget, "visible", visible, NULL);

        widget = GVA_WIDGET_PROPERTIES_PRELIMINARY_COCKTAIL_LABEL;
        visible = (strcmp (driver_cocktail, "preliminary") == 0);
        g_object_set (widget, "visible", visible, NULL);

        widget = GVA_WIDGET_PROPERTIES_PRELIMINARY_COLOR_LABEL;
        visible = (strcmp (driver_color, "preliminary") == 0);
        g_object_set (widget, "visible", visible, NULL);

        widget = GVA_WIDGET_PROPERTIES_PRELIMINARY_EMULATION_LABEL;
        visible = (strcmp (driver_emulation, "preliminary") == 0);
        g_object_set (widget, "visible", visible, NULL);

        widget = GVA_WIDGET_PROPERTIES_PRELIMINARY_PROTECTION_LABEL;
        visible = (strcmp (driver_protection, "preliminary") == 0);
        g_object_set (widget, "visible", visible, NULL);

        widget = GVA_WIDGET_PROPERTIES_PRELIMINARY_SOUND_LABEL;
        visible = (strcmp (driver_sound, "preliminary") == 0);
        g_object_set (widget, "visible", visible, NULL);

        g_free (driver_status);
        g_free (driver_emulation);
        g_free (driver_color);
        g_free (driver_sound);
        g_free (driver_graphic);
        g_free (driver_cocktail);
        g_free (driver_protection);
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

        if (rows > 0)
                gtk_widget_show (GVA_WIDGET_PROPERTIES_VIDEO_VBOX);
        else
                gtk_widget_hide (GVA_WIDGET_PROPERTIES_VIDEO_VBOX);

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

static gboolean
properties_update_timeout_cb (void)
{
        GtkTreeModel *model;
        const gchar *name;
        gchar *sql;
        GError *error = NULL;

        name = gva_tree_view_get_selected_game ();

        /* Leave the window alone if a game is not selected. */
        if (name == NULL)
                return FALSE;

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

                properties_update_bios (model, &iter);
                properties_update_cpu (name);
                properties_update_header (model, &iter);
                properties_update_history (model, &iter);
                properties_update_sound (name);
                properties_update_status (model, &iter);
                properties_update_video (name);

                g_object_unref (model);
        }

        update_timeout_source_id = 0;

        return FALSE;
}

static void
properties_selection_changed_cb (GtkTreeSelection *selection)
{
        /* Add a short delay to the properties window update to avoid
         * tying up the CPU while the user is rapidly scrolling through
         * the tree view. */
        if (update_timeout_source_id != 0)
                g_source_remove (update_timeout_source_id);
        update_timeout_source_id = g_timeout_add (
                UPDATE_TIMEOUT_MS, (GSourceFunc)
                properties_update_timeout_cb, NULL);
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
        GtkSettings *settings;
        GtkWidget *text_view;
        GHashTable *color_hash;
        PangoFontDescription *desc;
        const GdkColor *color;
        GtkWidget *widget;
        gchar *font_name;

#ifndef HISTORY_FILE
        GtkNotebook *notebook;
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

        gconf_bridge_bind_property (
                gconf_bridge_get (), GVA_GCONF_PROPERTIES_PAGE_KEY,
                G_OBJECT (GVA_WIDGET_PROPERTIES_NOTEBOOK), "page");

        g_signal_connect (
                gtk_tree_view_get_selection (view), "changed",
                G_CALLBACK (properties_selection_changed_cb), NULL);

        font_name = gva_get_monospace_font_name ();
        desc = pango_font_description_from_string (font_name);
        gtk_widget_modify_font (text_view, desc);
        pango_font_description_free (desc);
        g_free (font_name);

        settings = gtk_settings_get_default ();
        widget = GVA_WIDGET_PROPERTIES_STATUS_EVENT_BOX;
        g_object_get (settings, "color-hash", &color_hash, NULL);
        color = g_hash_table_lookup (color_hash, "tooltip_bg_color");
        gtk_widget_modify_bg (widget, GTK_STATE_NORMAL, color);

#ifndef HISTORY_FILE
        /* Hide the history page if we have no history file. */
        notebook = GTK_NOTEBOOK (GVA_WIDGET_PROPERTIES_NOTEBOOK);
        widget = gtk_notebook_get_nth_page (notebook, NOTEBOOK_PAGE_HISTORY);
        gtk_widget_hide (widget);
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
