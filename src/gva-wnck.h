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
 * SECTION: gva-wnck
 * @short_description: Game Window Management
 *
 * XXX Long description here.
 **/

#ifndef GVA_WNCK_H
#define GVA_WNCK_H

#include "gva-common.h"
#include "gva-process.h"

void            gva_wnck_listen_for_new_window  (GvaProcess *process,
                                                 const gchar *game);

#endif /* GVA_WNCK_H */
