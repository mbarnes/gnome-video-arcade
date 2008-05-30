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

#include "gva-wnck.h"

#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>

#include "gva-db.h"
#include "gva-error.h"

#define SQL_SELECT_GAME_WINDOW \
        "SELECT * FROM window WHERE name == '%s'"

#define TIMEOUT_INTERVAL        100 /* milliseconds */
#define TIMEOUT_SOURCE_ID_KEY   "timeout-source-id"

enum
{
        COLUMN_NAME,
        COLUMN_X,
        COLUMN_Y,
        COLUMN_WIDTH,
        COLUMN_HEIGHT,
        COLUMN_MAXIMIZED
};

static gulong window_opened_handler_id;

static const gchar *
wnck_window_get_game (WnckWindow *window)
{
        GvaProcess *process;
        const gchar *game;

        process = g_object_get_data (G_OBJECT (window), "process");
        g_return_val_if_fail (process != NULL, NULL);

        game = g_object_get_data (G_OBJECT (process), "game");
        g_return_val_if_fail (game != NULL, NULL);

        return game;
}

static void
wnck_window_initialize (WnckWindow *window)
{
        const gchar *game;
        sqlite3_stmt *stmt;
        gchar *sql;
        gint errcode;
        gboolean success;
        GError *error = NULL;

        game = wnck_window_get_game (window);

        sql = g_strdup_printf (SQL_SELECT_GAME_WINDOW, game);
        success = gva_db_prepare (sql, &stmt, &error);
        gva_error_handle (&error);
        g_free (sql);

        if (!success)
                return;

        errcode = sqlite3_step (stmt);

        /* Restore the window's previous geometry. */
        if (errcode == SQLITE_ROW)
        {
                gint x, y;
                gint width, height;
                gboolean maximized;

                x = sqlite3_column_int (stmt, COLUMN_X);
                y = sqlite3_column_int (stmt, COLUMN_Y);
                width = sqlite3_column_int (stmt, COLUMN_WIDTH);
                height = sqlite3_column_int (stmt, COLUMN_HEIGHT);
                maximized = sqlite3_column_int (stmt, COLUMN_MAXIMIZED);

                wnck_window_set_geometry (
                        window, WNCK_WINDOW_GRAVITY_CURRENT,
                        WNCK_WINDOW_CHANGE_X | WNCK_WINDOW_CHANGE_Y |
                        WNCK_WINDOW_CHANGE_WIDTH | WNCK_WINDOW_CHANGE_HEIGHT,
                        x, y, width, height);

                if (maximized)
                        wnck_window_maximize (window);
                else
                        wnck_window_unmaximize (window);
        }

        /* Create a new record using the current geometry. */
        else if (errcode == SQLITE_DONE)
        {
                gint x, y;
                gint width, height;
                gboolean maximized;

                maximized = wnck_window_is_maximized (window);
                wnck_window_get_geometry (window, &x, &y, &width, &height);

                sql = g_strdup_printf (
                        "INSERT INTO window VALUES "
                        "('%s', %d, %d, %d, %d, %d)",
                        game, x, y, width, height, maximized);
                gva_db_execute (sql, &error);
                gva_error_handle (&error);
                g_free (sql);
        }

        /* Something went wrong. */
        else
        {
                gva_db_set_error (&error, 0, NULL);
                gva_error_handle (&error);
        }

        sqlite3_finalize (stmt);
}

static gboolean
wnck_window_timeout_cb (WnckWindow *window)
{
        const gchar *game;
        gint x, y;
        gint width, height;
        gchar *sql;
        GError *error = NULL;

        if (wnck_window_is_fullscreen (window))
                goto exit;

        game = wnck_window_get_game (window);
        wnck_window_get_geometry (window, &x, &y, &width, &height);

        if (wnck_window_is_maximized (window))
                sql = g_strdup_printf (
                        "UPDATE window SET maximized = 1 "
                        "WHERE name = '%s'", game);
        else
                sql = g_strdup_printf (
                        "UPDATE window SET x = %d, y = %d, width = %d, "
                        "height = %d, maximized = 0 WHERE name = '%s'",
                        x, y, width, height, game);

        gva_db_execute (sql, &error);
        gva_error_handle (&error);
        g_free (sql);

exit:
        g_object_set_data (G_OBJECT (window), TIMEOUT_SOURCE_ID_KEY, NULL);

        return FALSE;
}

static void
wnck_window_timeout_cancel (WnckWindow *window)
{
        guint source_id;

        source_id = GPOINTER_TO_UINT (
                g_object_get_data (G_OBJECT (window), TIMEOUT_SOURCE_ID_KEY));

        if (source_id > 0)
                g_source_remove (source_id);

        g_object_set_data (G_OBJECT (window), TIMEOUT_SOURCE_ID_KEY, NULL);
}

static void
wnck_window_timeout_reset (WnckWindow *window)
{
        guint source_id;

        wnck_window_timeout_cancel (window);

        source_id = g_timeout_add_full (
                G_PRIORITY_DEFAULT, TIMEOUT_INTERVAL,
                (GSourceFunc) wnck_window_timeout_cb,
                g_object_ref (window), (GDestroyNotify) g_object_unref);

        g_object_set_data (
                G_OBJECT (window), TIMEOUT_SOURCE_ID_KEY,
                GUINT_TO_POINTER (source_id));
}

static void
wnck_window_closed_cb (WnckScreen *screen,
                       WnckWindow *window)
{
        wnck_window_timeout_cancel (window);
}

static void
wnck_weak_notify_cb (WnckScreen *screen,
                     GObject *where_the_process_was)
{
        /* The process exited before we could match a window to it. */
        g_signal_handlers_disconnect_matched (
                screen, G_SIGNAL_MATCH_DATA, 0, 0,
                NULL, NULL, where_the_process_was);

        window_opened_handler_id = 0;
}

static void
wnck_window_opened_cb (WnckScreen *screen,
                       WnckWindow *window,
                       GvaProcess *process)
{
        GPid pid;

        /* XXX This is totally non-portable. */
        pid = (GPid) wnck_window_get_pid (window);

        /* XXX The process ID for SDLMAME windows currently comes back 0.
         *     We can use this as a heuristic for matching SDLMAME windows
         *     to SDLMAME processes.  But it's not perfect. */
        if (pid != gva_process_get_pid (process) && pid != 0)
                return;

        /*** From this point forward, assume we have a match. ***/

        /* Exchange the weak reference for a real reference. */
        g_object_weak_unref (
                g_object_ref (process), (GWeakNotify)
                wnck_weak_notify_cb, screen);

        /* Bind the referenced process to the window. */
        g_object_set_data_full (
                G_OBJECT (window), "process", process,
                (GDestroyNotify) g_object_unref);

        /* No need to be notified of new windows anymore. */
        g_signal_handler_disconnect (screen, window_opened_handler_id);
        window_opened_handler_id = 0;

        wnck_window_initialize (window);

        g_signal_connect (
                window, "geometry-changed",
                G_CALLBACK (wnck_window_timeout_reset), NULL);

        g_signal_connect (
                window, "state-changed",
                G_CALLBACK (wnck_window_timeout_reset), NULL);

        g_signal_connect (
                screen, "window-closed",
                G_CALLBACK (wnck_window_closed_cb), NULL);
}

/**
 * gva_wnck_listen_for_new_window:
 * @process: a #GvaProcess
 * @game: the game that @process is running
 *
 * Implements spatial MAME windows.
 *
 * This function gets called immediately after a game is started.  It uses
 * libwnck to listen for a new window, and does its best to verify whether
 * it's a MAME window.  If it identifies the window, it restores the size,
 * position and maximize state of that particular game from the previous
 * session and records any further window geometry changes.  The game
 * database is used as the storage medium for window geometries.
 **/
void
gva_wnck_listen_for_new_window (GvaProcess *process,
                                const gchar *game)
{
        g_return_if_fail (GVA_IS_PROCESS (process));
        g_return_if_fail (game != NULL);

        if (window_opened_handler_id > 0)
        {
                g_warning ("GVA can only listen for one new window at a time");
                return;
        }

        g_object_set_data_full (
                G_OBJECT (process), "game",
                g_strdup (game), (GDestroyNotify) g_free);

        /* Keep a weak reference on the process until we can match it
         * to a newly opened window, at which time we'll add a real
         * reference and bind it to the corresponding WnckWindow.
         * If the process exits early we'll know to stop listening. */
        g_object_weak_ref (
                G_OBJECT (process),
                (GWeakNotify) wnck_weak_notify_cb,
                wnck_screen_get_default ());

        window_opened_handler_id = g_signal_connect (
                wnck_screen_get_default (), "window-opened",
                G_CALLBACK (wnck_window_opened_cb), process);
}
