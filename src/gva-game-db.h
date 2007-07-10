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

#ifndef GVA_GAME_DB_H
#define GVA_GAME_DB_H

#include "gva-common.h"
#include "gva-process.h"

G_BEGIN_DECLS

gboolean        gva_game_db_init                (GError **error);
GtkTreePath *   gva_game_db_lookup              (const gchar *romname);
GtkTreeModel *  gva_game_db_get_model           (void);
GvaProcess *    gva_game_db_update_samples      (GError **error);
GvaProcess *    gva_game_db_update_titles       (GError **error);
gchar *         gva_game_db_get_history         (const gchar *romname,
                                                 GError **error);

G_END_DECLS

#endif /* GVA_GAME_DB_H */
