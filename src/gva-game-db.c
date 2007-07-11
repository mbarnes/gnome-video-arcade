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

#include "gva-game-db.h"

#include <string.h>

#include "gva-error.h"
#include "gva-game-store.h"
#include "gva-xmame.h"

static void
game_db_insert (const gchar *romname)
{
        GtkTreeModel *model;
        GtkTreeIter iter;
        gboolean favorite;

        model = gva_game_db_get_model ();

        favorite = gva_favorites_contains (romname);

        gtk_list_store_append (GTK_LIST_STORE (model), &iter);

        gtk_list_store_set (
                GTK_LIST_STORE (model), &iter,
                GVA_GAME_STORE_COLUMN_NAME, romname,
                GVA_GAME_STORE_COLUMN_FAVORITE, favorite,
                -1);

        gva_game_store_index_insert (GVA_GAME_STORE (model), romname, &iter);
}

static void
game_db_add_sample (const gchar *romname, const gchar *status)
{
        GtkTreePath *path;

        path = gva_game_db_lookup (romname);

        if (path != NULL)
        {
                GtkTreeModel *model;
                GtkTreeIter iter;
                gboolean valid;

                model = gva_game_db_get_model ();
                valid = gtk_tree_model_get_iter (model, &iter, path);
                gtk_tree_path_free (path);
                g_assert (valid);

                gtk_list_store_set (
                        GTK_LIST_STORE (model), &iter,
                        GVA_GAME_STORE_COLUMN_SAMPLESET, status, -1);
        }
}

static void
game_db_add_title (const gchar *romname, const gchar *title)
{
        GtkTreePath *path;

        path = gva_game_db_lookup (romname);

        if (path != NULL)
        {
                GtkTreeModel *model;
                GtkTreeIter iter;
                gboolean valid;

                model = gva_game_db_get_model ();
                valid = gtk_tree_model_get_iter (model, &iter, path);
                gtk_tree_path_free (path);
                g_assert (valid);

                gtk_list_store_set (
                        GTK_LIST_STORE (model), &iter,
                        GVA_GAME_STORE_COLUMN_DESCRIPTION, title, -1);
        }
}

gboolean
gva_game_db_init (GError **error)
{
        GvaProcess *process;
        GPtrArray *array;

        array = gva_xmame_get_romset_files (error);
        if (array == NULL)
                return FALSE;

        g_ptr_array_foreach (array, (GFunc) game_db_insert, NULL);
        g_ptr_array_foreach (array, (GFunc) g_free, NULL);
        g_ptr_array_free (array, TRUE);

        return TRUE;
}

GtkTreePath *
gva_game_db_lookup (const gchar *romname)
{
        GtkTreeModel *model;

        g_return_val_if_fail (romname != NULL, NULL);

        model = gva_game_db_get_model ();

        return gva_game_store_index_lookup (GVA_GAME_STORE (model), romname);
}

GtkTreeModel *
gva_game_db_get_model (void)
{
        static GtkTreeModel *model = NULL;

        if (G_UNLIKELY (model == NULL))
                model = gva_game_store_new ();

        return model;
}

GvaProcess *
gva_game_db_update_samples (GError **error)
{
        return gva_xmame_verify_sample_sets (
                (GvaXmameCallback) game_db_add_sample, NULL, error);
}

GvaProcess *
gva_game_db_update_titles (GError **error)
{
        return gva_xmame_list_full (
                (GvaXmameCallback) game_db_add_title, NULL, error);
}
