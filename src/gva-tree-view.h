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

#ifndef GVA_TREE_VIEW_H
#define GVA_TREE_VIEW_H

#include "gva-common.h"

G_BEGIN_DECLS

void            gva_tree_view_init                   (void);
void            gva_tree_view_update                 (void);
const gchar *   gva_tree_view_get_selected_game      (void);
void            gva_tree_view_set_selected_game      (const gchar *romname);
gint            gva_tree_view_get_selected_view      (void);
void            gva_tree_view_set_selected_view      (gint view);
const gchar *   gva_tree_view_get_last_selected_game (void);
void            gva_tree_view_set_last_selected_game (const gchar *romname);
gint            gva_tree_view_get_last_selected_view (void);
void            gva_tree_view_set_last_selected_view (gint view);

G_END_DECLS

#endif /* GVA_TREE_VIEW_H */
