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

#include "gva-game-store.h"

#include <string.h>
#include <time.h>
#include "gva-time.h"
#include "gva-xmame.h"

enum {
        POPULATE_BEGIN,
        POPULATE_PROGRESS,
        POPULATE_END,
        LAST_SIGNAL
};

static gpointer parent_class = NULL;
static guint signals[LAST_SIGNAL] = { 0 };

static GHashTable *
game_store_get_index (GvaGameStore *game_store)
{
        GHashTable *index;

        index = g_object_get_data (G_OBJECT (game_store), "index");
        g_assert (index != NULL);

        return index;
}

static gint
game_store_time_compare (GtkTreeModel *model,
                         GtkTreeIter *iter_a,
                         GtkTreeIter *iter_b)
{
        GValue value_a;
        GValue value_b;
        gdouble diff;

        memset (&value_a, 0, sizeof (GValue));
        memset (&value_b, 0, sizeof (GValue));

        gtk_tree_model_get_value (
                model, iter_a, GVA_GAME_STORE_COLUMN_TIME, &value_a);
        gtk_tree_model_get_value (
                model, iter_b, GVA_GAME_STORE_COLUMN_TIME, &value_b);

        diff = difftime (
                *((time_t *) g_value_get_boxed (&value_a)),
                *((time_t *) g_value_get_boxed (&value_b)));

        g_value_unset (&value_a);
        g_value_unset (&value_b);

        return (diff == 0.0) ? 0 : (diff < 0.0) ? -1 : 1;
}

static GObject *
game_store_constructor (GType type,
                        guint n_construct_properties,
                        GObjectConstructParam *construct_properties)
{
        GType types[GVA_GAME_STORE_NUM_COLUMNS];
        GObject *object;
        gint n = 0;

        types[n++] = G_TYPE_STRING;     /* COLUMN_NAME */
        types[n++] = G_TYPE_STRING;     /* COLUMN_SOURCEFILE */
        types[n++] = G_TYPE_BOOLEAN;    /* COLUMN_RUNNABLE */
        types[n++] = G_TYPE_STRING;     /* COLUMN_CLONEOF */
        types[n++] = G_TYPE_STRING;     /* COLUMN_ROMOF */
        types[n++] = G_TYPE_STRING;     /* COLUMN_ROMSET */
        types[n++] = G_TYPE_STRING;     /* COLUMN_SAMPLEOF */
        types[n++] = G_TYPE_STRING;     /* COLUMN_SAMPLESET */
        types[n++] = G_TYPE_STRING;     /* COLUMN_DESCRIPTION */
        types[n++] = G_TYPE_STRING;     /* COLUMN_YEAR */
        types[n++] = G_TYPE_STRING;     /* COLUMN_MANUFACTURER */
        types[n++] = G_TYPE_STRING;     /* COLUMN_HISTORY */
        types[n++] = G_TYPE_STRING;     /* COLUMN_VIDEO_SCREEN */
        types[n++] = G_TYPE_STRING;     /* COLUMN_VIDEO_ORIENTATION */
        types[n++] = G_TYPE_INT;        /* COLUMN_VIDEO_WIDTH */
        types[n++] = G_TYPE_INT;        /* COLUMN_VIDEO_HEIGHT */
        types[n++] = G_TYPE_INT;        /* COLUMN_VIDEO_ASPECTX */
        types[n++] = G_TYPE_INT;        /* COLUMN_VIDEO_ASPECTY */
        types[n++] = G_TYPE_INT;        /* COLUMN_VIDEO_REFRESH */
        types[n++] = G_TYPE_INT;        /* COLUMN_SOUND_CHANNELS */
        types[n++] = G_TYPE_BOOLEAN;    /* COLUMN_INPUT_SERVICE */
        types[n++] = G_TYPE_BOOLEAN;    /* COLUMN_INPUT_TILT */
        types[n++] = G_TYPE_INT;        /* COLUMN_INPUT_PLAYERS */
        types[n++] = G_TYPE_STRING;     /* COLUMN_INPUT_CONTROL */
        types[n++] = G_TYPE_INT;        /* COLUMN_INPUT_BUTTONS */
        types[n++] = G_TYPE_INT;        /* COLUMN_INPUT_COINS */
        types[n++] = G_TYPE_STRING;     /* COLUMN_DRIVER_STATUS */
        types[n++] = G_TYPE_STRING;     /* COLUMN_DRIVER_EMULATION */
        types[n++] = G_TYPE_STRING;     /* COLUMN_DRIVER_COLOR */
        types[n++] = G_TYPE_STRING;     /* COLUMN_DRIVER_SOUND */
        types[n++] = G_TYPE_STRING;     /* COLUMN_DRIVER_GRAPHIC */
        types[n++] = G_TYPE_STRING;     /* COLUMN_DRIVER_COCKTAIL */
        types[n++] = G_TYPE_STRING;     /* COLUMN_DRIVER_PROTECTION */
        types[n++] = G_TYPE_STRING;     /* COLUMN_DRIVER_SAVESTATE */
        types[n++] = G_TYPE_INT;        /* COLUMN_DRIVER_PALETTESIZE */
        types[n++] = G_TYPE_BOOLEAN;    /* COLUMN_FAVORITE */
        types[n++] = G_TYPE_STRING;     /* COLUMN_INPFILE */
        types[n++] = GVA_TYPE_TIME;     /* COLUMN_TIME */

        g_assert (n == GVA_GAME_STORE_NUM_COLUMNS);

        /* Chain up to parent's constructor() method. */
        object = G_OBJECT_CLASS (parent_class)->constructor (
                type, n_construct_properties, construct_properties);

        gtk_list_store_set_column_types (
                GTK_LIST_STORE (object), G_N_ELEMENTS (types), types);

        gtk_tree_sortable_set_sort_func (
                GTK_TREE_SORTABLE (object), GVA_GAME_STORE_COLUMN_TIME,
                (GtkTreeIterCompareFunc) game_store_time_compare, NULL, NULL);

        return object;
}

static void
game_store_class_init (GvaGameStoreClass *class)
{
        GObjectClass *object_class;

        parent_class = g_type_class_peek_parent (class);

        object_class = G_OBJECT_CLASS (class);
        object_class->constructor = game_store_constructor;

        signals[POPULATE_BEGIN] = g_signal_new (
                "populate-begin",
                G_TYPE_FROM_CLASS (class),
                G_SIGNAL_RUN_LAST,
                0, NULL, NULL,
                g_cclosure_marshal_VOID__VOID,
                G_TYPE_NONE, 0);

        signals[POPULATE_PROGRESS] = g_signal_new (
                "populate-progress",
                G_TYPE_FROM_CLASS (class),
                G_SIGNAL_RUN_LAST,
                0, NULL, NULL,
                g_cclosure_marshal_VOID__INT,
                G_TYPE_NONE, 1,
                G_TYPE_INT);

        signals[POPULATE_END] = g_signal_new (
                "populate-end",
                G_TYPE_FROM_CLASS (class),
                G_SIGNAL_RUN_LAST,
                0, NULL, NULL,
                g_cclosure_marshal_VOID__VOID,
                G_TYPE_NONE, 0);
}

static void
game_store_init (GvaGameStore *game_store)
{
        GHashTable *index;

        index = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) gtk_tree_row_reference_free);

        g_object_set_data_full (
                G_OBJECT (game_store), "index", index,
                (GDestroyNotify) g_hash_table_destroy);
}

GType
gva_game_store_get_type (void)
{
        static GType type = 0;

        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (GvaGameStoreClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) game_store_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (GvaGameStore),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) game_store_init,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        GTK_TYPE_LIST_STORE, "GvaGameStore", &type_info, 0);
        }

        return type;
}

GtkTreeModel *
gva_game_store_new (void)
{
        return g_object_new (GVA_TYPE_GAME_STORE, NULL);
}

void
gva_game_store_clear (GvaGameStore *game_store)
{
        g_return_if_fail (GVA_IS_GAME_STORE (game_store));

        g_hash_table_remove_all (game_store_get_index (game_store));
        gtk_list_store_clear (GTK_LIST_STORE (game_store));
}

typedef struct
{
        GvaGameStore *game_store;
        sqlite3_stmt *stmt;
        GValue value;

} PopulateData;

static PopulateData *
populate_data_new (GvaGameStore *game_store,
                   sqlite3_stmt *stmt)
{
        PopulateData *data;

        data = g_slice_new0 (PopulateData);
        data->game_store = game_store;
        data->stmt = stmt;

        return data;
}

static void
populate_data_free (PopulateData *data)
{
        g_slice_free (PopulateData, data);
}

static gboolean
game_store_populate_idle_cb (PopulateData *data)
{
        GtkListStore *list_store;
        GtkTreeIter iter;
        gint n_columns, ii;
        gint errcode;
        static gint rows = 0;

        errcode = sqlite3_step (data->stmt);

        if (errcode == SQLITE_DONE)
        {
                sqlite3_finalize (data->stmt);
                populate_data_free (data);
                return FALSE;
        }

        if (errcode != SQLITE_ROW)
        {
                sqlite3_finalize (data->stmt);
                populate_data_free (data);
                return FALSE;
        }

        list_store = GTK_LIST_STORE (data->game_store);
        n_columns = sqlite3_column_count (data->stmt);

        gtk_list_store_append (list_store, &iter);

        for (ii = 0; ii < n_columns; ii++)
        {
                GType type;

                type = gtk_tree_model_get_column_type (
                        GTK_TREE_MODEL (list_store), ii);

                g_value_init (&data->value, type);

                if (type == G_TYPE_BOOLEAN)
                {
                        const gchar *text;

                        text = (const gchar *)
                                sqlite3_column_text (data->stmt, ii);
                        g_value_set_boolean (
                                &data->value,
                                g_ascii_strcasecmp (text, "yes") == 0);
                        /*gtk_list_store_set_value (
                                list_store, &iter, ii, &data->value);*/
                }
                else if (type == G_TYPE_INT)
                {
                        g_value_set_int (
                                &data->value,
                                sqlite3_column_int (data->stmt, ii));
                        /*gtk_list_store_set_value (
                                list_store, &iter, ii, &data->value);*/
                }
                else if (type == G_TYPE_STRING)
                {
                        g_value_set_string (
                                &data->value, (const gchar *)
                                sqlite3_column_text (data->stmt, ii));
                        /*gtk_list_store_set_value (
                                list_store, &iter, ii, &data->value);*/
                }
                else
                {
                        g_warning (
                                "Unexpected type '%s' for column %d",
                                G_VALUE_TYPE_NAME (&data->value), ii);
                }

                g_value_unset (&data->value);
        }

        g_print ("Populating %d rows\r", ++rows);

        return TRUE;
}

guint
gva_game_store_populate (GvaGameStore *game_store,
                         GError **error)
{
        sqlite3_stmt *stmt;

        g_return_val_if_fail (GVA_IS_GAME_STORE (game_store), 0);

        if (!gva_db_prepare ("SELECT * FROM game", &stmt, error))
                return 0;

        return g_idle_add (
                (GSourceFunc) game_store_populate_idle_cb,
                populate_data_new (game_store, stmt));
}

void
gva_game_store_index_insert (GvaGameStore *game_store,
                             const gchar *key,
                             GtkTreeIter *iter)
{
        GtkTreeModel *model;
        GtkTreePath *path;

        g_return_if_fail (GVA_IS_GAME_STORE (game_store));
        g_return_if_fail (key != NULL);
        g_return_if_fail (iter != NULL);

        model = GTK_TREE_MODEL (game_store);

        path = gtk_tree_model_get_path (model, iter);
        g_return_if_fail (path != NULL);
        g_hash_table_insert (
                game_store_get_index (game_store), g_strdup (key),
                gtk_tree_row_reference_new (model, path));
        gtk_tree_path_free (path);
}

GtkTreePath *
gva_game_store_index_lookup (GvaGameStore *game_store,
                             const gchar *key)
{
        GtkTreeRowReference *reference;

        g_return_val_if_fail (GVA_IS_GAME_STORE (game_store), NULL);
        g_return_val_if_fail (key != NULL, NULL);

        reference = g_hash_table_lookup (
                game_store_get_index (game_store), key);
        if (reference == NULL)
                return NULL;

        return gtk_tree_row_reference_get_path (reference);
}
