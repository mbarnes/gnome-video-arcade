/* Copyright 2007-2011 Matthew Barnes
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
 * SECTION: gva-mame
 * @short_description: MAME Communication Backend
 *
 * These functions define the interface for communication with MAME.
 **/

#ifndef GVA_MAME_H
#define GVA_MAME_H

#include "gva-common.h"
#include "gva-process.h"

G_BEGIN_DECLS

gint            gva_mame_command                (const gchar *arguments,
                                                 gchar ***stdout_lines,
                                                 gchar ***stderr_lines,
                                                 GError **error);
gchar *         gva_mame_get_version            (GError **error);
guint           gva_mame_get_version_int        (void);
guint           gva_mame_get_total_supported    (GError **error);
gchar *         gva_mame_get_config_value       (const gchar *config_key,
                                                 GError **error);
gboolean        gva_mame_has_config_value       (const gchar *config_key);
gchar **        gva_mame_get_search_paths       (const gchar *config_key,
                                                 GError **error);
GList *         gva_mame_get_input_files        (GError **error);
GvaProcess *    gva_mame_list_xml               (GError **error);
gchar *         gva_mame_verify_roms            (const gchar *name,
                                                 GError **error);
gchar *         gva_mame_verify_samples         (const gchar *name,
                                                 GError **error);
GvaProcess *    gva_mame_verify_all_roms        (GError **error);
GvaProcess *    gva_mame_verify_all_samples     (GError **error);
gboolean        gva_mame_verify_parse           (const gchar *line,
                                                 gchar **out_name,
                                                 gchar **out_status);
GvaProcess *    gva_mame_run_game               (const gchar *name,
                                                 GError **error);
GvaProcess *    gva_mame_record_game            (const gchar *name,
                                                 const gchar *inpname,
                                                 GError **error);
GvaProcess *    gva_mame_playback_game          (const gchar *name,
                                                 const gchar *inpname,
                                                 GError **error);
gchar *         gva_mame_get_save_state_file    (const gchar *name);
void            gva_mame_delete_save_state      (const gchar *name);
const gchar *   gva_mame_get_input_directory    (GError **error);
const gchar *   gva_mame_get_snapshot_directory (GError **error);
const gchar *   gva_mame_get_state_directory    (GError **error);

/* Test for supported options */

/**
 * gva_mame_supports_auto_save:
 *
 * Returns %TRUE if the MAME executable that
 * <emphasis>GNOME Video Arcade</emphasis> is configured to use
 * supports an "autosave" option.
 *
 * Returns: %TRUE if MAME supports the "autosave" option
 **/
#define gva_mame_supports_auto_save() \
        (gva_mame_has_config_value ("autosave"))

/**
 * gva_mame_supports_maximize:
 *
 * Returns %TRUE if the MAME executable that
 * <emphasis>GNOME Video Arcade</emphasis> is configured to use
 * supports a "maximize" option.
 *
 * Returns: %TRUE if MAME supports the "maximize" option
 **/
#define gva_mame_supports_maximize() \
        (gva_mame_has_config_value ("maximize"))

/**
 * gva_mame_supports_sound:
 *
 * Returns %TRUE if the MAME executable that
 * <emphasis>GNOME Video Arcade</emphasis> is configured to use
 * supports a "sound" option.
 *
 * Returns: %TRUE if MAME supports the "sound" option
 **/
#define gva_mame_supports_sound() \
        (gva_mame_has_config_value ("sound"))

/**
 * gva_mame_supports_window:
 *
 * Returns %TRUE if the MAME executable that
 * <emphasis>GNOME Video Arcade</emphasis> is configured to use
 * supports a "window" option.
 *
 * Returns: %TRUE if MAME supports the "window" option
 **/
#define gva_mame_supports_window() \
        (gva_mame_has_config_value ("window"))

G_END_DECLS

#endif /* GVA_MAME_H */
