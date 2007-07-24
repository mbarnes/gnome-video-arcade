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

#ifndef GVA_COLUMNS_H
#define GVA_COLUMNS_H

#include "gva-common.h"
#include "gva-game-store.h"

G_BEGIN_DECLS

GtkTreeViewColumn *  gva_columns_new_from_id   (GvaGameStoreColumn column_id);
GtkTreeViewColumn *  gva_columns_new_from_name (const gchar *column_name);
gboolean             gva_columns_lookup_id     (const gchar *column_name,
                                                GvaGameStoreColumn *column_id);
const gchar *        gva_columns_lookup_name   (GvaGameStoreColumn column_id);
void                 gva_columns_load          (GtkTreeView *view);
void                 gva_columns_save          (GtkTreeView *view);
GSList *             gva_columns_get_names     (GtkTreeView *view,
                                                gboolean visible_only);

G_END_DECLS

#endif /* GVA_COLUMNS_H */
