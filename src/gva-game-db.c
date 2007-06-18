#include "gva-game-db.h"

#include <string.h>
#include "gva-game-store.h"
#include "gva-xmame.h"

static GtkTreeModel *model = NULL;
static GHashTable *reference_ht = NULL;

static void
game_db_insert (const gchar *romname)
{
        GtkTreeRowReference *reference;
        GtkTreePath *path;
        GtkTreeIter iter;
        gboolean favorite;

        favorite = gva_favorites_contains (romname);

        gtk_list_store_append (GTK_LIST_STORE (model), &iter);

        gtk_list_store_set (
                GTK_LIST_STORE (model), &iter,
                GVA_GAME_STORE_COLUMN_ROMNAME, romname,
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
game_db_add_sample (const gchar *romname, const gchar *status)
{
        GtkTreePath *path;

        path = gva_game_db_lookup (romname);

        if (path != NULL)
        {
                GtkTreeIter iter;
                gboolean have_samples;
                gboolean valid;

                valid = gtk_tree_model_get_iter (model, &iter, path);
                gtk_tree_path_free (path);
                g_assert (valid);

                have_samples = (strcmp (status, "correct") == 0);

                gtk_list_store_set (
                        GTK_LIST_STORE (model), &iter,
                        GVA_GAME_STORE_COLUMN_USES_SAMPLES, TRUE,
                        GVA_GAME_STORE_COLUMN_HAVE_SAMPLES, have_samples,
                        -1);
        }
}

static void
game_db_add_title (const gchar *romname, const gchar *title)
{
        GtkTreePath *path;

        path = gva_game_db_lookup (romname);

        if (path != NULL)
        {
                GtkTreeIter iter;
                gboolean valid;

                valid = gtk_tree_model_get_iter (model, &iter, path);
                gtk_tree_path_free (path);
                g_assert (valid);

                gtk_list_store_set (
                        GTK_LIST_STORE (model), &iter,
                        GVA_GAME_STORE_COLUMN_TITLE, title, -1);
        }
}

gboolean
gva_game_db_init (GError **error)
{
        GvaProcess *process;
        GPtrArray *array;

        if (model != NULL)
                return TRUE;

        array = gva_xmame_get_romset_files (error);
        if (array == NULL)
                return FALSE;

        model = gva_game_store_new ();

        reference_ht = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) gtk_tree_row_reference_free);

        g_ptr_array_foreach (array, (GFunc) game_db_insert, NULL);
        g_ptr_array_foreach (array, (GFunc) g_free, NULL);
        g_ptr_array_free (array, TRUE);

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
