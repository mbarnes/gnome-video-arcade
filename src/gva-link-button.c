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

#include "gva-link-button.h"

static gpointer parent_class = NULL;

static void
link_button_size_request (GtkWidget *widget,
                          GtkRequisition *requisition)
{
        GtkWidget *child;

        child = gtk_bin_get_child (GTK_BIN (widget));
        if (child != NULL && GTK_WIDGET_VISIBLE (child))
                gtk_widget_size_request (child, requisition);
}

static void
link_button_size_allocate (GtkWidget *widget,
                           GtkAllocation *allocation)
{
        GtkWidget *child;
        GdkWindow *window;

        widget->allocation = *allocation;

        /* XXX GTK+ has no accessor function for this. */
        window = GTK_BUTTON (widget)->event_window;

        if (GTK_WIDGET_REALIZED (widget))
                gdk_window_move_resize (
                        window,
                        widget->allocation.x,
                        widget->allocation.y,
                        widget->allocation.width,
                        widget->allocation.height);

        child = gtk_bin_get_child (GTK_BIN (widget));
        gtk_widget_size_allocate (child, allocation);
}

static gboolean
link_button_expose_event (GtkWidget *widget,
                          GdkEventExpose *event)
{
        GtkWidgetClass *widget_class;

        /* Bypass GtkButton's expose_event() method. */
        widget_class = g_type_class_peek (GTK_TYPE_BIN);
        return widget_class->expose_event (widget, event);
}

static void
link_button_class_init (GvaLinkButtonClass *class)
{
        GtkWidgetClass *widget_class;

        parent_class = g_type_class_peek_parent (class);

        widget_class = GTK_WIDGET_CLASS (class);
        widget_class->size_request = link_button_size_request;
        widget_class->size_allocate = link_button_size_allocate;
        widget_class->expose_event = link_button_expose_event;
}

GType
gva_link_button_get_type (void)
{
        static GType type = 0;

        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info =
                {
                        sizeof (GvaLinkButtonClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) link_button_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (GvaLinkButton),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        GTK_TYPE_LINK_BUTTON, "GvaLinkButton", &type_info, 0);
        }

        return type;
}

/**
 * gva_link_button_new:
 * @label: the text of the button
 *
 * Creates a new #GvaLinkButton with @label as its text.
 *
 * Returns: a new #GtkLinkButton
 **/
GtkWidget *
gva_link_button_new (const gchar *label)
{
        return g_object_new (GVA_TYPE_LINK_BUTTON, "label", label, NULL);
}
