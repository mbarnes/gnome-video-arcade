/* Copyright 2007-2011 Matthew Barnes
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
 * SECTION: gva-util
 * @short_description: Miscellaneous Utilities
 *
 * These are miscellaneous utility functions that are used throughout the
 * application.
 **/

#ifndef GVA_UTIL_H
#define GVA_UTIL_H

#include "gva-common.h"

G_BEGIN_DECLS

/**
 * GvaDebugFlags:
 * @GVA_DEBUG_NONE:
 *      Print no messages.
 * @GVA_DEBUG_MAME:
 *      Print shell commands invoking MAME.
 * @GVA_DEBUG_SQL:
 *      Print SQL commands to the game database.
 * @GVA_DEBUG_IO:
 *      Print all communication between GVA and MAME.
 * @GVA_DEBUG_INP:
 *      Print information about input files.
 * @GVA_DEBUG_GST:
 *      Print GStreamer activity.
 *
 * These flags indicate which types of debugging messages will be triggered
 * at runtime. Debugging messages can be triggered by setting the GVA_DEBUG
 * environment variable to a colon-separated list of "mame", "sql" and "io".
 **/
typedef enum
{
        GVA_DEBUG_NONE  = 0,
        GVA_DEBUG_MAME  = 1 << (G_LOG_LEVEL_USER_SHIFT + 0),
        GVA_DEBUG_SQL   = 1 << (G_LOG_LEVEL_USER_SHIFT + 1),
        GVA_DEBUG_IO    = 1 << (G_LOG_LEVEL_USER_SHIFT + 2),
        GVA_DEBUG_INP   = 1 << (G_LOG_LEVEL_USER_SHIFT + 3),
        GVA_DEBUG_GST   = 1 << (G_LOG_LEVEL_USER_SHIFT + 4)
} GvaDebugFlags;

gchar *         gva_choose_inpname              (const gchar *game);
gchar *         gva_find_data_file              (const gchar *basename);
GvaDebugFlags   gva_get_debug_flags             (void);
const gchar *   gva_get_last_version            (void);
gchar *         gva_get_monospace_font_name     (void);
void            gva_get_time_elapsed            (GTimeVal *start_time,
                                                 GTimeVal *time_elapsed);
const gchar *   gva_get_user_data_dir           (void);
void            gva_help_display                (GtkWindow *parent,
                                                 const gchar *link_id);
gchar *         gva_search_collate_key          (const gchar *string);
gboolean        gva_spawn_with_pipes            (const gchar *command_line,
                                                 GPid *child_pid,
                                                 gint *standard_input,
                                                 gint *standard_output,
                                                 gint *standard_error,
                                                 GError **error);

G_END_DECLS

#endif /* GVA_UTIL_H */
