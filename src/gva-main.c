/* Copyright 2007-2015 Matthew Barnes
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
#include <string.h>

#include "gva-audit.h"
#include "gva-columns.h"
#include "gva-db.h"
#include "gva-error.h"
#include "gva-mame.h"
#include "gva-tree-view.h"
#include "gva-ui.h"
#include "gva-util.h"

/* The string literals are column names defined in gva-columns.c. */
#define SQL_COMPLETION_LIST \
        "SELECT DISTINCT name, 'name' FROM available UNION " \
        "SELECT DISTINCT bios, 'bios' FROM available UNION " \
        "SELECT DISTINCT category, 'category' FROM available UNION " \
        "SELECT DISTINCT sourcefile, 'sourcefile' FROM available UNION " \
        "SELECT DISTINCT description, 'description' FROM available UNION " \
        "SELECT DISTINCT manufacturer, 'manufacturer' FROM available UNION " \
        "SELECT DISTINCT year, 'year' FROM available;"

#define PROGRESS_BAR_PULSE_INTERVAL_MS 100

/* Entry completion columns */
enum
{
        COLUMN_NAME,
        COLUMN_TEXT,
        COLUMN_TYPE,
        COLUMN_CKEY
};

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

static gboolean
main_entry_completion_match (GtkEntryCompletion *completion,
                             const gchar *key,
                             GtkTreeIter *iter)
{
        GtkTreeModel *model;
        gchar *s1, *s2;
        gboolean match;

        model = gtk_entry_completion_get_model (completion);
        gtk_tree_model_get (model, iter, COLUMN_CKEY, &s1, -1);
        g_return_val_if_fail (s1 != NULL, FALSE);

        s2 = gva_search_collate_key (key);
        match = (strstr (s1, s2) != NULL);

        g_free (s1);
        g_free (s2);

        return match;
}

static gboolean
main_entry_completion_match_selected_cb (GtkEntryCompletion *completion,
                                         GtkTreeModel *model,
                                         GtkTreeIter *iter)
{
        gchar *column_name;
        gchar *search_text;

        gtk_tree_model_get (
                model, iter,
                COLUMN_NAME, &column_name,
                COLUMN_TEXT, &search_text, -1);

        gva_main_set_last_selected_match (column_name, search_text);
        gva_main_execute_search ();
        gtk_widget_grab_focus (GVA_WIDGET_MAIN_TREE_VIEW);

        g_free (column_name);
        g_free (search_text);

        return TRUE;
}

static void
main_menu_item_select_cb (GtkMenuItem *item,
                          GtkAction *action)
{
        gchar *tooltip;

        g_object_get (G_OBJECT (action), "tooltip", &tooltip, NULL);
        if (tooltip != NULL)
                gva_main_statusbar_push (menu_tooltip_cid, "%s", tooltip);
        g_free (tooltip);
}

static void
main_menu_item_deselect_cb (GtkMenuItem *item)
{
        gva_main_statusbar_pop (menu_tooltip_cid);
}

static gboolean
main_progress_bar_pulse_cb (gpointer user_data)
{
        GtkProgressBar *progress_bar = GTK_PROGRESS_BAR (user_data);

        gtk_progress_bar_pulse (progress_bar);

        return TRUE;
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
        GSettings *settings;
        gchar *text;

        settings = gva_get_settings ();

        gva_tree_view_init ();

        gtk_box_pack_start (
                GTK_BOX (GVA_WIDGET_MAIN_VBOX),
                gva_ui_get_managed_widget ("/main-menu"),
                FALSE, FALSE, 0);

        gtk_box_reorder_child (
                GTK_BOX (GVA_WIDGET_MAIN_VBOX),
                gva_ui_get_managed_widget ("/main-menu"), 0);

        gtk_widget_set_sensitive (
                GVA_WIDGET_MAIN_MUTE_BUTTON,
                gva_mame_supports_sound ());

        g_settings_bind (
                settings, GVA_SETTING_SOUND_MUTED,
                GVA_WIDGET_MAIN_MUTE_BUTTON, "muted",
                G_SETTINGS_BIND_DEFAULT);

        gtk_window_move (
                GTK_WINDOW (GVA_WIDGET_MAIN_WINDOW),
                g_settings_get_int (settings, "window-x"),
                g_settings_get_int (settings, "window-y"));

        gtk_window_resize (
                GTK_WINDOW (GVA_WIDGET_MAIN_WINDOW),
                g_settings_get_int (settings, "window-width"),
                g_settings_get_int (settings, "window-height"));

        if (g_settings_get_boolean (settings, "window-maximized"))
                gtk_window_maximize (GTK_WINDOW (GVA_WIDGET_MAIN_WINDOW));

        /* Initialize the search entry. */
        text = gva_main_get_last_search_text ();
        gtk_entry_set_text (GTK_ENTRY (GVA_WIDGET_MAIN_SEARCH_ENTRY), text);
        g_free (text);

        gva_ui_lock ();

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

        gva_main_progress_bar_show ();
        gva_main_progress_bar_set_fraction (0.0);
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
        gva_main_progress_bar_hide ();

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
        gboolean main_loop_quit = FALSE;
        gboolean success = FALSE;
        guint timeout_id = 0;

        context_id = gva_main_statusbar_get_context_id (G_STRFUNC);

        process = gva_audit_roms (error);
        if (process == NULL)
                goto exit;

        process2 = gva_audit_samples (error);
        if (process2 == NULL)
                goto exit;

        gva_main_progress_bar_show ();
        gva_main_progress_bar_set_fraction (0.0);
        gva_main_statusbar_push (context_id, _("Analyzing ROM files..."));

        timeout_id = g_timeout_add (
                PROGRESS_BAR_PULSE_INTERVAL_MS,
                main_progress_bar_pulse_cb,
                GVA_WIDGET_MAIN_PROGRESS_BAR);

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
        gva_main_progress_bar_hide ();

exit:
        if (timeout_id > 0)
                g_source_remove (timeout_id);

        if (process != NULL)
                g_object_unref (process);

        if (process2 != NULL)
                g_object_unref (process2);

        return success;
}

/**
 * gva_main_init_search_completion:
 * @error: return location for a #GError, or %NULL
 *
 * Initializes autocompletion in the search entry.  This must be done
 * <emphasis>after</emphasis> the game database is built and ROMs are
 * analyzed.  If an error occurs, the function returns %FALSE and sets
 * @error.
 *
 * Returns: %TRUE if autocompletion was initialized successfully,
 *          %FALSE if an error occurred
 **/
gboolean
gva_main_init_search_completion (GError **error)
{
        GtkEntryCompletion *completion;
        GtkCellRenderer *renderer;
        GtkListStore *store;
        GtkTreeIter iter;
        GtkEntry *entry;
        sqlite3_stmt *stmt;
        gint errcode;

        GList *list;

        if (!gva_db_prepare (SQL_COMPLETION_LIST, &stmt, error))
                return FALSE;

        store = gtk_list_store_new (
                4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

        while ((errcode = sqlite3_step (stmt)) == SQLITE_ROW)
        {
                GvaGameStoreColumn column_id;
                const gchar *column_name;
                const gchar *column_title;
                const gchar *search_text;
                gchar *collation_key;

                search_text = (const gchar *) sqlite3_column_text (stmt, 0);
                column_name = (const gchar *) sqlite3_column_text (stmt, 1);
                gva_columns_lookup_id (column_name, &column_id);
                column_title = gva_columns_lookup_title (column_id);

                if (search_text == NULL || *search_text == '\0')
                        continue;

                gtk_list_store_append (store, &iter);
                collation_key = gva_search_collate_key (search_text);
                gtk_list_store_set (
                        store, &iter,
                        COLUMN_NAME, column_name,
                        COLUMN_TEXT, search_text,
                        COLUMN_TYPE, column_title,
                        COLUMN_CKEY, collation_key, -1);
                g_free (collation_key);
        }

        sqlite3_finalize (stmt);

        if (errcode != SQLITE_DONE)
        {
                gva_db_set_error (error, 0, NULL);
                g_object_unref (store);
                return FALSE;
        }

        completion = gtk_entry_completion_new ();
        gtk_entry_completion_set_match_func (
                completion, (GtkEntryCompletionMatchFunc)
                main_entry_completion_match, NULL, NULL);
        gtk_entry_completion_set_minimum_key_length (completion, 3);
        gtk_entry_completion_set_model (completion, GTK_TREE_MODEL (store));
        gtk_entry_completion_set_text_column (completion, COLUMN_TEXT);

        g_signal_connect (
                completion, "match-selected",
                G_CALLBACK (main_entry_completion_match_selected_cb), NULL);

        list = gtk_cell_layout_get_cells (GTK_CELL_LAYOUT (completion));
        g_object_set (list->data, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
        g_list_free (list);

        renderer = gtk_cell_renderer_text_new ();
        g_object_set (renderer, "sensitive", FALSE, "xalign", 1.0, NULL);
        gtk_cell_layout_pack_start (
                GTK_CELL_LAYOUT (completion), renderer, FALSE);
        gtk_cell_layout_add_attribute (
                GTK_CELL_LAYOUT (completion), renderer, "text", COLUMN_TYPE);

        entry = GTK_ENTRY (GVA_WIDGET_MAIN_SEARCH_ENTRY);
        gtk_entry_set_completion (entry, completion);

        return TRUE;
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
 * gva_main_cursor_busy:
 *
 * Sets the mouse cursor to busy.  Generally useful before starting a
 * long-running foreground task.
 **/
void
gva_main_cursor_busy (void)
{
        GdkCursor *cursor;
        GdkDisplay *display;
        GtkWidget *widget;
        GdkWindow *window;

        widget = GVA_WIDGET_MAIN_WINDOW;
        window = gtk_widget_get_window (widget);
        display = gtk_widget_get_display (widget);
        cursor = gdk_cursor_new_for_display (display, GDK_WATCH);
        gdk_window_set_cursor (window, cursor);
        g_object_unref (cursor);
}

/**
 * gva_main_cursor_normal:
 *
 * Sets the mouse cursor back to normal.  Generally useful after completing
 * a long-running foreground task.
 **/
void
gva_main_cursor_normal (void)
{
        GtkWidget *widget;
        GdkWindow *window;

        widget = GVA_WIDGET_MAIN_WINDOW;
        window = gtk_widget_get_window (widget);
        gdk_window_set_cursor (window, NULL);
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
        GtkWindow *window;

        window = GTK_WINDOW (GVA_WIDGET_MAIN_WINDOW);
        gtk_window_set_has_resize_grip (window, FALSE);

        gva_main_progress_bar_set_fraction (0.0);
        gtk_widget_show (GVA_WIDGET_MAIN_PROGRESS_BAR);
        gva_main_cursor_busy ();
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
        GtkWindow *window;

        window = GTK_WINDOW (GVA_WIDGET_MAIN_WINDOW);
        gtk_window_set_has_resize_grip (window, TRUE);

        gtk_widget_hide (GVA_WIDGET_MAIN_PROGRESS_BAR);
        gva_main_cursor_normal ();
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
 * gva_main_clear_search:
 *
 * Clears the search entry and, if the Search Results view is active,
 * any results from the previous search.
 **/
void
gva_main_clear_search (void)
{
        GtkWidget *widget;

        widget = GVA_WIDGET_MAIN_SEARCH_ENTRY;
        gtk_entry_set_text (GTK_ENTRY (widget), "");
        gva_main_set_last_selected_match (NULL, NULL);

        if (gva_tree_view_get_selected_view () == 2)
                gva_main_execute_search ();
}

/**
 * gva_main_execute_search:
 *
 * Executes a game database search and configures the main window to
 * display the results.  More precisely, the function saves the search
 * entry contents to GSettings, switches to the Search Results view,
 * forces an update, ensures a row in the resulting game list is selected,
 * and gives focus to the main tree view.
 *
 * The SQL expression used in the database search is retrieved from
 * gva_main_get_search_expression().  It is applied while updating the
 * Search Results view.
 **/
void
gva_main_execute_search (void)
{
        GtkTreeSelection *selection;
        GtkTreeModel *model;
        GtkTreeView *view;
        GtkTreeIter iter;
        GtkEntry *entry;
        gchar *text;

        entry = GTK_ENTRY (GVA_WIDGET_MAIN_SEARCH_ENTRY);
        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);
        selection = gtk_tree_view_get_selection (view);

        /* Save the search entry text. */
        text = g_strdup (gtk_entry_get_text (entry));
        gtk_entry_set_text (entry, g_strstrip (text));
        gva_main_set_last_search_text (text);
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

        /* Select something in the tree view.  Parts of this are
         * copied from gva_tree_view_set_selected_game(). */
        if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        {
                if (gtk_tree_model_get_iter_first (model, &iter))
                {
                        GtkTreePath *path;

                        path = gtk_tree_model_get_path (model, &iter);
                        gtk_tree_view_set_cursor (view, path, NULL, FALSE);
                        gtk_tree_view_scroll_to_cell (
                                view, path, NULL, TRUE, 0.5, 0.0);
                        gtk_tree_path_free (path);
                }
        }
}

/**
 * gva_main_get_last_search_text:
 *
 * Returns the most recent search entry text from either the current or
 * previous session of <emphasis>GNOME Video Arcade</emphasis>.
 *
 * Returns: the most recent search text
 **/
gchar *
gva_main_get_last_search_text (void)
{
        GSettings *settings;
        gchar *text;

        settings = gva_get_settings ();
        text = g_settings_get_string (settings, GVA_SETTING_SEARCH);

        return (text != NULL) ? g_strstrip (text) : g_strdup ("");
}

/**
 * gva_main_set_last_search_text:
 * @text: the search entry text
 *
 * Writes @text to GSettings key <filename>search</filename>.
 *
 * This is used to remember the search entry text from the previous session
 * of <emphasis>GNOME Video Arcade</emphasis> so that the same text can be
 * preset in the search entry at startup.
 **/
void
gva_main_set_last_search_text (const gchar *text)
{
        GSettings *settings;

        g_return_if_fail (text != NULL);

        settings = gva_get_settings ();
        g_settings_set_string (settings, GVA_SETTING_SEARCH, text);
}

/**
 * gva_main_get_last_selected_match:
 * @column_name: return location for the column name
 * @search_text: return location for the search text
 *
 * Returns the most recently selected match in a search completion list
 * from either the current or previous session of <emphasis>GNOME Video
 * Arcade</emphasis>.  If the completion feature was not used in the
 * most recent search, @column_name and @search_text are set to %NULL
 * and the function returns %FALSE.
 *
 * Returns: %TRUE if match values were successfully retrieved from
 *          GSettings, %FALSE otherwise
 **/
gboolean
gva_main_get_last_selected_match (gchar **column_name,
                                  gchar **search_text)
{
        GSettings *settings;

        g_return_val_if_fail (column_name != NULL, FALSE);
        g_return_val_if_fail (search_text != NULL, FALSE);

        settings = gva_get_settings ();

        *column_name = *search_text = NULL;

        g_settings_get (
                settings, GVA_SETTING_SELECTED_MATCH,
                "(ss)", column_name, search_text);

        /* The value may be unset.  Treat it as a failure. */
        if (*column_name == NULL || *search_text == NULL)
                goto fail;

        g_strstrip (*column_name);
        g_strstrip (*search_text);

        /* Both strings must be non-empty. */
        if (**column_name == '\0' || **search_text == '\0')
                goto fail;

        return TRUE;

fail:
        g_free (*column_name);
        g_free (*search_text);

        *column_name = *search_text = NULL;

        return FALSE;
}

/**
 * gva_main_set_last_selected_match:
 * @column_name: the column name of the completion match
 * @search_text: the search text of the completion match
 *
 * Writes @column_name and @search_text to GSettings key
 * <filename>sql-expression</filename> as a string tuple.
 *
 * This is used to remember whether the search results from the previous
 * session of <emphasis>GNOME Video Arcade</emphasis> were the result of
 * selecting a match from the search completion list.  If so, the values
 * are also used to restore the contents of the Search Results view.
 **/
void
gva_main_set_last_selected_match (const gchar *column_name,
                                  const gchar *search_text)
{
        GSettings *settings;

        settings = gva_get_settings ();

        g_settings_set (
                settings, GVA_SETTING_SELECTED_MATCH, "(ss)",
                (column_name != NULL) ? column_name : "",
                (search_text != NULL) ? search_text : "");
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
        gva_main_set_last_selected_match (NULL, NULL);
        gva_main_execute_search ();
        gtk_widget_grab_focus (GVA_WIDGET_MAIN_TREE_VIEW);
}

/**
 * gva_main_search_entry_changed_cb:
 * @entry: the search entry
 *
 * Handler for #GtkEditable::changed signals to the search entry.
 *
 * Updates the sensitivity of the clear search icon.
 **/
void
gva_main_search_entry_changed_cb (GtkEntry *entry)
{
        GtkEntryIconPosition position;
        gboolean sensitive;
        const gchar *text;

        text = gtk_entry_get_text (entry);
        position = GTK_ENTRY_ICON_SECONDARY;
        sensitive = (text != NULL) && (*text != '\0');

        gtk_entry_set_icon_sensitive (entry, position, sensitive);
}

/**
 * gva_main_search_entry_icon_release_cb:
 * @entry: the search entry
 * @position: the position of the clicked icon
 * @event: the button release event
 *
 * Handler for #GtkEntry::icon-release signals to the search entry.
 *
 * If @position is %GTK_ENTRY_ICON_SECONDARY, clears the search entry.
 **/
void
gva_main_search_entry_icon_release_cb (GtkEntry *entry,
                                       GtkEntryIconPosition position,
                                       GdkEvent *event)
{
        if (position == GTK_ENTRY_ICON_SECONDARY)
                gva_main_clear_search ();
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
                if (!gtk_widget_has_focus (GTK_WIDGET (entry)))
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

        custom = gtk_table_new (5, 2, FALSE);
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
        gtk_table_attach_defaults (GTK_TABLE (custom), widget, 0, 1, 4, 5);
        gtk_widget_show (widget);
        g_free (text);

        text = g_strdup_printf ("• %s", _("ROM Name"));
        widget = gtk_label_new (text);
        gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
        gtk_table_attach_defaults (GTK_TABLE (custom), widget, 1, 2, 1, 2);
        gtk_widget_show (widget);
        g_free (text);

        text = g_strdup_printf ("• %s", _("BIOS Name"));
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
 * gva_main_window_configure_event_cb:
 * @window: the main window
 * @event: a #GdkEventConfigure
 *
 * Handler for #GtkWidget::configure-event signals to the main window.
 *
 * Saves the main window state to dconf.
 *
 * Returns: %FALSE always
 **/
gboolean
gva_main_window_configure_event_cb (GtkWindow *window,
                                    GdkEventConfigure *event)
{
        gva_save_window_state (
                window,
                "window-width",
                "window-height",
                "window-maximized",
                "window-x",
                "window-y");

        return FALSE;
}

/**
 * gva_main_window_window_state_event_cb:
 * @window: the main window
 * @event: a #GdkEventWindowState
 *
 * Handler for #GtkWidget::window-state-event signals to the main window.
 *
 * Saves the main window state to dconf.
 *
 * Returns: %FALSE always
 **/
gboolean
gva_main_window_window_state_event_cb (GtkWindow *window,
                                       GdkEventWindowState *event)
{
        gva_save_window_state (
                window,
                "window-width",
                "window-height",
                "window-maximized",
                "window-x",
                "window-y");

        return FALSE;
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
