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
 * SECTION: gva-search
 * @short_description: Search Window
 *
 * These functions manipulate the Search window.
 **/

#ifndef GVA_SEARCH_H
#define GVA_SEARCH_H

#include "gva-common.h"

G_BEGIN_DECLS

void            gva_search_init                 (void);
gchar *         gva_search_get_last_search      (void);
void            gva_search_set_last_search      (const gchar *text);

/* Signal Handlers */

void            gva_search_find_clicked_cb      (GtkEntry *entry,
                                                 GtkButton *button);

G_END_DECLS

#endif /* GVA_SEARCH_H */

