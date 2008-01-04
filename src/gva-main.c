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

#include "gva-main.h"

#include <stdarg.h>

#include "gva-audit.h"
#include "gva-db.h"
#include "gva-mame.h"
#include "gva-tree-view.h"
#include "gva-ui.h"

static guint menu_tooltip_cid;

static void
main_build_database_progress_cb (GvaProcess *process,
                                 GParamSpec *pspec,
                                 gpointer user_data)
{
        guint total_supported = GPOINTER_TO_UINT (user_data);
        GtkProgressBar *progress_bar;
        gdouble fraction = 0.0;

        if (total_supported != 0)
        {
                guint progress;

                progress = gva_process_get_progress (process);
                fraction = (gdouble) progress / (gdouble) total_supported;
                fraction = CLAMP (fraction, 0.0, 1.0);
        }

        progress_bar = GTK_PROGRESS_BAR (GVA_WIDGET_MAIN_PROGRESS_BAR);
        gtk_progress_bar_set_fraction (progress_bar, fraction);
}

static void
main_menu_item_select_cb (GtkItem *item, GtkAction *action)
{
        gchar *tooltip;

        g_object_get (G_OBJECT (action), "tooltip", &tooltip, NULL);
        if (tooltip != NULL)
                gva_main_statusbar_push (menu_tooltip_cid, "%s", tooltip);
        g_free (tooltip);
}

static void
main_menu_item_deselect_cb (GtkItem *item)
{
        gva_main_statusbar_pop (menu_tooltip_cid);
}

/**
 * gva_main_init:
 *
 * Initializes the main window.
 *
 * This function should be called once when the application starts.
 **/
void
gva_main_init (void)
{
        gva_tree_view_init ();

        gtk_box_pack_start (
                GTK_BOX (GVA_WIDGET_MAIN_VBOX),
                gva_ui_get_managed_widget ("/main-menu"),
                FALSE, FALSE, 0);

        gtk_action_connect_proxy (
                GVA_ACTION_VIEW_AVAILABLE,
                GVA_WIDGET_MAIN_VIEW_BUTTON_0);

        gtk_action_connect_proxy (
                GVA_ACTION_VIEW_FAVORITES,
                GVA_WIDGET_MAIN_VIEW_BUTTON_1);

        gtk_action_connect_proxy (
                GVA_ACTION_VIEW_RESULTS,
                GVA_WIDGET_MAIN_VIEW_BUTTON_2);

        gtk_action_connect_proxy (
                GVA_ACTION_PROPERTIES,
                GVA_WIDGET_MAIN_PROPERTIES_BUTTON);

        gtk_action_connect_proxy (
                GVA_ACTION_START,
                GVA_WIDGET_MAIN_START_GAME_BUTTON);

        gconf_bridge_bind_window (
                gconf_bridge_get (), GVA_GCONF_WINDOW_PREFIX,
                GTK_WINDOW (GVA_WIDGET_MAIN_WINDOW), TRUE, FALSE);

#ifndef WITH_GNOME
        /* Requires that we link against libgnome. */
        gtk_action_set_sensitive (GVA_ACTION_CONTENTS, FALSE);
#endif

        gtk_action_set_sensitive (GVA_ACTION_PROPERTIES, FALSE);
        gtk_action_set_sensitive (GVA_ACTION_RECORD, FALSE);
        gtk_action_set_sensitive (GVA_ACTION_START, FALSE);
        gtk_action_set_sensitive (GVA_ACTION_VIEW_AVAILABLE, FALSE);
        gtk_action_set_sensitive (GVA_ACTION_VIEW_FAVORITES, FALSE);
        gtk_action_set_sensitive (GVA_ACTION_VIEW_RESULTS, FALSE);

        gtk_action_set_visible (GVA_ACTION_INSERT_FAVORITE, FALSE);
        gtk_action_set_visible (GVA_ACTION_REMOVE_FAVORITE, FALSE);

        gtk_window_present (GTK_WINDOW (GVA_WIDGET_MAIN_WINDOW));
}

/**
 * gva_main_build_database:
 * @error: return location for a #GError, or %NULL
 *
 * Executes the lengthy process of constructing the games database.
 * The function displays a progress bar in the main status bar that
 * tracks the database construction.  The function is synchronous;
 * it blocks until database construction is complete or aborted.
 *
 * Returns: %TRUE if the database construction was successful,
 *          %FALSE if construction failed or was aborted
 **/
gboolean
gva_main_build_database (GError **error)
{
        GtkWidget *progress_bar;
        GdkCursor *cursor;
        GdkDisplay *display;
        GdkWindow *window;
        GvaProcess *process;
        GvaProcess *process2 = NULL;
        guint context_id;
        guint total_supported;
        gboolean main_loop_quit = FALSE;
        gboolean success = FALSE;

        /* XXX Comment this code! */

        progress_bar = GVA_WIDGET_MAIN_PROGRESS_BAR;

        window = gtk_widget_get_parent_window (progress_bar);
        display = gtk_widget_get_display (progress_bar);
        cursor = gdk_cursor_new_for_display (display, GDK_WATCH);
        gdk_window_set_cursor (window, cursor);

        gtk_widget_show (progress_bar);
        context_id = gva_main_statusbar_get_context_id (G_STRFUNC);
        total_supported = gva_mame_get_total_supported (NULL);

        process = gva_db_build (error);
        if (process == NULL)
                goto fail;

        gva_main_statusbar_push (context_id, _("Building game database..."));

        g_signal_connect (
                process, "notify::progress",
                G_CALLBACK (main_build_database_progress_cb),
                GUINT_TO_POINTER (total_supported));

        while (!gva_process_has_exited (process, NULL))
                main_loop_quit = gtk_main_iteration ();

        if (main_loop_quit)
                goto exit;

        process = gva_audit_roms (error);
        if (process == NULL)
                goto fail;

        process2 = gva_audit_samples (error);
        if (process2 == NULL)
                goto fail;

        gva_main_statusbar_pop (context_id);
        gva_main_statusbar_push (context_id, _("Analyzing ROM files..."));
        gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), 0.0);

        g_signal_connect (
                process, "notify::progress",
                G_CALLBACK (main_build_database_progress_cb),
                GUINT_TO_POINTER (total_supported));

        while (!gva_process_has_exited (process, NULL))
                main_loop_quit = gtk_main_iteration ();

        if (main_loop_quit)
                goto exit;

        while (!gva_process_has_exited (process2, NULL))
                main_loop_quit = gtk_main_iteration ();

        if (main_loop_quit)
                goto exit;

        success = TRUE;

fail:
        gva_main_statusbar_pop (context_id);
        gtk_widget_hide (progress_bar);

        gdk_window_set_cursor (window, NULL);
        gdk_cursor_unref (cursor);

exit:
        if (process != NULL)
                g_object_unref (process);

        if (process2 != NULL)
                g_object_unref (process2);

        return success;
}

/**
 * gva_main_connect_proxy_cb:
 * @manager: a #GtkUIManager
 * @action: a #GtkAction
 * @proxy: a #GtkWidget
 *
 * Callback for @manager's "%connect-proxy" signal.  The function configures
 * main menu items to display the appropriate tooltip in the status bar when
 * the mouse hovers over them.  @action supplies the tooltip.
 **/
void
gva_main_connect_proxy_cb (GtkUIManager *manager,
                           GtkAction *action,
                           GtkWidget *proxy)
{
        /* Show GtkMenuItem tooltips in the statusbar. */
        if (GTK_IS_MENU_ITEM (proxy))
        {
                GtkStatusbar *statusbar;

                statusbar = GTK_STATUSBAR (GVA_WIDGET_MAIN_STATUSBAR);

                g_signal_connect (
                        proxy, "select",
                        G_CALLBACK (main_menu_item_select_cb), action);
                g_signal_connect (
                        proxy, "deselect",
                        G_CALLBACK (main_menu_item_deselect_cb), NULL);

                menu_tooltip_cid =
                        gva_main_statusbar_get_context_id (G_STRFUNC);
        }
}

/**
 * gva_main_statusbar_get_context_id:
 * @context_description: textual description of what context the new
 * message is being used in
 *
 * Thin wrapper for gtk_statusbar_get_context_id() that uses the main
 * window's status bar.
 *
 * Returns a new context identifier, given a description of the actual
 * context.  Note that the description is <emphasis>not</emphasis> shown
 * in the UI.
 *
 * Returns: a context identifier
 **/
guint
gva_main_statusbar_get_context_id (const gchar *context_description)
{
        GtkStatusbar *statusbar;

        g_return_val_if_fail (context_description != NULL, 0);

        statusbar = GTK_STATUSBAR (GVA_WIDGET_MAIN_STATUSBAR);

        return gtk_statusbar_get_context_id (statusbar, context_description);
}

/**
 * gva_main_statusbar_push:
 * @context_id: a context identifier
 * @format: a standard <function>printf()</function> format string
 * @...: the arguments to insert into the format string
 *
 * Thin wrapper for gtk_statusbar_push() that uses the main window's
 * status bar.
 *
 * Pushes a new message onto the status bar's stack.  As a convenience,
 * the function takes a <function>printf()</function>-style format string
 * and variable length argument list.
 *
 * Returns: a message identifier that can be used with
 * gva_main_statusbar_remove()
 **/
guint
gva_main_statusbar_push (guint context_id,
                         const gchar *format,
                         ...)
{
        GtkStatusbar *statusbar;
        guint message_id;
        va_list args;
        gchar *text;

        g_return_val_if_fail (format != NULL, 0);

        statusbar = GTK_STATUSBAR (GVA_WIDGET_MAIN_STATUSBAR);

        va_start (args, format);
        text = g_strdup_vprintf (format, args);
        message_id = gtk_statusbar_push (statusbar, context_id, text);
        g_free (text);
        va_end (args);

        return message_id;
}

/**
 * gva_main_statusbar_pop:
 * @context_id: a context identifier
 *
 * Thin wrapper for gtk_statusbar_pop() that uses the main window's
 * status bar.
 *
 * Removes the first message in the status bar's stack with the given
 * context id.
 *
 * Note that this may not change the displayed message, if the message
 * at the top of the stack has a different context id.
 **/
void
gva_main_statusbar_pop (guint context_id)
{
        GtkStatusbar *statusbar;

        statusbar = GTK_STATUSBAR (GVA_WIDGET_MAIN_STATUSBAR);

        gtk_statusbar_pop (statusbar, context_id);
}

/**
 * gva_main_statusbar_remove:
 * @context_id: a context identifier
 * @message_id: a message identifier
 *
 * Thin wrapper for gtk_statusbar_remove() that uses the main window's
 * status bar.
 *
 * Forces the removal of a message from a status bar's stack.  The exact
 * @context_id and @message_id must be specified.
 **/
void
gva_main_statusbar_remove (guint context_id,
                           guint message_id)
{
        GtkStatusbar *statusbar;

        statusbar = GTK_STATUSBAR (GVA_WIDGET_MAIN_STATUSBAR);

        gtk_statusbar_remove (statusbar, context_id, message_id);
}

/**
 * gva_main_window_destroy_cb:
 * @window: the main window
 *
 * Handler for #GtkObject::destroy signals to the main window.
 *
 * Activates the #GVA_ACTION_QUIT action.
 **/
void
gva_main_window_destroy_cb (GtkWindow *window)
{
        gtk_action_activate (GVA_ACTION_QUIT);
}
