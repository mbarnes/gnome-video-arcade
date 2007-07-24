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
        sensitive = visible && (strcmp (sampleset, "good") == 0);

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
columns_factory_manufacturer (GvaGameStoreColumn column_id)
{
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;

        renderer = gtk_cell_renderer_text_new ();

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);
        gtk_tree_view_column_set_title (column, _("Manufacturer"));
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        gtk_tree_view_column_add_attribute (
                column, renderer, "text", column_id);

        return column;
}

static GtkTreeViewColumn *
columns_factory_name (GvaGameStoreColumn column_id)
{
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;

        renderer = gtk_cell_renderer_text_new ();

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);
        gtk_tree_view_column_set_title (column, _("ROM Name"));
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        gtk_tree_view_column_add_attribute (
                column, renderer, "text", column_id);

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

static GtkTreeViewColumn *
columns_factory_year (GvaGameStoreColumn column_id)
{
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;

        renderer = gtk_cell_renderer_text_new ();

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, column_id);
        gtk_tree_view_column_set_title (column, _("Year"));
        gtk_tree_view_column_pack_start (column, renderer, TRUE);

        gtk_tree_view_column_add_attribute (
                column, renderer, "text", column_id);

        return column;
}

static struct
{
        const gchar *name;
        FactoryFunc factory;
}
column_info[GVA_GAME_STORE_NUM_COLUMNS] =
{
        { "name",               columns_factory_name },
        { "favorite",           columns_factory_favorite },
        { "sourcefile",         NULL },
        { "runnable",           NULL },
        { "cloneof",            NULL },
        { "romof",              NULL },
        { "romset",             NULL },
        { "sampleof",           NULL },
        { "sampleset",          columns_factory_sampleset },
        { "description",        columns_factory_description },
        { "year",               columns_factory_year },
        { "manufacturer",       columns_factory_manufacturer },
        { "sound_channels",     NULL },
        { "input_service",      NULL },
        { "input_tilt",         NULL },
        { "input_players",      NULL },
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
        { "inpfile",            NULL },
        { "time",               NULL }
};

static gchar *default_column_order[] =
{
        "favorite",
        "description",
        "year",
        "manufacturer",
        "name",
        "sampleset"
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
        GvaGameStoreColumn column_id;

        if (!gva_columns_lookup_id (column_name, &column_id))
                return NULL;

        return gva_columns_new_from_id (column_id);
}

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

const gchar *
gva_columns_lookup_name (GvaGameStoreColumn column_id)
{
        if (CLAMP (column_id, 0, G_N_ELEMENTS (column_info)) != column_id)
                return FALSE;

        return column_info[column_id].name;
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

        client = gconf_client_get_default ();
        all_columns = gconf_client_get_list (
                client, GVA_GCONF_ALL_COLUMNS_KEY, GCONF_VALUE_STRING, &error);
        gva_error_handle (&error);
        visible_columns = gconf_client_get_list (
                client, GVA_GCONF_COLUMNS_KEY, GCONF_VALUE_STRING, &error);
        gva_error_handle (&error);
        g_object_unref (client);

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

                        g_signal_connect_swapped (
                                column, "notify::visible",
                                G_CALLBACK (gva_columns_save), view);
                }
        }

        g_slist_foreach (visible_columns, (GFunc) g_free, NULL);
        g_slist_free (visible_columns);

        gva_columns_save (view);
}

void
gva_columns_save (GtkTreeView *view)
{
        GConfClient *client;
        GSList *list;
        GError *error = NULL;

        g_return_if_fail (GTK_IS_TREE_VIEW (view));

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

gchar **
gva_columns_get_selected (guint *length)
{
        /* XXX Kill this function. */

        GConfClient *client;
        GSList *list;
        gchar **column_names;
        gint ii;
        GError *error = NULL;

        client = gconf_client_get_default ();
        list = gconf_client_get_list (
                client, GVA_GCONF_ALL_COLUMNS_KEY,
                GCONF_VALUE_STRING, &error);
        gva_error_handle (&error);
        g_object_unref (client);

        column_names = g_new0 (gchar *, g_slist_length (list) + 1);

        for (ii = 0; list != NULL; ii++)
        {
                column_names[ii] = list->data;
                list = g_slist_delete_link (list, list);
        }

        if (length != NULL)
                *length = g_strv_length (column_names);

        return column_names;
}

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
