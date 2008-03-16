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
 * SECTION: gva-tree-view
 * @short_description: Main Tree View
 *
 * These functions manipulate the tree view widget in the main window.
 **/

#ifndef GVA_TREE_VIEW_H
#define GVA_TREE_VIEW_H

#include "gva-common.h"
#include "gva-game-store.h"

G_BEGIN_DECLS

void           gva_tree_view_init                    (void);
GtkTreePath *  gva_tree_view_lookup                  (const gchar *game);
gboolean       gva_tree_view_update                  (GError **error);
gboolean       gva_tree_view_run_query               (const gchar *expr,
                                                      GError **error);
GtkTreeModel * gva_tree_view_get_model               (void);
const gchar *  gva_tree_view_get_selected_game       (void);
void           gva_tree_view_set_selected_game       (const gchar *game);
gint           gva_tree_view_get_selected_view       (void);
void           gva_tree_view_set_selected_view       (gint view);
const gchar *  gva_tree_view_get_last_selected_game  (void);
void           gva_tree_view_set_last_selected_game  (const gchar *game);
void           gva_tree_view_get_last_sort_column_id (GvaGameStoreColumn *column_id,
                                                      GtkSortType *order);
void           gva_tree_view_set_last_sort_column_id (GvaGameStoreColumn column_id,
                                                      GtkSortType order);

/* Signal Handlers */

gboolean       gva_tree_view_button_press_event_cb   (GtkTreeView *view,
                                                      GdkEventButton *event);
gboolean       gva_tree_view_popup_menu_cb           (GtkTreeView *view);
gboolean       gva_tree_view_query_tooltip_cb        (GtkTreeView *view,
                                                      gint x,
                                                      gint y,
                                                      gboolean keyboard_mode,
                                                      GtkTooltip *tooltip);
void           gva_tree_view_row_activated_cb        (GtkTreeView *view,
                                                      GtkTreePath *path,
                                                      GtkTreeViewColumn *column);

G_END_DECLS

#endif /* GVA_TREE_VIEW_H */
