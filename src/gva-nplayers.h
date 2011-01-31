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
 * SECTION: gva-nplayers
 * @short_description: Number of Players Information
 *
 * These functions provide a the maximum number of players for a game
 * and whether the play is alternating or simultaneous.  The information
 * is read from a <filename>nplayers.ini</filename> file.
 **/

#ifndef GVA_NPLAYERS_H
#define GVA_NPLAYERS_H

#include "gva-common.h"

G_BEGIN_DECLS

gboolean        gva_nplayers_init               (GError **error);
gboolean        gva_nplayers_lookup             (const gchar *game,
                                                 gint *max_alternating,
                                                 gint *max_simultaneous,
                                                 GError **error);
const gchar *   gva_nplayers_describe           (gint max_alternating,
                                                 gint max_simultaneous);

G_END_DECLS

#endif /* GVA_NPLAYERS_H */
