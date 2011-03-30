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

#include "gva-music-button.h"

#include "gva-history.h"
#include "gva-util.h"

#define MUSIC_URI       "http://www.arcade-history.com/mp3/"

#define GVA_MUSIC_BUTTON_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE \
        ((obj), GVA_TYPE_MUSIC_BUTTON, GvaMusicButtonPrivate))

struct _GvaMusicButtonPrivate
{
#ifdef HAVE_GSTREAMER
        GstElement *element;
#endif

        const gchar *game;

        gchar *status;
        guint status_idle_id;

        guint seeking : 1;
};

enum
{
        PROP_0,
        PROP_GAME,
        PROP_STATUS
};

enum
{
        PAUSE,
        PLAY,
        LAST_SIGNAL
};

static gulong signals[LAST_SIGNAL];

G_DEFINE_TYPE (
        GvaMusicButton,
        gva_music_button,
        GTK_TYPE_BUTTON)

#ifdef HAVE_GSTREAMER

static gboolean
music_button_notify_status_cb (GvaMusicButton *music_button)
{
        music_button->priv->status_idle_id = 0;
        g_object_notify (G_OBJECT (music_button), "status");

        return FALSE;
}

static void
music_button_set_status (GvaMusicButton *music_button,
                         const gchar *status)
{
        g_free (music_button->priv->status);
        music_button->priv->status = g_strdup (status);

        /* Rate-limit status notifications. */
        if (music_button->priv->status_idle_id == 0)
                music_button->priv->status_idle_id = g_idle_add (
                        (GSourceFunc) music_button_notify_status_cb,
                        music_button);
}

static void
music_button_handle_eos (GvaMusicButton *music_button,
                         GstMessage *message)
{
        GstElement *element;
        GstSeekFlags flags;
        GstFormat format;

        element = music_button->priv->element;

        /* Repeat music clip. */
        format = GST_FORMAT_TIME;
        flags = GST_SEEK_FLAG_FLUSH;
        gst_element_seek_simple (element, format, flags, 0);

        music_button->priv->seeking = TRUE;
}

static void
music_button_handle_error (GvaMusicButton *music_button,
                           GstMessage *message)
{
        GstElement *element;
        const gchar *status = NULL;
        GError *error = NULL;

        element = music_button->priv->element;
        gst_element_set_state (element, GST_STATE_NULL);

        gst_message_parse_error (message, &error, NULL);

        if (error->domain == GST_RESOURCE_ERROR)
                status = _("No music available");

        if (status == NULL)
                status = _("Unable to play music");

        music_button_set_status (music_button, status);
        gtk_widget_set_sensitive (GTK_WIDGET (music_button), FALSE);

        g_log (
                G_LOG_DOMAIN, GVA_DEBUG_GST,
                "%s (code %d):",
                g_quark_to_string (error->domain),
                error->code);

        g_log (
                G_LOG_DOMAIN, GVA_DEBUG_GST,
                "%s", error->message);

        g_error_free (error);
}

static void
music_button_handle_buffering (GvaMusicButton *music_button,
                               GstMessage *message)
{
        gchar *status;
        gint percent;

        gst_message_parse_buffering (message, &percent);

        status = g_strdup_printf (_("Buffering %d%%..."), percent);
        music_button_set_status (music_button, status);
        g_free (status);
}

static void
music_button_handle_state_changed (GvaMusicButton *music_button,
                                   GstMessage *message)
{
        GtkWidget *image;
        GtkIconSize icon_size;
        GstState old_state;
        GstState new_state;
        GstState pending;
        gchar *stock_id;

        image = gtk_button_get_image (GTK_BUTTON (music_button));
        gtk_image_get_stock (GTK_IMAGE (image), &stock_id, &icon_size);

        gst_message_parse_state_changed (
                message, &old_state, &new_state, &pending);

        switch (new_state)
        {
                case GST_STATE_NULL:
                        stock_id = GTK_STOCK_MEDIA_PLAY;
                        break;

                case GST_STATE_READY:
                        stock_id = GTK_STOCK_MEDIA_PLAY;
                        break;

                case GST_STATE_PAUSED:
                        if (music_button->priv->seeking)
                                stock_id = GTK_STOCK_MEDIA_PAUSE;
                        else
                                stock_id = GTK_STOCK_MEDIA_PLAY;
                        break;

                case GST_STATE_PLAYING:
                        stock_id = GTK_STOCK_MEDIA_PAUSE;
                        music_button->priv->seeking = FALSE;
                        break;

                default:
                        break;
        }

        music_button_set_status (music_button, NULL);
        gtk_image_set_from_stock (GTK_IMAGE (image), stock_id, icon_size);

        g_log (
                G_LOG_DOMAIN, GVA_DEBUG_GST,
                "%s -> %s -> %s",
                gst_element_state_get_name (old_state),
                gst_element_state_get_name (new_state),
                gst_element_state_get_name (pending));
}

static gboolean
music_button_bus_cb (GstBus *bus,
                     GstMessage *message,
                     GvaMusicButton *music_button)
{
        g_log (
                G_LOG_DOMAIN, GVA_DEBUG_GST,
                "%s", GST_MESSAGE_TYPE_NAME (message));

        switch (GST_MESSAGE_TYPE (message))
        {
                case GST_MESSAGE_EOS:
                        music_button_handle_eos (
                                music_button, message);
                        break;

                case GST_MESSAGE_ERROR:
                        music_button_handle_error (
                                music_button, message);
                        break;

                case GST_MESSAGE_BUFFERING:
                        music_button_handle_buffering (
                                music_button, message);
                        break;

                case GST_MESSAGE_STATE_CHANGED:
                        music_button_handle_state_changed (
                                music_button, message);
                        break;

                default:
                        break;
        }

        return TRUE;
}

static void
music_button_setup_element (GvaMusicButton *music_button)
{
        GstElement *element;
        const gchar *game;
        gchar *uri;
        guint id;

        element = music_button->priv->element;

        game = gva_music_button_get_game (music_button);
        id = (game != NULL) ? gva_history_lookup_id (game) : 0;

        gst_element_set_state (element, GST_STATE_NULL);

        uri = (id > 0) ? g_strdup_printf (MUSIC_URI "%u.mp3", id) : NULL;
        g_object_set (element, "uri", uri, NULL);
        g_free (uri);

        music_button_set_status (music_button, _("Connecting..."));
        gtk_widget_set_sensitive (GTK_WIDGET (music_button), TRUE);

        gst_element_set_state (element, GST_STATE_PAUSED);
}

#endif /* HAVE_GSTREAMER */

static void
music_button_set_property (GObject *object,
                           guint property_id,
                           const GValue *value,
                           GParamSpec *pspec)
{
        switch (property_id)
        {
                case PROP_GAME:
                        gva_music_button_set_game (
                                GVA_MUSIC_BUTTON (object),
                                g_value_get_string (value));
                        return;
        }

        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
music_button_get_property (GObject *object,
                           guint property_id,
                           GValue *value,
                           GParamSpec *pspec)
{
        switch (property_id)
        {
                case PROP_GAME:
                        g_value_set_string (
                                value, gva_music_button_get_game (
                                GVA_MUSIC_BUTTON (object)));
                        return;

                case PROP_STATUS:
                        g_value_set_string (
                                value, gva_music_button_get_status (
                                GVA_MUSIC_BUTTON (object)));
                        return;
        }

        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
music_button_dispose (GObject *object)
{
        GvaMusicButtonPrivate *priv;

        priv = GVA_MUSIC_BUTTON_GET_PRIVATE (object);

#ifdef HAVE_GSTREAMER
        if (priv->element != NULL)
        {
                gst_element_set_state (priv->element, GST_STATE_NULL);
                g_object_unref (priv->element);
                priv->element = NULL;
        }
#endif

        /* Chain up to parent's dispose() method. */
        G_OBJECT_CLASS (gva_music_button_parent_class)->dispose (object);
}

static void
music_button_finalize (GObject *object)
{
        GvaMusicButtonPrivate *priv;

        priv = GVA_MUSIC_BUTTON_GET_PRIVATE (object);

        if (priv->status_idle_id > 0)
                g_source_remove (priv->status_idle_id);

        g_free (priv->status);

        /* Chain up to parent's finalize() method. */
        G_OBJECT_CLASS (gva_music_button_parent_class)->finalize (object);
}

static void
music_button_constructed (GObject *object)
{
        gtk_button_set_image (
                GTK_BUTTON (object),
                gtk_image_new_from_stock (
                GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_BUTTON));
}

static void
music_button_clicked (GtkButton *button)
{
#ifdef HAVE_GSTREAMER
        GvaMusicButton *music_button;
        GstElement *element;
        GstState state;

        music_button = GVA_MUSIC_BUTTON (button);
        element = music_button->priv->element;

        gst_element_get_state (element, &state, NULL, 0);

        switch (state)
        {
                case GST_STATE_NULL:
                case GST_STATE_READY:
                case GST_STATE_PAUSED:
                        gva_music_button_play (music_button);
                        break;

                case GST_STATE_PLAYING:
                        gva_music_button_pause (music_button);
                        break;

                default:
                        break;
        }
#endif
}

static void
music_button_pause (GvaMusicButton *music_button)
{
#ifdef HAVE_GSTREAMER
        GstElement *element;

        element = music_button->priv->element;

        gst_element_set_state (element, GST_STATE_PAUSED);
#endif
}

static void
music_button_play (GvaMusicButton *music_button)
{
#ifdef HAVE_GSTREAMER
        GstElement *element;

        element = music_button->priv->element;

        gst_element_set_state (element, GST_STATE_PLAYING);
#endif
}

static void
gva_music_button_class_init (GvaMusicButtonClass *class)
{
        GObjectClass *object_class;
        GtkButtonClass *button_class;

        g_type_class_add_private (class, sizeof (GvaMusicButtonPrivate));

        object_class = G_OBJECT_CLASS (class);
        object_class->set_property = music_button_set_property;
        object_class->get_property = music_button_get_property;
        object_class->dispose = music_button_dispose;
        object_class->finalize = music_button_finalize;
        object_class->constructed = music_button_constructed;

        button_class = GTK_BUTTON_CLASS (class);
        button_class->clicked = music_button_clicked;

        class->pause = music_button_pause;
        class->play = music_button_play;

        /**
         * GvaMusicButton:game
         *
         * The game for which to play a music clip.
         **/
        g_object_class_install_property (
                object_class,
                PROP_GAME,
                g_param_spec_string (
                        "game",
                        NULL,
                        NULL,
                        NULL,
                        G_PARAM_READWRITE |
                        G_PARAM_STATIC_STRINGS));

        /**
         * GvaMusicButton:status
         *
         * Status message about the music clip.
         **/
        g_object_class_install_property (
                object_class,
                PROP_STATUS,
                g_param_spec_string (
                        "status",
                        NULL,
                        NULL,
                        NULL,
                        G_PARAM_READABLE |
                        G_PARAM_STATIC_STRINGS));

        /**
         * GvaMusicButton::pause
         * @music_button: the #GvaMusicButton that received the signal
         *
         * The ::pause signal is emitted when the user pauses a music clip.
         **/
        signals[PAUSE] = g_signal_new (
                "pause",
                G_TYPE_FROM_CLASS (class),
                G_SIGNAL_RUN_LAST,
                G_STRUCT_OFFSET (GvaMusicButtonClass, pause),
                NULL, NULL,
                g_cclosure_marshal_VOID__VOID,
                G_TYPE_NONE, 0);

        /**
         * GvaMusicButton::play
         * @music_button: the #GvaMusicButton
         *
         * The ::play signal is emitted when the user plays a music clip.
         **/
        signals[PLAY] = g_signal_new (
                "play",
                G_TYPE_FROM_CLASS (class),
                G_SIGNAL_RUN_LAST,
                G_STRUCT_OFFSET (GvaMusicButtonClass, play),
                NULL, NULL,
                g_cclosure_marshal_VOID__VOID,
                G_TYPE_NONE, 0);
}

static void
gva_music_button_init (GvaMusicButton *music_button)
{
#ifdef HAVE_GSTREAMER
        GstElement *element;
#endif

        music_button->priv = GVA_MUSIC_BUTTON_GET_PRIVATE (music_button);

#ifdef HAVE_GSTREAMER
        element = gst_element_factory_make ("playbin", "gva-music-player");
        music_button->priv->element = element;

        gst_bus_add_watch (
                gst_element_get_bus (element),
                (GstBusFunc) music_button_bus_cb, music_button);
#endif
}

/**
 * gva_music_button_new:
 *
 * Creates a new #GvaMusicButton.
 *
 * Returns: a new #GvaMusicButton
 **/
GtkWidget *
gva_music_button_new (void)
{
        return g_object_new (GVA_TYPE_MUSIC_BUTTON, NULL);
}

/**
 * gva_music_button_play:
 * @music_button: a #GvaMusicButton
 *
 * Plays a music clip from the game specified by the GvaMusicButton:game
 * property.  The clip will repeat indefinitely until paused or a different
 * game is chosen.
 **/
void
gva_music_button_play (GvaMusicButton *music_button)
{
        g_return_if_fail (GVA_IS_MUSIC_BUTTON (music_button));

        g_signal_emit (music_button, signals[PLAY], 0);
}

/**
 * gva_music_button_pause:
 * @music_button: a #GvaMusicButton
 *
 * Pauses a music clip from the game specified by the GvaMusicButton:game
 * property.
 **/
void
gva_music_button_pause (GvaMusicButton *music_button)
{
        g_return_if_fail (GVA_IS_MUSIC_BUTTON (music_button));

        g_signal_emit (music_button, signals[PAUSE], 0);
}

/**
 * gva_music_button_get_game:
 * @music_button: a #GvaMusicButton
 *
 * Returns the name of the game for which to play a music clip.
 *
 * Returns: the game for which to play a music clip
 **/
const gchar *
gva_music_button_get_game (GvaMusicButton *music_button)
{
        g_return_val_if_fail (GVA_IS_MUSIC_BUTTON (music_button), NULL);

        return music_button->priv->game;
}

/**
 * gva_music_button_set_game:
 * @music_button: a #GvaMusicButton
 * @game: the name of a game
 *
 * Sets the name of the game for which to play a music clip.  Use
 * gva_music_button_play() to play the clip.
 **/
void
gva_music_button_set_game (GvaMusicButton *music_button,
                           const gchar *game)
{
        g_return_if_fail (GVA_IS_MUSIC_BUTTON (music_button));

        if (game != NULL)
                game = g_intern_string (game);

        music_button->priv->game = game;

#ifdef HAVE_GSTREAMER
        music_button_setup_element (music_button);
#endif

        g_object_notify (G_OBJECT (music_button), "game");
}

/**
 * gva_music_button_get_status:
 * @music_button: a #GvaMusicButton
 *
 * Returns a status message about the music clip, such as buffering
 * progress or whether a music clip is available for the selected game.
 *
 * Returns: a status message about the music clip
 **/
const gchar *
gva_music_button_get_status (GvaMusicButton *music_button)
{
        g_return_val_if_fail (GVA_IS_MUSIC_BUTTON (music_button), NULL);

        return music_button->priv->status;
}
