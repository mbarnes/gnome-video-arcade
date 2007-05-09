#include "gva-game-store.h"

#include <string.h>
#include <time.h>
#include "gva-time.h"
#include "gva-xmame.h"

static gpointer parent_class = NULL;

static gint
game_store_time_compare (GtkTreeModel *model, GtkTreeIter *iter_a,
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
game_store_constructor (GType type, guint n_construct_properties,
                        GObjectConstructParam *construct_properties)
{
        GType types[GVA_GAME_STORE_NUM_COLUMNS];
        GObject *object;

        types[0] = G_TYPE_STRING;       /* GVA_GAME_STORE_COLUMN_INPFILE */
        types[1] = G_TYPE_STRING;       /* GVA_GAME_STORE_COLUMN_ROMNAME */
        types[2] = G_TYPE_STRING;       /* GVA_GAME_STORE_COLUMN_TITLE */
        types[3] = GVA_TYPE_TIME;       /* GVA_GAME_STORE_COLUMN_TIME */
        types[4] = G_TYPE_BOOLEAN;      /* GVA_GAME_STORE_COLUMN_FAVORITE */

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
