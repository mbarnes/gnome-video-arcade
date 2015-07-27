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
 * SECTION: gva-mame-common
 * @short_description: Functions common to all MAME backends
 *
 * These functions are common to all supported MAME implementations.
 **/

#ifndef GVA_MAME_COMMON_H
#define GVA_MAME_COMMON_H

#include "gva-common.h"
#include "gva-mame.h"

G_BEGIN_DECLS

gint               gva_mame_command             (const gchar *arguments,
                                                 gchar ***stdout_lines,
                                                 gchar ***stderr_lines,
                                                 GError **error);

G_END_DECLS

#endif /* GVA_MAME_COMMON_H */
