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

#ifndef GVA_FAVORITES_H
#define GVA_FAVORITES_H

#include "gva-common.h"

G_BEGIN_DECLS

GSList *        gva_favorites_copy              (void);
void            gva_favorites_insert            (const gchar *romname);
void            gva_favorites_remove            (const gchar *romname);
gboolean        gva_favorites_contains          (const gchar *romname);

G_END_DECLS

#endif /* GVA_FAVORITES_H */
