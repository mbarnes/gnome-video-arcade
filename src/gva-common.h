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

#ifndef GVA_COMMON_H
#define GVA_COMMON_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include <sqlite3.h>
#include <gconf/gconf-client.h>

#define GVA_GCONF_PREFIX                "/apps/" PACKAGE
#define GVA_GCONF_ALL_COLUMNS_KEY       GVA_GCONF_PREFIX "/all-columns"
#define GVA_GCONF_AUTO_SAVE_KEY         GVA_GCONF_PREFIX "/auto-save"
#define GVA_GCONF_COLUMNS_KEY           GVA_GCONF_PREFIX "/columns"
#define GVA_GCONF_FAVORITES_KEY         GVA_GCONF_PREFIX "/favorites"
#define GVA_GCONF_FULL_SCREEN_KEY       GVA_GCONF_PREFIX "/full-screen"
#define GVA_GCONF_SELECTED_GAME_KEY     GVA_GCONF_PREFIX "/selected-game"
#define GVA_GCONF_SELECTED_VIEW_KEY     GVA_GCONF_PREFIX "/selected-view"
#define GVA_GCONF_SORT_COLUMN_KEY       GVA_GCONF_PREFIX "/sort-column"
#define GVA_GCONF_VERSION_KEY           GVA_GCONF_PREFIX "/version"

G_BEGIN_DECLS

/* Command Line Options */
extern gboolean opt_build_database;
extern gboolean opt_which_emulator;

G_END_DECLS

#endif /* GVA_COMMON_H */
