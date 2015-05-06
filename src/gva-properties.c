/* Copyright 2007-2011 Matthew Barnes
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
#include "gva-music-button.h"
#include "gva-preferences.h"
#include "gva-tree-view.h"
#include "gva-ui.h"
#include "gva-util.h"

/* Update Delay (0.05 sec) */
#define UPDATE_TIMEOUT_MS 50

#define SQL_SELECT_NAME \
        "SELECT * FROM available WHERE name = \"%s\""

#define SQL_SELECT_PARENT \
        "SELECT game.name, game.description, available.name NOTNULL " \
        "FROM game LEFT JOIN available ON game.name = available.name " \
        "WHERE game.name = \"%s\" ORDER BY game.description"

#define SQL_SELECT_CLONES \
        "SELECT game.name, game.description, available.name NOTNULL " \
        "FROM game LEFT JOIN available ON game.name = available.name " \
        "WHERE game.cloneof = \"%s\" ORDER BY game.description"

#define SQL_SELECT_CPU \
        "SELECT COUNT(*), name, clock FROM chip " \
        "WHERE game = \"%s\" AND type = \"cpu\" " \
        "GROUP BY name, clock ORDER BY CAST (clock AS INT) DESC LIMIT %lu"

#define SQL_SELECT_SOUND \
        "SELECT COUNT(*), name, clock FROM chip " \
        "WHERE game = \"%s\" AND type = \"audio\" " \
        "GROUP BY name, clock ORDER BY CAST (clock AS INT) DESC LIMIT %lu"

#define SQL_SELECT_VIDEO \
        "SELECT type, rotate, width, height, refresh " \
        "FROM display WHERE game = \"%s\" LIMIT %lu"

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

static const gchar *current_game;
static guint update_timeout_source_id;

static void
properties_scroll_to_top (void)
{
        GtkScrolledWindow *scrolled_window;
        GtkAdjustment *adjustment;

        scrolled_window = GTK_SCROLLED_WINDOW (
                GVA_WIDGET_PROPERTIES_HISTORY_SCROLLED_WINDOW);
        adjustment = gtk_scrolled_window_get_vadjustment (scrolled_window);
        gtk_adjustment_set_value (adjustment, 0.0);

        scrolled_window = GTK_SCROLLED_WINDOW (
                GVA_WIDGET_PROPERTIES_TECHNICAL_SCROLLED_WINDOW);
        adjustment = gtk_scrolled_window_get_vadjustment (scrolled_window);
        gtk_adjustment_set_value (adjustment, 0.0);
}

static void
properties_link_clicked_cb (GtkLabel *label,
                            const gchar *game)
{
        GtkWidget *widget;

        widget = GVA_WIDGET_PROPERTIES_TECHNICAL_SCROLLED_WINDOW;
        gtk_widget_grab_focus (widget);

        gva_properties_show_game (game);
        gva_tree_view_set_selected_game (game);
}

static void
properties_add_game_label (GtkBox *box,
                           const gchar *game,
                           const gchar *description,
                           gboolean available)
{
        GtkWidget *widget;

        widget = gtk_label_new (NULL);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);

        if (available)
        {
                gchar *markup;

                markup = g_markup_printf_escaped (
                        "<a href=\"%s\" title=\"%s\">%s</a>",
                        game, _("Show this game"), description);

                gtk_label_set_markup (GTK_LABEL (widget), markup);

                g_signal_connect (
                        widget, "activate-link",
                        G_CALLBACK (properties_link_clicked_cb), NULL);

                g_free (markup);
        }
        else
                gtk_label_set_text (GTK_LABEL (widget), description);

        gtk_box_pack_start (box, widget, FALSE, FALSE, 0);
        gtk_widget_show (widget);
}

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
properties_update_clones (GtkTreeModel *model,
                          GtkTreeIter *iter)
{
        GtkBox *box;
        GList *children;
        gchar **result = NULL;
        gchar *game;
        gint rows, columns, ii;
        GError *error = NULL;

        box = GTK_BOX (GVA_WIDGET_PROPERTIES_ORIGINAL_LINKS);
        children = gtk_container_get_children (GTK_CONTAINER (box));
        g_list_foreach (children, (GFunc) gtk_widget_destroy, NULL);
        g_list_free (children);

        gtk_tree_model_get (
                model, iter, GVA_GAME_STORE_COLUMN_CLONEOF, &game, -1);

        if (game != NULL)
        {
                gchar *sql;

                sql = g_strdup_printf (SQL_SELECT_PARENT, game);
                if (!gva_db_get_table (sql, &result, &rows, &columns, &error))
                {
                        gva_error_handle (&error);
                        rows = columns = 0;
                }
                g_free (sql);
                g_free (game);
        }
        else
                rows = columns = 0;

        if (rows > 0)
                gtk_widget_show (GVA_WIDGET_PROPERTIES_ORIGINAL_VBOX);
        else
                gtk_widget_hide (GVA_WIDGET_PROPERTIES_ORIGINAL_VBOX);

        for (ii = 0; ii < rows; ii++)
        {
                const gchar *parent_name;
                const gchar *description;
                gboolean available;
                gchar **values;

                values = result + ((ii + 1) * columns);

                parent_name = values[0];
                description = values[1];
                available = (*values[2] == '1');

                properties_add_game_label (
                        box, parent_name, description, available);
        }

        g_strfreev (result);
        result = NULL;

        box = GTK_BOX (GVA_WIDGET_PROPERTIES_ALTERNATE_LINKS);
        children = gtk_container_get_children (GTK_CONTAINER (box));
        g_list_foreach (children, (GFunc) gtk_widget_destroy, NULL);
        g_list_free (children);

        gtk_tree_model_get (
                model, iter, GVA_GAME_STORE_COLUMN_NAME, &game, -1);

        if (game != NULL)
        {
                gchar *sql;

                sql = g_strdup_printf (SQL_SELECT_CLONES, game);
                if (!gva_db_get_table (sql, &result, &rows, &columns, &error))
                {
                        gva_error_handle (&error);
                        rows = columns = 0;
                }
                g_free (sql);
                g_free (game);
        }
        else
                rows = columns = 0;

        if (rows > 0)
                gtk_widget_show (GVA_WIDGET_PROPERTIES_ALTERNATE_VBOX);
        else
                gtk_widget_hide (GVA_WIDGET_PROPERTIES_ALTERNATE_VBOX);

        for (ii = 0; ii < rows; ii++)
        {
                const gchar *clone_name;
                const gchar *description;
                gboolean available;
                gchar **values;

                values = result + ((ii + 1) * columns);

                clone_name = values[0];
                description = values[1];
                available = (*values[2] == '1');

                properties_add_game_label (
                        box, clone_name, description, available);
        }

        g_strfreev (result);
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

        if (!gva_db_get_table (sql, &result, &rows, &columns, &error))
        {
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
properties_update_music (const gchar *name)
{
        GvaMusicButton *music_button;

        music_button = GVA_MUSIC_BUTTON (GVA_WIDGET_PROPERTIES_MUSIC_BUTTON);
        gva_music_button_set_game (music_button, name);

        if (gtk_widget_get_visible (GVA_WIDGET_PROPERTIES_WINDOW)
                && gva_preferences_get_auto_play ())
                gva_music_button_play (music_button);
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

        if (!gva_db_get_table (sql, &result, &rows, &columns, &error))
        {
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
        gtk_widget_set_visible (widget, visible);

        gtk_image_set_from_stock (
                GTK_IMAGE (GVA_WIDGET_PROPERTIES_STATUS_IMAGE),
                stock_id, GTK_ICON_SIZE_DND);
        gtk_widget_set_visible (widget, visible);

        widget = GVA_WIDGET_PROPERTIES_IMPERFECT_COLOR_LABEL;
        visible = (strcmp (driver_color, "imperfect") == 0);
        gtk_widget_set_visible (widget, visible);

        widget = GVA_WIDGET_PROPERTIES_IMPERFECT_GRAPHIC_LABEL;
        visible = (strcmp (driver_graphic, "imperfect") == 0);
        gtk_widget_set_visible (widget, visible);

        widget = GVA_WIDGET_PROPERTIES_IMPERFECT_SOUND_LABEL;
        visible = (strcmp (driver_sound, "imperfect") == 0);
        gtk_widget_set_visible (widget, visible);

        widget = GVA_WIDGET_PROPERTIES_PRELIMINARY_COCKTAIL_LABEL;
        visible = (strcmp (driver_cocktail, "preliminary") == 0);
        gtk_widget_set_visible (widget, visible);

        widget = GVA_WIDGET_PROPERTIES_PRELIMINARY_COLOR_LABEL;
        visible = (strcmp (driver_color, "preliminary") == 0);
        gtk_widget_set_visible (widget, visible);

        widget = GVA_WIDGET_PROPERTIES_PRELIMINARY_EMULATION_LABEL;
        visible = (strcmp (driver_emulation, "preliminary") == 0);
        gtk_widget_set_visible (widget, visible);

        widget = GVA_WIDGET_PROPERTIES_PRELIMINARY_PROTECTION_LABEL;
        visible = (strcmp (driver_protection, "preliminary") == 0);
        gtk_widget_set_visible (widget, visible);

        widget = GVA_WIDGET_PROPERTIES_PRELIMINARY_SOUND_LABEL;
        visible = (strcmp (driver_sound, "preliminary") == 0);
        gtk_widget_set_visible (widget, visible);

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

        if (!gva_db_get_table (sql, &result, &rows, &columns, &error))
        {
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
        const gchar *name;

        name = gva_tree_view_get_selected_game ();

        if (name != NULL)
                gva_properties_show_game (name);

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

static void
properties_notify_music_status_cb (GvaMusicButton *music_button)
{
        GtkWidget *label;
        const gchar *status;
        gchar *markup;

        label = GVA_WIDGET_PROPERTIES_MUSIC_STATUS;
        status = gva_music_button_get_status (music_button);

        /* Use whitespace to keep the widget height stable. */
        if (status == NULL)
                status = " ";

        markup = g_markup_printf_escaped ("<small>%s</small>", status);
        gtk_label_set_markup (GTK_LABEL (label), markup);
        g_free (markup);
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
        GtkTreeView *view;
        GSettings *settings;
        GtkStyleContext *context;
        GtkWidget *text_view;
        PangoFontDescription *desc;
        GtkWidget *widget;
        gchar *font_name;

#ifndef HISTORY_FILE
        GtkNotebook *notebook;
#endif

        settings = gva_get_settings ();

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);
        text_view = GVA_WIDGET_PROPERTIES_HISTORY_TEXT_VIEW;

        g_settings_bind (
                settings, GVA_SETTING_PROPERTIES_PAGE,
                GVA_WIDGET_PROPERTIES_NOTEBOOK, "page",
                G_SETTINGS_BIND_DEFAULT);

        g_signal_connect (
                gtk_tree_view_get_selection (view), "changed",
                G_CALLBACK (properties_selection_changed_cb), NULL);

        g_signal_connect (
                GVA_WIDGET_PROPERTIES_MUSIC_BUTTON, "notify::status",
                G_CALLBACK (properties_notify_music_status_cb), NULL);

        gtk_window_resize (
                GTK_WINDOW (GVA_WIDGET_PROPERTIES_WINDOW),
                g_settings_get_int (settings, "properties-width"),
                g_settings_get_int (settings, "properties-height"));

        if (g_settings_get_boolean (settings, "properties-maximized"))
                gtk_window_maximize (GTK_WINDOW (GVA_WIDGET_PROPERTIES_WINDOW));

        font_name = gva_get_monospace_font_name ();
        desc = pango_font_description_from_string (font_name);
        gtk_widget_override_font (text_view, desc);
        pango_font_description_free (desc);
        g_free (font_name);

        widget = GVA_WIDGET_PROPERTIES_STATUS_FRAME;
        context = gtk_widget_get_style_context (widget);
        gtk_style_context_add_class (context, GTK_STYLE_CLASS_WARNING);
        widget = gtk_bin_get_child (GTK_BIN (widget));
        context = gtk_widget_get_style_context (widget);
        gtk_style_context_add_class (context, GTK_STYLE_CLASS_WARNING);

#ifndef HAVE_GSTREAMER
        gtk_widget_hide (GVA_WIDGET_PROPERTIES_MUSIC_TABLE);
#endif

#ifndef HISTORY_FILE
        /* Hide the history page if we have no history file. */
        notebook = GTK_NOTEBOOK (GVA_WIDGET_PROPERTIES_NOTEBOOK);
        widget = gtk_notebook_get_nth_page (notebook, NOTEBOOK_PAGE_HISTORY);
        gtk_widget_hide (widget);
#endif
}

/**
 * gva_properties_show_game:
 * @game: the name of a game
 *
 * Shows information about @game in the Properties window.
 **/
void
gva_properties_show_game (const gchar *game)
{
        GtkTreeModel *model;
        gchar *sql;
        GError *error = NULL;

        g_return_if_fail (game != NULL);

        game = g_intern_string (game);

        /* Refreshing the Properties window is expensive.  If the
         * requested game is already being shown, do nothing. */
        if (game == current_game)
                return;

        sql = g_strdup_printf (SQL_SELECT_NAME, game);
        model = gva_game_store_new_from_query (sql, &error);
        gva_error_handle (&error);
        g_free (sql);

        if (model != NULL)
        {
                GtkTreeIter iter;
                gboolean valid;

                valid = gtk_tree_model_get_iter_first (model, &iter);
                g_assert (valid);

                current_game = game;
                properties_scroll_to_top ();

                properties_update_bios (model, &iter);
                properties_update_clones (model, &iter);
                properties_update_cpu (game);
                properties_update_header (model, &iter);
                properties_update_history (model, &iter);
                properties_update_music (game);
                properties_update_sound (game);
                properties_update_status (model, &iter);
                properties_update_video (game);

                g_object_unref (model);
        }
}

/**
 * gva_properties_hide_cb:
 * @window: the "Properties" window
 * @button: the "Close" button
 *
 * Handler for #GtkWidget::show signals to the "Properties" window.
 *
 * Stops in-game music clip.
 **/
void
gva_properties_hide_cb (GtkWindow *window)
{
        GvaMusicButton *music_button;

        music_button = GVA_MUSIC_BUTTON (GVA_WIDGET_PROPERTIES_MUSIC_BUTTON);
        gva_music_button_pause (music_button);
}

/**
 * gva_properties_show_cb:
 * @window: the "Properties" window
 *
 * Handler for #GtkWidget::show signals to the "Properties" window.
 *
 * Resets all scrolled windows to the top, and starts in-game music clip
 * if the "auto-play" preference is enabled.
 **/
void
gva_properties_show_cb (GtkWindow *window)
{
        GvaMusicButton *music_button;

        music_button = GVA_MUSIC_BUTTON (GVA_WIDGET_PROPERTIES_MUSIC_BUTTON);

        if (gva_preferences_get_auto_play ())
                gva_music_button_play (music_button);

        properties_scroll_to_top ();
}

/**
 * gva_properties_configure_event_cb:
 * @window: the "Properties" window
 * @event: a #GdkEventConfigure
 *
 * Handler for #GtkWidget::configure-event signals to the "Properties"
 * window.
 *
 * Saves the "Properties" window state to dconf.
 *
 * Returns: %FALSE always
 **/
gboolean
gva_properties_configure_event_cb (GtkWindow *window,
                                   GdkEventConfigure *event)
{
        gva_save_window_state (
                window,
                "properties-width",
                "properties-height",
                "properties-maximized",
                NULL, NULL);

        return FALSE;
}

/**
 * gva_properties_window_state_event_cb:
 * @window: the "Properties" window
 * @event: a #GdkEventWindowState
 *
 * Handler for #GtkWidget::window-state-event signals to the "Properties"
 * window.
 *
 * Saves the "Properties" window state to dconf.
 *
 * Returns: %FALSE always
 **/
gboolean
gva_properties_window_state_event_cb (GtkWindow *window,
                                      GdkEventWindowState *event)
{
        gva_save_window_state (
                window,
                "properties-width",
                "properties-height",
                "properties-maximized",
                NULL, NULL);

        return FALSE;
}
