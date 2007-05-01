#include "gva-game-store.h"

#include "gva-xmame.h"

#define GAME_STORE_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE \
        ((obj), GVA_TYPE_GAME_STORE, GameStorePrivate))

typedef struct _GameStorePrivate GameStorePrivate;

struct _GameStorePrivate
{
        GHashTable *rom_index;
};

static gpointer parent_class = NULL;

/* Helper for gva_game_store_load_games() */
static void
game_store_add_game (gchar *romname, gchar *title, GvaGameStore *game_store)
{
        GameStorePrivate *priv = GAME_STORE_GET_PRIVATE (game_store);
        GtkTreeRowReference *tree_row_reference;
        GtkTreePath *tree_path;
        GtkTreeIter tree_iter;

        gtk_list_store_append (GTK_LIST_STORE (game_store), &tree_iter);

        gtk_list_store_set (
                GTK_LIST_STORE (game_store), &tree_iter,
                GVA_GAME_STORE_COLUMN_ROMNAME, romname,
                GVA_GAME_STORE_COLUMN_TITLE, title,
                -1);

        tree_path = gtk_tree_model_get_path (
                GTK_TREE_MODEL (game_store), &tree_iter);
        g_assert (tree_path != NULL);
        tree_row_reference = gtk_tree_row_reference_new (
                GTK_TREE_MODEL (game_store), tree_path);
        g_assert (tree_row_reference != NULL);
        g_hash_table_insert (
                priv->rom_index, g_strdup (romname), tree_row_reference);
        gtk_tree_path_free (tree_path);
}

static GObject *
game_store_constructor (GType type, guint n_construct_properties,
                        GObjectConstructParam *construct_properties)
{
        GType types[] = { G_TYPE_STRING, G_TYPE_STRING };
        GObject *object;

        g_assert (G_N_ELEMENTS (types) == GVA_GAME_STORE_NUM_COLUMNS);

        /* Chain up to parent's constructor() method. */
        object = G_OBJECT_CLASS (parent_class)->constructor (
                type, n_construct_properties, construct_properties);

        gtk_list_store_set_column_types (
                GTK_LIST_STORE (object), G_N_ELEMENTS (types), types);

        return object;
}

static void
game_store_finalize (GObject *object)
{
        GameStorePrivate *priv = GAME_STORE_GET_PRIVATE (object);

        g_hash_table_destroy (priv->rom_index);

        /* Chain up to parent's finalize() method. */
        G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
game_store_class_init (GvaGameStoreClass *class)
{
        GObjectClass *object_class;

        parent_class = g_type_class_peek_parent (class);
        g_type_class_add_private (class, sizeof (GameStorePrivate));

        object_class = G_OBJECT_CLASS (class);
        object_class->constructor = game_store_constructor;
        object_class->finalize = game_store_finalize;
}

static void
game_store_init (GvaGameStore *game_store)
{
        GameStorePrivate *priv = GAME_STORE_GET_PRIVATE (game_store);

        priv->rom_index = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) gtk_tree_row_reference_free);
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

GvaGameStore *
gva_game_store_new (void)
{
        return g_object_new (GVA_TYPE_GAME_STORE, NULL);
}

gboolean
gva_game_store_load_games (GvaGameStore *game_store, GError **error)
{
        GHashTable *hash_table;

        g_return_val_if_fail (game_store != NULL, FALSE);

        hash_table = gva_xmame_list_full (error);
        if (hash_table == NULL)
                return FALSE;

        g_debug ("Loaded %u game titles", g_hash_table_size (hash_table));

        g_hash_table_foreach (
                hash_table, (GHFunc) game_store_add_game, game_store);

        g_hash_table_destroy (hash_table);

        return TRUE;
}

GtkTreeRowReference *
gva_game_store_lookup (GvaGameStore *game_store, const gchar *romname)
{
        GameStorePrivate *priv;

        g_return_val_if_fail (game_store != NULL, NULL);
        g_return_val_if_fail (romname != NULL, NULL);

        priv = GAME_STORE_GET_PRIVATE (game_store);

        return g_hash_table_lookup (priv->rom_index, romname);
}
