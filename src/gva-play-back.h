/* Copyright 2007-2015 Matthew Barnes
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
 * SECTION: gva-play-back
 * @short_description: Recorded Games Window
 *
 * These functions manipulate the Recorded Games window.
 **/

#ifndef GVA_PLAY_BACK_H
#define GVA_PLAY_BACK_H

#include "gva-common.h"

G_BEGIN_DECLS

void            gva_play_back_init              (void);
void            gva_play_back_show              (const gchar *inpname);

/* Signal Handlers */

void            gva_play_back_clicked_cb        (GtkButton *button);
void            gva_play_back_close_clicked_cb  (GtkWindow *window,
                                                 GtkButton *button);
void            gva_play_back_delete_clicked_cb (GtkTreeView *view,
                                                 GtkButton *button);
void            gva_play_back_row_activated_cb  (GtkTreeView *view,
                                                 GtkTreePath *path,
                                                 GtkTreeViewColumn *column);
void            gva_play_back_window_hide_cb    (GtkWindow *window);

G_END_DECLS

#endif /* GVA_PLAY_BACK_H */
