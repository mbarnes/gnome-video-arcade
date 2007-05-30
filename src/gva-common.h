#ifndef GVA_COMMON_H
#define GVA_COMMON_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include <gconf/gconf-client.h>

#define GVA_GCONF_PREFIX                "/apps/" PACKAGE
#define GVA_GCONF_AUTO_SAVE_KEY         GVA_GCONF_PREFIX "/auto-save"
#define GVA_GCONF_FAVORITES_KEY         GVA_GCONF_PREFIX "/favorites"
#define GVA_GCONF_FULL_SCREEN_KEY       GVA_GCONF_PREFIX "/full-screen"
#define GVA_GCONF_SELECTED_GAME_KEY     GVA_GCONF_PREFIX "/selected-game"
#define GVA_GCONF_SELECTED_VIEW_KEY     GVA_GCONF_PREFIX "/selected-view"

#endif /* GVA_COMMON_H */
