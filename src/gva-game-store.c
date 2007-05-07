#include "gva-game-store.h"

#include "gva-xmame.h"

static GType column_types[] =
{
        G_TYPE_STRING,          /* GVA_GAME_STORE_COLUMN_INPNAME */
        G_TYPE_STRING,          /* GVA_GAME_STORE_COLUMN_ROMNAME */
        G_TYPE_STRING,          /* GVA_GAME_STORE_COLUMN_TITLE */
        G_TYPE_STRING,          /* GVA_GAME_STORE_COLUMN_DATE_TIME */
        G_TYPE_BOOLEAN          /* GVA_GAME_STORE_COLUMN_FAVORITE */
};

static gpointer parent_class = NULL;

static GObject *
game_store_constructor (GType type, guint n_construct_properties,
                        GObjectConstructParam *construct_properties)
{
        GObject *object;

        g_assert (G_N_ELEMENTS (column_types) == GVA_GAME_STORE_NUM_COLUMNS);

        /* Chain up to parent's constructor() method. */
        object = G_OBJECT_CLASS (parent_class)->constructor (
                type, n_construct_properties, construct_properties);

        gtk_list_store_set_column_types (
                GTK_LIST_STORE (object),
                G_N_ELEMENTS (column_types), column_types);

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
                        (GInstanceInitFunc) NULL,
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
