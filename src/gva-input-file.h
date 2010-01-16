/* Copyright 2007-2010 Matthew Barnes
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
 * SECTION: gva-input-file
 * @short_description: A stable interface for MAME INP files
 *
 * A #GvaInputFile provides metadata about a MAME INP file.  It handles
 * both current and historical INP formats.
 **/

#ifndef GVA_INPUT_FILE_H
#define GVA_INPUT_FILE_H

#include "gva-common.h"
#include "gva-process.h"

/* Standard GObject macros */
#define GVA_TYPE_INPUT_FILE \
        (gva_input_file_get_type ())
#define GVA_INPUT_FILE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), GVA_TYPE_INPUT_FILE, GvaInputFile))
#define GVA_INPUT_FILE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), GVA_TYPE_INPUT_FILE, GvaInputFileClass))
#define GVA_IS_INPUT_FILE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), GVA_TYPE_INPUT_FILE))
#define GVA_IS_INPUT_FILE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), GVA_TYPE_INPUT_FILE))
#define GVA_INPUT_FILE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), GVA_TYPE_INPUT_FILE, GvaInputFileClass))

G_BEGIN_DECLS

typedef struct _GvaInputFile GvaInputFile;
typedef struct _GvaInputFileClass GvaInputFileClass;
typedef struct _GvaInputFilePrivate GvaInputFilePrivate;

/**
 * GvaInputFile:
 *
 * Contains only private data that should be read and manipulated using the
 * functions below.
 **/
struct _GvaInputFile
{
        GObject parent;
        GvaInputFilePrivate *priv;
};

struct _GvaInputFileClass
{
        GObjectClass parent_class;
};

GType           gva_input_file_get_type         (void);
GvaInputFile *  gva_input_file_new              (const gchar *filename);
gboolean        gva_input_file_read             (GvaInputFile *input_file,
                                                 GError **error);
GvaProcess *    gva_input_file_play_back        (GvaInputFile *input_file,
                                                 GError **error);
const gchar *   gva_input_file_get_filename     (GvaInputFile *input_file);
const gchar *   gva_input_file_get_format       (GvaInputFile *input_file);
const gchar *   gva_input_file_get_game         (GvaInputFile *input_file);
const gchar *   gva_input_file_get_origin       (GvaInputFile *input_file);
time_t          gva_input_file_get_timestamp    (GvaInputFile *input_file);

G_END_DECLS

#endif /* GVA_INPUT_FILE_H */
