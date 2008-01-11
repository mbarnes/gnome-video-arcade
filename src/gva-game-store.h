/* Copyright 2007, 2008 Matthew Barnes
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

/**
 * SECTION: gva-game-store
 * @short_description: A #GtkTreeModel that stores game information
 *
 * A #GvaGameStore stores information from the game database as a
 * #GtkTreeModel.
 **/

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

typedef enum
{
        GVA_GAME_STORE_COLUMN_NAME,               /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_CATEGORY,           /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_FAVORITE,           /* G_TYPE_BOOLEAN */
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
        GVA_GAME_STORE_COLUMN_SOUND_CHANNELS,     /* G_TYPE_INT */
        GVA_GAME_STORE_COLUMN_INPUT_SERVICE,      /* G_TYPE_BOOLEAN */
        GVA_GAME_STORE_COLUMN_INPUT_TILT,         /* G_TYPE_BOOLEAN */
        GVA_GAME_STORE_COLUMN_INPUT_PLAYERS,      /* G_TYPE_INT */
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
        GVA_GAME_STORE_COLUMN_COMMENT,            /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_INODE,              /* G_TYPE_INT64 */
        GVA_GAME_STORE_COLUMN_INPFILE,            /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_TIME,               /* GVA_TYPE_TIME */
        GVA_GAME_STORE_NUM_COLUMNS
} GvaGameStoreColumn;

/**
 * GvaGameStore:
 *
 * Contains only private data that should be read and manipulated using the
 * functions below.
 **/
struct _GvaGameStore
{
        GtkTreeStore parent;
};

struct _GvaGameStoreClass
{
        GtkTreeStoreClass parent_class;
};

GType           gva_game_store_get_type         (void);
GtkTreeModel *  gva_game_store_new              (void);
GtkTreeModel *  gva_game_store_new_from_query   (const gchar *sql,
                                                 GError **error);
void            gva_game_store_clear            (GvaGameStore *game_store);
void            gva_game_store_index_insert     (GvaGameStore *game_store,
                                                 const gchar *key,
                                                 GtkTreeIter *iter);
GtkTreePath *   gva_game_store_index_lookup     (GvaGameStore *game_store,
                                                 const gchar *key);

G_END_DECLS

#endif /* GVA_GAME_STORE_H */
