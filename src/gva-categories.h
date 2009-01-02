/* Copyright 2007-2009 Matthew Barnes
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
 * SECTION: gva-categories
 * @short_description: Category and MAME Version Information
 *
 * These functions provide a game's category and its initial MAME version.
 * The information is read from a <filename>catver.ini</filename> file.
 **/

#ifndef GVA_CATEGORIES_H
#define GVA_CATEGORIES_H

#include "gva-common.h"

G_BEGIN_DECLS

gboolean        gva_categories_init             (GError **error);
gchar *         gva_categories_lookup           (const gchar *game,
                                                 GError **error);
gchar *         gva_mame_version_lookup         (const gchar *game,
                                                 GError **error);

G_END_DECLS

#endif /* GVA_CATEGORIES_H */
