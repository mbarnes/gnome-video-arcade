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
 * SECTION: gva-time
 * @short_description: A #GBoxed type for time_t
 *
 * A portable #GType for <type>time_t</type>.
 *
 * IEEE 1003.1 does not specify the size of <type>time_t</type>, nor even
 * whether it's an integer or floating-point type, so we have to represent
 * the value as a subtype of #G_TYPE_BOXED.
 **/

#ifndef GVA_TIME_H
#define GVA_TIME_H

#include "gva-common.h"

/**
 * GVA_TYPE_TIME:
 *
 * The #GType for a boxed type holding a <type>time_t</type> value.
 **/
#define GVA_TYPE_TIME   (gva_time_get_type ())

G_BEGIN_DECLS

GType           gva_time_get_type               (void);

G_END_DECLS

#endif /* GVA_TIME_H */
