/* Copyright 2007-2010 Matthew Barnes
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
 * SECTION: gva-mute-button
 * @short_description: Button for muting sound
 *
 * A #GvaMuteButton toggles between a muted icon and a full volume icon.
 **/

#ifndef GVA_MUTE_BUTTON_H
#define GVA_MUTE_BUTTON_H

#include "gva-common.h"

/* Standard GObject macros */
#define GVA_TYPE_MUTE_BUTTON \
        (gva_mute_button_get_type ())
#define GVA_MUTE_BUTTON(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), GVA_TYPE_MUTE_BUTTON, GvaMuteButton))
#define GVA_MUTE_BUTTON_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), GVA_TYPE_MUTE_BUTTON, GvaMuteButtonClass))
#define GVA_IS_MUTE_BUTTON(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), GVA_TYPE_MUTE_BUTTON))
#define GVA_IS_MUTE_BUTTON_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), GVA_TYPE_MUTE_BUTTON))
#define GVA_MUTE_BUTTON_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), GVA_TYPE_MUTE_BUTTON, GvaMuteButtonClass))

G_BEGIN_DECLS

typedef struct _GvaMuteButton GvaMuteButton;
typedef struct _GvaMuteButtonClass GvaMuteButtonClass;
typedef struct _GvaMuteButtonPrivate GvaMuteButtonPrivate;

/**
 * GvaMuteButton:
 *
 * Contains only private data that should be read and manipulated using the
 * functions below.
 **/
struct _GvaMuteButton
{
        GtkButton parent;
        GvaMuteButtonPrivate *priv;
};

struct _GvaMuteButtonClass
{
        GtkButtonClass parent_class;
};

GType           gva_mute_button_get_type        (void);
GtkWidget *     gva_mute_button_new             (void);
gboolean        gva_mute_button_get_muted       (GvaMuteButton *mute_button);
void            gva_mute_button_set_muted       (GvaMuteButton *mute_button,
                                                 gboolean muted);

G_END_DECLS

#endif /* GVA_MUTE_BUTTON_H */
