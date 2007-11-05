/* Copyright 2007 Matthew Barnes
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

#include "gva-db.h"
#include "gva-error.h"
#include "gva-mame.h"

#define SQL_DELETE_NOT_FOUND \
        "DELETE FROM game WHERE romset == \"not found\";"

#define SQL_SELECT_BAD_GAMES \
        "SELECT game, description FROM game WHERE romset == 'bad'"

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
                GtkTreePath *path;
                GtkTreeIter child;
                gchar *name;

                gtk_tree_model_get (model, &iter, 0, &name, -1);
                path = gva_game_store_index_lookup (
                        GVA_GAME_STORE (model), name);
                g_assert (path != NULL);
                g_free (name);

                iter_valid = gtk_tree_model_get_iter (model, &iter, path);

                iter_valid = gtk_tree_model_iter_next (model, &iter);
        }

        return model;
}

static void
audit_read (GvaProcess *process,
            GvaAuditData *data)
{
        gchar *line;
        const gchar *name;
        const gchar *status = NULL;
        const gchar *token;
        GString *string;
        gpointer value;

        /* Output for -verifyroms is as follows:
         * 
         * romset puckman is good
         * romset puckmana [puckman] is good
         * romset puckmanf [puckman] is good
         * ...
         * digdug  : 51xx.bin (1024 bytes) - NOT FOUND
         * digdug  : 53xx.bin (1024 bytes) - NOT FOUND
         * romset digdug is bad
         * ...
         *
         * - Status may be "good", "bad", or "best available".
         *
         * - Older MAMEs used "correct" and "incorrect" instead of
         *   "good" and "bad".  Convert to the newer form if seen.
         *
         * - Similar output for -verifysamples.
         */

        line = g_strchomp (gva_process_stdout_read_line (process));

        value = GUINT_TO_POINTER (data->output->len);
        g_ptr_array_add (data->output, line);

        if ((token = strtok (line, " ")) == NULL)
                return;

        if (strcmp (token, data->column) != 0)
                return;

        name = strtok (NULL, " ");
        while ((token = strtok (NULL, " ")) != NULL)
                status = token;

        if (name == NULL || status == NULL)
                return;

        /* Normalize the status. */
        if (strcmp (status, "correct") == 0)
                status = "good";
        else if (strcmp (status, "incorrect") == 0)
                status = "bad";

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

        gva_process_inc_progress (process);
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
        GError *error = NULL;

        if (process->error != NULL)
                return;

        gva_db_transaction_begin (&error);
        gva_error_handle (&error);

        g_hash_table_foreach (
                data->status_index, (GHFunc) audit_exit_foreach, data);

        /* This part only really applies to romsets, but I suppose there's
         * no harm in executing it twice. */
        gva_db_execute (SQL_DELETE_NOT_FOUND, &error);
        gva_error_handle (&error);

        gva_db_transaction_commit (&error);
        gva_error_handle (&error);
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

        process = gva_mame_verify_roms (error);
        if (process == NULL)
                return NULL;

        data = audit_data_new ("romset");

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

        process = gva_mame_verify_roms (error);
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
