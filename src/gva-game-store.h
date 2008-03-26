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

/**
 * GvaGameStoreColumn:
 * @GVA_GAME_STORE_COLUMN_NAME:
 *      Corresponds to the "available.name" database field.
 * @GVA_GAME_STORE_COLUMN_BIOS:
 *      Corresponds to the "available.bios" database field.
 * @GVA_GAME_STORE_COLUMN_CATEGORY:
 *      Corresponds to the "available.category" database field.
 * @GVA_GAME_STORE_COLUMN_FAVORITE:
 *      Corresponds to the "available.favorite" database field.
 * @GVA_GAME_STORE_COLUMN_SOURCEFILE:
 *      Corresponds to the "available.sourcefile" database field.
 * @GVA_GAME_STORE_COLUMN_ISBIOS:
 *      Corresponds to the "available.isbios" database field.
 * @GVA_GAME_STORE_COLUMN_RUNNABLE:
 *      Corresponds to the "available.runnable" database field.
 * @GVA_GAME_STORE_COLUMN_CLONEOF:
 *      Corresponds to the "available.cloneof" database field.
 * @GVA_GAME_STORE_COLUMN_ROMOF:
 *      Corresponds to the "available.romof" database field.
 * @GVA_GAME_STORE_COLUMN_ROMSET:
 *      Corresponds to the "available.romset" database field.
 * @GVA_GAME_STORE_COLUMN_SAMPLEOF:
 *      Corresponds to the "available.sampleof" database field.
 * @GVA_GAME_STORE_COLUMN_SAMPLESET:
 *      Corresponds to the "available.sampleset" database field.
 * @GVA_GAME_STORE_COLUMN_DESCRIPTION:
 *      Corresponds to the "available.description" database field.
 * @GVA_GAME_STORE_COLUMN_YEAR:
 *      Corresponds to the "available.year" database field.
 * @GVA_GAME_STORE_COLUMN_MANUFACTURER:
 *      Corresponds to the "available.manufacturer" database field.
 * @GVA_GAME_STORE_COLUMN_SOUND_CHANNELS:
 *      Corresponds to the "available.sound_channels" database field.
 * @GVA_GAME_STORE_COLUMN_INPUT_SERVICE:
 *      Corresponds to the "available.input_service" database field.
 * @GVA_GAME_STORE_COLUMN_INPUT_TILT:
 *      Corresponds to the "available.input_tile" database field.
 * @GVA_GAME_STORE_COLUMN_INPUT_PLAYERS:
 *      Corresponds to the "available.input_players" database field.
 * @GVA_GAME_STORE_COLUMN_INPUT_BUTTONS:
 *      Corresponds to the "available.input_buttons" database field.
 * @GVA_GAME_STORE_COLUMN_INPUT_COINS:
 *      Corresponds to the "available.input_coins" database field.
 * @GVA_GAME_STORE_COLUMN_DRIVER_STATUS:
 *      Corresponds to the "available.driver_status" database field.
 * @GVA_GAME_STORE_COLUMN_DRIVER_EMULATION:
 *      Corresponds to the "available.driver_emulation" database field.
 * @GVA_GAME_STORE_COLUMN_DRIVER_COLOR:
 *      Corresponds to the "available.driver_color" database field.
 * @GVA_GAME_STORE_COLUMN_DRIVER_SOUND:
 *      Corresponds to the "available.driver_sound" database field.
 * @GVA_GAME_STORE_COLUMN_DRIVER_GRAPHIC:
 *      Corresponds to the "available.driver_graphic" database field.
 * @GVA_GAME_STORE_COLUMN_DRIVER_COCKTAIL:
 *      Corresponds to the "available.driver_cocktail" database field.
 * @GVA_GAME_STORE_COLUMN_DRIVER_PROTECTION:
 *      Corresponds to the "available.driver_protection" database field.
 * @GVA_GAME_STORE_COLUMN_DRIVER_SAVESTATE:
 *      Corresponds to the "available.driver_savestate" database field.
 * @GVA_GAME_STORE_COLUMN_DRIVER_PALETTESIZE:
 *      Corresponds to the "available.driver_palettesize" database field.
 * @GVA_GAME_STORE_COLUMN_COMMENT:
 *      Corresponds to the "playback.comment" database field.
 * @GVA_GAME_STORE_COLUMN_INODE:
 *      Corresponds to the "playback.inode" database field.
 * @GVA_GAME_STORE_COLUMN_INPFILE:
 *      Filename of a recorded game (not in the games database).
 * @GVA_GAME_STORE_COLUMN_TIME:
 *      Timestamp of a recorded game (not in the games database).
 * @GVA_GAME_STORE_NUM_COLUMNS:
 *      Total number of game store columns.
 *
 * Most of these values correspond to fields in the games database.
 **/
typedef enum
{
        GVA_GAME_STORE_COLUMN_NAME,               /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_BIOS,               /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_CATEGORY,           /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_FAVORITE,           /* G_TYPE_BOOLEAN */
        GVA_GAME_STORE_COLUMN_SOURCEFILE,         /* G_TYPE_STRING */
        GVA_GAME_STORE_COLUMN_ISBIOS,             /* G_TYPE_BOOLEAN */
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
