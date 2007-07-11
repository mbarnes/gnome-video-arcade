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

#include "gva-columns.h"

#include <string.h>

#include "gva-cell-renderer-pixbuf.h"
#include "gva-error.h"
#include "gva-favorites.h"
#include "gva-tree-view.h"
#include "gva-ui.h"

typedef GtkTreeViewColumn * (*FactoryFunc) (GvaGameStoreColumn);

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
        widget = column->tree_view;
        /*widget = gtk_tree_view_column_get_tree_view (column);*/
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

        visible = (sampleset != NULL);
        sensitive = visible && (strcmp (sampleset, "correct") == 0);

        g_object_set (
                renderer, "sensitive", sensitive, "visible", visible, NULL);

        g_free (sampleset);
}

static GdkPixbuf *
columns_get_icon_name (const gchar *icon_name)
{
        GtkIconTheme *icon_theme;
        GdkPixbuf *pixbuf;
        GdkPixbuf *scaled;
        gboolean valid;
        gint size;
        GError *error = NULL;

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

        g_object_unref (pixbuf);

        return scaled;
}

static GtkTreeViewColumn *
columns_factory_description (GvaGameStoreColumn column_id)
{
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;

        renderer = gtk_cell_renderer_text_new ();
        g_object_set (renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_expand (column, TRUE);
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);
        gtk_tree_view_column_set_title (column, _("Title"));
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        gtk_tree_view_column_add_attribute (
                column, renderer, "text", column_id);

        return column;
}

static GtkTreeViewColumn *
columns_factory_favorite (GvaGameStoreColumn column_id)
{
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        GdkPixbuf *pixbuf;

        pixbuf = columns_get_icon_name ("emblem-favorite");

        renderer = gva_cell_renderer_pixbuf_new ();
        g_object_set (renderer, "pixbuf", pixbuf, NULL);

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);
        gtk_tree_view_column_set_title (column, _("Favorite"));
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        gtk_tree_view_column_add_attribute (
                column, renderer, "sensitive", column_id);

        g_signal_connect (
                renderer, "clicked",
                G_CALLBACK (columns_favorite_clicked_cb), column);

        if (pixbuf != NULL)
                g_object_unref (pixbuf);

        return column;
}

static GtkTreeViewColumn *
columns_factory_sampleset (GvaGameStoreColumn column_id)
{
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        GdkPixbuf *pixbuf;

        pixbuf = columns_get_icon_name ("emblem-sound");

        renderer = gtk_cell_renderer_pixbuf_new ();
        g_object_set (renderer, "pixbuf", pixbuf, NULL);

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);
        gtk_tree_view_column_set_title (column, _("Samples"));
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        gtk_tree_view_column_set_cell_data_func (
                column, renderer, (GtkTreeCellDataFunc)
                columns_sampleset_set_properties, NULL, NULL);

        if (pixbuf != NULL)
                g_object_unref (pixbuf);

        return column;
}

static struct
{
        const gchar *name;
        FactoryFunc factory;
}
column_info[GVA_GAME_STORE_NUM_COLUMNS] =
{
        { "name",               NULL },
        { "sourcefile",         NULL },
        { "runnable",           NULL },
        { "cloneof",            NULL },
        { "romof",              NULL },
        { "romset",             NULL },
        { "sampleof",           NULL },
        { "sampleset",          columns_factory_sampleset },
        { "description",        columns_factory_description },
        { "year",               NULL },
        { "manufacturer",       NULL },
        { "history",            NULL },
        { "video_screen",       NULL },
        { "video_orientation",  NULL },
        { "video_width",        NULL },
        { "video_height",       NULL },
        { "video_aspectx",      NULL },
        { "video_aspecty",      NULL },
        { "video_refresh",      NULL },
        { "sound_channels",     NULL },
        { "input_service",      NULL },
        { "input_tilt",         NULL },
        { "input_players",      NULL },
        { "input_control",      NULL },
        { "input_buttons",      NULL },
        { "input_coins",        NULL },
        { "driver_status",      NULL },
        { "driver_emulation",   NULL },
        { "driver_color",       NULL },
        { "driver_sound",       NULL },
        { "driver_graphic",     NULL },
        { "driver_cocktail",    NULL },
        { "driver_protection",  NULL },
        { "driver_savestate",   NULL },
        { "driver_palettesize", NULL },
        { "favorite",           columns_factory_favorite },
        { "inpfile",            NULL },
        { "time",               NULL }
};

GtkTreeViewColumn *
gva_columns_new_from_id (GvaGameStoreColumn column_id)
{
        GtkTreeViewColumn *column;

        g_return_val_if_fail (column_id >= 0, NULL);
        g_return_val_if_fail (column_id < G_N_ELEMENTS (column_info), NULL);
        g_return_val_if_fail (column_info[column_id].factory != NULL, NULL);

        column = column_info[column_id].factory (column_id);

        g_object_set_data (
                G_OBJECT (column), "name",
                (gpointer) column_info[column_id].name);

        return column;
}

GtkTreeViewColumn *
gva_columns_new_from_name (const gchar *column_name)
{
        gint ii;

        for (ii = 0; ii < G_N_ELEMENTS (column_info); ii++)
                if (strcmp (column_name, column_info[ii].name) == 0)
                        break;

        return gva_columns_new_from_id ((GvaGameStoreColumn) ii);
}

void
gva_columns_load (GtkTreeView *view)
{
        GSList *list, *iter;
        GConfClient *client;
        GError *error = NULL;

        g_return_if_fail (GTK_IS_TREE_VIEW (view));

        client = gconf_client_get_default ();
        list = gconf_client_get_list (
                client, GVA_GCONF_COLUMNS_KEY,
                GCONF_VALUE_STRING, &error);
        gva_error_handle (&error);
        g_object_unref (client);

        /* Restore the GConf default if the list comes back empty. */
        if (list == NULL)
        {
                list = g_slist_prepend (
                        list, g_strdup (column_info
                        [GVA_GAME_STORE_COLUMN_SAMPLESET].name));
                list = g_slist_prepend (
                        list, g_strdup (column_info
                        [GVA_GAME_STORE_COLUMN_DESCRIPTION].name));
                list = g_slist_prepend (
                        list, g_strdup (column_info
                        [GVA_GAME_STORE_COLUMN_FAVORITE].name));
        }

        for (iter = list; iter != NULL; iter = iter->next)
        {
                GtkTreeViewColumn *column;

                column = gva_columns_new_from_name (iter->data);
                gtk_tree_view_append_column (view, column);
        }

        g_slist_foreach (list, (GFunc) g_free, NULL);
        g_slist_free (list);
}

void
gva_columns_save (GtkTreeView *view)
{
        GList *columns, *iter;
        GSList *list = NULL;
        GConfClient *client;
        GError *error = NULL;

        g_return_if_fail (GTK_IS_TREE_VIEW (view));

        columns = gtk_tree_view_get_columns (view);

        if (columns == NULL)
                return;

        for (iter = columns; iter != NULL; iter = iter->next)
        {
                gpointer data;

                data = g_object_get_data (iter->data, "name");
                list = g_slist_append (list, data);
        }

        g_list_free (columns);

        client = gconf_client_get_default ();
        gconf_client_set_list (
                client, GVA_GCONF_COLUMNS_KEY,
                GCONF_VALUE_STRING, list, &error);
        gva_error_handle (&error);
        g_object_unref (client);

        g_slist_free (list);
}
