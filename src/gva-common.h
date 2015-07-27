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
 * SECTION: gva-common
 * @short_description: Common Definitions
 *
 * Common symbols used throughout GNOME Video Arcade.
 **/

#ifndef GVA_COMMON_H
#define GVA_COMMON_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include <sqlite3.h>
#include <libsoup/soup.h>

#define GVA_SETTING_ALL_COLUMNS                 "all-columns"
#define GVA_SETTING_AUTO_SAVE                   "auto-save"
#define GVA_SETTING_COLUMNS                     "columns"
#define GVA_SETTING_FAVORITES                   "favorites"
#define GVA_SETTING_FULL_SCREEN                 "full-screen"
#define GVA_SETTING_ERROR_FILE                  "error-file"
#define GVA_SETTING_PROPERTIES_PAGE             "properties-page"
#define GVA_SETTING_PROPERTIES_PREFIX           "properties"
#define GVA_SETTING_SEARCH                      "search"
#define GVA_SETTING_SELECTED_GAME               "selected-game"
#define GVA_SETTING_SELECTED_MATCH              "selected-match"
#define GVA_SETTING_SELECTED_VIEW               "selected-view"
#define GVA_SETTING_SHOW_CLONES                 "show-clones"
#define GVA_SETTING_SORT_COLUMN                 "sort-column"
#define GVA_SETTING_SOUND_MUTED                 "sound-muted"
#define GVA_SETTING_VERSION                     "version"
#define GVA_SETTING_WINDOW_PREFIX               "window"

G_BEGIN_DECLS

/* Command Line Options */
extern gboolean opt_build_database;
extern gchar *opt_inspect;
extern gboolean opt_version;
extern gboolean opt_which_emulator;

G_END_DECLS

#endif /* GVA_COMMON_H */
