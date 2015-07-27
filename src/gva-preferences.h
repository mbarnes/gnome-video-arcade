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
 * SECTION: gva-preferences
 * @short_description: Preferences Window
 *
 * These functions manipulate the Preferences window.
 **/

#ifndef GVA_PREFERENCES_H
#define GVA_PREFERENCES_H

#include "gva-common.h"

G_BEGIN_DECLS

void           gva_preferences_init             (void);
gboolean       gva_preferences_get_auto_save    (void);
void           gva_preferences_set_auto_save    (gboolean auto_save);
gboolean       gva_preferences_get_full_screen  (void);
void           gva_preferences_set_full_screen  (gboolean full_screen);
gboolean       gva_preferences_get_show_clones  (void);
void           gva_preferences_set_show_clones  (gboolean show_clones);

/* Signal Handlers */

void           gva_preferences_close_clicked_cb (GtkWindow *window,
                                                 GtkButton *button);

G_END_DECLS

#endif /* GVA_PREFERENCES_H */
