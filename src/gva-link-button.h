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
 * SECTION: gva-link-button
 * @short_description: Create buttons bound to a URL
 *
 * A #GvaLinkButton is a #GtkLinkButton without shadows or extra padding.
 **/

#ifndef GVA_LINK_BUTTON_H
#define GVA_LINK_BUTTON_H

#include "gva-common.h"

/* Standard GObject macros */
#define GVA_TYPE_LINK_BUTTON \
        (gva_link_button_get_type ())
#define GVA_LINK_BUTTON(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), GVA_TYPE_LINK_BUTTON, GvaLinkButton))
#define GVA_LINK_BUTTON_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), GVA_TYPE_LINK_BUTTON, GvaLinkButtonClass))
#define GVA_IS_LINK_BUTTON(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), GVA_TYPE_LINK_BUTTON))
#define GVA_IS_LINK_BUTTON_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), GVA_TYPE_LINK_BUTTON))
#define GVA_LINK_BUTTON_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), GVA_TYPE_LINK_BUTTON, GvaLinkButtonClass))

G_BEGIN_DECLS

typedef struct _GvaLinkButton GvaLinkButton;
typedef struct _GvaLinkButtonClass GvaLinkButtonClass;

/**
 * GvaLinkButton:
 *
 * Contains only private data that should be read and manipulated using the
 * functions below.
 **/
struct _GvaLinkButton
{
        GtkLinkButton parent;
};

struct _GvaLinkButtonClass
{
        GtkLinkButtonClass parent_class;
};

GType           gva_link_button_get_type        (void);
GtkWidget *     gva_link_button_new             (const gchar *text);

G_END_DECLS

#endif /* GVA_LINK_BUTTON_H */
