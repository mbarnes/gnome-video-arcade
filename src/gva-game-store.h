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
        GVA_GAME_STORE_COLUMN_NAME,               /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_SOURCEFILE,         /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_RUNNABLE,           /* G_TYPE_BOOLEAN */
        GVA_GAME_STORE_COLUMN_CLONEOF,            /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_ROMOF,              /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_ROMSET,             /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_SAMPLEOF,           /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_SAMPLESET,          /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_DESCRIPTION,        /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_YEAR,               /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_MANUFACTURER,       /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_HISTORY,            /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_VIDEO_SCREEN,       /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_VIDEO_ORIENTATION,  /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_VIDEO_WIDTH,        /* G_TYPE_INT */
        GVA_GAME_STORE_COLUMN_VIDEO_HEIGHT,       /* G_TYPE_INT */
        GVA_GAME_STORE_COLUMN_VIDEO_ASPECTX,      /* G_TYPE_INT */
        GVA_GAME_STORE_COLUMN_VIDEO_ASPECTY,      /* G_TYPE_INT */
        GVA_GAME_STORE_COLUMN_VIDEO_REFRESH,      /* G_TYPE_INT */
        GVA_GAME_STORE_COLUMN_SOUND_CHANNELS,     /* G_TYPE_INT */
        GVA_GAME_STORE_COLUMN_INPUT_SERVICE,      /* G_TYPE_BOOLEAN */
        GVA_GAME_STORE_COLUMN_INPUT_TILT,         /* G_TYPE_BOOLEAN */
        GVA_GAME_STORE_COLUMN_INPUT_PLAYERS,      /* G_TYPE_INT */
        GVA_GAME_STORE_COLUMN_INPUT_CONTROL,      /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_INPUT_BUTTONS,      /* G_TYPE_INT */
        GVA_GAME_STORE_COLUMN_INPUT_COINS,        /* G_TYPE_INT */
        GVA_GAME_STORE_COLUMN_DRIVER_STATUS,      /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_DRIVER_EMULATION,   /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_DRIVER_COLOR,       /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_DRIVER_SOUND,       /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_DRIVER_GRAPHIC,     /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_DRIVER_COCKTAIL,    /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_DRIVER_PROTECTION,  /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_DRIVER_SAVESTATE,   /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_DRIVER_PALETTESIZE, /* G_TYPE_INT */
        GVA_GAME_STORE_COLUMN_FAVORITE,           /* G_TYPE_BOOLEAN */
        GVA_GAME_STORE_COLUMN_INPFILE,            /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_TIME,               /* GVA_TYPE_TIME */
        GVA_GAME_STORE_COLUMN_USES_SAMPLES,       /* G_TYPE_BOOLEAN */
        GVA_GAME_STORE_COLUMN_HAVE_SAMPLES,       /* G_TYPE_BOOLEAN */
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

GType           gva_game_store_get_type         (void);
GtkTreeModel *  gva_game_store_new              (void);
void            gva_game_store_clear            (GvaGameStore *game_store);
guint           gva_game_store_populate         (GvaGameStore *game_store,
                                                 GError **error);
void            gva_game_store_index_insert     (GvaGameStore *game_store,
                                                 const gchar *key,
                                                 GtkTreeIter *iter);
GtkTreePath *   gva_game_store_index_lookup     (GvaGameStore *game_store,
                                                 const gchar *key);

G_END_DECLS

#endif /* GVA_GAME_STORE_H */
