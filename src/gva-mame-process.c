/* Copyright 2007-2010 Matthew Barnes
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

#include "gva-mame-process.h"

#include <string.h>
#include <sys/wait.h>

#include "gva-error.h"
#include "gva-util.h"

static gpointer parent_class = NULL;

static void
mame_process_check_for_error (GvaProcess *process, const gchar *line)
{
        gchar *copy;

        if (process->error != NULL)
                return;

        copy = g_alloca (strlen (line) + 1);
        strcpy (copy, line);

        if (strlen (g_strstrip (copy)) == 0)
                return;

        if (g_ascii_strncasecmp (copy, "error: ", 7) == 0)
        {
                gchar *error_message;

                error_message = copy + 7;
                *error_message = g_ascii_toupper (*error_message);

                g_set_error (
                        &process->error, GVA_ERROR, GVA_ERROR_MAME,
                        "%s", error_message);
        }
}

static gchar *
mame_process_stdout_read_line (GvaProcess *process)
{
        gchar *line;

        /* Chain up to parent's stdout_read_line() method. */
        line = GVA_PROCESS_CLASS (parent_class)->stdout_read_line (process);

        mame_process_check_for_error (process, line);

        return line;
}

static gchar *
mame_process_stderr_read_line (GvaProcess *process)
{
        gchar *line;

        /* Chain up to parent's stderr_read_line() method. */
        line = GVA_PROCESS_CLASS (parent_class)->stderr_read_line (process);

        mame_process_check_for_error (process, line);

        return line;
}

static void
mame_process_exited (GvaProcess *process, gint status)
{
        if (WIFEXITED (status) && (gva_get_debug_flags () & GVA_DEBUG_MAME))
        {
                GPid pid;

                status = WEXITSTATUS (status);
                pid = gva_process_get_pid (process);
                g_debug ("Process %d exited with status %d", pid, status);
        }
}

static void
mame_process_class_init (GvaProcessClass *class)
{
        GvaProcessClass *process_class;

        parent_class = g_type_class_peek_parent (class);

        process_class = GVA_PROCESS_CLASS (class);
        process_class->stdout_read_line = mame_process_stdout_read_line;
        process_class->stderr_read_line = mame_process_stderr_read_line;
        process_class->exited = mame_process_exited;
}

GType
gva_mame_process_get_type (void)
{
        static GType type = 0;

        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info =
                {
                        sizeof (GvaMameProcessClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) mame_process_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (GvaMameProcess),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        GVA_TYPE_PROCESS, "GvaMameProcess", &type_info, 0);
        }

        return type;
}

/**
 * gva_mame_process_new:
 * @pid: child process ID
 * @priority: priority for the event sources
 * @standard_input: file descriptor for the child's stdin
 * @standard_output: file descriptor for the child's stdout
 * @standard_error: file descriptor for the child's stderr
 *
 * Creates a new #GvaMameProcess from the given parameters.  A #GSource is
 * created at the given @priority for each of the file descriptors.  The
 * internal process value is initialized to zero.
 *
 * Returns: a new #GvaMameProcess
 **/
GvaProcess *
gva_mame_process_new (GPid pid,
                      gint priority,
                      gint standard_input,
                      gint standard_output,
                      gint standard_error)
{
        g_return_val_if_fail (standard_input >= 0, NULL);
        g_return_val_if_fail (standard_output >= 0, NULL);
        g_return_val_if_fail (standard_error >= 0, NULL);

        return g_object_new (
                GVA_TYPE_MAME_PROCESS,
                "pid", pid,
                "priority", priority,
                "stdin", standard_input,
                "stdout", standard_output,
                "stderr", standard_error,
                NULL);
}

/**
 * gva_mame_process_spawn:
 * @arguments: command-line arguments
 * @priority: priority for the event sources
 * @error: return location for a #GError, or %NULL
 *
 * Spawns a child process running MAME with @arguments.  The resulting
 * #GvaProcess instance will monitor the child process for error messages.
 * If an error occurs while spawning, the function returns %NULL and sets
 * @error.
 *
 * Returns: a new #GvaProcess, or %NULL if an error occurred
 **/
GvaProcess *
gva_mame_process_spawn (const gchar *arguments,
                        gint priority,
                        GError **error)
{
        gint standard_input;
        gint standard_output;
        gint standard_error;
        gchar *command_line;
        GPid child_pid;
        gboolean success;

        g_return_val_if_fail (arguments != NULL, FALSE);

        command_line = g_strdup_printf ("%s %s", MAME_PROGRAM, arguments);

        success = gva_spawn_with_pipes (
                command_line, &child_pid, &standard_input,
                &standard_output, &standard_error, error);

        if (gva_get_debug_flags () & GVA_DEBUG_MAME)
                g_debug (
                        "Spawned \"%s\" as process %d",
                        command_line, (gint) child_pid);

        g_free (command_line);

        if (!success)
                return NULL;

        return gva_mame_process_new (
                child_pid, priority, standard_input,
                standard_output, standard_error);
}
