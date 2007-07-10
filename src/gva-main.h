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

#ifndef GVA_MAIN_H
#define GVA_MAIN_H

#include "gva-common.h"

G_BEGIN_DECLS

void          gva_main_init                     (void);
void          gva_main_connect_proxy_cb         (GtkUIManager *manager,
                                                 GtkAction *action,
                                                 GtkWidget *proxy);
guint         gva_main_statusbar_get_context_id (const gchar *context);
guint         gva_main_statusbar_push           (guint context_id,
                                                 const gchar *format,
                                                 ...);
void          gva_main_statusbar_pop            (guint context_id);
void          gva_main_statusbar_remove         (guint context_id,
                                                 guint message_id);

G_END_DECLS

#endif /* GVA_MAIN_H */
