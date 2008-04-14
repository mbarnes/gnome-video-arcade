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

#include "gva-mame-process.h"

#include <string.h>
#include <wait.h>

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
        /* Check for errors on unread lines. */
        g_strfreev (gva_process_stdout_read_lines (process));
        g_strfreev (gva_process_stderr_read_lines (process));

        if (process->error != NULL)
                return;

        if (WIFEXITED (status) && WEXITSTATUS (status) != 0)
                g_set_error (
                        &process->error, GVA_ERROR, GVA_ERROR_MAME,
                        _("Child process exited with status (%d)"),
                        WEXITSTATUS (status));
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
        GvaProcess *process;
        gchar *command_line;

        g_return_val_if_fail (arguments != NULL, FALSE);

        if (gva_get_debug_flags () & GVA_DEBUG_MAME)
                g_debug ("%s %s", MAME_PROGRAM, arguments);

        command_line = g_strdup_printf ("%s %s", MAME_PROGRAM, arguments);
        process = gva_process_spawn (command_line, priority, error);
        g_free (command_line);

        return process;
}
