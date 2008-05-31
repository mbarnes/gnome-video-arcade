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
 * SECTION: gva-main
 * @short_description: Main Window
 *
 * These functions manipulate the main window, excluding the tree view.
 * The tree view functions are documented separately in the
 * <link linkend="gnome-video-arcade-gva-tree-view">gva-tree-view</link>
 * section.
 **/

#ifndef GVA_MAIN_H
#define GVA_MAIN_H

#include "gva-common.h"
#include "gva-process.h"

G_BEGIN_DECLS

void          gva_main_init                      (void);
gboolean      gva_main_build_database            (GError **error);
gboolean      gva_main_analyze_roms              (GError **error);
gboolean      gva_main_init_search_completion    (GError **error);
void          gva_main_connect_proxy_cb          (GtkUIManager *manager,
                                                  GtkAction *action,
                                                  GtkWidget *proxy);
void          gva_main_cursor_busy               (void);
void          gva_main_cursor_normal             (void);
void          gva_main_progress_bar_show         (void);
void          gva_main_progress_bar_hide         (void);
void          gva_main_progress_bar_set_fraction (gdouble fraction);
guint         gva_main_statusbar_get_context_id  (const gchar *context_description);
guint         gva_main_statusbar_push            (guint context_id,
                                                  const gchar *format,
                                                  ...);
void          gva_main_statusbar_pop             (guint context_id);
void          gva_main_statusbar_remove          (guint context_id,
                                                  guint message_id);
void          gva_main_execute_search            (void);
gchar *       gva_main_get_last_search_text      (void);
void          gva_main_set_last_search_text      (const gchar *text);
gboolean      gva_main_get_last_selected_match   (gchar **column_name,
                                                  gchar **search_text);
void          gva_main_set_last_selected_match   (const gchar *column_name,
                                                  const gchar *search_text);

/* Signal Handlers */

void          gva_main_search_entry_activate_cb  (GtkEntry *entry);
void          gva_main_search_entry_notify_cb    (GtkEntry *entry,
                                                  GParamSpec *pspec);
gboolean      gva_main_search_query_tooltip_cb   (GtkWidget *widget,
                                                  gint x,
                                                  gint y,
                                                  gboolean keyboard_mode,
                                                  GtkTooltip *tooltip);
void          gva_main_window_destroy_cb         (GtkWindow *window);

G_END_DECLS

#endif /* GVA_MAIN_H */
