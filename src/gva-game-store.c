/* Copyright 2007-2010 Matthew Barnes
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

#include "gva-columns.h"
#include "gva-db.h"
#include "gva-error.h"
#include "gva-favorites.h"
#include "gva-time.h"

#define DEFAULT_SORT_COLUMN     GVA_GAME_STORE_COLUMN_DESCRIPTION

static gpointer parent_class = NULL;

static GHashTable *
game_store_get_index (GvaGameStore *game_store)
{
        GHashTable *index;

        index = g_object_get_data (G_OBJECT (game_store), "index");
        g_assert (index != NULL);

        return index;
}

static gint
game_store_compare (GtkTreeModel *model,
                    GtkTreeIter *iter_a,
                    GtkTreeIter *iter_b,
                    gpointer user_data)
{
        GType type;
        GValue value_a;
        GValue value_b;
        gint column;
        gint result;

        memset (&value_a, 0, sizeof (GValue));
        memset (&value_b, 0, sizeof (GValue));

        column = GPOINTER_TO_INT (user_data);
        type = gtk_tree_model_get_column_type (model, column);
        gtk_tree_model_get_value (model, iter_a, column, &value_a);
        gtk_tree_model_get_value (model, iter_b, column, &value_b);

        if (type == G_TYPE_BOOLEAN)
        {
                gboolean va, vb;
                va = g_value_get_boolean (&value_a);
                vb = g_value_get_boolean (&value_b);
                result = (va < vb) ? -1 : (va == vb) ? 0 : 1;
        }
        else if (type == G_TYPE_INT)
        {
                gint va, vb;
                va = g_value_get_int (&value_a);
                vb = g_value_get_int (&value_b);
                result = (va < vb) ? -1 : (va == vb) ? 0 : 1;
        }
        else if (type == G_TYPE_STRING)
        {
                const gchar *va, *vb;
                va = g_value_get_string (&value_a);
                vb = g_value_get_string (&value_b);
                if (va == NULL) va = "";
                if (vb == NULL) vb = "";
                result = strcmp (va, vb);
        }
        else if (type == GVA_TYPE_TIME)
        {
                gdouble diff;
                time_t *va, *vb;
                va = (time_t *) g_value_get_boxed (&value_a);
                vb = (time_t *) g_value_get_boxed (&value_b);
                diff = difftime (*va, *vb);
                result = (diff < 0.0) ? -1 : (diff == 0.0) ? 0 : 1;
        }
        else
        {
                g_assert_not_reached ();
        }

        g_value_unset (&value_a);
        g_value_unset (&value_b);

        /* If the values are equal, compare the default sort column
         * (being careful not to introduce infinite recursion!). */

        if (result != 0 || column == DEFAULT_SORT_COLUMN)
                return result;

        user_data = GINT_TO_POINTER (DEFAULT_SORT_COLUMN);
        return game_store_compare (model, iter_a, iter_b, user_data);
}

static GObject *
game_store_constructor (GType type,
                        guint n_construct_properties,
                        GObjectConstructParam *construct_properties)
{
        GType types[GVA_GAME_STORE_NUM_COLUMNS];
        GObject *object;
        gint column = 0;

        types[column++] = G_TYPE_STRING;     /* COLUMN_NAME */
        types[column++] = G_TYPE_STRING;     /* COLUMN_BIOS */
        types[column++] = G_TYPE_STRING;     /* COLUMN_CATEGORY */
        types[column++] = G_TYPE_BOOLEAN;    /* COLUMN_FAVORITE */
        types[column++] = G_TYPE_STRING;     /* COLUMN_SOURCEFILE */
        types[column++] = G_TYPE_BOOLEAN;    /* COLUMN_ISBIOS */
        types[column++] = G_TYPE_BOOLEAN;    /* COLUMN_RUNNABLE */
        types[column++] = G_TYPE_STRING;     /* COLUMN_CLONEOF */
        types[column++] = G_TYPE_STRING;     /* COLUMN_ROMOF */
        types[column++] = G_TYPE_STRING;     /* COLUMN_ROMSET */
        types[column++] = G_TYPE_STRING;     /* COLUMN_SAMPLEOF */
        types[column++] = G_TYPE_STRING;     /* COLUMN_SAMPLESET */
        types[column++] = G_TYPE_STRING;     /* COLUMN_DESCRIPTION */
        types[column++] = G_TYPE_STRING;     /* COLUMN_YEAR */
        types[column++] = G_TYPE_STRING;     /* COLUMN_MANUFACTURER */
        types[column++] = G_TYPE_INT;        /* COLUMN_SOUND_CHANNELS */
        types[column++] = G_TYPE_BOOLEAN;    /* COLUMN_INPUT_SERVICE */
        types[column++] = G_TYPE_BOOLEAN;    /* COLUMN_INPUT_TILT */
        types[column++] = G_TYPE_INT;        /* COLUMN_INPUT_PLAYERS */
        types[column++] = G_TYPE_INT;        /* COLUMN_INPUT_PLAYERS_ALT */
        types[column++] = G_TYPE_INT;        /* COLUMN_INPUT_PLAYERS_SIM */
        types[column++] = G_TYPE_INT;        /* COLUMN_INPUT_BUTTONS */
        types[column++] = G_TYPE_INT;        /* COLUMN_INPUT_COINS */
        types[column++] = G_TYPE_STRING;     /* COLUMN_DRIVER_STATUS */
        types[column++] = G_TYPE_STRING;     /* COLUMN_DRIVER_EMULATION */
        types[column++] = G_TYPE_STRING;     /* COLUMN_DRIVER_COLOR */
        types[column++] = G_TYPE_STRING;     /* COLUMN_DRIVER_SOUND */
        types[column++] = G_TYPE_STRING;     /* COLUMN_DRIVER_GRAPHIC */
        types[column++] = G_TYPE_STRING;     /* COLUMN_DRIVER_COCKTAIL */
        types[column++] = G_TYPE_STRING;     /* COLUMN_DRIVER_PROTECTION */
        types[column++] = G_TYPE_STRING;     /* COLUMN_DRIVER_SAVESTATE */
        types[column++] = G_TYPE_INT;        /* COLUMN_DRIVER_PALETTESIZE */
        types[column++] = GVA_TYPE_TIME;     /* COLUMN_LAST_PLAYED */
        types[column++] = G_TYPE_STRING;     /* COLUMN_COMMENT */
        types[column++] = G_TYPE_INT64;      /* COLUMN_INODE */
        types[column++] = G_TYPE_STRING;     /* COLUMN_INPFILE */
        types[column++] = GVA_TYPE_TIME;     /* COLUMN_TIME */

        g_assert (column == GVA_GAME_STORE_NUM_COLUMNS);

        /* Chain up to parent's constructor() method. */
        object = G_OBJECT_CLASS (parent_class)->constructor (
                type, n_construct_properties, construct_properties);

        gtk_tree_store_set_column_types (
                GTK_TREE_STORE (object), G_N_ELEMENTS (types), types);

        gtk_tree_sortable_set_default_sort_func (
                GTK_TREE_SORTABLE (object),
                (GtkTreeIterCompareFunc) game_store_compare,
                GINT_TO_POINTER (DEFAULT_SORT_COLUMN), NULL);

        for (column = 0; column < GVA_GAME_STORE_NUM_COLUMNS; column++)
                gtk_tree_sortable_set_sort_func (
                        GTK_TREE_SORTABLE (object), column,
                        (GtkTreeIterCompareFunc) game_store_compare,
                        GINT_TO_POINTER (column), NULL);

        return object;
}

static void
game_store_class_init (GvaGameStoreClass *class)
{
        GObjectClass *object_class;

        parent_class = g_type_class_peek_parent (class);

        object_class = G_OBJECT_CLASS (class);
        object_class->constructor = game_store_constructor;
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
                static const GTypeInfo type_info =
                {
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
                        GTK_TYPE_TREE_STORE, "GvaGameStore", &type_info, 0);
        }

        return type;
}

/**
 * gva_game_store_new:
 *
 * Creates a new #GvaGameStore with pre-defined columns and settings.
 *
 * Returns: a new #GvaGameStore
 **/
GtkTreeModel *
gva_game_store_new (void)
{
        return g_object_new (GVA_TYPE_GAME_STORE, NULL);
}

/**
 * gva_game_store_new_from_query:
 * @sql: an SQL query
 * @error: return location for a #GError, or %NULL
 *
 * This may be the most powerful function in
 * <emphasis>GNOME Video Arcade</emphasis>.
 *
 * Creates a new #GvaGameStore by executing the given SQL query on the games
 * database and converting the results to tree model rows.  The resulting
 * #GtkTreeModel can then be plugged into a #GtkTreeView.
 *
 * XXX Say more here.
 *
 * Returns: a new #GvaGameStore
 **/
GtkTreeModel *
gva_game_store_new_from_query (const gchar *sql,
                               GError **error)
{
        GtkTreeModel *model;
        sqlite3_stmt *stmt;
        GvaGameStoreColumn *column_ids;
        GValue *column_values;
        const gchar *name;
        gint n_columns, ii;
        gint name_column = -1;
        gint errcode;

        g_return_val_if_fail (sql != NULL, NULL);

        /* Prepare to execute the query. */

        if (!gva_db_prepare (sql, &stmt, error))
                return NULL;

        /* Allocate an extra column for favorites. */
        model = gva_game_store_new ();
        n_columns = sqlite3_column_count (stmt);
        column_ids = g_newa (GvaGameStoreColumn, n_columns + 1);
        column_values = g_new0 (GValue, n_columns + 1);

        for (ii = 0; ii < n_columns; ii++)
        {
                GType type;
                const gchar *column_name;

                column_name = sqlite3_column_name (stmt, ii);
                if (!gva_columns_lookup_id (column_name, &column_ids[ii]))
                {
                        g_set_error (
                                error, GVA_ERROR, GVA_ERROR_QUERY,
                                "Unrecognized column \"%s\"",
                                column_name);
                        goto fail;
                }
                if (column_ids[ii] == GVA_GAME_STORE_COLUMN_NAME)
                        name_column = ii;

                type = gtk_tree_model_get_column_type (model, column_ids[ii]);
                g_value_init (&column_values[ii], type);
        }

        /* Tack on an extra value for favorites.  This information is not
         * stored in the database so we want to supply it for every query. */
        column_ids[n_columns] = GVA_GAME_STORE_COLUMN_FAVORITE;
        g_value_init (&column_values[n_columns], G_TYPE_BOOLEAN);

        if (name_column < 0)
        {
                g_set_error (
                        error, GVA_ERROR, GVA_ERROR_QUERY,
                        "Query result must include a \"name\" column");
                goto fail;
        }

        /* Execute the query. */

        while ((errcode = sqlite3_step (stmt)) == SQLITE_ROW)
        {
                GtkTreeIter iter;
                GValue *value;

                /* Append a new row to the tree store. */
                gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);

                /* Populate the row with available values. */
                for (ii = 0; ii < n_columns; ii++)
                {
                        GType type;

                        value = &column_values[ii];
                        type = G_VALUE_TYPE (value);

                        if (type == G_TYPE_BOOLEAN)
                        {
                                const gchar *text;
                                gboolean v_boolean;

                                text = (const gchar *)
                                        sqlite3_column_text (stmt, ii);
                                v_boolean = (text != NULL) &&
                                        (strcmp (text, "yes") == 0);
                                g_value_set_boolean (value, v_boolean);
                        }
                        else if (type == G_TYPE_INT)
                        {
                                gint v_int;

                                v_int = sqlite3_column_int (stmt, ii);
                                g_value_set_int (value, v_int);
                        }
                        else if (type == G_TYPE_STRING)
                        {
                                const gchar *v_string;

                                v_string = (const gchar *)
                                        sqlite3_column_text (stmt, ii);
                                if (v_string == NULL)
                                        v_string = "";
                                g_value_set_string (value, v_string);
                        }
                        else if (type == GVA_TYPE_TIME)
                        {
                                sqlite3_int64 v_int64;
                                time_t v_time;

                                /* XXX Is this widely portable? */
                                v_int64 = sqlite3_column_int64 (stmt, ii);
                                v_time = (time_t) v_int64;
                                g_value_set_boxed (value, &v_time);
                        }
                        else
                        {
                                g_assert_not_reached ();
                        }
                }

                name = (const gchar *) sqlite3_column_text (stmt, name_column);

                value = &column_values[n_columns];
                g_value_set_boolean (value, gva_favorites_contains (name));

                /* Store the row in the tree model. */
                gtk_tree_store_set_valuesv (
                        GTK_TREE_STORE (model), &iter,
                        (gint *) column_ids, column_values, n_columns + 1);

                /* Add an entry for this row to the index. */
                gva_game_store_index_insert (
                        GVA_GAME_STORE (model), g_strdup (name), &iter);

                /* Keep the UI responsive. */
                if (gtk_main_iteration_do (FALSE))
                        goto fail;
        }

        /* Query complete. */

        if (errcode == SQLITE_DONE)
                goto exit;

        gva_db_set_error (error, 0, NULL);

fail:
        g_object_unref (model);
        model = NULL;

exit:
        for (ii = 0; ii < n_columns; ii++)
                g_value_unset (&column_values[ii]);

        g_free (column_values);
        sqlite3_finalize (stmt);

        return model;
}

/**
 * gva_game_store_clear:
 * @game_store: a #GvaGameStore
 *
 * Removes all rows from @game_store and clears the internal index.
 **/
void
gva_game_store_clear (GvaGameStore *game_store)
{
        g_return_if_fail (GVA_IS_GAME_STORE (game_store));

        g_hash_table_remove_all (game_store_get_index (game_store));
        gtk_tree_store_clear (GTK_TREE_STORE (game_store));
}

/**
 * gva_game_store_index_insert:
 * @game_store: a #GvaGameStore
 * @key: an index key
 * @iter: a #GtkTreeIter pointing to a row in @game_store
 *
 * Adds an entry to @game_store's internal index.  You will want to call
 * this immediately after adding a new row to @game_store, such as with
 * gtk_tree_store_append().
 **/
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

/**
 * gva_game_store_index_lookup:
 * @game_store: a #GvaGameStore
 * @key: an index key
 *
 * Looks up the row corresponding to @key in @game_store and returns a
 * #GtkTreePath to it, or %NULL if the row was not found.
 *
 * Returns: a #GtkTreePath to the row corresponding to @key, or %NULL
 **/
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
