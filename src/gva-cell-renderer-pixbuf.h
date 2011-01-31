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
 * SECTION: gva-cell-renderer-pixbuf
 * @short_description: A clickable #GtkCellRendererPixbuf
 *
 * This is a clickable #GtkCellRendererPixbuf subclass.  It implements the
 * @activate method to emit a #GtkCellRendererPixbuf::clicked signal when
 * the cell is activated.
 **/

#ifndef GVA_CELL_RENDERER_PIXBUF_H
#define GVA_CELL_RENDERER_PIXBUF_H

#include "gva-common.h"

/* Standard GObject macros */
#define GVA_TYPE_CELL_RENDERER_PIXBUF \
        (gva_cell_renderer_pixbuf_get_type ())
#define GVA_CELL_RENDERER_PIXBUF(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), GVA_TYPE_CELL_RENDERER_PIXBUF, GvaCellRendererPixbuf))
#define GVA_CELL_RENDERER_PIXBUF_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), GVA_TYPE_CELL_RENDERER_PIXBUF, GvaCellRendererPixbufClass))
#define GVA_IS_CELL_RENDERER_PIXBUF(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), GVA_TYPE_CELL_RENDERER_PIXBUF))
#define GVA_IS_CELL_RENDERER_PIXBUF_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), GVA_TYPE_CELL_RENDERER_PIXBUF))
#define GVA_CELL_RENDERER_PIXBUF_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), GVA_TYPE_CELL_RENDERER_PIXBUF, GvaCellRendererPixbufClass))

G_BEGIN_DECLS

typedef struct _GvaCellRendererPixbuf GvaCellRendererPixbuf;
typedef struct _GvaCellRendererPixbufClass GvaCellRendererPixbufClass;

/**
 * GvaCellRendererPixbuf:
 *
 * Contains only private data that should be read and manipulated using the
 * functions below.
 **/
struct _GvaCellRendererPixbuf
{
        GtkCellRendererPixbuf parent;
};

struct _GvaCellRendererPixbufClass
{
        GtkCellRendererPixbufClass parent_class;

        void (*clicked) (GvaCellRendererPixbuf *cell_renderer_pixbuf,
                         GtkTreePath *tree_path);
};

GType             gva_cell_renderer_pixbuf_get_type   (void);
GtkCellRenderer * gva_cell_renderer_pixbuf_new        (void);

G_END_DECLS

#endif /* GVA_CELL_RENDERER_PIXBUF_H */
