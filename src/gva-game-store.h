#ifndef GVA_GAME_STORE_H
#define GVA_GAME_STORE_H

#include "gva-common.h"

/* Standard GObject macros */
#define GVA_TYPE_GAME_STORE \
        (gva_game_store_get_type ())
#define GVA_GAME_STORE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), GVA_TYPE_GAME_STORE, GvaGameStore))
#define GVA_GAME_STORE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), GVA_TYPE_GAME_STORE, GvaGameStoreClass))
#define GVA_IS_GAME_STORE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), GVA_TYPE_GAME_STORE))
#define GVA_IS_GAME_STORE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), GVA_TYPE_GAME_STORE))
#define GVA_GAME_STORE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), GVA_TYPE_GAME_STORE, GvaGameStoreClass))

G_BEGIN_DECLS

typedef struct _GvaGameStore GvaGameStore;
typedef struct _GvaGameStoreClass GvaGameStoreClass;

enum
{
        GVA_GAME_STORE_COLUMN_INPFILE,          /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_ROMNAME,          /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_TITLE,            /* G_TYPE_STRING */
	GVA_GAME_STORE_COLUMN_TIME,	        /* GVA_TYPE_TIME */
        GVA_GAME_STORE_COLUMN_FAVORITE,         /* G_TYPE_BOOLEAN */
        GVA_GAME_STORE_NUM_COLUMNS
};

struct _GvaGameStore
{
        GtkListStore parent;
};

struct _GvaGameStoreClass
{
        GtkListStoreClass parent_class;
};

GType                 gva_game_store_get_type   (void);
GtkTreeModel *        gva_game_store_new        (void);

G_END_DECLS

#endif /* GVA_GAME_STORE_H */
