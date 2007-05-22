#include "gva-game-db.h"

#include <string.h>
#include "gva-game-store.h"
#include "gva-xmame.h"

static GtkTreeModel *model = NULL;
static GHashTable *reference_ht = NULL;

static void
game_db_insert (gchar *romname, gchar *title, gchar *rompath)
{
        GtkTreeRowReference *reference;
        GtkTreePath *path;
        GtkTreeIter iter;
        gboolean available;
        gboolean favorite;
        gchar *romfile;

        romfile = g_build_filename (rompath, romname, NULL);
        romfile = g_realloc (romfile, strlen (romfile) + 8);
        strcat (romfile, ".zip");
        available = g_file_test (romfile, G_FILE_TEST_IS_REGULAR);
        g_free (romfile);

        /* Only store available games. */
        if (!available)
                return;

        favorite = gva_favorites_contains (romname);

        gtk_list_store_append (GTK_LIST_STORE (model), &iter);

        gtk_list_store_set (
                GTK_LIST_STORE (model), &iter,
                GVA_GAME_STORE_COLUMN_ROMNAME, romname,
                GVA_GAME_STORE_COLUMN_TITLE, title,
                GVA_GAME_STORE_COLUMN_FAVORITE, favorite,
                -1);

        path = gtk_tree_model_get_path (model, &iter);
        g_assert (path != NULL);
        reference = gtk_tree_row_reference_new (model, path);
        g_assert (reference != NULL);
        g_hash_table_insert (reference_ht, g_strdup (romname), reference);
        gtk_tree_path_free (path);
}

static void
game_db_update_sample (gchar *romname, GtkTreeRowReference *reference,
                       GHashTable *samples)
{
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;
        gboolean valid;
        gboolean uses_samples;
        gboolean have_samples;
        gchar *status;

        model = gtk_tree_row_reference_get_model (reference);
        path = gtk_tree_row_reference_get_path (reference);
        valid = gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_path_free (path);
        g_assert (valid);

        status = g_hash_table_lookup (samples, romname);
        uses_samples = (status != NULL);
        have_samples = uses_samples && (strcmp (status, "correct") == 0);

        gtk_list_store_set (
                GTK_LIST_STORE (model), &iter,
                GVA_GAME_STORE_COLUMN_USES_SAMPLES, uses_samples,
                GVA_GAME_STORE_COLUMN_HAVE_SAMPLES, have_samples,
                -1);
}

gboolean
gva_game_db_init (GError **error)
{
        GHashTable *hash_table;
        gchar *rompath;

        if (model != NULL)
                return TRUE;

        hash_table = gva_xmame_list_full (error);
        if (hash_table == NULL)
                return FALSE;

        rompath = gva_xmame_get_config_value ("rompath", error);
        if (rompath == NULL)
        {
                g_hash_table_destroy (hash_table);
                return FALSE;
        }

        model = gva_game_store_new ();

        reference_ht = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) gtk_tree_row_reference_free);

        g_hash_table_foreach (hash_table, (GHFunc) game_db_insert, rompath);

        g_hash_table_destroy (hash_table);
        g_free (rompath);

        /* Do this AFTER the games are loaded! */
        gtk_tree_sortable_set_sort_column_id (
                GTK_TREE_SORTABLE (model),
                GVA_GAME_STORE_COLUMN_TITLE,
                GTK_SORT_ASCENDING);

        return TRUE;
}

GtkTreePath *
gva_game_db_lookup (const gchar *romname)
{
        GtkTreeRowReference *reference;

        g_return_val_if_fail (model != NULL, NULL);
        g_return_val_if_fail (romname != NULL, NULL);
        g_return_val_if_fail (reference_ht != NULL, NULL);

        reference = g_hash_table_lookup (reference_ht, romname);
        if (reference == NULL)
                return NULL;

        return gtk_tree_row_reference_get_path (reference);
}

GtkTreeModel *
gva_game_db_get_model (void)
{
        return model;
}

gboolean
gva_game_db_update_samples (GError **error)
{
        GHashTable *hash_table;

        g_return_val_if_fail (model != NULL, FALSE);
        g_return_val_if_fail (reference_ht != NULL, FALSE);

        hash_table = gva_xmame_verify_sample_sets (error);
        if (hash_table == NULL)
                return FALSE;

        g_hash_table_foreach (
                reference_ht, (GHFunc) game_db_update_sample, hash_table);

        g_hash_table_destroy (hash_table);

        return TRUE;
}
