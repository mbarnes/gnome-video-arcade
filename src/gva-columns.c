/* Copyright 2007-2009 Matthew Barnes
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

#include "gva-columns.h"

#include <langinfo.h>
#include <string.h>

#include "gva-cell-renderer-pixbuf.h"
#include "gva-error.h"
#include "gva-favorites.h"
#include "gva-tree-view.h"
#include "gva-ui.h"

#define MAX_PLAYER_ICONS 8

typedef GtkTreeViewColumn * (*FactoryFunc) (GvaGameStoreColumn);
typedef gboolean (*TooltipFunc) (GtkTreeModel *, GtkTreeIter *, GtkTooltip *);

static GdkPixbuf *
columns_get_icon_name (const gchar *icon_name)
{
        static GHashTable *cache = NULL;
        GtkIconTheme *icon_theme;
        GdkPixbuf *pixbuf;
        GdkPixbuf *scaled;
        gboolean valid;
        gint size;
        GError *error = NULL;

        if (G_UNLIKELY (cache == NULL))
                cache = g_hash_table_new_full (
                        g_str_hash, g_str_equal,
                        (GDestroyNotify) g_free,
                        (GDestroyNotify) g_object_unref);

        scaled = g_hash_table_lookup (cache, icon_name);
        if (scaled != NULL)
                return scaled;

        icon_theme = gtk_icon_theme_get_default ();
        valid = gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &size, NULL);
        g_assert (valid);

        pixbuf = gtk_icon_theme_load_icon (
                icon_theme, icon_name, size, 0, &error);

        if (pixbuf == NULL)
        {
                gva_error_handle (&error);
                return NULL;
        }

        scaled = gdk_pixbuf_scale_simple (
                pixbuf, size, size, GDK_INTERP_BILINEAR);

        g_hash_table_insert (cache, g_strdup (icon_name), scaled);

        g_object_unref (pixbuf);

        return scaled;
}

static void
columns_favorite_clicked_cb (GvaCellRendererPixbuf *renderer,
                             GtkTreePath *path,
                             GtkTreeViewColumn *column)
{
        GtkWidget *widget;
        const gchar *name;

        /* The row that was clicked is not yet selected.  We need to
         * select it first so that gva_tree_view_get_selected_game()
         * returns the correct name. */
        widget = gtk_tree_view_column_get_tree_view (column);
        gtk_tree_view_set_cursor (GTK_TREE_VIEW (widget), path, NULL, FALSE);
        gtk_widget_grab_focus (widget);

        name = gva_tree_view_get_selected_game ();
        g_assert (name != NULL);

        if (gva_favorites_contains (name))
                gtk_action_activate (GVA_ACTION_REMOVE_FAVORITE);
        else
                gtk_action_activate (GVA_ACTION_INSERT_FAVORITE);
}

static void
columns_comment_edited_cb (GtkCellRendererText *renderer,
                           gchar *path_string,
                           gchar *new_text,
                           GtkTreeViewColumn *column)
{
        GtkWidget *widget;
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;
        gint column_id;
        gboolean valid;

        widget = gtk_tree_view_column_get_tree_view (column);
        column_id = gtk_tree_view_column_get_sort_column_id (column);
        model = gtk_tree_view_get_model (GTK_TREE_VIEW (widget));
        path = gtk_tree_path_new_from_string (path_string);
        valid = gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_path_free (path);
        g_return_if_fail (valid);

        gtk_tree_store_set (
                GTK_TREE_STORE (model), &iter, column_id, new_text, -1);
}

static void
columns_bios_set_properties (GtkTreeViewColumn *column,
                             GtkCellRenderer *renderer,
                             GtkTreeModel *model,
                             GtkTreeIter *iter)
{
        GvaGameStoreColumn column_id;
        gchar *bios, *cp;
        gsize length;

        column_id = gtk_tree_view_column_get_sort_column_id (column);
        gtk_tree_model_get (model, iter, column_id, &bios, -1);

        /* A lot of BIOS descriptions end with the word "BIOS".
         * Strip it off, since the column is already titled "BIOS". */
        length = strlen (bios);
        cp = bios + length - 5;
        if (g_ascii_strcasecmp (cp, " BIOS") == 0)
                *cp = '\0';

        g_object_set (renderer, "text", bios, NULL);

        g_free (bios);
}

static void
columns_driver_status_set_properties (GtkTreeViewColumn *column,
                                      GtkCellRenderer *renderer,
                                      GtkTreeModel *model,
                                      GtkTreeIter *iter)
{
        GdkPixbuf *pixbuf = NULL;
        gchar *driver_status;
        gchar *driver_emulation;
        gchar *driver_protection;

        gtk_tree_model_get (
                model, iter,
                GVA_GAME_STORE_COLUMN_DRIVER_STATUS, &driver_status,
                GVA_GAME_STORE_COLUMN_DRIVER_EMULATION, &driver_emulation,
                GVA_GAME_STORE_COLUMN_DRIVER_PROTECTION, &driver_protection,
                -1);

        /* XXX For the "good" icon I want some kind of positive symbol; a
         *     checkmark or thumbs up or something.  The GTK_STOCK_APPLY 
         *     image seems to be the closest match at this time (a green
         *     checkmark), but may not be suitable for all icon themes. */

        /* Remember, we don't own the pixbuf reference. */
        if (strcmp (driver_status, "good") == 0)
                pixbuf = columns_get_icon_name (GTK_STOCK_APPLY);
        else if (strcmp (driver_emulation, "preliminary") == 0)
                pixbuf = columns_get_icon_name (GTK_STOCK_DIALOG_ERROR);
        else if (strcmp (driver_protection, "preliminary") == 0)
                pixbuf = columns_get_icon_name (GTK_STOCK_DIALOG_ERROR);
        else if (strcmp (driver_status, "imperfect") == 0)
                pixbuf = columns_get_icon_name (GTK_STOCK_DIALOG_WARNING);
        else if (strcmp (driver_status, "preliminary") == 0)
                pixbuf = columns_get_icon_name (GTK_STOCK_DIALOG_WARNING);

        g_object_set (
                renderer, "pixbuf", pixbuf,
                "visible", (pixbuf != NULL), NULL);

        g_free (driver_status);
        g_free (driver_emulation);
        g_free (driver_protection);
}

static void
columns_input_players_set_properties (GtkTreeViewColumn *column,
                                      GtkCellRenderer *renderer,
                                      GtkTreeModel *model,
                                      GtkTreeIter *iter,
                                      gpointer user_data)
{
        GvaGameStoreColumn column_id;
        gint max_players;
        gboolean visible;

        column_id = gtk_tree_view_column_get_sort_column_id (column);
        gtk_tree_model_get (model, iter, column_id, &max_players, -1);

        visible = GPOINTER_TO_INT (user_data) < max_players;
        g_object_set (renderer, "visible", visible, NULL);
}

static void
columns_sampleset_set_properties (GtkTreeViewColumn *column,
                                  GtkCellRenderer *renderer,
                                  GtkTreeModel *model,
                                  GtkTreeIter *iter)
{
        GvaGameStoreColumn column_id;
        gboolean sensitive;
        gboolean visible;
        gchar *sampleset;

        column_id = gtk_tree_view_column_get_sort_column_id (column);
        gtk_tree_model_get (model, iter, column_id, &sampleset, -1);

        visible = (sampleset != NULL && *sampleset != '\0');
        sensitive = visible && (strcmp (sampleset, "good") == 0);

        g_object_set (
                renderer, "sensitive", sensitive, "visible", visible, NULL);

        g_free (sampleset);
}

static void
columns_time_set_properties (GtkTreeViewColumn *column,
                             GtkCellRenderer *renderer,
                             GtkTreeModel *model,
                             GtkTreeIter *iter)
{
        GvaGameStoreColumn column_id;
        GValue value;
        gchar text[256];

        memset (&value, 0, sizeof (GValue));
        column_id = gtk_tree_view_column_get_sort_column_id (column);
        gtk_tree_model_get_value (model, iter, column_id, &value);

        strftime (
                text, sizeof (text), nl_langinfo (D_T_FMT),
                localtime (g_value_get_boxed (&value)));

        g_object_set (renderer, "text", text, NULL);

        g_value_unset (&value);
}

static gboolean
columns_show_popup_menu (GdkEventButton *event,
                         GtkTreeViewColumn *column)
{
        GtkMenu *menu;
        GtkWidget *view;
        const gchar *column_title;
        gchar *label, *tooltip;

        column_title = gtk_tree_view_column_get_title (column);
        view = gtk_tree_view_column_get_tree_view (column);

        /* Let the "Add Column" action know where to insert the column. */
        g_object_set_data (G_OBJECT (view), "popup-menu-column", column);

        /* Update the "Remove Column" item in the popup menu. */
        label = g_strdup_printf (
                _("Remove %s Column"), column_title);
        tooltip = g_strdup_printf (
                _("Remove the \"%s\" column from the game list"),
                column_title);
        g_object_set (
                GVA_ACTION_REMOVE_COLUMN, "label",
                label, "tooltip", tooltip, NULL);
        g_free (tooltip);
        g_free (label);

        menu = GTK_MENU (gva_ui_get_managed_widget ("/column-popup"));

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
columns_button_press_event_cb (GtkTreeViewColumn *column,
                               GdkEventButton *event)
{
        if (event->button == 3 && event->type == GDK_BUTTON_PRESS)
                return columns_show_popup_menu (event, column);

        return FALSE;
}

static gboolean
columns_popup_menu_cb (GtkTreeViewColumn *column)
{
        return columns_show_popup_menu (NULL, column);
}

static void
columns_setup_popup_menu (GtkTreeViewColumn *column)
{
        g_return_if_fail (column->button != NULL);

        g_signal_connect_swapped (
                column->button, "button-press-event",
                G_CALLBACK (columns_button_press_event_cb), column);

        g_signal_connect_swapped (
                column->button, "popup-menu",
                G_CALLBACK (columns_popup_menu_cb), column);

}

/*****************************************************************************
 * Column Factory Callbacks
 *****************************************************************************/

static GtkTreeViewColumn *
columns_factory_bios (GvaGameStoreColumn column_id)
{
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        gtk_tree_view_column_set_cell_data_func (
                column, renderer, (GtkTreeCellDataFunc)
                columns_bios_set_properties, NULL, NULL);

        return column;
}

static GtkTreeViewColumn *
columns_factory_category (GvaGameStoreColumn column_id)
{
#ifdef CATEGORY_FILE
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        gtk_tree_view_column_add_attribute (
                column, renderer, "text", column_id);

        return column;
#else
        return NULL;
#endif
}

static GtkTreeViewColumn *
columns_factory_comment (GvaGameStoreColumn column_id)
{
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_expand (column, TRUE);
        gtk_tree_view_column_set_reorderable (column, FALSE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);

        renderer = gtk_cell_renderer_text_new ();
        g_object_set (renderer, "editable", TRUE, NULL);
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        g_signal_connect (
                renderer, "edited",
                G_CALLBACK (columns_comment_edited_cb), column);

        gtk_tree_view_column_add_attribute (
                column, renderer, "text", column_id);

        return column;
}

static GtkTreeViewColumn *
columns_factory_description (GvaGameStoreColumn column_id)
{
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_expand (column, TRUE);
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);
        gtk_tree_view_column_set_spacing (column, 3);

        renderer = gtk_cell_renderer_text_new ();
        g_object_set (renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        gtk_tree_view_column_add_attribute (
                column, renderer, "text", column_id);

        return column;
}

static GtkTreeViewColumn *
columns_factory_driver_status (GvaGameStoreColumn column_id)
{
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);

        renderer = gtk_cell_renderer_pixbuf_new ();
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        gtk_tree_view_column_set_cell_data_func (
                column, renderer, (GtkTreeCellDataFunc)
                columns_driver_status_set_properties, NULL, NULL);

        return column;
}

static GtkTreeViewColumn *
columns_factory_favorite (GvaGameStoreColumn column_id)
{
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;
        GdkPixbuf *pixbuf;

        /* Remember, we don't own the pixbuf reference. */
        pixbuf = columns_get_icon_name ("emblem-favorite");

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);

        renderer = gva_cell_renderer_pixbuf_new ();
        g_object_set (renderer, "pixbuf", pixbuf, NULL);
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        gtk_tree_view_column_add_attribute (
                column, renderer, "sensitive", column_id);

        g_signal_connect (
                renderer, "clicked",
                G_CALLBACK (columns_favorite_clicked_cb), column);

        return column;
}

static GtkTreeViewColumn *
columns_factory_input_players (GvaGameStoreColumn column_id)
{
        GtkTreeViewColumn *column;
        GdkPixbuf *pixbuf;
        gint ii;

        /* Remember, we don't own the pixbuf reference. */
        pixbuf = columns_get_icon_name ("stock_person");

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);

        for (ii = 0; ii < MAX_PLAYER_ICONS; ii++)
        {
                GtkCellRenderer *renderer;

                renderer = gtk_cell_renderer_pixbuf_new ();
                g_object_set (renderer, "pixbuf", pixbuf, NULL);
                gtk_tree_view_column_pack_start (column, renderer, FALSE);

                gtk_tree_view_column_set_cell_data_func (
                        column, renderer, (GtkTreeCellDataFunc)
                        columns_input_players_set_properties,
                        GINT_TO_POINTER (ii), NULL);
        }

        return column;
}

static GtkTreeViewColumn *
columns_factory_manufacturer (GvaGameStoreColumn column_id)
{
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        gtk_tree_view_column_add_attribute (
                column, renderer, "text", column_id);

        return column;
}

static GtkTreeViewColumn *
columns_factory_name (GvaGameStoreColumn column_id)
{
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        gtk_tree_view_column_add_attribute (
                column, renderer, "text", column_id);

        return column;
}

static GtkTreeViewColumn *
columns_factory_sampleset (GvaGameStoreColumn column_id)
{
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;
        GdkPixbuf *pixbuf;

        /* Remember, we don't own the pixbuf reference. */
        pixbuf = columns_get_icon_name ("emblem-sound");

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);

        renderer = gtk_cell_renderer_pixbuf_new ();
        g_object_set (renderer, "pixbuf", pixbuf, NULL);
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        gtk_tree_view_column_set_cell_data_func (
                column, renderer, (GtkTreeCellDataFunc)
                columns_sampleset_set_properties, NULL, NULL);

        return column;
}

static GtkTreeViewColumn *
columns_factory_sourcefile (GvaGameStoreColumn column_id)
{
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        gtk_tree_view_column_add_attribute (
                column, renderer, "text", column_id);

        return column;
}

static GtkTreeViewColumn *
columns_factory_time (GvaGameStoreColumn column_id)
{
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_reorderable (column, FALSE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        gtk_tree_view_column_set_cell_data_func (
                column, renderer, (GtkTreeCellDataFunc)
                columns_time_set_properties, NULL, NULL);

        return column;
}

static GtkTreeViewColumn *
columns_factory_year (GvaGameStoreColumn column_id)
{
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);

        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        gtk_tree_view_column_add_attribute (
                column, renderer, "text", column_id);

        return column;
}

/*****************************************************************************
 * Column Tooltip Callbacks
 *****************************************************************************/

static gboolean
columns_tooltip_driver_status (GtkTreeModel *model,
                               GtkTreeIter *iter,
                               GtkTooltip *tooltip)
{
        GtkWidget *table;
        GtkWidget *widget;
        const gchar *text;
        const gchar *stock_id;
        gchar *driver_status;
        gchar *driver_emulation;
        gchar *driver_color;
        gchar *driver_sound;
        gchar *driver_graphic;
        gchar *driver_cocktail;
        gchar *driver_protection;
        gboolean show_tooltip;

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

        show_tooltip =
                strcmp (driver_status, "imperfect") == 0 ||
                strcmp (driver_status, "preliminary") == 0;
        if (!show_tooltip)
                goto exit;

        if (strcmp (driver_emulation, "preliminary") == 0)
                stock_id = GTK_STOCK_DIALOG_ERROR;
        else if (strcmp (driver_protection, "preliminary") == 0)
                stock_id = GTK_STOCK_DIALOG_ERROR;
        else
                stock_id = GTK_STOCK_DIALOG_WARNING;

        table = gtk_table_new (9, 2, FALSE);
        gtk_table_set_col_spacings (GTK_TABLE (table), 6);
        gtk_table_set_row_spacing (GTK_TABLE (table), 0, 6);
        gtk_widget_show (table);

        widget = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_DND);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.5, 0.0);
        gtk_table_attach (
                GTK_TABLE (table), widget, 0, 1, 0, 9,
                0, GTK_EXPAND | GTK_FILL, 0, 0);
        gtk_widget_show (widget);

        /* The same text is in gnome-video-arcade.glade,
         * so it has to be translated with markup anyway. */
        text = _("<b>There are known problems with this game:</b>");
        widget = gtk_label_new (text);
        gtk_label_set_use_markup (GTK_LABEL (widget), TRUE);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
        gtk_table_attach (
                GTK_TABLE (table), widget, 1, 2, 0, 1,
                GTK_EXPAND | GTK_FILL, 0, 0, 0);
        gtk_widget_show (widget);

        /* The labels begin with a UTF-8 encoded bullet character. */

        text = _("• The colors aren't 100% accurate.");
        widget = gtk_label_new (text);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
        gtk_table_attach (
                GTK_TABLE (table), widget, 1, 2, 1, 2,
                GTK_EXPAND | GTK_FILL, 0, 0, 0);
        if (strcmp (driver_color, "imperfect") == 0)
                gtk_widget_show (widget);

        text = _("• The colors are completely wrong.");
        widget = gtk_label_new (text);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
        gtk_table_attach (
                GTK_TABLE (table), widget, 1, 2, 2, 3,
                GTK_EXPAND | GTK_FILL, 0, 0, 0);
        if (strcmp (driver_color, "preliminary") == 0)
                gtk_widget_show (widget);

        text = _("• The video emulation isn't 100% accurate.");
        widget = gtk_label_new (text);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
        gtk_table_attach (
                GTK_TABLE (table), widget, 1, 2, 3, 4,
                GTK_EXPAND | GTK_FILL, 0, 0, 0);
        if (strcmp (driver_graphic, "imperfect") == 0)
                gtk_widget_show (widget);

        text = _("• The sound emulation isn't 100% accurate.");
        widget = gtk_label_new (text);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
        gtk_table_attach (
                GTK_TABLE (table), widget, 1, 2, 4, 5,
                GTK_EXPAND | GTK_FILL, 0, 0, 0);
        if (strcmp (driver_sound, "imperfect") == 0)
                gtk_widget_show (widget);

        text = _("• The game lacks sound.");
        widget = gtk_label_new (text);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
        gtk_table_attach (
                GTK_TABLE (table), widget, 1, 2, 5, 6,
                GTK_EXPAND | GTK_FILL, 0, 0, 0);
        if (strcmp (driver_sound, "preliminary") == 0)
                gtk_widget_show (widget);

        text = _("• Screen flipping in cocktail mode is not supported.");
        widget = gtk_label_new (text);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
        gtk_table_attach (
                GTK_TABLE (table), widget, 1, 2, 6, 7,
                GTK_EXPAND | GTK_FILL, 0, 0, 0);
        if (strcmp (driver_cocktail, "preliminary") == 0)
                gtk_widget_show (widget);

        text = _("• <b>THIS GAME DOESN'T WORK.</b>");
        widget = gtk_label_new (text);
        gtk_label_set_use_markup (GTK_LABEL (widget), TRUE);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
        gtk_table_attach (
                GTK_TABLE (table), widget, 1, 2, 7, 8,
                GTK_EXPAND | GTK_FILL, 0, 0, 0);
        if (strcmp (driver_emulation, "preliminary") == 0)
                gtk_widget_show (widget);

        text = _("• The game has protection which isn't fully emulated.");
        widget = gtk_label_new (text);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
        gtk_table_attach (
                GTK_TABLE (table), widget, 1, 2, 8, 9,
                GTK_EXPAND | GTK_FILL, 0, 0, 0);
        if (strcmp (driver_protection, "preliminary") == 0)
                gtk_widget_show (widget);

        gtk_tooltip_set_custom (tooltip, table);

exit:
        g_free (driver_status);
        g_free (driver_emulation);
        g_free (driver_color);
        g_free (driver_sound);
        g_free (driver_graphic);
        g_free (driver_cocktail);
        g_free (driver_protection);

        return show_tooltip;
}

static gboolean
columns_tooltip_favorite (GtkTreeModel *model,
                          GtkTreeIter *iter,
                          GtkTooltip *tooltip)
{
        const gchar *text;
        gboolean favorite;

        gtk_tree_model_get (
                model, iter, GVA_GAME_STORE_COLUMN_FAVORITE, &favorite, -1);

        if (favorite)
                text = _("Click here to remove from favorites");
        else
                text = _("Click here to add to favorites");

        gtk_tooltip_set_text (tooltip, text);

        return TRUE;
}

static gboolean
columns_tooltip_input_players (GtkTreeModel *model,
                               GtkTreeIter *iter,
                               GtkTooltip *tooltip)
{
        const gchar *text;
        gint max_players;

        gtk_tree_model_get (
                model, iter,
                GVA_GAME_STORE_COLUMN_INPUT_PLAYERS, &max_players, -1);

        /* Keep this in sync with MAX_PLAYER_ICONS. */
        switch (max_players)
        {
                case 1:
                        text = _("One player only");
                        break;

                case 2:
                        text = _("One or two players");
                        break;

                case 3:
                        text = _("Up to three players");
                        break;

                case 4:
                        text = _("Up to four players");
                        break;

                case 5:
                        text = _("Up to five players");
                        break;

                case 6:
                        text = _("Up to six players");
                        break;

                case 7:
                        text = _("Up to seven players");
                        break;

                case 8:
                        text = _("Up to eight players");
                        break;

                default:
                        return FALSE;
        }

        gtk_tooltip_set_text (tooltip, text);

        return TRUE;
}

static gboolean
columns_tooltip_summary (GtkTreeModel *model,
                         GtkTreeIter *iter,
                         GtkTooltip *tooltip)
{
        gchar *description;
        gchar *manufacturer;
        gchar *year;
        gchar *markup;

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
                "<b>%s</b>\n<small>%s %s</small>",
                description, year, manufacturer);
        gtk_tooltip_set_markup (tooltip, markup);
        g_free (markup);

        g_free (description);
        g_free (manufacturer);
        g_free (year);

        return TRUE;
}

static struct
{
        const gchar *name;
        const gchar *title;
        FactoryFunc factory;
        TooltipFunc tooltip;
}
column_info[GVA_GAME_STORE_NUM_COLUMNS] =
{
        { "name",               N_("ROM Name"),
                                columns_factory_name },
        { "bios",               N_("BIOS"),
                                columns_factory_bios },
        { "category",           N_("Category"),
                                columns_factory_category },
        { "favorite",           N_("Favorite"),
                                columns_factory_favorite,
                                columns_tooltip_favorite },
        { "sourcefile",         N_("Driver"),
                                columns_factory_sourcefile },
        { "isbios",             NULL },
        { "runnable",           NULL },
        { "cloneof",            NULL },
        { "romof",              NULL },
        { "romset",             NULL },
        { "sampleof",           NULL },
        { "sampleset",          N_("Samples"),
                                columns_factory_sampleset },
        { "description",        N_("Title"),
                                columns_factory_description,
                                columns_tooltip_summary },
        { "year",               N_("Year"),
                                columns_factory_year,
                                columns_tooltip_summary },
        { "manufacturer",       N_("Manufacturer"),
                                columns_factory_manufacturer,
                                columns_tooltip_summary },
        { "sound_channels",     NULL },
        { "input_service",      NULL },
        { "input_tilt",         NULL },
        { "input_players",      N_("Players"),
                                columns_factory_input_players,
                                columns_tooltip_input_players },
        { "input_buttons",      NULL },
        { "input_coins",        NULL },
        { "driver_status",      N_("Status"),
                                columns_factory_driver_status,
                                columns_tooltip_driver_status },
        { "driver_emulation",   NULL },
        { "driver_color",       NULL },
        { "driver_sound",       NULL },
        { "driver_graphic",     NULL },
        { "driver_cocktail",    NULL },
        { "driver_protection",  NULL },
        { "driver_savestate",   NULL },
        { "driver_palettesize", NULL },
        { "comment",            N_("Comment"),
                                columns_factory_comment },
        { "inode",              NULL },
        { "inpfile",            NULL },
        { "time",               N_("Played On"),
                                columns_factory_time }
};

static gchar *default_column_order[] =
{
        "favorite",
        "description",
        "year",
        "manufacturer",
#ifdef CATEGORY_FILE
        "category",
#endif
        "bios",
        "driver_status",
        "input_players",
        "name",
        "sourcefile",
        "sampleset"
};

/**
 * gva_columns_new_from_id:
 * @column_id: the ID of the column to create
 *
 * Creates a new #GtkTreeViewColumn from the given @column_id, configured
 * for use in the main tree view.
 *
 * Returns: a new #GtkTreeViewColumn
 **/
GtkTreeViewColumn *
gva_columns_new_from_id (GvaGameStoreColumn column_id)
{
        GtkTreeViewColumn *column;
        const gchar *title;

        g_return_val_if_fail (column_id >= 0, NULL);
        g_return_val_if_fail (column_id < G_N_ELEMENTS (column_info), NULL);
        g_return_val_if_fail (column_info[column_id].factory != NULL, NULL);

        column = column_info[column_id].factory (column_id);

        title = gettext (column_info[column_id].title);
        gtk_tree_view_column_set_title (column, title);

        g_object_set_data (
                G_OBJECT (column), "name",
                (gpointer) column_info[column_id].name);

        return column;
}

/**
 * gva_columns_new_from_name:
 * @column_name: the name of the column to create
 *
 * Creates a new #GtkTreeViewColumn from the given @column_name (as stored
 * in GConf), configured for use in the main tree view.
 *
 * Returns: a new #GtkTreeViewColumn
 **/
GtkTreeViewColumn *
gva_columns_new_from_name (const gchar *column_name)
{
        GvaGameStoreColumn column_id;

        if (!gva_columns_lookup_id (column_name, &column_id))
                return NULL;

        return gva_columns_new_from_id (column_id);
}

/**
 * gva_columns_lookup_id:
 * @column_name: the name of the column to lookup
 * @column_id: return location for the column ID
 *
 * Looks up the numeric column ID corresponding to @column_name, and write
 * the result to @column_id if found.
 *
 * Returns: @TRUE if a column ID was found, @FALSE otherwise
 **/
gboolean
gva_columns_lookup_id (const gchar *column_name,
                       GvaGameStoreColumn *column_id)
{
        gint ii;

        g_return_val_if_fail (column_name != NULL, FALSE);
        g_return_val_if_fail (column_id != NULL, FALSE);

        for (ii = 0; ii < G_N_ELEMENTS (column_info); ii++)
        {
                if (strcmp (column_name, column_info[ii].name) == 0)
                {
                        *column_id = (GvaGameStoreColumn) ii;
                        return TRUE;
                }
        }

        return FALSE;
}

/**
 * gva_columns_lookup_name:
 * @column_id: the ID of the column to lookup
 *
 * Looks up the column name corresponding to @column_id.
 *
 * Returns: the column name, or @NULL if not found
 **/
const gchar *
gva_columns_lookup_name (GvaGameStoreColumn column_id)
{
        if (CLAMP (column_id, 0, G_N_ELEMENTS (column_info)) != column_id)
                return NULL;

        return column_info[column_id].name;
}

/**
 * gva_columns_lookup_title:
 * @column_id: the ID of the column to lookup
 *
 * Looks up the column title corresponding to @column_id.
 *
 * Returns: the column title, or @NULL if not found
 **/
const gchar *
gva_columns_lookup_title (GvaGameStoreColumn column_id)
{
        if (CLAMP (column_id, 0, G_N_ELEMENTS (column_info)) != column_id)
                return NULL;

        return column_info[column_id].title;
}

/* Helper for gva_columns_load() */
static gboolean
columns_load_remove_name (GSList **p_list, const gchar *name)
{
        GSList *link;

        link = g_slist_find_custom (*p_list, name, (GCompareFunc) strcmp);

        if (link == NULL)
                return FALSE;

        g_free (link->data);
        *p_list = g_slist_delete_link (*p_list, link);

        return TRUE;
}

/**
 * gva_columns_load:
 * @view: a #GtkTreeView
 *
 * Loads @view with columns in the order stored in the GConf key
 * <filename>/apps/gnome-video-arcade/all-columns</filename>, but only
 * makes visible those columns listed in
 * <filename>/apps/gnome-video-arcade/columns</filename>.  Newly supported
 * columns are appended to @view but remain invisible until explicitly
 * selected in the Preferences window.
 *
 * Each column is loaded by reading the column name from GConf and passing
 * it to gva_columns_new_from_name() to create the #GtkTreeViewColumn.
 **/
void
gva_columns_load (GtkTreeView *view)
{
        GConfClient *client;
        GSList *all_columns;
        GSList *new_columns;
        GSList *visible_columns;
        gint ii;
        GError *error = NULL;

        g_return_if_fail (GTK_IS_TREE_VIEW (view));

        /* Adding columns to the tree view will cause it to emit
         * "columns-changed" signals, for which gva_columns_save() is a
         * handler.  Prevent the handler from modifying GConf keys while
         * we're loading. */
        g_signal_handlers_block_by_func (view, gva_columns_save, NULL);

        client = gconf_client_get_default ();
        all_columns = gconf_client_get_list (
                client, GVA_GCONF_ALL_COLUMNS_KEY, GCONF_VALUE_STRING, &error);
        gva_error_handle (&error);
        visible_columns = gconf_client_get_list (
                client, GVA_GCONF_COLUMNS_KEY, GCONF_VALUE_STRING, &error);
        gva_error_handle (&error);
        g_object_unref (client);

        if (visible_columns == NULL)
        {
                /* Fall back to the default columns. */
                visible_columns = g_slist_append (
                        visible_columns, g_strdup ("favorite"));
                visible_columns = g_slist_append (
                        visible_columns, g_strdup ("description"));
        }

        new_columns = NULL;
        for (ii = 0; ii < G_N_ELEMENTS (default_column_order); ii++)
        {
                gchar *name = g_strdup (default_column_order[ii]);
                new_columns = g_slist_append (new_columns, name);
        }

        while (all_columns != NULL)
        {
                gchar *name = all_columns->data;
                GtkTreeViewColumn *column;
                gboolean visible;

                column = gva_columns_new_from_name (name);
                columns_load_remove_name (&new_columns, name);
                visible = columns_load_remove_name (&visible_columns, name);

                g_free (name);
                all_columns = g_slist_delete_link (all_columns, all_columns);

                if (column != NULL)
                {
                        gtk_tree_view_column_set_visible (column, visible);
                        gtk_tree_view_append_column (view, column);
                        columns_setup_popup_menu (column);

                        g_signal_connect_swapped (
                                column, "notify::visible",
                                G_CALLBACK (gva_columns_save), view);
                }
        }

        while (visible_columns != NULL)
        {
                gchar *name = visible_columns->data;
                GtkTreeViewColumn *column;

                column = gva_columns_new_from_name (name);
                columns_load_remove_name (&new_columns, name);

                g_free (name);
                visible_columns = g_slist_delete_link (
                        visible_columns, visible_columns);

                if (column != NULL)
                {
                        gtk_tree_view_column_set_visible (column, TRUE);
                        gtk_tree_view_append_column (view, column);
                        columns_setup_popup_menu (column);

                        g_signal_connect_swapped (
                                column, "notify::visible",
                                G_CALLBACK (gva_columns_save), view);
                }
        }

        while (new_columns != NULL)
        {
                gchar *name = new_columns->data;
                GtkTreeViewColumn *column;

                column = gva_columns_new_from_name (name);

                g_free (name);
                new_columns = g_slist_delete_link (new_columns, new_columns);

                if (column != NULL)
                {
                        gtk_tree_view_column_set_visible (column, FALSE);
                        gtk_tree_view_append_column (view, column);
                        columns_setup_popup_menu (column);

                        g_signal_connect_swapped (
                                column, "notify::visible",
                                G_CALLBACK (gva_columns_save), view);
                }
        }

        g_signal_handlers_unblock_by_func (view, gva_columns_save, NULL);

        gva_columns_save (view);
}

/**
 * gva_columns_save:
 * @view: a #GtkTreeView
 *
 * Writes the column order and visible columns of @view to the GConf
 * keys <filename>/apps/gnome-video-arcade/all-columns</filename> and
 * <filename>/apps/gnome-video-arcade/columns</filename> respectively,
 * using gva_columns_get_names() to extract the column names.
 **/
void
gva_columns_save (GtkTreeView *view)
{
        GConfClient *client;
        GSList *list;
        GError *error = NULL;

        g_return_if_fail (GTK_IS_TREE_VIEW (view));

        /* This function is also a "columns-changed" signal handler.
         * Abort the save if the tree view is being destroyed. */
        if (GTK_OBJECT_FLAGS (view) & GTK_IN_DESTRUCTION)
                return;

        client = gconf_client_get_default ();

        list = gva_columns_get_names (view, FALSE);
        gconf_client_set_list (
                client, GVA_GCONF_ALL_COLUMNS_KEY,
                GCONF_VALUE_STRING, list, &error);
        gva_error_handle (&error);
        g_slist_free (list);

        list = gva_columns_get_names (view, TRUE);
        gconf_client_set_list (
                client, GVA_GCONF_COLUMNS_KEY,
                GCONF_VALUE_STRING, list, &error);
        gva_error_handle (&error);
        g_slist_free (list);

        g_object_unref (client);
}

/**
 * gva_columns_get_names:
 * @view: a #GtkTreeView
 * @visible_only: only extract visible columns
 *
 * Extracts a list of column names from @view, using gva_columns_lookup_name()
 * to convert each numeric column ID to a name.  If @visible_only is %TRUE
 * then only visible columns are included in the list.  The column name
 * strings are owned by @view and should not be freed; only the list itself
 * should be freed using g_slist_free().
 *
 * Returns: a #GSList of column names
 **/
GSList *
gva_columns_get_names (GtkTreeView *view,
                       gboolean visible_only)
{
        GList *list;
        GSList *names = NULL;

        g_return_val_if_fail (GTK_IS_TREE_VIEW (view), NULL);

        list = g_list_reverse (gtk_tree_view_get_columns (view));

        while (list != NULL)
        {
                GtkTreeViewColumn *column = list->data;
                gboolean visible;

                visible = gtk_tree_view_column_get_visible (column);

                if (visible || !visible_only)
                        names = g_slist_prepend (
                                names, g_object_get_data (
                                G_OBJECT (column), "name"));

                list = g_list_delete_link (list, list);
        }

        return names;
}

/* Helper for gva_columns_get_names_full() */
static void
columns_add_dependency (GSList **p_list, const gchar *name)
{
        if (!g_slist_find_custom (*p_list, name, (GCompareFunc) strcmp))
                *p_list = g_slist_prepend (*p_list, (gpointer) name);
}

/**
 * gva_columns_get_names_full:
 * @view: a #GtkTreeView
 *
 * Extracts a list of column names from @view, plus any additional column
 * names from the game database necessary to render the tree view cells.
 * The column name strings are owned by @view and should not be freed;
 * only the list itself should be freed using g_slist_free().
 *
 * Returns: a #GSList of column names
 **/
GSList *
gva_columns_get_names_full (GtkTreeView *view)
{
        GSList *names, *iter;

        g_return_val_if_fail (GTK_IS_TREE_VIEW (view), NULL);

        names = gva_columns_get_names (view, FALSE);

        /* XXX All the dependency information lives here for now.
         *     It might make more sense in the column_info table,
         *     with some private API for lookups. */

        for (iter = names; iter != NULL; iter = iter->next)
        {
                const gchar *column_name = iter->data;

                /* Need to know if a ROM set is not a game but a BIOS,
                 * so we can exclude it from the game list. */
                if (strcmp (column_name, "name") == 0)
                        columns_add_dependency (&names, "isbios");

                /* Any of the description, manufacturer, and year columns
                 * require the other two for the summary tooltip. */
                if (strcmp (column_name, "description") == 0)
                {
                        columns_add_dependency (&names, "manufacturer");
                        columns_add_dependency (&names, "year");
                }
                else if (strcmp (column_name, "manufacturer") == 0)
                {
                        columns_add_dependency (&names, "description");
                        columns_add_dependency (&names, "year");
                }
                else if (strcmp (column_name, "year") == 0)
                {
                        columns_add_dependency (&names, "description");
                        columns_add_dependency (&names, "manufacturer");
                }

                /* We give detailed information in the status tooltip. */
                if (strcmp (column_name, "driver_status") == 0)
                {
                        columns_add_dependency (&names, "driver_emulation");
                        columns_add_dependency (&names, "driver_color");
                        columns_add_dependency (&names, "driver_sound");
                        columns_add_dependency (&names, "driver_graphic");
                        columns_add_dependency (&names, "driver_cocktail");
                        columns_add_dependency (&names, "driver_protection");
                }
        }

        return names;
}

/**
 * gva_columns_query_tooltip:
 * @column: a #GtkTreeViewColumn
 * @path: a #GtkTreePath
 * @tooltip: a #GtkTooltip
 *
 * Configures @tooltip for the given @column and @path.
 *
 * Returns: %TRUE if the tooltip should be shown
 **/
gboolean
gva_columns_query_tooltip (GtkTreeViewColumn *column,
                           GtkTreePath *path,
                           GtkTooltip *tooltip)
{
        TooltipFunc tooltip_func;
        GvaGameStoreColumn column_id;
        GtkTreeModel *model;
        GtkWidget *widget;
        GtkTreeIter iter;
        gboolean valid;

        g_return_val_if_fail (GTK_IS_TREE_VIEW_COLUMN (column), FALSE);
        g_return_val_if_fail (path != NULL, FALSE);
        g_return_val_if_fail (GTK_IS_TOOLTIP (tooltip), FALSE);

        column_id = gtk_tree_view_column_get_sort_column_id (column);
        g_return_val_if_fail (column_id >= 0, FALSE);
        g_return_val_if_fail (column_id < G_N_ELEMENTS (column_info), FALSE);

        tooltip_func = column_info[column_id].tooltip;
        widget = gtk_tree_view_column_get_tree_view (column);
        model = gtk_tree_view_get_model (GTK_TREE_VIEW (widget));
        valid = gtk_tree_model_get_iter (model, &iter, path);
        g_return_val_if_fail (valid, FALSE);

        return tooltip_func ? tooltip_func (model, &iter, tooltip) : FALSE;
}
