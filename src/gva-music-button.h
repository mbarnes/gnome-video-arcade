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
 * SECTION: gva-music-button
 * @short_description: Button for streaming music
 *
 * A #GvaMusicButton toggles between playing and pausing in-game music
 * clips streamed from http://www.arcade-history.com/.
 *
 * This requires arcade history information from a 'history.dat' file.
 **/

#ifndef GVA_MUSIC_BUTTON_H
#define GVA_MUSIC_BUTTON_H

#include "gva-common.h"

/* Standard GObject macros */
#define GVA_TYPE_MUSIC_BUTTON \
        (gva_music_button_get_type ())
#define GVA_MUSIC_BUTTON(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), GVA_TYPE_MUSIC_BUTTON, GvaMusicButton))
#define GVA_MUSIC_BUTTON_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), GVA_TYPE_MUSIC_BUTTON, GvaMusicButtonClass))
#define GVA_IS_MUSIC_BUTTON(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), GVA_TYPE_MUSIC_BUTTON))
#define GVA_IS_MUSIC_BUTTON_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), GVA_TYPE_MUSIC_BUTTON))
#define GVA_MUSIC_BUTTON_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), GVA_TYPE_MUSIC_BUTTON, GvaMusicButtonClass))

G_BEGIN_DECLS

typedef struct _GvaMusicButton GvaMusicButton;
typedef struct _GvaMusicButtonClass GvaMusicButtonClass;
typedef struct _GvaMusicButtonPrivate GvaMusicButtonPrivate;

/**
 * GvaMusicButton:
 *
 * Contains only private data that should be read and manipulated using the
 * functions below.
 **/
struct _GvaMusicButton
{
        GtkButton parent;
        GvaMusicButtonPrivate *priv;
};

struct _GvaMusicButtonClass
{
        GtkButtonClass parent_class;

        /* Signals */
        void            (*play)                 (GvaMusicButton *music_button);
        void            (*pause)                (GvaMusicButton *music_button);
};

GType           gva_music_button_get_type       (void);
GtkWidget *     gva_music_button_new            (void);
void            gva_music_button_play           (GvaMusicButton *music_button);
void            gva_music_button_pause          (GvaMusicButton *music_button);
const gchar *   gva_music_button_get_game       (GvaMusicButton *music_button);
void            gva_music_button_set_game       (GvaMusicButton *music_button,
                                                 const gchar *game);
const gchar *   gva_music_button_get_status     (GvaMusicButton *music_button);

G_END_DECLS

#endif /* GVA_MUSIC_BUTTON_H */
