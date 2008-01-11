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

#include "gva-play-back.h"

#include <errno.h>
#include <langinfo.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "gva-columns.h"
#include "gva-db.h"
#include "gva-error.h"
#include "gva-game-store.h"
#include "gva-mame.h"
#include "gva-time.h"
#include "gva-tree-view.h"
#include "gva-ui.h"

#define SQL_DELETE_PLAY_BACK \
        "DELETE FROM playback"

#define SQL_INSERT_PLAY_BACK \
        "INSERT INTO playback VALUES (@name, @inode, @comment)"

static gint
play_back_confirm_deletion (void)
{
        GtkWidget *dialog;
        gint response;

        dialog = gtk_message_dialog_new_with_markup (
                GTK_WINDOW (GVA_WIDGET_PLAY_BACK_WINDOW),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE,
                "<big><b>%s</b></big>",
                _("Delete selected game recordings?"));

        gtk_message_dialog_format_secondary_text (
                GTK_MESSAGE_DIALOG (dialog), "%s",
                _("This operation will permanently erase the recorded games "
                  "you have selected.  Are you sure you want to proceed?"));

        gtk_dialog_add_button (
                GTK_DIALOG (dialog), _("Do Not Delete"), GTK_RESPONSE_REJECT);

        gtk_dialog_add_button (
                GTK_DIALOG (dialog), GTK_STOCK_DELETE, GTK_RESPONSE_ACCEPT);

        response = gtk_dialog_run (GTK_DIALOG (dialog));

        gtk_widget_destroy (dialog);

        return response;
}

static void
play_back_delete_inpfile (GtkTreeRowReference *reference)
{
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;
        gboolean valid;
        gchar *inpfile;

        model = gtk_tree_row_reference_get_model (reference);

        path = gtk_tree_row_reference_get_path (reference);
        valid = gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_path_free (path);
        g_assert (valid);

        gtk_tree_model_get (
                model, &iter, GVA_GAME_STORE_COLUMN_INPFILE, &inpfile, -1);
        g_assert (inpfile != NULL);

        errno = 0;
        if (g_unlink (inpfile) == 0)
                gtk_tree_store_remove (GTK_TREE_STORE (model), &iter);
        else
                g_warning ("%s: %s", inpfile, g_strerror (errno));

        g_free (inpfile);
}

static void
play_back_selection_changed_cb (GtkTreeSelection *tree_selection)
{
        gint selected_rows;

        selected_rows =
                gtk_tree_selection_count_selected_rows (tree_selection);
        gtk_widget_set_sensitive (
                GVA_WIDGET_PLAY_BACK_BUTTON, (selected_rows == 1));
        gtk_widget_set_sensitive (
                GVA_WIDGET_PLAY_BACK_DELETE_BUTTON, (selected_rows >= 1));
}

static void
play_back_add_input_file (const gchar *inpfile,
                          const gchar *name,
                          GvaGameStore *game_store)
{
        GtkTreeIter iter;
        gchar *comment = NULL;
        gchar **result = NULL;
        gchar *inpname;
        gchar *sql;
        gint rows;
        gint columns;
        struct stat st;
        time_t *time;
        GError *error = NULL;

        if (g_stat (inpfile, &st) != 0)
        {
                g_warning ("%s: %s", inpfile, g_strerror (errno));
                return;
        }

        time = &st.st_ctime;

        /* Try to fetch the comment from the database. */
        sql = g_strdup_printf (
                "SELECT comment FROM playback WHERE "
                "name == '%s' AND inode == %" G_GINT64_FORMAT,
                name, (gint64) st.st_ino);
        if (gva_db_get_table (sql, &result, &rows, &columns, &error))
        {
                if (rows > 0)
                        comment = g_strdup (result[1]);
                g_strfreev (result);
        }
        gva_error_handle (&error);
        g_free (sql);

        /* If that fails, use the game title. */
        if (comment == NULL)
        {
                sql = g_strdup_printf (
                        "SELECT description FROM available "
                        "WHERE name == '%s'", name);
                if (gva_db_get_table (sql, &result, &rows, &columns, &error))
                {
                        if (rows > 0)
                                comment = g_strdup (result[1]);
                        g_strfreev (result);
                }
                gva_error_handle (&error);
                g_free (sql);
        }

        /* The game may not be available anymore. */
        if (comment == NULL)
        {
                g_warning ("%s: Game '%s' not found", inpfile, name);
                return;
        }

        gtk_tree_store_append (GTK_TREE_STORE (game_store), &iter, NULL);

        gtk_tree_store_set (
                GTK_TREE_STORE (game_store), &iter,
                GVA_GAME_STORE_COLUMN_NAME, name,
                GVA_GAME_STORE_COLUMN_COMMENT, comment,
                GVA_GAME_STORE_COLUMN_INODE, (gint64) st.st_ino,
                GVA_GAME_STORE_COLUMN_INPFILE, inpfile,
                GVA_GAME_STORE_COLUMN_TIME, time,
                -1);

        inpname = g_strdelimit (g_path_get_basename (inpfile), ".", '\0');
        gva_game_store_index_insert (game_store, inpname, &iter);
        g_free (inpname);

        g_free (comment);
}

/**
 * gva_play_back_init:
 *
 * Initializes the Recorded Games window.
 *
 * This function should be called once when the application starts.
 **/
void
gva_play_back_init (void)
{
        GtkWindow *window;
        GtkTreeView *view;
        GtkTreeViewColumn *column;

        window = GTK_WINDOW (GVA_WIDGET_PLAY_BACK_WINDOW);
        view = GTK_TREE_VIEW (GVA_WIDGET_PLAY_BACK_TREE_VIEW);

        gtk_tree_view_set_model (view, gva_game_store_new ());

        gtk_tree_selection_set_mode (
                gtk_tree_view_get_selection (view),
                GTK_SELECTION_MULTIPLE);

        /* Time Column */
        column = gva_columns_new_from_id (GVA_GAME_STORE_COLUMN_TIME);
        gtk_tree_view_append_column (view, column);

        /* Comment Column */
        column = gva_columns_new_from_id (GVA_GAME_STORE_COLUMN_COMMENT);
        gtk_tree_view_append_column (view, column);

        g_signal_connect (
                gtk_tree_view_get_selection (view), "changed",
                G_CALLBACK (play_back_selection_changed_cb), NULL);

        play_back_selection_changed_cb (gtk_tree_view_get_selection (view));
}

/**
 * gva_play_back_show:
 * @inpname: an input filename or %NULL
 *
 * Refreshes the contents of the Recorded Games window, attempts to
 * select the row corresponding to @inpname (if @inpname is not %NULL),
 * and finally shows the window.
 **/
void
gva_play_back_show (const gchar *inpname)
{
        GHashTable *hash_table;
        GvaGameStore *game_store;
        GtkTreeView *view;
        GError *error = NULL;

        view = GTK_TREE_VIEW (GVA_WIDGET_PLAY_BACK_TREE_VIEW);
        game_store = GVA_GAME_STORE (gtk_tree_view_get_model (view));

        hash_table = gva_mame_get_input_files (&error);
        gva_error_handle (&error);

        if (hash_table != NULL)
        {
                gva_game_store_clear (game_store);
                g_hash_table_foreach (
                        hash_table, (GHFunc) play_back_add_input_file,
                        game_store);
                g_hash_table_destroy (hash_table);
        }

        if (inpname != NULL)
        {
                GtkTreePath *path;

                path = gva_game_store_index_lookup (game_store, inpname);

                if (path != NULL)
                {
                        gtk_tree_view_set_cursor (view, path, NULL, FALSE);
                        gtk_tree_view_scroll_to_cell (
                                view, path, NULL, TRUE, 0.5, 0.0);
                        gtk_widget_grab_focus (GTK_WIDGET (view));
                        gtk_tree_path_free (path);
                }
        }

        gtk_window_present (GTK_WINDOW (GVA_WIDGET_PLAY_BACK_WINDOW));
}

/**
 * gva_play_back_clicked_cb:
 * @button: the "Play Back" button
 *
 * Handler for #GtkButton::clicked signals to the "Play Back" button.
 *
 * Activates the #GVA_ACTION_PLAY_BACK action.
 **/
void
gva_play_back_clicked_cb (GtkButton *button)
{
        gtk_action_activate (GVA_ACTION_PLAY_BACK);
}

/**
 * gva_play_back_close_clicked_cb:
 * @window: the "Recorded Games" window
 * @button: the "Close" button
 *
 * Handler for #GtkButton::clicked signals to the "Close" button.
 *
 * Hides @window.
 **/
void
gva_play_back_close_clicked_cb (GtkWindow *window,
                                GtkButton *button)
{
        gtk_widget_hide (GTK_WIDGET (window));
}

/**
 * gva_play_back_delete_clicked_cb:
 * @view: the "Recorded Games" tree view
 * @button: the "Delete" button
 *
 * Handler for #GtkButton::clicked signals to the "Delete" button.
 *
 * Displays a confirmation dialog, then deletes the MAME input files
 * corresponding to the selected rows in @view.
 *
 **/
void
gva_play_back_delete_clicked_cb (GtkTreeView *view,
                                 GtkButton *button)
{
        GtkTreeModel *model;
        GList *list, *iter;

        if (play_back_confirm_deletion () != GTK_RESPONSE_ACCEPT)
                return;

        list = gtk_tree_selection_get_selected_rows (
                gtk_tree_view_get_selection (view), &model);
        for (iter = list; iter != NULL; iter = iter->next)
        {
                GtkTreePath *path = iter->data;

                iter->data = gtk_tree_row_reference_new (model, path);
                gtk_tree_path_free (path);
        }

        g_list_foreach (list, (GFunc) play_back_delete_inpfile, NULL);
        g_list_foreach (list, (GFunc) gtk_tree_row_reference_free, NULL);
        g_list_free (list);
}

/**
 * gva_play_back_row_activated_cb:
 * @view: the "Recorded Games" tree view
 * @path: the #GtkTreePath for the activated row
 * @column: the #GtkTreeViewColumn in which the activation occurred
 *
 * Handler for #GtkTreeView::row-activated signals to the "Recorded Games"
 * tree view.
 *
 * Activates the #GVA_ACTION_PLAY_BACK action.
 **/
void
gva_play_back_row_activated_cb (GtkTreeView *view,
                                GtkTreePath *path,
                                GtkTreeViewColumn *column)
{
        gtk_action_activate (GVA_ACTION_PLAY_BACK);
}

/**
 * gva_play_back_window_hide_cb:
 * @window: the "Recorded Games" window
 *
 * Handler for #GtkWidget::hide signals to the "Recorded Games" window.
 *
 * Saves the contents of the "Recorded Games" tree view to the game database.
 **/
void
gva_play_back_window_hide_cb (GtkWindow *window)
{
        GtkTreeView *view;
        GtkTreeModel *model;
        GtkTreeIter iter;
        sqlite3_stmt *stmt;
        gboolean valid;
        GError *error = NULL;

        view = GTK_TREE_VIEW (GVA_WIDGET_PLAY_BACK_TREE_VIEW);
        model = gtk_tree_view_get_model (view);
        valid = gtk_tree_model_get_iter_first (model, &iter);

        if (!gva_db_transaction_begin (&error))
                goto exit;

        if (!gva_db_execute (SQL_DELETE_PLAY_BACK, &error))
                goto rollback;

        if (!gva_db_prepare (SQL_INSERT_PLAY_BACK, &stmt, &error))
                goto rollback;

        while (valid)
        {
                gchar *comment;
                gchar *name;
                gchar *utf8;
                gint64 inode;
                gint index;
                gint errcode;

                gtk_tree_model_get (
                        model, &iter,
                        GVA_GAME_STORE_COLUMN_NAME, &name,
                        GVA_GAME_STORE_COLUMN_INODE, &inode,
                        GVA_GAME_STORE_COLUMN_COMMENT, &comment,
                        -1);

                index = sqlite3_bind_parameter_index (stmt, "@name");
                utf8 = g_locale_to_utf8 (name, -1, NULL, NULL, &error);
                gva_error_handle (&error);

                if (utf8 == NULL)
                {
                        g_free (name);
                        g_free (comment);
                        sqlite3_finalize (stmt);
                        goto rollback;
                }

                errcode = sqlite3_bind_text (stmt, index, utf8, -1, g_free);

                if (errcode != SQLITE_OK)
                {
                        g_free (name);
                        g_free (comment);
                        sqlite3_finalize (stmt);
                        goto rollback;
                }

                index = sqlite3_bind_parameter_index (stmt, "@inode");
                errcode = sqlite3_bind_int64 (stmt, index, inode);

                if (errcode != SQLITE_OK)
                {
                        g_free (name);
                        g_free (comment);
                        sqlite3_finalize (stmt);
                        goto rollback;
                }

                index = sqlite3_bind_parameter_index (stmt, "@comment");
                utf8 = g_locale_to_utf8 (comment, -1, NULL, NULL, &error);
                gva_error_handle (&error);

                if (utf8 == NULL)
                {
                        g_free (name);
                        g_free (comment);
                        sqlite3_finalize (stmt);
                        goto rollback;
                }

                errcode = sqlite3_bind_text (stmt, index, utf8, -1, g_free);

                if (errcode != SQLITE_OK)
                {
                        g_free (name);
                        g_free (comment);
                        sqlite3_finalize (stmt);
                        goto rollback;
                }

                g_free (name);
                g_free (comment);

                if (sqlite3_step (stmt) != SQLITE_DONE)
                {
                        gva_db_set_error (&error, 0, NULL);
                        gva_error_handle (&error);
                        sqlite3_finalize (stmt);
                        goto rollback;
                }

                sqlite3_reset (stmt);
                sqlite3_clear_bindings (stmt);

                valid = gtk_tree_model_iter_next (model, &iter);
        }

        sqlite3_finalize (stmt);

        if (!gva_db_transaction_commit (&error))
                goto rollback;

        goto exit;

rollback:
        gva_error_handle (&error);
        gva_db_transaction_rollback (&error);

exit:
        gva_error_handle (&error);
}
