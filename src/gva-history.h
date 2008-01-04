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
 * SECTION: gva-history
 * @short_description: Arcade History Management
 *
 * These functions manage the arcade history information shown in the
 * Properties window.
 **/

#ifndef GVA_HISTORY_H
#define GVA_HISTORY_H

#include "gva-common.h"

G_BEGIN_DECLS

gboolean        gva_history_init                (GError **error);
gchar *         gva_history_lookup              (const gchar *game,
                                                 GError **error);

G_END_DECLS

#endif /* GVA_HISTORY_H */
