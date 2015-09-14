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

#include "gva-audit.h"

#include <string.h>

#include "gva-columns.h"
#include "gva-db.h"
#include "gva-error.h"
#include "gva-game-store.h"
#include "gva-mame.h"
#include "gva-ui.h"
#include "gva-util.h"

#define SQL_SELECT_BAD_GAMES \
        "SELECT name, description FROM game WHERE romset == 'bad'"

typedef struct _GvaAuditData GvaAuditData;

struct _GvaAuditData
{
        GPtrArray *output;
        GHashTable *output_index;
        GHashTable *status_index;
        const gchar *column;
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

        data = g_slice_new (GvaAuditData);
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

static gboolean
audit_build_model (GtkTreeStore *tree_store,
                   GvaAuditData *data,
                   GError **error)
{
        GtkTreeModel *game_store;
        GtkTreeIter iter;
        gboolean iter_valid;

        game_store = gva_game_store_new_from_query (
                SQL_SELECT_BAD_GAMES, error);
        if (game_store == NULL)
                return FALSE;

        gtk_tree_store_clear (tree_store);

        iter_valid = gtk_tree_model_get_iter_first (game_store, &iter);

        while (iter_valid)
        {
                GtkTreeIter parent;
                GtkTreeIter child;
                guint index;
                gchar *name;
                gchar *description;

                gtk_tree_model_get (
                        game_store, &iter,
                        GVA_GAME_STORE_COLUMN_NAME, &name,
                        GVA_GAME_STORE_COLUMN_DESCRIPTION, &description,
                        -1);

                gtk_tree_store_append (tree_store, &parent, NULL);
                gtk_tree_store_set (tree_store, &parent, 0, description, -1);

                index = GPOINTER_TO_UINT (g_hash_table_lookup (
                        data->output_index, name));

                while (index > 0)
                {
                        const gchar *line;

                        line = g_ptr_array_index (data->output, --index);

                        if (!g_str_has_prefix (line, name))
                                break;

                        gtk_tree_store_prepend (tree_store, &child, &parent);
                        gtk_tree_store_set (tree_store, &child, 0, line, -1);
                }

                g_free (name);
                g_free (description);

                iter_valid = gtk_tree_model_iter_next (game_store, &iter);
        }

        gtk_tree_sortable_set_sort_column_id (
                GTK_TREE_SORTABLE (tree_store), 0, GTK_SORT_ASCENDING);

        g_object_unref (game_store);

        return TRUE;
}

static void
audit_read (GvaProcess *process,
            GvaAuditData *data)
{
        gchar *line;
        gchar *name;
        gchar *status;
        GString *string;
        gpointer value;

        line = g_strchomp (gva_process_stdout_read_line (process));

        value = GUINT_TO_POINTER (data->output->len);
        g_ptr_array_add (data->output, g_strdup (line));

        if (!gva_mame_verify_parse (line, &name, &status))
                goto exit;

        g_hash_table_insert (data->output_index, g_strdup (name), value);

        /* Build a quoted, comma-separated list of games. */
        string = g_hash_table_lookup (data->status_index, status);
        if (string != NULL)
                g_string_append_printf (string, ", \"%s\"", name);
        else
        {
                string = g_string_sized_new (1024);
                g_string_printf (string, "\"%s\"", name);

                g_hash_table_insert (
                        data->status_index,
                        g_strdup (status), string);
        }

        g_free (name);
        g_free (status);

exit:
        g_free (line);
}

/* Helper for audit_exit() */
static void
audit_exit_foreach (gchar *status,
                    GString *names,
                    GvaAuditData *data)
{
        gchar *sql;
        GError *error = NULL;

        sql = g_strdup_printf (
                "UPDATE game SET %s=\"%s\" WHERE name IN (%s)",
                data->column, status, names->str);
        gva_db_execute (sql, &error);
        gva_error_handle (&error);
        g_free (sql);
}

static void
audit_exit (GvaProcess *process,
            gint status,
            GvaAuditData *data)
{
        gchar *sql;
        GError *error = NULL;

        if (process->error != NULL)
                return;

        gva_db_transaction_begin (&error);
        gva_error_handle (&error);

        sql = g_strdup_printf ("UPDATE game SET %s=NULL", data->column);
        gva_db_execute (sql, &error);
        gva_error_handle (&error);
        g_free (sql);

        g_hash_table_foreach (
                data->status_index, (GHFunc) audit_exit_foreach, data);

        gva_db_transaction_commit (&error);
        gva_error_handle (&error);
}

static gchar *
audit_run_save_dialog (void)
{
        const gchar *key = GVA_SETTING_ERROR_FILE;
        GtkFileChooser *file_chooser;
        GtkWidget *dialog;
        GSettings *settings;
        gchar *filename;
        gchar *folder;
        gchar *name;

        settings = gva_get_settings ();

        dialog = gtk_file_chooser_dialog_new (
                _("Save As"),
                GTK_WINDOW (GVA_WIDGET_AUDIT_WINDOW),
                GTK_FILE_CHOOSER_ACTION_SAVE,
                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                NULL);

        file_chooser = GTK_FILE_CHOOSER (dialog);

        /* Suggest the previous filename, if available. */

        filename = g_settings_get_string (settings, key);

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
                g_settings_set_string (settings, key, filename);
        }

        gtk_widget_destroy (dialog);

        return filename;
}

static void
audit_show_dialog (GvaProcess *process,
                   gint status,
                   GvaAuditData *data)
{
        GtkTreeView *tree_view;
        GtkTreeModel *model;
        GError *error = NULL;

        if (process->error != NULL)
                return;

        tree_view = GTK_TREE_VIEW (GVA_WIDGET_AUDIT_TREE_VIEW);
        model = gtk_tree_view_get_model (tree_view);

        if (!audit_build_model (GTK_TREE_STORE (model), data, &error))
        {
                gva_error_handle (&error);
                return;
        }

        if (gtk_tree_model_iter_n_children (model, NULL) > 0)
                gtk_window_present (GTK_WINDOW (GVA_WIDGET_AUDIT_WINDOW));
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

        gtk_tree_model_get (model, iter, 0, &text, -1);
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
        gchar *mame_version;
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

        g_free (mame_version);
        g_string_free (contents, TRUE);
}

/**
 * gva_audit_detect_changes:
 *
 * Attempts to detect ROM and sample file changes since the last program
 * execution by scanning the timestamps on the directories listed in the
 * "rompath" and "samplepath" configuration values.  It returns %TRUE if
 * any of those timestamps are more recent than the game database.
 *
 * Returns: %TRUE if changes were detected
 **/
gboolean
gva_audit_detect_changes (void)
{
        gboolean changes = FALSE;
        gchar **directories;
        struct stat st;
        guint length, ii;
        GError *error = NULL;

        if (g_stat (gva_db_get_filename (), &st) < 0)
                return FALSE;

        directories = gva_mame_get_search_paths ("rompath", &error);
        length = (directories != NULL) ? g_strv_length (directories) : 0;
        gva_error_handle (&error);

        for (ii = 0; ii < length; ii++)
                changes |= gva_db_is_older_than (directories[ii]);

        g_strfreev (directories);

        directories = gva_mame_get_search_paths ("samplepath", &error);
        length = (directories != NULL) ? g_strv_length (directories) : 0;
        gva_error_handle (&error);

        for (ii = 0; ii < length; ii++)
                changes |= gva_db_is_older_than (directories[ii]);

        g_strfreev (directories);

        return changes;
}
