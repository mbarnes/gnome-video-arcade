/* Copyright 2007-2011 Matthew Barnes
 *
 * This file is part of GNOME Video Arcade.
 *
 * Based on totem-scrsaver.h by Bastien Nocera.
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
 * SECTION: gva-screen-saver
 * @short_description: Inhibit the screen saver
 *
 * #GvaScreenSaver inhibits the screen saver from starting while running a
 * game in full-screen mode.  It prefers the "org.gnome.ScreenSaver" D-Bus
 * interface, but can also make calls to Xlib directly.
 **/

#ifndef GVA_SCREEN_SAVER_H
#define GVA_SCREEN_SAVER_H

#include "gva-common.h"

#define GVA_TYPE_SCREEN_SAVER \
        (gva_screen_saver_get_type ())
#define GVA_SCREEN_SAVER(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), GVA_TYPE_SCREEN_SAVER, GvaScreenSaver))
#define GVA_SCREEN_SAVER_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), GVA_TYPE_SCREEN_SAVER, GvaScreenSaverClass))
#define GVA_IS_SCREEN_SAVER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), GVA_TYPE_SCREEN_SAVER))
#define GVA_IS_SCREEN_SAVER_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), GVA_TYPE_SCREEN_SAVER))
#define GVA_SCREEN_SAVER_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), GVA_TYPE_SCREEN_SAVER, GvaScreenSaverClass))

G_BEGIN_DECLS

typedef struct _GvaScreenSaver GvaScreenSaver;
typedef struct _GvaScreenSaverClass GvaScreenSaverClass;
typedef struct _GvaScreenSaverPrivate GvaScreenSaverPrivate;

/**
 * GvaScreenSaver:
 *
 * Contains only private data that should be read and manipulated using the
 * functions below.
 **/
struct _GvaScreenSaver
{
        GObject parent;
        GvaScreenSaverPrivate *priv;
};

struct _GvaScreenSaverClass
{
        GObjectClass parent_class;
};

GType           gva_screen_saver_get_type       (void) G_GNUC_CONST;
GvaScreenSaver *gva_screen_saver_new            (void);
void            gva_screen_saver_enable         (GvaScreenSaver *screen_saver);
void            gva_screen_saver_disable        (GvaScreenSaver *screen_saver);
const gchar *   gva_screen_saver_get_reason     (GvaScreenSaver *screen_saver);
void            gva_screen_saver_set_reason     (GvaScreenSaver *screen_saver,
                                                 const gchar *reason);

G_END_DECLS

#endif /* GVA_SCREEN_SAVER_H */
