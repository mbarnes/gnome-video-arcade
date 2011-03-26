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

#include "gva-cell-renderer-pixbuf.h"

#define GVA_CELL_RENDERER_PIXBUF_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE \
        ((obj), GVA_TYPE_CELL_RENDERER_PIXBUF, GvaCellRendererPixbufPrivate))

struct _GvaCellRendererPixbufPrivate
{
        gboolean active;
};

enum
{
        PROP_0,
        PROP_ACTIVE
};

enum
{
        CLICKED,
        LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (
        GvaCellRendererPixbuf,
        gva_cell_renderer_pixbuf,
        GTK_TYPE_CELL_RENDERER_PIXBUF)

static void
cell_renderer_pixbuf_set_property (GObject *object,
                                   guint property_id,
                                   const GValue *value,
                                   GParamSpec *pspec)
{
        switch (property_id)
        {
                case PROP_ACTIVE:
                        gva_cell_renderer_pixbuf_set_active (
                                GVA_CELL_RENDERER_PIXBUF (object),
                                g_value_get_boolean (value));
                        return;
        }

        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
cell_renderer_pixbuf_get_property (GObject *object,
                                   guint property_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
        switch (property_id)
        {
                case PROP_ACTIVE:
                        g_value_set_boolean (
                                value, gva_cell_renderer_pixbuf_get_active (
                                GVA_CELL_RENDERER_PIXBUF (object)));
                        return;
        }

        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static gboolean
cell_renderer_pixbuf_activate (GtkCellRenderer *cell,
                               GdkEvent *event,
                               GtkWidget *widget,
                               const gchar *path,
                               const GdkRectangle *background_area,
                               const GdkRectangle *cell_area,
                               GtkCellRendererState flags)
{
        GtkTreePath *tree_path;

        tree_path = gtk_tree_path_new_from_string (path);
        g_assert (tree_path != NULL);
        g_signal_emit (cell, signals[CLICKED], 0, tree_path);
        gtk_tree_path_free (tree_path);

        return TRUE;
}

static void
cell_renderer_pixbuf_render (GtkCellRenderer *cell,
                             cairo_t *cr,
                             GtkWidget *widget,
                             const GdkRectangle *background_area,
                             const GdkRectangle *cell_area,
                             GtkCellRendererState flags)
{
        GvaCellRendererPixbufPrivate *priv;
        gboolean sensitive;

        priv = GVA_CELL_RENDERER_PIXBUF_GET_PRIVATE (cell);

        sensitive = gtk_cell_renderer_get_sensitive (cell);
        gtk_cell_renderer_set_sensitive (cell, priv->active);

        /* Chain up to parent's render() method. */
        GTK_CELL_RENDERER_CLASS (gva_cell_renderer_pixbuf_parent_class)->
                render (cell, cr, widget, background_area, cell_area, flags);

        gtk_cell_renderer_set_sensitive (cell, sensitive);
}

static void
gva_cell_renderer_pixbuf_class_init (GvaCellRendererPixbufClass *class)
{
        GObjectClass *object_class;
        GtkCellRendererClass *cell_renderer_class;

        g_type_class_add_private (class, sizeof (GvaCellRendererPixbufPrivate));

        object_class = G_OBJECT_CLASS (class);
        object_class->set_property = cell_renderer_pixbuf_set_property;
        object_class->get_property = cell_renderer_pixbuf_get_property;

        cell_renderer_class = GTK_CELL_RENDERER_CLASS (class);
        cell_renderer_class->activate = cell_renderer_pixbuf_activate;
        cell_renderer_class->render = cell_renderer_pixbuf_render;

        /**
         * GvaCellRendererPixbuf:active
         *
         * Whether to draw the pixbuf as active.
         **/
        g_object_class_install_property (
                object_class,
                PROP_ACTIVE,
                g_param_spec_boolean (
                        "active",
                        NULL,
                        NULL,
                        FALSE,
                        G_PARAM_READWRITE));

        /**
         * GvaCellRendererPixbuf::clicked:
         * @renderer: the #GvaCellRendererPixbuf that received the signal
         * @tree_path: the #GtkTreePath to the row containing the activated
         * pixbuf
         *
         * The ::clicked signal is emitted when the cell is activated.
         **/
        signals[CLICKED] = g_signal_new (
                "clicked",
                G_OBJECT_CLASS_TYPE (class),
                G_SIGNAL_RUN_LAST,
                G_STRUCT_OFFSET (GvaCellRendererPixbufClass, clicked),
                NULL, NULL,
                g_cclosure_marshal_VOID__BOXED,
                G_TYPE_NONE, 1,
                GTK_TYPE_TREE_PATH);
}

static void
gva_cell_renderer_pixbuf_init (GvaCellRendererPixbuf *cell)
{
        cell->priv = GVA_CELL_RENDERER_PIXBUF_GET_PRIVATE (cell);

        g_object_set (cell, "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE, NULL);
}

/**
 * gva_cell_renderer_pixbuf_new:
 *
 * Creates a new #GvaCellRendererPixbuf object.
 *
 * Returns: a new #GvaCellRendererPixbuf
 **/
GtkCellRenderer *
gva_cell_renderer_pixbuf_new (void)
{
        return g_object_new (GVA_TYPE_CELL_RENDERER_PIXBUF, NULL);
}

/**
 * gva_cell_renderer_pixbuf_get_active:
 * @cell: a #GvaCellRendererPixbuf
 *
 * Returns whether @cell should be drawn as active.
 *
 * Returns: whether to draw the pixbuf as active
 **/
gboolean
gva_cell_renderer_pixbuf_get_active (GvaCellRendererPixbuf *cell)
{
        g_return_val_if_fail (GVA_IS_CELL_RENDERER_PIXBUF (cell), FALSE);

        return cell->priv->active;
}

/**
 * gva_cell_renderer_pixbuf_set_active:
 * @cell: a #GvaCellRendererPixbuf
 * @active: whether to draw the pixbuf as active
 *
 * Sets whether @cell should be drawn as active.
 **/
void
gva_cell_renderer_pixbuf_set_active (GvaCellRendererPixbuf *cell,
                                     gboolean active)
{
        g_return_if_fail (GVA_IS_CELL_RENDERER_PIXBUF (cell));

        cell->priv->active = active;

        g_object_notify (G_OBJECT (cell), "active");
}
