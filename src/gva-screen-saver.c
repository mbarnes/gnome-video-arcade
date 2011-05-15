/* Copyright 2007-2011 Matthew Barnes
 *
 * This file is part of GNOME Video Arcade.
 *
 * Based on totem-scrsaver.c by Bastien Nocera.
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

#include "config.h"

#include <glib/gi18n.h>

#include <gdk/gdk.h>

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/keysym.h>
#endif /* GDK_WINDOWING_X11 */

#include "gva-screen-saver.h"

#define SCREEN_SAVER_SERVICE   "org.gnome.ScreenSaver"
#define SCREEN_SAVER_PATH      "/org/gnome/ScreenSaver"
#define SCREEN_SAVER_INTERFACE "org.gnome.ScreenSaver"

#define GVA_SCREEN_SAVER_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE \
        ((obj), GVA_TYPE_SCREEN_SAVER, GvaScreenSaverPrivate))

struct _GvaScreenSaverPrivate
{
        gboolean disabled;
        gchar *reason;

        GDBusProxy *gss_proxy;
        gboolean have_screen_saver_dbus;
        guint32 cookie;

        /* To save the X11 screen saver info */
        gint timeout;
        gint interval;
        gint prefer_blanking;
        gint allow_exposures;
};

enum
{
        PROP_0,
        PROP_REASON
};

G_DEFINE_TYPE (GvaScreenSaver, gva_screen_saver, G_TYPE_OBJECT)

static gboolean
screen_saver_is_running_dbus (GvaScreenSaver *screen_saver)
{
        return screen_saver->priv->have_screen_saver_dbus;
}

static void
screen_saver_inhibit_cb (GDBusProxy *proxy,
                         GAsyncResult *result,
                         GvaScreenSaver *screen_saver)
{
        GVariant *value;
        GError *error = NULL;

        value = g_dbus_proxy_call_finish (proxy, result, &error);

        if (value != NULL)
        {
                guint32 cookie = 0;

                /* save the cookie */
                if (g_variant_is_of_type (value, G_VARIANT_TYPE ("(u)")))
                        g_variant_get (value, "(u)", &cookie);
                screen_saver->priv->cookie = cookie;
                g_variant_unref (value);
        }
        else
        {
                g_return_if_fail (error != NULL);
                g_warning (
                        "Problem inhibiting the screen saver: %s",
                        error->message);
                g_error_free (error);
        }

        g_object_unref (screen_saver);
}

static void
screen_saver_uninhibit_cb (GDBusProxy *proxy,
                           GAsyncResult *result,
                           GvaScreenSaver *screen_saver)
{
        GVariant *value;
        GError *error = NULL;

        value = g_dbus_proxy_call_finish (proxy, result, &error);

        if (value != NULL)
        {
                /* Clear the cookie. */
                screen_saver->priv->cookie = 0;
                g_variant_unref (value);
        }
        else
        {
                g_return_if_fail (error != NULL);
                g_warning (
                        "Problem uninhibiting the screen saver: %s",
                        error->message);
                g_error_free (error);
        }

        g_object_unref (screen_saver);
}

static void
screen_saver_inhibit_dbus (GvaScreenSaver *screen_saver,
                           gboolean inhibit)
{
        GDBusProxy *proxy;

        if (!screen_saver->priv->have_screen_saver_dbus)
                return;

        proxy = screen_saver->priv->gss_proxy;

        if (inhibit) {
                g_return_if_fail (screen_saver->priv->reason != NULL);
                g_dbus_proxy_call (
                        proxy,
                        "Inhibit",
                        g_variant_new (
                                "(ss)",
                                g_get_application_name (),
                                screen_saver->priv->reason),
                        G_DBUS_CALL_FLAGS_NO_AUTO_START,
                        -1,
                        NULL,
                        (GAsyncReadyCallback) screen_saver_inhibit_cb,
                        g_object_ref (screen_saver));
        } else {
                g_dbus_proxy_call (
                        proxy,
                        "UnInhibit",
                        g_variant_new ("(u)", screen_saver->priv->cookie),
                        G_DBUS_CALL_FLAGS_NO_AUTO_START,
                        -1,
                        NULL,
                        (GAsyncReadyCallback) screen_saver_uninhibit_cb,
                        g_object_ref (screen_saver));
        }
}

static void
screen_saver_enable_dbus (GvaScreenSaver *screen_saver)
{
        screen_saver_inhibit_dbus (screen_saver, FALSE);
}

static void
screen_saver_disable_dbus (GvaScreenSaver *screen_saver)
{
        screen_saver_inhibit_dbus (screen_saver, TRUE);
}

static void
screen_saver_update_dbus_presence (GvaScreenSaver *screen_saver)
{
        GDBusProxy *proxy;
        gchar *name_owner;

        proxy = screen_saver->priv->gss_proxy;
        name_owner = g_dbus_proxy_get_name_owner (proxy);

        if (name_owner != NULL)
        {
                screen_saver->priv->have_screen_saver_dbus = TRUE;
                g_free (name_owner);

                /* Screen saver just appeared, or reappeared. */
                if (screen_saver->priv->reason != NULL)
                        screen_saver_disable_dbus (screen_saver);
        }
        else
        {
                screen_saver->priv->have_screen_saver_dbus = FALSE;
        }
}

static void
screen_saver_dbus_owner_changed_cb (GDBusProxy *proxy,
                                    GParamSpec *pspec,
                                    GvaScreenSaver *screen_saver)
{
        screen_saver_update_dbus_presence (screen_saver);
}

static void
screen_saver_dbus_proxy_new_cb (GObject *source,
                                GAsyncResult *result,
                                GvaScreenSaver *screen_saver)
{
        GDBusProxy *proxy;

        proxy = g_dbus_proxy_new_for_bus_finish (result, NULL);

        if (proxy == NULL)
                return;

        g_signal_connect (
                proxy, "notify::g-name-owner",
                G_CALLBACK (screen_saver_dbus_owner_changed_cb),
                screen_saver);

        screen_saver->priv->gss_proxy = proxy;

        screen_saver_update_dbus_presence (screen_saver);
}

#ifdef GDK_WINDOWING_X11
static void
screen_saver_enable_x11 (GvaScreenSaver *screen_saver)
{
        GdkDisplay *display;

        display = gdk_display_get_default ();

        XLockDisplay (GDK_DISPLAY_XDISPLAY (display));
        XSetScreenSaver (
                GDK_DISPLAY_XDISPLAY (display),
                screen_saver->priv->timeout,
                screen_saver->priv->interval,
                screen_saver->priv->prefer_blanking,
                screen_saver->priv->allow_exposures);
        XUnlockDisplay (GDK_DISPLAY_XDISPLAY (display));
}

static void
screen_saver_disable_x11 (GvaScreenSaver *screen_saver)
{
        GdkDisplay *display;

        display = gdk_display_get_default ();

        XLockDisplay (GDK_DISPLAY_XDISPLAY (display));
        XGetScreenSaver (
                GDK_DISPLAY_XDISPLAY (display),
                &screen_saver->priv->timeout,
                &screen_saver->priv->interval,
                &screen_saver->priv->prefer_blanking,
                &screen_saver->priv->allow_exposures);
        XSetScreenSaver (
                GDK_DISPLAY_XDISPLAY (display), 0, 0,
                DontPreferBlanking, DontAllowExposures);
        XUnlockDisplay (GDK_DISPLAY_XDISPLAY (display));
}
#endif

static void
screen_saver_set_property (GObject *object,
                           guint property_id,
                           const GValue *value,
                           GParamSpec *pspec)
{
        switch (property_id)
        {
                case PROP_REASON:
                        gva_screen_saver_set_reason (
                                GVA_SCREEN_SAVER (object),
                                g_value_get_string (value));
                        return;
        }

        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
screen_saver_get_property (GObject *object,
                           guint property_id,
                           GValue *value,
                           GParamSpec *pspec)
{
        switch (property_id)
        {
                case PROP_REASON:
                        g_value_set_string (
                                value, gva_screen_saver_get_reason (
                                GVA_SCREEN_SAVER (object)));
                        return;
        }

        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
screen_saver_dispose (GObject *object)
{
        GvaScreenSaverPrivate *priv;

        priv = GVA_SCREEN_SAVER_GET_PRIVATE (object);

        if (priv->gss_proxy != NULL)
        {
                g_object_unref (priv->gss_proxy);
                priv->gss_proxy = NULL;
        }

        /* Chain up to parent's dispose() method. */
        G_OBJECT_CLASS (gva_screen_saver_parent_class)->dispose (object);
}

static void
screen_saver_finalize (GObject *object)
{
        GvaScreenSaverPrivate *priv;

        priv = GVA_SCREEN_SAVER_GET_PRIVATE (object);

        g_free (priv->reason);

        /* Chain up to parent's finalize() method. */
        G_OBJECT_CLASS (gva_screen_saver_parent_class)->finalize (object);
}

static void
gva_screen_saver_class_init (GvaScreenSaverClass *class)
{
        GObjectClass *object_class;

        g_type_class_add_private (class, sizeof (GvaScreenSaverPrivate));

        object_class = G_OBJECT_CLASS (class);
        object_class->set_property = screen_saver_set_property;
        object_class->get_property = screen_saver_get_property;
        object_class->dispose = screen_saver_dispose;
        object_class->finalize = screen_saver_finalize;

        /**
         * GvaScreenSaver:reason
         *
         * The reason for disabling the screen saver.
         **/
        g_object_class_install_property (
                object_class,
                PROP_REASON,
                g_param_spec_string (
                        "reason",
                        NULL,
                        NULL,
                        NULL,
                        G_PARAM_READWRITE |
                        G_PARAM_STATIC_STRINGS));
}

static void
gva_screen_saver_init (GvaScreenSaver *screen_saver)
{
        screen_saver->priv = GVA_SCREEN_SAVER_GET_PRIVATE (screen_saver);

        g_dbus_proxy_new_for_bus (
                G_BUS_TYPE_SESSION,
                G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                NULL,
                SCREEN_SAVER_SERVICE,
                SCREEN_SAVER_PATH,
                SCREEN_SAVER_INTERFACE,
                NULL, (GAsyncReadyCallback)
                screen_saver_dbus_proxy_new_cb,
                screen_saver);
}

/**
 * gva_screen_saver_new:
 *
 * Creates a #GvaScreenSaver object.
 *
 * If the GNOME screen saver is running, it uses its D-Bus interface
 * to inhibit the screen saver; otherwise it falls back to using the
 * X screen saver functionality for this.
 *
 * Returns: a newly created #GvaScreenSaver
 */
GvaScreenSaver *
gva_screen_saver_new (void)
{
        return g_object_new (GVA_TYPE_SCREEN_SAVER, NULL);
}

/**
 * gva_screen_saver_disable:
 * @screen_saver: a #GvaScreenSaver
 *
 * Disables the screen saver.
 **/
void
gva_screen_saver_disable (GvaScreenSaver *screen_saver)
{
        g_return_if_fail (GVA_IS_SCREEN_SAVER (screen_saver));

        if (screen_saver->priv->disabled != FALSE)
                return;

        screen_saver->priv->disabled = TRUE;

        if (screen_saver_is_running_dbus (screen_saver))
                screen_saver_disable_dbus (screen_saver);
        else 
#ifdef GDK_WINDOWING_X11
                screen_saver_disable_x11 (screen_saver);
#else
#warning Unimplemented
        {}
#endif
}

/**
 * gva_screen_saver_enable:
 * @screen_saver: a #GvaScreenSaver
 *
 * Enables the screen saver.
 **/
void
gva_screen_saver_enable (GvaScreenSaver *screen_saver)
{
        g_return_if_fail (GVA_IS_SCREEN_SAVER (screen_saver));

        if (screen_saver->priv->disabled == FALSE)
                return;

        screen_saver->priv->disabled = FALSE;

        if (screen_saver_is_running_dbus (screen_saver))
                screen_saver_enable_dbus (screen_saver);
        else
#ifdef GDK_WINDOWING_X11
                screen_saver_enable_x11 (screen_saver);
#else
#warning Unimplemented
        {}
#endif
}

/**
 * gva_screen_saver_get_reason:
 * @screen_saver: a #GvaScreenSaver
 *
 * Returns the reason for disabling the screen saver.
 *
 * Returns: the reason for disabling the screen saver
 **/
const gchar *
gva_screen_saver_get_reason (GvaScreenSaver *screen_saver)
{
        g_return_val_if_fail (GVA_IS_SCREEN_SAVER (screen_saver), NULL);

        return screen_saver->priv->reason;
}

/**
 * gva_screen_saver_set_reason:
 * @screen_saver: a #GvaScreenSaver
 * @reason: the reason for disabling the screen saver
 *
 * Sets the reason for disabling the screen saver.  This string is passed
 * to the "org.gnome.ScreenSaver" D-Bus interface on subsequent calls to
 * gva_screen_saver_disable().
 **/
void
gva_screen_saver_set_reason (GvaScreenSaver *screen_saver,
                             const gchar *reason)
{
        g_return_if_fail (GVA_IS_SCREEN_SAVER (screen_saver));

        g_free (screen_saver->priv->reason);
        screen_saver->priv->reason = g_strdup (reason);

        g_object_notify (G_OBJECT (screen_saver), "reason");
}
