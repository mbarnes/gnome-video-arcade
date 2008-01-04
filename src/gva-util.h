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

typedef enum _GvaDebugFlags GvaDebugFlags;

/**
 * GvaDebugFlags:
 * @GVA_DEBUG_NONE:
 *      Print no messages.
 * @GVA_ERROR_MAME:
 *      Print shell commands invoking MAME.
 * @GVA_DEBUG_SQL:
 *      Print SQL commands to the game database.
 *
 * These flags indicate which types of debugging messages will be triggered
 * at runtime. Debugging messages can be triggered by setting the GVA_DEBUG
 * environment variable to a colon-separated list of "mame" and "sql".
 **/
enum _GvaDebugFlags
{
        GVA_DEBUG_NONE  = 0,
        GVA_DEBUG_MAME  = 1 << 0,
        GVA_DEBUG_SQL   = 1 << 1
};

gchar *         gva_choose_inpname              (const gchar *game);
gchar *         gva_find_data_file              (const gchar *basename);
GvaDebugFlags   gva_get_debug_flags             (void);
const gchar *   gva_get_last_version            (void);
gchar *         gva_get_monospace_font_name     (void);
void            gva_get_time_elapsed            (GTimeVal *start_time,
                                                 GTimeVal *time_elapsed);
const gchar *   gva_get_user_data_dir           (void);

G_END_DECLS

#endif /* GVA_UTIL_H */
