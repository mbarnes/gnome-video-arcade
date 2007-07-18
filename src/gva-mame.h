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

#ifndef GVA_MAME_H
#define GVA_MAME_H

#include "gva-common.h"
#include "gva-process.h"

G_BEGIN_DECLS

typedef void    (*GvaMameCallback)              (const gchar *name,
                                                 const gchar *game_data,
                                                 gpointer user_data);

gint            gva_mame_command                (const gchar *arguments,
                                                 gchar ***stdout_lines,
                                                 gchar ***stderr_lines,
                                                 GError **error);

gchar *         gva_mame_get_version            (GError **error);
guint           gva_mame_get_total_supported    (GError **error);
gchar *         gva_mame_get_config_value       (const gchar *config_key,
                                                 GError **error);
gboolean        gva_mame_has_config_value       (const gchar *config_key);
GHashTable *    gva_mame_get_input_files        (GError **error);
GvaProcess *    gva_mame_list_xml               (GError **error);
GvaProcess *    gva_mame_verify_romsets         (GvaMameCallback callback,
                                                 gpointer user_data,
                                                 GError **error);
GvaProcess *    gva_mame_verify_samplesets      (GvaMameCallback callback,
                                                 gpointer user_data,
                                                 GError **error);
GvaProcess *    gva_mame_run_game               (const gchar *name,
                                                 GError **error);
GvaProcess *    gva_mame_record_game            (const gchar *name,
                                                 const gchar *inpname,
                                                 GError **error);
GvaProcess *    gva_mame_playback_game          (const gchar *name,
                                                 const gchar *inpname,
                                                 GError **error);
gboolean        gva_mame_clear_state            (const gchar *name,
                                                 GError **error);
gboolean        gva_mame_supports_auto_save     (void);
gboolean        gva_mame_supports_full_screen   (void);

G_END_DECLS

#endif /* GVA_MAME_H */
