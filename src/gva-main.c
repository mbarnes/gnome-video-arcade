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
#include "gva-error.h"
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
        gdouble fraction = 0.0;

        if (total_supported != 0)
        {
                guint progress;

                progress = gva_process_get_progress (process);
                fraction = (gdouble) progress / (gdouble) total_supported;
                fraction = CLAMP (fraction, 0.0, 1.0);
        }

        gva_main_progress_bar_set_fraction (fraction);
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
        gchar *text;

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

        /* Initialize the search entry. */
        text = gva_main_get_last_search ();
        gtk_entry_set_text (GTK_ENTRY (GVA_WIDGET_MAIN_SEARCH_ENTRY), text);
        g_free (text);

#ifndef WITH_GNOME
        /* Requires that we link against libgnome. */
        gtk_action_set_sensitive (GVA_ACTION_CONTENTS, FALSE);
#endif

        gtk_action_set_sensitive (GVA_ACTION_PROPERTIES, FALSE);
        gtk_action_set_sensitive (GVA_ACTION_RECORD, FALSE);
        gtk_action_set_sensitive (GVA_ACTION_SEARCH, FALSE);
        gtk_action_set_sensitive (GVA_ACTION_SHOW_CLONES, FALSE);
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
 * The function updates the main window's progress bar to help track
 * the database construction.  The function is synchronous; it blocks
 * until database construction is complete or aborted.
 *
 * Returns: %TRUE if the database construction was successful,
 *          %FALSE if construction failed or was aborted
 **/
gboolean
gva_main_build_database (GError **error)
{
        GvaProcess *process;
        guint context_id;
        guint total_supported;
        gboolean main_loop_quit = FALSE;
        gboolean success = FALSE;

        /* XXX Comment this code! */

        context_id = gva_main_statusbar_get_context_id (G_STRFUNC);
        total_supported = gva_mame_get_total_supported (NULL);

        process = gva_db_build (error);
        if (process == NULL)
                goto exit;

        gva_main_statusbar_push (context_id, _("Building game database..."));

        g_signal_connect (
                process, "notify::progress",
                G_CALLBACK (main_build_database_progress_cb),
                GUINT_TO_POINTER (total_supported));

        while (!gva_process_has_exited (process, NULL))
                main_loop_quit = gtk_main_iteration ();

        if (main_loop_quit)
                goto exit;

        success = TRUE;

        gva_main_statusbar_pop (context_id);

exit:
        if (process != NULL)
                g_object_unref (process);

        return success;
}

/**
 * gva_main_analyze_roms:
 * @error: return location for a #GError, or %NULL
 *
 * Executes the lengthy process of analyzing all available ROM and sample
 * sets for correctness and then updating the games database with the new
 * status information.  The function updates the main window's progress bar
 * to help track the analysis.  The function is synchronous; it blocks until
 * the analysis is complete or aborted.
 *
 * Returns: %TRUE if the analysis completed successfully,
 *          %FALSE if the analysis failed or was aborted
 **/
gboolean
gva_main_analyze_roms (GError **error)
{
        GvaProcess *process;
        GvaProcess *process2 = NULL;
        guint context_id;
        guint total_supported;
        gboolean main_loop_quit = FALSE;
        gboolean success = FALSE;

        context_id = gva_main_statusbar_get_context_id (G_STRFUNC);
        total_supported = gva_mame_get_total_supported (NULL);

        process = gva_audit_roms (error);
        if (process == NULL)
                goto exit;

        process2 = gva_audit_samples (error);
        if (process2 == NULL)
                goto exit;

        gva_main_statusbar_push (context_id, _("Analyzing ROM files..."));

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

        gva_main_statusbar_pop (context_id);

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
 * gva_main_progress_bar_show:
 *
 * Shows the progress bar in the main window's status bar and sets the
 * mouse cursor to busy.  Generally useful before starting a long-running
 * foreground task.
 **/
void
gva_main_progress_bar_show (void)
{
        GdkCursor *cursor;
        GdkDisplay *display;
        GtkWidget *widget;
        GdkWindow *window;

        widget = GVA_WIDGET_MAIN_PROGRESS_BAR;
        window = gtk_widget_get_parent_window (widget);
        display = gtk_widget_get_display (widget);
        cursor = gdk_cursor_new_for_display (display, GDK_WATCH);
        gva_main_progress_bar_set_fraction (0.0);
        gdk_window_set_cursor (window, cursor);
        gdk_cursor_unref (cursor);
        gtk_widget_show (widget);
}

/**
 * gva_main_progress_bar_hide:
 *
 * Hides the progress bar in the main window's status bar and sets the
 * mouse cursor back to normal.  Generally useful after completing a
 * long-running foreground task.
 **/
void
gva_main_progress_bar_hide (void)
{
        GtkWidget *widget;
        GdkWindow *window;

        widget = GVA_WIDGET_MAIN_PROGRESS_BAR;
        window = gtk_widget_get_parent_window (widget);
        gdk_window_set_cursor (window, NULL);
        gtk_widget_hide (widget);
}

/**
 * gva_main_progress_bar_set_fraction:
 * @fraction: fraction of the task that's been completed
 *
 * Thin wrapper for gtk_progress_bar_set_fraction() that uses the main
 * window's progress bar.
 *
 * Causes the progress bar to "fill in" the given fraction of the bar.
 * The fraction should be between 0.0 and 1.0, inclusive.
 **/
void
gva_main_progress_bar_set_fraction (gdouble fraction)
{
        static GtkWidget *widget = NULL;

        /* If this function gets called once, it will likely get
         * called lots.  So cache a pointer to the widget so we
         * don't have to look it up each time.
         */
        if (G_UNLIKELY (widget == NULL))
                widget = GVA_WIDGET_MAIN_PROGRESS_BAR;

        gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (widget), fraction);
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
 * gva_main_get_last_search:
 *
 * Returns the criteria of the most recent search in either the current
 * or previous session of <emphasis>GNOME Video Arcade</emphasis>.
 *
 * Returns: the criteria of the most recent search
 **/
gchar *
gva_main_get_last_search (void)
{
        GConfClient *client;
        gchar *text;
        GError *error = NULL;

        client = gconf_client_get_default ();
        text = gconf_client_get_string (client, GVA_GCONF_SEARCH_KEY, &error);
        gva_error_handle (&error);
        g_object_unref (client);

        return (text != NULL) ? g_strstrip (text) : g_strdup ("");
}

/**
 * gva_main_set_last_search:
 * @text: the search text
 *
 * Writes @text to GConf key
 * <filename>/apps/gnome-video-arcade/search</filename>.
 *
 * This is used to remember the search text from the previous session of
 * <emphasis>GNOME Video Arcade</emphasis> so that the same text can be
 * preset in the search entry at startup.
 **/
void
gva_main_set_last_search (const gchar *text)
{
        GConfClient *client;
        GError *error = NULL;

        g_return_if_fail (text != NULL);

        client = gconf_client_get_default ();
        gconf_client_set_string (client, GVA_GCONF_SEARCH_KEY, text, &error);
        gva_error_handle (&error);
        g_object_unref (client);
}

/**
 * gva_main_search_entry_activate_cb:
 * @entry: the search entry
 *
 * Handler for #GtkEntry::activate signals to the search entry.
 *
 * Saves the contents of the search entry and switches the main window to
 * the "Search Results" view.
 **/
void
gva_main_search_entry_activate_cb (GtkEntry *entry)
{
        gchar *text;

        text = g_strdup (gtk_entry_get_text (entry));
        gtk_entry_set_text (entry, g_strstrip (text));
        gva_main_set_last_search (text);
        g_free (text);

        /* Force a tree view update. */
        if (gva_tree_view_get_selected_view () != 2)
                gva_tree_view_set_selected_view (2);
        else
        {
                GError *error = NULL;

                gva_tree_view_update (&error);
                gva_error_handle (&error);
        }
}

/**
 * gva_main_search_entry_notify_cb:
 * @entry: the search entry
 * @pspec: a #GParamSpec
 *
 * Handler for #GObject::notify signals to the search entry.
 *
 * Hides the search interface when the search entry loses input focus.
 **/
void
gva_main_search_entry_notify_cb (GtkEntry *entry,
                                 GParamSpec *pspec)
{
        if (g_str_equal (pspec->name, "has-focus"))
                if (!GTK_WIDGET_HAS_FOCUS (GTK_ENTRY (entry)))
                        gtk_widget_hide (GVA_WIDGET_MAIN_SEARCH_HBOX);
}

/**
 * gva_main_search_query_tooltip_cb:
 * @widget: the object which received the signal
 * @x: the x coordinate of the cursor position where the request has been
 *     emitted, relative to @widget->window
 * @y: the y coordinate of the cursor position where the request has been
 *     emitted, relative to @widget->window
 * @keyboard_mode: @TRUE if the tooltip was triggerd using the keyboard
 * @tooltip: a #GtkTooltip
 *
 * Handler for #GtkWidget::query-tooltip signals to the search interface.
 *
 * Displays some tips for searching.
 *
 * Returns: always %TRUE (show the tooltip)
 **/
gboolean
gva_main_search_query_tooltip_cb (GtkWidget *widget,
                                  gint x,
                                  gint y,
                                  gboolean keyboard_mode,
                                  GtkTooltip *tooltip)
{
        GtkWidget *custom;
        gchar *text;

        custom = gtk_table_new (4, 2, FALSE);
        gtk_table_set_col_spacings (GTK_TABLE (custom), 12);
        gtk_table_set_row_spacing (GTK_TABLE (custom), 0, 6);
        gtk_tooltip_set_custom (tooltip, custom);

        widget = gtk_label_new (_("Search for any of the following:"));
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
        gtk_table_attach_defaults (GTK_TABLE (custom), widget, 0, 2, 0, 1);
        gtk_widget_show (widget);

        /* The labels begin with a UTF-8 encoded bullet character. */

        text = g_strdup_printf ("• %s", _("Game Title"));
        widget = gtk_label_new (text);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
        gtk_table_attach_defaults (GTK_TABLE (custom), widget, 0, 1, 1, 2);
        gtk_widget_show (widget);
        g_free (text);

        text = g_strdup_printf ("• %s", _("Manufacturer"));
        widget = gtk_label_new (text);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
        gtk_table_attach_defaults (GTK_TABLE (custom), widget, 0, 1, 2, 3);
        gtk_widget_show (widget);
        g_free (text);

        text = g_strdup_printf ("• %s", _("Copyright Year"));
        widget = gtk_label_new (text);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
        gtk_table_attach_defaults (GTK_TABLE (custom), widget, 0, 1, 3, 4);
        gtk_widget_show (widget);
        g_free (text);

        text = g_strdup_printf ("• %s", _("Category"));
        widget = gtk_label_new (text);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
        gtk_table_attach_defaults (GTK_TABLE (custom), widget, 1, 2, 1, 2);
        gtk_widget_show (widget);
        g_free (text);

        text = g_strdup_printf ("• %s", _("ROM Name"));
        widget = gtk_label_new (text);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
        gtk_table_attach_defaults (GTK_TABLE (custom), widget, 1, 2, 2, 3);
        gtk_widget_show (widget);
        g_free (text);

        text = g_strdup_printf ("• %s", _("Driver Name"));
        widget = gtk_label_new (text);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
        gtk_table_attach_defaults (GTK_TABLE (custom), widget, 1, 2, 3, 4);
        gtk_widget_show (widget);
        g_free (text);

        return TRUE;
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
