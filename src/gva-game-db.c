#include "gva-game-db.h"

#include "gva-game-store.h"
#include "gva-xmame.h"

static GtkTreeModel *db = NULL;
static GHashTable *index = NULL;

static void
game_db_insert (gchar *romname, gchar *title, GtkTreeModel *model)
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
                GVA_GAME_STORE_COLUMN_TITLE, title,
                GVA_GAME_STORE_COLUMN_FAVORITE, favorite,
                -1);

        path = gtk_tree_model_get_path (model, &iter);
        g_assert (path != NULL);
        reference = gtk_tree_row_reference_new (model, path);
        g_assert (reference != NULL);
        g_hash_table_insert (index, g_strdup (romname), reference);
        gtk_tree_path_free (path);
}

gboolean
gva_game_db_init (GError **error)
{
        GHashTable *hash_table;
        GtkTreeModel *model;

        if (db != NULL)
                return TRUE;

        hash_table = gva_xmame_list_full (error);
        if (hash_table == NULL)
                return FALSE;

        model = gva_game_store_new ();

        index = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) gtk_tree_row_reference_free);

        db = gtk_tree_model_sort_new_with_model (model);

        gtk_tree_sortable_set_sort_column_id (
                GTK_TREE_SORTABLE (db),
                GVA_GAME_STORE_COLUMN_TITLE,
                GTK_SORT_ASCENDING);

        g_hash_table_foreach (hash_table, (GHFunc) game_db_insert, model);

        g_hash_table_destroy (hash_table);

        return TRUE;
}

GtkTreePath *
gva_game_db_lookup (const gchar *romname)
{
        GtkTreeRowReference *reference;
        GtkTreePath *child_path, *path;

        g_return_val_if_fail (db != NULL, NULL);
        g_return_val_if_fail (index != NULL, NULL);
        g_return_val_if_fail (romname != NULL, NULL);

        reference = g_hash_table_lookup (index, romname);
        if (reference == NULL)
                return NULL;

        child_path = gtk_tree_row_reference_get_path (reference);
        path = gtk_tree_model_sort_convert_child_path_to_path (
                GTK_TREE_MODEL_SORT (db), child_path);
        gtk_tree_path_free (child_path);

        return path;
}

GtkTreeModel *
gva_game_db_get_model (void)
{
        return db;
}
