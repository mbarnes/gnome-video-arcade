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

#include "gva-dbus.h"

#include <dbus/dbus-glib.h>

#include "gva-error.h"
#include "gva-preferences.h"

#define SCREEN_SAVER_SERVICE            "org.gnome.ScreenSaver"
#define SCREEN_SAVER_PATH               "/org/gnome/ScreenSaver"
#define SCREEN_SAVER_INTERFACE          "org.gnome.ScreenSaver"

static DBusGConnection *connection;
static DBusGProxy *screen_saver_proxy;

static void
dbus_screen_saver_uninhibit (GvaProcess *process,
                             gint status,
                             gpointer user_data)
{
        guint cookie = GPOINTER_TO_UINT (user_data);
        GError *error = NULL;

        g_return_if_fail (cookie > 0);

        dbus_g_proxy_call (
                screen_saver_proxy,
                "UnInhibit", &error,
                G_TYPE_UINT, cookie,
                G_TYPE_INVALID,
                G_TYPE_INVALID);

        gva_error_handle (&error);
}

/**
 * gva_dbus_init:
 * @error: return location for a #GError, or %NULL
 *
 * Connects to D-Bus and sets up proxy objects.  If an error occurs,
 * the function returns %FALSE and sets @error.
 *
 * This function should be called once when the application starts.
 *
 * Returns: %TRUE on success, %FALSE if an error occurred
 **/
gboolean
gva_dbus_init (GError **error)
{
        connection = dbus_g_bus_get (DBUS_BUS_SESSION, error);

        if (connection == NULL)
                return FALSE;

        screen_saver_proxy = dbus_g_proxy_new_for_name (
                connection,
                SCREEN_SAVER_SERVICE,
                SCREEN_SAVER_PATH,
                SCREEN_SAVER_INTERFACE);

        return TRUE;
}

/**
 * gva_dbus_inhibit_screen_saver:
 * @process: a #GvaProcess
 * @reason: the reason for the inhibit
 * @error: return location for a #GError, or %NULL
 *
 * If the full screen preference is enabled, inhibits GNOME screen
 * saver for the duration of @process.  If an error occurs, the
 * function returns %FALSE and sets @error.
 *
 * Returns: %TRUE on success, %FALSE if an error occurred
 **/
gboolean
gva_dbus_inhibit_screen_saver (GvaProcess *process,
                               const gchar *reason,
                               GError **error)
{
        gboolean success;
        guint cookie = 0;

        g_return_val_if_fail (GVA_IS_PROCESS (process), FALSE);
        g_return_val_if_fail (reason != NULL, FALSE);

        /* Only inhibit screen saver in full screen mode. */
        if (!gva_preferences_get_full_screen ())
                return TRUE;

        success = dbus_g_proxy_call (
                screen_saver_proxy,
                "Inhibit", error,
                G_TYPE_STRING, PACKAGE_NAME,
                G_TYPE_STRING, reason,
                G_TYPE_INVALID,
                G_TYPE_UINT, &cookie,
                G_TYPE_INVALID);

        if (success)
                g_signal_connect (
                        process, "exited",
                        G_CALLBACK (dbus_screen_saver_uninhibit),
                        GUINT_TO_POINTER (cookie));

        return success;
}
