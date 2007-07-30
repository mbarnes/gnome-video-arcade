/* Copyright 2007 Matthew Barnes
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
 **/

#ifndef GVA_UTIL_H
#define GVA_UTIL_H

#include "gva-common.h"

G_BEGIN_DECLS

gchar *         gva_choose_inpname              (const gchar *game);
gchar *         gva_find_data_file              (const gchar *basename);
const gchar *   gva_get_last_version            (void);
gchar *         gva_get_monospace_font_name     (void);
void            gva_get_time_elapsed            (GTimeVal *start_time,
                                                 GTimeVal *time_elapsed);
const gchar *   gva_get_user_data_dir           (void);

G_END_DECLS

#endif /* GVA_UTIL_H */
