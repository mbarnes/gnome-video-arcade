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

#include "gva-mute-button.h"

#define GVA_MUTE_BUTTON_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE \
        ((obj), GVA_TYPE_MUTE_BUTTON, GvaMuteButtonPrivate))

struct _GvaMuteButtonPrivate
{
        GtkWidget *image;
        gboolean muted;
};

enum
{
        PROP_0,
        PROP_MUTED
};

G_DEFINE_TYPE (
        GvaMuteButton,
        gva_mute_button,
        GTK_TYPE_BUTTON)

static void
mute_button_set_property (GObject *object,
                          guint property_id,
                          const GValue *value,
                          GParamSpec *pspec)
{
        switch (property_id)
        {
                case PROP_MUTED:
                        gva_mute_button_set_muted (
                                GVA_MUTE_BUTTON (object),
                                g_value_get_boolean (value));
                        return;
        }

        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
mute_button_get_property (GObject *object,
                          guint property_id,
                          GValue *value,
                          GParamSpec *pspec)
{
        switch (property_id)
        {
                case PROP_MUTED:
                        g_value_set_boolean (
                                value, gva_mute_button_get_muted (
                                GVA_MUTE_BUTTON (object)));
                        return;
        }

        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
mute_button_dispose (GObject *object)
{
        GvaMuteButtonPrivate *priv;

        priv = GVA_MUTE_BUTTON_GET_PRIVATE (object);

        if (priv->image != NULL)
        {
                g_object_unref (priv->image);
                priv->image = NULL;
        }

        /* Chain up to parent's dispose() method. */
        G_OBJECT_CLASS (gva_mute_button_parent_class)->dispose (object);
}

static void
mute_button_clicked (GtkButton *button)
{
        GvaMuteButton *mute_button = GVA_MUTE_BUTTON (button);
        gboolean muted;

        muted = gva_mute_button_get_muted (mute_button);
        gva_mute_button_set_muted (mute_button, !muted);
}

static void
gva_mute_button_class_init (GvaMuteButtonClass *class)
{
        GObjectClass *object_class;
        GtkButtonClass *button_class;

        g_type_class_add_private (class, sizeof (GvaMuteButtonPrivate));

        object_class = G_OBJECT_CLASS (class);
        object_class->set_property = mute_button_set_property;
        object_class->get_property = mute_button_get_property;
        object_class->dispose = mute_button_dispose;

        button_class = GTK_BUTTON_CLASS (class);
        button_class->clicked = mute_button_clicked;

        g_object_class_install_property (
                object_class,
                PROP_MUTED,
                g_param_spec_boolean (
                        "muted",
                        _("Muted"),
                        _("Whether the button state is muted"),
                        FALSE,
                        G_PARAM_READWRITE |
                        G_PARAM_CONSTRUCT |
                        G_PARAM_STATIC_STRINGS));
}

static void
gva_mute_button_init (GvaMuteButton *mute_button)
{
        GtkWidget *widget;

        mute_button->priv = GVA_MUTE_BUTTON_GET_PRIVATE (mute_button);

        gtk_widget_set_can_focus (GTK_WIDGET (mute_button), FALSE);
        gtk_button_set_relief (GTK_BUTTON (mute_button), GTK_RELIEF_NONE);

        widget = gtk_image_new ();
        gtk_container_add (GTK_CONTAINER (mute_button), widget);
        mute_button->priv->image = g_object_ref (widget);
        gtk_widget_show (widget);
}

/**
 * gva_mute_button_new:
 *
 * Creates a new #GvaMuteButton.
 *
 * Returns: a new #GvaMuteButton
 **/
GtkWidget *
gva_mute_button_new (void)
{
        return g_object_new (GVA_TYPE_MUTE_BUTTON, NULL);
}

/**
 * gva_mute_button_get_muted:
 * @mute_button: a #GvaMuteButton
 *
 * Returns the muted state of @mute_button.
 *
 * Returns: the muted state of @mute_button
 **/
gboolean
gva_mute_button_get_muted (GvaMuteButton *mute_button)
{
        g_return_val_if_fail (GVA_IS_MUTE_BUTTON (mute_button), FALSE);

        return mute_button->priv->muted;
}

/**
 * gva_mute_button_set_muted:
 * @mute_button: a #GvaMuteButton
 * @muted: muted state
 *
 * Sets the muted state of @mute_button to @muted.  The button's icon
 * and tooltip will be updated appropriately.
 **/
void
gva_mute_button_set_muted (GvaMuteButton *mute_button,
                           gboolean muted)
{
        GtkImage *image;
        const gchar *icon_name;
        const gchar *tooltip;

        g_return_if_fail (GVA_IS_MUTE_BUTTON (mute_button));

        image = GTK_IMAGE (mute_button->priv->image);

        if (muted)
        {
                icon_name = "audio-volume-muted";
                tooltip = _("In-game sound is muted");
        }
        else
        {
                icon_name = "audio-volume-high";
                tooltip = _("In-game sound is enabled");
        }

        mute_button->priv->muted = muted;
        gtk_image_set_from_icon_name (image, icon_name, GTK_ICON_SIZE_BUTTON);
        gtk_widget_set_tooltip_text (GTK_WIDGET (mute_button), tooltip);

        g_object_notify (G_OBJECT (mute_button), "muted");
}
