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

#include "gva-audit.h"

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "gva-columns.h"
#include "gva-db.h"
#include "gva-error.h"
#include "gva-game-store.h"
#include "gva-mame.h"
#include "gva-tree-view.h"
#include "gva-ui.h"

#define SQL_SELECT_BAD_GAMES \
        "SELECT name, description FROM game WHERE romset == 'bad'"

typedef struct _GvaAuditData GvaAuditData;
typedef struct _GvaScanResults GvaScanResults;

struct _GvaAuditData
{
        GPtrArray *output;
        GHashTable *output_index;
        GHashTable *status_index;
        const gchar *column;
};

struct _GvaScanResults
{
        GSList *verify;
        GSList *delete;
};

static void
audit_string_free (GString *string)
{
        g_string_free (string, TRUE);
}

static GvaAuditData *
audit_data_new (const gchar *column)
{
        GvaAuditData *data;
        GHashTable *output_index;
        GHashTable *status_index;

        output_index = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) NULL);

        status_index = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) audit_string_free);

        data = g_slice_new0 (GvaAuditData);
        data->output = g_ptr_array_new ();
        data->output_index = output_index;
        data->status_index = status_index;
        data->column = column;

        return data;
}

static void
audit_data_free (GvaAuditData *data)
{
        g_ptr_array_foreach (data->output, (GFunc) g_free, NULL);
        g_ptr_array_free (data->output, TRUE);
        g_hash_table_destroy (data->output_index);
        g_hash_table_destroy (data->status_index);
        g_slice_free (GvaAuditData, data);
}

static GvaScanResults *
audit_scan_results_new (void)
{
        return g_slice_new0 (GvaScanResults);
}

static void
audit_scan_results_free (GvaScanResults *results)
{
        g_slist_foreach (results->verify, (GFunc) g_free, NULL);
        g_slist_foreach (results->delete, (GFunc) g_free, NULL);
        g_slist_free (results->verify);
        g_slist_free (results->delete);
        g_slice_free (GvaScanResults, results);
}

static void
audit_scan_results_add_to_verify_list (GvaScanResults *results,
                                       const gchar *name)
{
        results->verify = g_slist_prepend (results->verify, g_strdup (name));
}

static void
audit_scan_results_add_to_delete_list (GvaScanResults *results,
                                       const gchar *name)
{
        results->delete = g_slist_prepend (results->delete, g_strdup (name));
}

static void
audit_scan_results_reverse (GvaScanResults *results)
{
        results->verify = g_slist_reverse (results->verify);
        results->delete = g_slist_reverse (results->delete);
}

/* A "status index" is a certain type of hash table used to update the
 * "romset" and "sampleset" fields in the game database.  The hash key
 * is a quoted status value (or a NULL literal), and the hash value is
 * a GString containing a comma-separated list of quoted game names. */

static void
audit_status_index_commit_foreach (const gchar *status,
                                   const GString *names,
                                   const gchar *column)
{
        gchar *sql;
        GError *error = NULL;

        /* Both status and the name list values are already
         * quoted, except when status is SQL literal NULL. */
        sql = g_strdup_printf (
                "UPDATE game SET %s=%s WHERE name IN (%s)",
                column, status, names->str);
        gva_db_execute (sql, &error);
        gva_error_handle (&error);
        g_free (sql);
}

static void
audit_status_index_commit (GHashTable *index,
                           const gchar *column)
{
        g_hash_table_foreach (
                index, (GHFunc) audit_status_index_commit_foreach,
                (gpointer) column);
}

static void
audit_status_index_insert (GHashTable *index,
                           const gchar *name,
                           const gchar *status)
{
        GString *string;
        gchar *key = "NULL";

        /* Keys are quoted to allow for SQL literal NULL. */
        if (status != NULL)
        {
                key = g_alloca (strlen (status) + 3);
                g_sprintf (key, "\"%s\"", status);
        }

        /* Build a quoted, comma-separated list of games. */
        string = g_hash_table_lookup (index, key);
        if (string != NULL)
                g_string_append_printf (string, ", \"%s\"", name);
        else
        {
                string = g_string_sized_new (1024);
                g_string_printf (string, "\"%s\"", name);
                g_hash_table_insert (index, g_strdup (key), string);
        }
}

static GtkTreeModel *
audit_build_model (GvaAuditData *data,
                   GError **error)
{
        GtkTreeModel *model;
        GtkTreeIter iter;
        gboolean iter_valid;

        model = gva_game_store_new_from_query (SQL_SELECT_BAD_GAMES, error);
        if (model == NULL)
                return NULL;

        iter_valid = gtk_tree_model_get_iter_first (model, &iter);

        while (iter_valid)
        {
                GtkTreeIter child;
                const gchar *line;
                guint index;
                gchar *name;

                gtk_tree_model_get (model, &iter, 0, &name, -1);
                index = GPOINTER_TO_UINT (g_hash_table_lookup (
                        data->output_index, name));

                line = g_ptr_array_index (data->output, --index);
                while (g_str_has_prefix (line, name))
                {
                        gtk_tree_store_prepend (
                                GTK_TREE_STORE (model), &child, &iter);
                        gtk_tree_store_set (
                                GTK_TREE_STORE (model), &child,
                                GVA_GAME_STORE_COLUMN_DESCRIPTION, line, -1);
                        line = g_ptr_array_index (data->output, --index);
                }

                g_free (name);

                iter_valid = gtk_tree_model_iter_next (model, &iter);
        }

        gtk_tree_sortable_set_sort_column_id (
                GTK_TREE_SORTABLE (model),
                GVA_GAME_STORE_COLUMN_DESCRIPTION, GTK_SORT_ASCENDING);

        return model;
}

static void
audit_read (GvaProcess *process,
            GvaAuditData *data)
{
        gchar *line;
        gchar *name;
        gchar *status;
        gpointer value;

        line = g_strchomp (gva_process_stdout_read_line (process));

        value = GUINT_TO_POINTER (data->output->len);
        g_ptr_array_add (data->output, g_strdup (line));

        if (!gva_mame_verify_parse (line, &name, &status))
                goto exit;

        g_hash_table_insert (data->output_index, g_strdup (name), value);
        audit_status_index_insert (data->status_index, name, status);

        gva_process_inc_progress (process);

        g_free (name);
        g_free (status);

exit:
        g_free (line);
}

static void
audit_exit (GvaProcess *process,
            gint status,
            GvaAuditData *data)
{
        GError *error = NULL;

        if (process->error != NULL)
                return;

        gva_db_transaction_begin (&error);
        gva_error_handle (&error);

        audit_status_index_commit (data->status_index, data->column);

        gva_db_transaction_commit (&error);
        gva_error_handle (&error);
}

static gchar *
audit_run_save_dialog (void)
{
        const gchar *key = GVA_GCONF_ERROR_FILE_KEY;
        GtkFileChooser *file_chooser;
        GtkWidget *dialog;
        GConfClient *client;
        gchar *filename;
        gchar *folder;
        gchar *name;
        GError *error = NULL;

        client = gconf_client_get_default ();

        dialog = gtk_file_chooser_dialog_new (
                _("Save As"),
                GTK_WINDOW (GVA_WIDGET_AUDIT_WINDOW),
                GTK_FILE_CHOOSER_ACTION_SAVE,
                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                NULL);

        file_chooser = GTK_FILE_CHOOSER (dialog);

        /* Suggest the previous filename, if available. */

        filename = gconf_client_get_string (client, key, &error);
        gva_error_handle (&error);

        if (filename != NULL && *filename != '\0')
        {
                name = g_path_get_basename (filename);
                folder = g_path_get_dirname (filename);
        }
        else
        {
                name = g_strdup ("rom-errors.txt");
                folder = g_strdup (g_get_home_dir ());
        }

        gtk_file_chooser_set_current_folder (file_chooser, folder);
        gtk_file_chooser_set_current_name (file_chooser, name);
        gtk_file_chooser_set_do_overwrite_confirmation (file_chooser, TRUE);

        g_free (name);
        g_free (folder);
        g_free (filename);

        filename = NULL;

        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
        {
                filename = gtk_file_chooser_get_filename (file_chooser);
                gconf_client_set_string (client, key, filename, &error);
                gva_error_handle (&error);
        }

        gtk_widget_destroy (dialog);
        g_object_unref (client);

        return filename;
}

static void
audit_show_dialog (GvaProcess *process,
                   gint status,
                   GvaAuditData *data)
{
        GtkTreeView *view;
        GtkTreeModel *model;
        GError *error = NULL;

        model = audit_build_model (data, &error);
        gva_error_handle (&error);
        if (model == NULL)
                return;

        view = GTK_TREE_VIEW (GVA_WIDGET_AUDIT_TREE_VIEW);
        gtk_tree_view_set_model (view, model);

        if (gtk_tree_model_iter_n_children (model, NULL) > 0)
                gtk_window_present (GTK_WINDOW (GVA_WIDGET_AUDIT_WINDOW));
}

static GvaScanResults *
audit_scan_files (const gchar *config_value,
                  const gchar *column_name,
                  GError **error)
{
        /* FIXME We're not taking clones into consideration.  If a game
         *       is added, deleted or modified, we'll want to update the
         *       status of all clones of that game.
         *
         *       Probably need to rethink the data structures we're using
         *       to store scan results.  Instead of lists, maybe a single
         *       hash table of names-to-filenames would work better.
         *
         *       Might also need to build a data structure to represent
         *       parent/clone relationships.  Start with this:
         *
         *       SELECT cloneof, name FROM game WHERE cloneof != "";
         */

        GvaScanResults *results;
        gboolean scan_needed = FALSE;
        gchar **directories;
        GHashTable *index;
        sqlite3_stmt *stmt;
        time_t db_mtime;
        struct stat st;
        gchar *sql;
        gint errcode;
        guint ii;

        /* This does not get propagated. */
        GError *recoverable_error = NULL;

        /* Get the database modification time. */
        if (stat (gva_db_get_filename (), &st) == 0)
                db_mtime = st.st_mtime;
        else
        {
                g_set_error (
                        error, G_FILE_ERROR,
                        g_file_error_from_errno (errno),
                        "%s", g_strerror (errno));
                return NULL;
        }

        /* Ask MAME for a list of directories. */
        directories = gva_mame_get_search_paths (config_value, error);
        if (directories == NULL)
                return NULL;

        /* Compare directory modification times against the database
         * modification time.  If any of the directories were modified
         * more recently than the database, assume a scan is needed. */
        for (ii = 0; directories[ii] != NULL; ii++)
                if (stat (directories[ii], &st) == 0)
                {
                        scan_needed |= (db_mtime < st.st_mtime);
                        scan_needed |= (db_mtime < st.st_ctime);
                }

        results = audit_scan_results_new ();

        /* If a scan is not needed we can exit early.  The scan results
         * will indicate that nothing has changed. */
        if (!scan_needed)
                goto exit;

        index = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) g_free);

        /* Build an index of directory contents to simulate searching the
         * directory list for a file.  Deal with collisions by iterating
         * over the directories in reverse order. */
        while (ii > 0)
        {
                const gchar *basename;
                const gchar *dirname;
                GDir *dir;

                dirname = directories[--ii];

                if (!g_file_test (dirname, G_FILE_TEST_IS_DIR))
                        continue;

                /* Failure to open a directory is recoverable,
                 * but at least emit a warning about it. */
                dir = g_dir_open (dirname, 0, &recoverable_error);
                gva_error_handle (&recoverable_error);

                if (dir == NULL)
                        continue;

                while ((basename = g_dir_read_name (dir)) != NULL)
                {
                        gchar *key, *value;

                        /* Suppose basename is "pacman.zip", then
                         * key = "pacman", value = "/path/to/pacman.zip" */
                        key = g_strdelimit (g_strdup (basename), ".", '\0');
                        value = g_build_filename (dirname, basename, NULL);
                        g_hash_table_insert (index, key, value);
                }

                g_dir_close (dir);
        }

        g_strfreev (directories);

        /* So far so good.  Now ask the game database for the last
         * known status of each file so we can compare notes. */
        sql = g_strdup_printf ("SELECT name, %s FROM game", column_name);
        if (!gva_db_prepare (sql, &stmt, error))
        {
                audit_scan_results_free (results);
                g_hash_table_destroy (index);
                g_free (sql);
                return NULL;
        }
        g_free (sql);

#define MSG_FORMAT "%s '%s' was recently %s"

        /* Compare notes.  If we discover a discrepency, emit a message
         * stating so and add the file to the appropriate list in the scan
         * results.  Note: The column name also serves as a description of
         * the type of files we're scanning. */
        while ((errcode = sqlite3_step (stmt)) == SQLITE_ROW)
        {
                const gchar *name;
                const gchar *status;
                const gchar *filename;

                name = (const gchar *) sqlite3_column_text (stmt, 0);
                status = (const gchar *) sqlite3_column_text (stmt, 1);
                filename = g_hash_table_lookup (index, name);

                /* Was the file recently added? */
                if (status == NULL && filename != NULL)
                {
                        g_message (MSG_FORMAT, column_name, name, "added");
                        audit_scan_results_add_to_verify_list (results, name);
                }

                /* Was the file recently deleted? */
                else if (status != NULL && filename == NULL)
                {
                        g_message (MSG_FORMAT, column_name, name, "deleted");
                        audit_scan_results_add_to_delete_list (results, name);
                }

                /* Was the file recently modified? */
                else if (stat (filename, &st) == 0 && db_mtime < st.st_mtime)
                {
                        g_message (MSG_FORMAT, column_name, name, "modified");
                        audit_scan_results_add_to_verify_list (results, name);
                }
        }

#undef MSG_FORMAT

        g_hash_table_destroy (index);
        sqlite3_finalize (stmt);

        if (errcode != SQLITE_DONE)
        {
                gva_db_set_error (error, 0, NULL);
                audit_scan_results_free (results);
                return NULL;
        }

        audit_scan_results_reverse (results);

exit:
        return results;
}

static gboolean
audit_process_scan_results (GvaScanResults *results,
                            const gchar *column_name,
                            GError **error)
{
        GHashTable *index;
        GSList *link;

        index = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) audit_string_free);

        for (link = results->delete; link != NULL; link = link->next)
        {
                const gchar *name = link->data;
                audit_status_index_insert (index, name, NULL);
        }

        for (link = results->verify; link != NULL; link = link->next)
        {
                const gchar *name = link->data;
                gchar *status;

                status = gva_mame_verify_roms (name, error);
                if (status == NULL)
                {
                        g_hash_table_destroy (index);
                        return FALSE;
                }
                else if (strcmp (status, "not found") == 0)
                        audit_status_index_insert (index, name, NULL);
                else if (strcmp (status, "not supported") == 0)
                        audit_status_index_insert (index, name, NULL);
                else
                        audit_status_index_insert (index, name, status);

                g_free (status);
        }

        audit_status_index_commit (index, column_name);
        g_hash_table_destroy (index);

        return TRUE;
}

/**
 * gva_audit_init:
 *
 * Initializes the ROM audit window.
 *
 * This function should be called once when the application starts.
 **/
void
gva_audit_init (void)
{
        GtkTreeViewColumn *column;
        GtkTreeView *view;

        view = GTK_TREE_VIEW (GVA_WIDGET_AUDIT_TREE_VIEW);
        column = gva_columns_new_from_id (GVA_GAME_STORE_COLUMN_DESCRIPTION);
        gtk_tree_view_append_column (view, column);

        gtk_action_connect_proxy (
                GVA_ACTION_SAVE_ERRORS,
                GVA_WIDGET_AUDIT_SAVE_BUTTON);
}

/**
 * gva_audit_roms:
 * @error: return location for a #GError, or %NULL
 *
 * Starts the lengthy process of auditing the integrity of the available
 * ROM sets and returns a #GvaProcess to track it.  The results of the
 * audit are written to the "romset" column of the game database.  If an
 * error occurs while starting the audit, it returns %NULL and sets @error.
 *
 * Returns: a new #GvaProcess, or %NULL
 **/
GvaProcess *
gva_audit_roms (GError **error)
{
        GvaProcess *process;
        GvaAuditData *data;

        process = gva_mame_verify_all_roms (error);
        if (process == NULL)
                return NULL;

        data = audit_data_new ("romset");

        g_signal_connect (
                process, "stdout-ready",
                G_CALLBACK (audit_read), data);

        g_signal_connect (
                process, "exited",
                G_CALLBACK (audit_exit), data);

        g_signal_connect (
                process, "exited",
                G_CALLBACK (audit_show_dialog), data);

        g_signal_connect_swapped (
                process, "exited",
                G_CALLBACK (audit_data_free), data);

        return process;
}

/**
 * gva_audit_samples:
 * @error: return location for a #GError, or %NULL
 *
 * Starts the length process of auditing the integrity of the available
 * sample sets and returns a #GvaProcess to track it.  The results of the
 * audit are written to the "sampleset" column of the game database.  If
 * an error occurs while starting the audit, it returns %NULL and sets
 * @error.
 *
 * Returns: a new #GvaProcess, or %NULL
 **/
GvaProcess *
gva_audit_samples (GError **error)
{
        GvaProcess *process;
        GvaAuditData *data;

        process = gva_mame_verify_all_samples (error);
        if (process == NULL)
                return NULL;

        data = audit_data_new ("sampleset");

        g_signal_connect (
                process, "stdout-ready",
                G_CALLBACK (audit_read), data);

        g_signal_connect (
                process, "exited",
                G_CALLBACK (audit_exit), data);

        g_signal_connect_swapped (
                process, "exited",
                G_CALLBACK (audit_data_free), data);

        return process;
}

/* Helper for audit_save_errors() */
static gboolean
audit_save_errors_foreach (GtkTreeModel *model,
                           GtkTreePath *path,
                           GtkTreeIter *iter,
                           GString *contents)
{
        gchar *text;

        gtk_tree_model_get (
                model, iter, GVA_GAME_STORE_COLUMN_DESCRIPTION, &text, -1);
        if (gtk_tree_path_get_depth (path) > 1)
                g_string_append_len (contents, "  ", 2);
        g_string_append_printf (contents, "%s\n", text);
        g_free (text);

        return FALSE;
}

/**
 * gva_audit_save_errors:
 *
 * Saves the results of the most recent ROM file audit to a file.
 **/
void
gva_audit_save_errors (void)
{
        GtkTreeView *view;
        GtkTreeModel *model;
        GString *contents;
        const gchar *mame_version;
        gchar *filename;
        GError *error = NULL;

        view = GTK_TREE_VIEW (GVA_WIDGET_AUDIT_TREE_VIEW);
        model = gtk_tree_view_get_model (view);
        g_return_if_fail (model != NULL);

        mame_version = gva_mame_get_version (&error);
        gva_error_handle (&error);

        /* Build the contents of the file. */
        contents = g_string_sized_new (4096);
        g_string_append_printf (
                contents, "%s - ROM Audit Results\n", PACKAGE_STRING);
        if (mame_version != NULL)
                g_string_append_printf (contents, "Using %s\n", mame_version);
        else
                g_string_append (contents, "Using unknown M.A.M.E. version\n");
        g_string_append_c (contents, '\n');
        gtk_tree_model_foreach (
                model, (GtkTreeModelForeachFunc)
                audit_save_errors_foreach, contents);

        /* Prompt the user for a filename. */
        filename = audit_run_save_dialog ();
        if (filename != NULL)
        {
                g_file_set_contents (filename, contents->str, -1, &error);
                gva_error_handle (&error);
                g_free (filename);
        }

        g_string_free (contents, TRUE);
}

GvaAuditResult
gva_quick_audit (GError **error)
{
        GvaAuditResult audit_result;
        GvaScanResults *rom_results = NULL;
        GvaScanResults *sample_results = NULL;
        guint need_to_verify;

        audit_result = GVA_AUDIT_RESULT_ERROR;

        rom_results = audit_scan_files ("rompath", "romset", error);
        if (rom_results == NULL)
                goto exit;

        sample_results = audit_scan_files ("samplepath", "sampleset", error);
        if (sample_results == NULL)
                goto exit;

        need_to_verify =
                g_slist_length (rom_results->verify) +
                g_slist_length (sample_results->verify);

        if (need_to_verify > 100)
        {
                audit_result = GVA_AUDIT_RESULT_TOO_MANY;
                goto exit;
        }

        if (!audit_process_scan_results (rom_results, "romset", error))
                goto exit;

        if (!audit_process_scan_results (sample_results, "sampleset", error))
                goto exit;

        audit_result = GVA_AUDIT_RESULT_SUCCESS;

exit:
        if (rom_results != NULL)
                audit_scan_results_free (rom_results);
        if (sample_results != NULL)
                audit_scan_results_free (sample_results);

        return audit_result;
}
