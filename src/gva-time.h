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

#ifndef GVA_TIME_H
#define GVA_TIME_H

#include "gva-common.h"

/* Create a portable GType for time_t.
 *
 * IEEE 1003.1 does not specify the size of time_t, nor even whether it's
 * an integer or floating-point type.  So represent the value as a subtype
 * of G_TYPE_BOXED.
 */

#define GVA_TYPE_TIME   (gva_time_get_type ())

G_BEGIN_DECLS

GType           gva_time_get_type               (void);
void            gva_time_set_tree_sortable_func (GtkTreeSortable *sortable,
                                                 gint sort_column_id);

G_END_DECLS

#endif /* GVA_TIME_H */
