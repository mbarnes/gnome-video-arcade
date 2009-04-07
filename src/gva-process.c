/* Copyright 2007-2009 Matthew Barnes
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

#include "gva-process.h"

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

#include "gva-error.h"
#include "gva-util.h"

#define GVA_PROCESS_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE \
        ((obj), GVA_TYPE_PROCESS, GvaProcessPrivate))

enum {
        PROP_0,
        PROP_PID,
        PROP_STDIN,
        PROP_STDOUT,
        PROP_STDERR,
        PROP_PRIORITY,
        PROP_PROGRESS
};

enum {
        STDOUT_READY,
        STDERR_READY,
        EXITED,
        LAST_SIGNAL
};

struct _GvaProcessPrivate
{
        GPid pid;  /* XXX assume this is a gint, not portable */
        gint priority;
        GIOChannel *stdin_channel;
        GIOChannel *stdout_channel;
        GIOChannel *stderr_channel;
        GQueue *stdout_lines;
        GQueue *stderr_lines;

        guint child_source_id;
        guint stdout_source_id;
        guint stderr_source_id;

        GTimeVal start_time;

        guint progress;
        gboolean exited;
        gint status;
};

static gpointer parent_class = NULL;
static guint signals[LAST_SIGNAL] = { 0 };
static GSList *active_list = NULL;

static GIOChannel *
process_new_channel (gint fd)
{
        GIOChannel *channel = NULL;

        if (fd > 0)
        {
                GError *error = NULL;

                channel = g_io_channel_unix_new (fd);
                g_io_channel_set_close_on_unref (channel, TRUE);
                g_io_channel_set_encoding (channel, NULL, &error);
                gva_error_handle (&error);
        }

        return channel;
}

static void
process_propagate_error (GvaProcess *process, GError *error)
{
        if (error != NULL)
        {
                if (process->error == NULL)
                        g_propagate_error (&process->error, error);
                else
                        g_error_free (error);
        }
}

static void
process_source_removed (GvaProcess *process)
{
        gint n_active_sources = 0;

        g_assert (!process->priv->exited);

        n_active_sources += (process->priv->child_source_id > 0);
        n_active_sources += (process->priv->stdout_source_id > 0);
        n_active_sources += (process->priv->stderr_source_id > 0);

        if (n_active_sources == 0)
        {
                GSList *link;

                process->priv->exited = TRUE;

                g_signal_emit (
                        process, signals[EXITED], 0,
                        process->priv->status);

                link = g_slist_find (active_list, process);
                g_assert (link != NULL);
                g_object_unref (link->data);
                active_list = g_slist_delete_link (active_list, link);
        }
}

static void
process_exited (GPid pid,
                gint status,
                GvaProcess *process)
{
        process->priv->child_source_id = 0;
        process->priv->status = status;

        g_spawn_close_pid (pid);

        if (WIFSIGNALED (status))
        {
                GError *error = NULL;

                g_set_error (
                        &error, GVA_ERROR, GVA_ERROR_SYSTEM,
                        "Child process terminated: %s",
                        g_strsignal (WTERMSIG (status)));
                process_propagate_error (process, error);
        }
}

static void
process_read_line (GvaProcess *process,
                   GIOChannel *channel,
                   GQueue *queue,
                   guint signal_id)
{
        GIOStatus status;
        gchar *line;
        GError *error = NULL;

        status = g_io_channel_read_line (channel, &line, NULL, NULL, &error);

        if (status == G_IO_STATUS_NORMAL)
        {
                g_assert (line != NULL);
                g_queue_push_tail (queue, line);
                g_signal_emit (process, signal_id, 0);
        }
        else
                process_propagate_error (process, error);
}

static gboolean
process_stdout_ready (GIOChannel *channel,
                      GIOCondition condition,
                      GvaProcess *process)
{
        if (condition & G_IO_IN)
        {
                /* For better performance, keep reading lines as long as
                 * there's more data available.  This assumes the stderr
                 * pipe is relatively quiet.  Otherwise the stderr pipe
                 * could overflow and cause a deadlock between this and
                 * the child process. */

                do
                {
                        process_read_line (
                                process, channel,
                                process->priv->stdout_lines,
                                signals[STDOUT_READY]);

                        /* Break immediately if we have a G_IO_HUP. */
                        condition = (condition & G_IO_HUP) |
                                g_io_channel_get_buffer_condition (channel);
                }
                while (condition == G_IO_IN);

                return TRUE;
        }

        process->priv->stdout_source_id = 0;

        return FALSE;
}

static gboolean
process_stderr_ready (GIOChannel *channel,
                      GIOCondition condition,
                      GvaProcess *process)
{
        if (condition & G_IO_IN)
        {
                /* Do NOT loop here, as we do for stdout. */

                process_read_line (
                        process, channel,
                        process->priv->stderr_lines,
                        signals[STDERR_READY]);

                return TRUE;
        }

        process->priv->stderr_source_id = 0;

        return FALSE;
}

static void
process_debug_message (GvaProcess *process,
                       const gchar *line,
                       const gchar *sep)
{
        GPid pid;
        gchar *copy;

        /* Copy the string just to remove the newline. */
        pid = gva_process_get_pid (process);
        copy = g_strchomp (g_strdup (line));

        g_debug ("Process %d %s %s", (gint) pid, sep, copy);

        g_free (copy);
}

static GObject *
process_constructor (GType type,
                     guint n_construct_properties,
                     GObjectConstructParam *construct_properties)
{
        GvaProcessPrivate *priv;
        GObject *object;

        /* Chain up to parent's constructor() method. */
        object = G_OBJECT_CLASS (parent_class)->constructor (
                type, n_construct_properties, construct_properties);

        priv = GVA_PROCESS_GET_PRIVATE (object);

        priv->child_source_id = g_child_watch_add_full (
                priv->priority, priv->pid,
                (GChildWatchFunc) process_exited, object,
                (GDestroyNotify) process_source_removed);

        priv->stdout_source_id = g_io_add_watch_full (
                priv->stdout_channel, priv->priority,
                G_IO_IN | G_IO_HUP, (GIOFunc) process_stdout_ready,
                object, (GDestroyNotify) process_source_removed);

        priv->stderr_source_id = g_io_add_watch_full (
                priv->stderr_channel, priv->priority,
                G_IO_IN | G_IO_HUP, (GIOFunc) process_stderr_ready,
                object, (GDestroyNotify) process_source_removed);

        return object;
}

static void
process_set_property (GObject *object,
                      guint property_id,
                      const GValue *value,
                      GParamSpec *pspec)
{
        GvaProcessPrivate *priv = GVA_PROCESS_GET_PRIVATE (object);
        gint fd;

        switch (property_id)
        {
                case PROP_PID:
                        /* XXX Not portable */
                        priv->pid = g_value_get_uint (value);
                        return;

                case PROP_STDIN:
                        fd = g_value_get_int (value);
                        priv->stdin_channel = process_new_channel (fd);
                        return;

                case PROP_STDOUT:
                        fd = g_value_get_int (value);
                        priv->stdout_channel = process_new_channel (fd);
                        return;

                case PROP_STDERR:
                        fd = g_value_get_int (value);
                        priv->stderr_channel = process_new_channel (fd);
                        return;

                case PROP_PRIORITY:
                        priv->priority = g_value_get_int (value);
                        return;

                case PROP_PROGRESS:
                        gva_process_set_progress (
                                GVA_PROCESS (object),
                                g_value_get_uint (value));
                        return;
        }

        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);

}

static void
process_get_property (GObject *object,
                      guint property_id,
                      GValue *value,
                      GParamSpec *pspec)
{
        GvaProcessPrivate *priv = GVA_PROCESS_GET_PRIVATE (object);
        GIOChannel *channel;

        switch (property_id)
        {
                case PROP_PID:
                        /* XXX Not portable */
                        g_value_set_uint (value, priv->pid);
                        return;

                case PROP_STDIN:
                        channel = priv->stdin_channel;
                        g_value_set_int (
                                value, (channel != NULL) ?
                                g_io_channel_unix_get_fd (channel) : -1);
                        return;

                case PROP_STDOUT:
                        channel = priv->stdout_channel;
                        g_value_set_int (
                                value, (channel != NULL) ?
                                g_io_channel_unix_get_fd (channel) : -1);
                        return;

                case PROP_STDERR:
                        channel = priv->stderr_channel;
                        g_value_set_int (
                                value, (channel != NULL) ?
                                g_io_channel_unix_get_fd (channel) : -1);
                        return;

                case PROP_PRIORITY:
                        g_value_set_int (value, priv->priority);
                        return;

                case PROP_PROGRESS:
                        g_value_set_uint (
                                value, gva_process_get_progress (
                                GVA_PROCESS (object)));
                        return;
        }

        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
process_finalize (GObject *object)
{
        GvaProcess *process = GVA_PROCESS (object);

        gva_error_handle (&process->error);

        if (process->priv->stdin_channel != NULL)
                g_io_channel_unref (process->priv->stdin_channel);
        if (process->priv->stdout_channel != NULL)
                g_io_channel_unref (process->priv->stdout_channel);
        if (process->priv->stderr_channel != NULL)
                g_io_channel_unref (process->priv->stderr_channel);

        while (!g_queue_is_empty (process->priv->stdout_lines))
                g_free (g_queue_pop_head (process->priv->stdout_lines));

        while (!g_queue_is_empty (process->priv->stderr_lines))
                g_free (g_queue_pop_head (process->priv->stderr_lines));

        g_queue_free (process->priv->stdout_lines);
        g_queue_free (process->priv->stderr_lines);

        /* Chain up to parent's finalize() method. */
        G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gchar *
process_stdout_read_line (GvaProcess *process)
{
        return g_queue_pop_head (process->priv->stdout_lines);
}

static gchar *
process_stderr_read_line (GvaProcess *process)
{
        return g_queue_pop_head (process->priv->stderr_lines);
}

static void
process_class_init (GvaProcessClass *class)
{
        GObjectClass *object_class;

        parent_class = g_type_class_peek_parent (class);
        g_type_class_add_private (class, sizeof (GvaProcessPrivate));

        object_class = G_OBJECT_CLASS (class);
        object_class->constructor = process_constructor;
        object_class->set_property = process_set_property;
        object_class->get_property = process_get_property;
        object_class->finalize = process_finalize;

        class->stdout_read_line = process_stdout_read_line;
        class->stderr_read_line = process_stderr_read_line;

        /**
         * GvaProcess:pid:
         *
         * The ID of the child process.
         **/
        g_object_class_install_property (
                object_class,
                PROP_PID,
                g_param_spec_uint (
                        "pid",
                        NULL,
                        NULL,
                        0, G_MAXUINT, 0,
                        G_PARAM_READWRITE |
                        G_PARAM_CONSTRUCT_ONLY));

        /**
         * GvaProcess:stdin:
         *
         * The file descriptor for the child process' stdin pipe.
         **/
        g_object_class_install_property (
                object_class,
                PROP_STDIN,
                g_param_spec_int (
                        "stdin",
                        NULL,
                        NULL,
                        -1, G_MAXINT, -1,
                        G_PARAM_READWRITE |
                        G_PARAM_CONSTRUCT_ONLY));

        /**
         * GvaProcess:stdout:
         *
         * The file descriptor for the child process' stdout pipe.
         **/
        g_object_class_install_property (
                object_class,
                PROP_STDOUT,
                g_param_spec_int (
                        "stdout",
                        NULL,
                        NULL,
                        -1, G_MAXINT, -1,
                        G_PARAM_READWRITE |
                        G_PARAM_CONSTRUCT_ONLY));

        /**
         * GvaProcess:stderr:
         *
         * The file descriptor for the child process' stderr pipe.
         **/
        g_object_class_install_property (
                object_class,
                PROP_STDERR,
                g_param_spec_int (
                        "stderr",
                        NULL,
                        NULL,
                        -1, G_MAXINT, -1,
                        G_PARAM_READWRITE |
                        G_PARAM_CONSTRUCT_ONLY));

        /**
         * GvaProcess:priority:
         *
         * Priority of the event sources that watch for incoming data.
         **/
        g_object_class_install_property (
                object_class,
                PROP_PRIORITY,
                g_param_spec_int (
                        "priority",
                        NULL,
                        NULL,
                        G_MININT, G_MAXINT,
                        G_PRIORITY_DEFAULT_IDLE,
                        G_PARAM_READWRITE |
                        G_PARAM_CONSTRUCT_ONLY));

        /**
         * GvaProcess:progress:
         *
         * Progress value, the meaning of which is defined by the
         * application.
         **/
        g_object_class_install_property (
                object_class,
                PROP_PROGRESS,
                g_param_spec_uint (
                        "progress",
                        NULL,
                        NULL,
                        0, G_MAXUINT, 0,
                        G_PARAM_READWRITE));

        /**
         * GvaProcess::stdout-ready:
         * @process: the #GvaProcess that received the signal
         *
         * The ::stdout-ready signal is emitted when one or more lines
         * from the child process' stdout pipe are available for reading.
         **/
        signals[STDOUT_READY] = g_signal_new (
                "stdout-ready",
                G_TYPE_FROM_CLASS (class),
                G_SIGNAL_RUN_LAST,
                G_STRUCT_OFFSET (GvaProcessClass, stdout_ready),
                NULL, NULL,
                g_cclosure_marshal_VOID__VOID,
                G_TYPE_NONE, 0);

        /**
         * GvaProcess::stderr-ready:
         * @process: the #GvaProcess that received the signal
         *
         * The ::stderr-ready signal is emitted when one or more lines
         * from the child process' stderr pipe are available for reading.
         **/
        signals[STDERR_READY] = g_signal_new (
                "stderr-ready",
                G_TYPE_FROM_CLASS (class),
                G_SIGNAL_RUN_LAST,
                G_STRUCT_OFFSET (GvaProcessClass, stderr_ready),
                NULL, NULL,
                g_cclosure_marshal_VOID__VOID,
                G_TYPE_NONE, 0);

        /**
         * GvaProcess::exited:
         * @process: the #GvaProcess that received the signal
         * @status: the exit status of the child process
         *
         * The ::exited signal is emitted when the child process exits.
         **/
        signals[EXITED] = g_signal_new (
                "exited",
                G_TYPE_FROM_CLASS (class),
                G_SIGNAL_RUN_FIRST,
                G_STRUCT_OFFSET (GvaProcessClass, exited),
                NULL, NULL,
                g_cclosure_marshal_VOID__INT,
                G_TYPE_NONE, 1,
                G_TYPE_INT);
}

static void
process_init (GvaProcess *process)
{
        process->priv = GVA_PROCESS_GET_PRIVATE (process);

        process->priv->stdout_lines = g_queue_new ();
        process->priv->stderr_lines = g_queue_new ();

        g_get_current_time (&process->priv->start_time);
        active_list = g_slist_prepend (active_list, g_object_ref (process));
}

GType
gva_process_get_type (void)
{
        static GType type = 0;

        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info =
                {
                        sizeof (GvaProcessClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) process_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (GvaProcess),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) process_init,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        G_TYPE_OBJECT, "GvaProcess", &type_info, 0);
        }

        return type;
}

/**
 * gva_process_new:
 * @pid: child process ID
 * @priority: priority for the event sources
 * @standard_input: file descriptor for the child's stdin
 * @standard_output: file descriptor for the child's stdout
 * @standard_error: file descriptor for the child's stderr
 *
 * Creates a new #GvaProcess from the given parameters.  A #GSource is
 * created at the given @priority for each of the file descriptors.  The
 * internal progress value is initialized to zero.
 *
 * Returns: a new #GvaProcess
 **/
GvaProcess *
gva_process_new (GPid pid,
                 gint priority,
                 gint standard_input,
                 gint standard_output,
                 gint standard_error)
{
        g_return_val_if_fail (standard_input >= 0, NULL);
        g_return_val_if_fail (standard_output >= 0, NULL);
        g_return_val_if_fail (standard_error >= 0, NULL);

        return g_object_new (
                GVA_TYPE_PROCESS,
                "pid", pid,
                "priority", priority,
                "stdin", standard_input,
                "stdout", standard_output,
                "stderr", standard_error,
                NULL);
}

/**
 * gva_process_spawn:
 * @command_line: a command line
 * @priority: priority for the event sources
 * @error: return location for a #GError, or %NULL
 *
 * Spawns a child process with @command_line and returns the resulting
 * #GvaProcess.  If an error occurs while spawning, the function returns
 * %NULL and sets @error.
 *
 * Returns: a new #GvaProcess, or %NULL if an error occurred
 **/
GvaProcess *
gva_process_spawn (const gchar *command_line,
                   gint priority,
                   GError **error)
{
        gint standard_input;
        gint standard_output;
        gint standard_error;
        GPid child_pid;
        gboolean success;

        g_return_val_if_fail (command_line != NULL, NULL);

        success = gva_spawn_with_pipes (
                command_line, &child_pid, &standard_input,
                &standard_output, &standard_error, error);

        if (!success)
                return NULL;

        return gva_process_new (
                child_pid, priority, standard_input,
                standard_output, standard_error);
}

/**
 * gva_process_get_pid:
 * @process: a #GvaProcess
 *
 * Returns the process ID for @process.
 *
 * Returns: process ID for @process
 **/
GPid
gva_process_get_pid (GvaProcess *process)
{
        g_return_val_if_fail (GVA_IS_PROCESS (process), (GPid) 0);

        return process->priv->pid;
}

/**
 * gva_process_write_stdin:
 * @process: a #GvaProcess
 * @data: the data to write to the child process
 * @length: length of the data, or -1 if the data is nul-terminated
 * @error: return location for a #GError, or %NULL
 *
 * Writes @data to the stdin pipe of a child process represented by @process.
 * If an error occurs, it returns %FALSE and sets @error.
 *
 * Returns: %TRUE on success, %FALSE if an error occurred
 **/
gboolean
gva_process_write_stdin (GvaProcess *process,
                         const gchar *data,
                         gssize length,
                         GError **error)
{
        GIOStatus status = G_IO_STATUS_AGAIN;
        gsize bytes_written;

        g_return_val_if_fail (GVA_IS_PROCESS (process), FALSE);
        g_return_val_if_fail (data != NULL, FALSE);

        if (gva_get_debug_flags () & GVA_DEBUG_IO)
                process_debug_message (process, data, "<<<");

        while (status == G_IO_STATUS_AGAIN)
                status = g_io_channel_write_chars (
                        process->priv->stdin_channel, data,
                        length, &bytes_written, error);
        if (status == G_IO_STATUS_NORMAL)
        {
                if (length == -1)
                        length = strlen (data);
                g_assert (bytes_written == length);

                status = G_IO_STATUS_AGAIN;
                while (status == G_IO_STATUS_AGAIN)
                        status = g_io_channel_flush (
                                process->priv->stdin_channel, error);
        }
        g_assert (status != G_IO_STATUS_EOF);

        return (status == G_IO_STATUS_NORMAL);
}

/**
 * gva_process_stdout_num_lines:
 * @process: a #GvaProcess
 *
 * Returns the number of lines available for reading from the stdout pipe
 * of the child process represented by @process.
 *
 * Returns: number of lines available for reading
 **/
guint
gva_process_stdout_num_lines (GvaProcess *process)
{
        g_return_val_if_fail (GVA_IS_PROCESS (process), 0);

        return g_queue_get_length (process->priv->stdout_lines);
}

/**
 * gva_process_stderr_num_lines:
 * @process: a #GvaProcess
 *
 * Returns the number of lines available for reading from the stderr pipe
 * of the child process represented by @process.
 *
 * Returns: number of lines available for reading
 **/
guint
gva_process_stderr_num_lines (GvaProcess *process)
{
        g_return_val_if_fail (GVA_IS_PROCESS (process), 0);

        return g_queue_get_length (process->priv->stderr_lines);
}

/**
 * gva_process_stdout_read_line:
 * @process: a #GvaProcess
 *
 * Reads a line from the stdout pipe of the child process represented by
 * @process.  This function does not block; it returns %NULL if no lines
 * are available.  Use gva_process_stdout_num_lines() to peek at whether
 * any lines are available.  The line should be freed with g_free() when
 * no longer needed.
 *
 * Returns: a line from the child process' stdout, or %NULL
 **/
gchar *
gva_process_stdout_read_line (GvaProcess *process)
{
        GvaProcessClass *class;
        gchar *line;

        g_return_val_if_fail (GVA_IS_PROCESS (process), NULL);

        class = GVA_PROCESS_GET_CLASS (process);
        g_return_val_if_fail (class->stdout_read_line != NULL, NULL);
        line = class->stdout_read_line (process);

        if (gva_get_debug_flags () & GVA_DEBUG_IO)
                process_debug_message (process, line, ">>>");

        return line;
}

/**
 * gva_process_stderr_read_line:
 * @process: a #GvaProcess
 *
 * Reads a line from the stderr pipe of the child process represented by
 * @process.  This function does not block; it returns %NULL if no lines
 * are available.  Use gva_process_stderr_num_lines() to peek at whether
 * any lines are available.  The line should be freed with g_free() when
 * no longer needed.
 *
 * Returns: a line from the child process' stderr, or %NULL
 **/
gchar *
gva_process_stderr_read_line (GvaProcess *process)
{
        GvaProcessClass *class;
        gchar *line;

        g_return_val_if_fail (GVA_IS_PROCESS (process), NULL);

        class = GVA_PROCESS_GET_CLASS (process);
        g_return_val_if_fail (class->stderr_read_line != NULL, NULL);
        line = class->stderr_read_line (process);

        if (gva_get_debug_flags () & GVA_DEBUG_IO)
                process_debug_message (process, line, "!!!");

        return line;
}

/**
 * gva_process_stdout_read_lines:
 * @process: a #GvaProcess
 *
 * Returns a %NULL-terminated array of lines from the stdout pipe of the
 * child process represented by @process.  This function does not block;
 * it returns %NULL if no lines are available.  Use
 * gva_process_stdout_num_lines() to peek at the number of lines available.
 * The array should be freed with g_strfreev() when no longer needed.
 *
 * Returns: a %NULL-terminated array of lines from the child process'
 *          stdout, or %NULL
 **/
gchar **
gva_process_stdout_read_lines (GvaProcess *process)
{
        guint num_lines, ii;
        gchar **lines = NULL;

        g_return_val_if_fail (GVA_IS_PROCESS (process), NULL);

        num_lines = gva_process_stdout_num_lines (process);

        if (num_lines > 0)
        {
                lines = g_new0 (gchar *, num_lines + 1);

                for (ii = 0; ii < num_lines; ii++)
                {
                        lines[ii] = gva_process_stdout_read_line (process);
                        g_assert (lines[ii] != NULL);
                }
        }

        return lines;
}

/**
 * gva_process_stderr_read_lines:
 * @process: a #GvaProcess
 *
 * Returns a %NULL-terminated array of lines from the stderr pipe of the
 * child process represented by @process.  This function does not block;
 * it returns %NULL if no lines are available.  Use
 * gva_process_stderr_num_lines() to peek at the number of lines available.
 * The array should be freed with g_strfreev() when no longer needed.
 *
 * Returns: a %NULL-terminated array of lines from the child process'
 *          stderr, or %NULL
 **/
gchar **
gva_process_stderr_read_lines (GvaProcess *process)
{
        guint num_lines, ii;
        gchar **lines = NULL;

        g_return_val_if_fail (GVA_IS_PROCESS (process), NULL);

        num_lines = gva_process_stderr_num_lines (process);

        if (num_lines > 0)
        {
                lines = g_new0 (gchar *, num_lines + 1);

                for (ii = 0; ii < num_lines; ii++)
                {
                        lines[ii] = gva_process_stderr_read_line (process);
                        g_assert (lines[ii] != NULL);
                }
        }

        return lines;
}

/**
 * gva_process_get_progress:
 * @process: a #GvaProcess
 *
 * Returns the current progress value for @process.  It is up to the
 * application to set this value using gva_process_set_progress() or
 * gva_process_inc_progress().
 *
 * Returns: progress value
 **/
guint
gva_process_get_progress (GvaProcess *process)
{
        g_return_val_if_fail (GVA_IS_PROCESS (process), 0);

        return process->priv->progress;
}

/**
 * gva_process_inc_progress:
 * @process: a #GvaProcess
 *
 * Increments the progress value for @process.  The progress value is
 * just a counter; it is up to the application to establish an upper
 * bound for the value.
 **/
void
gva_process_inc_progress (GvaProcess *process)
{
        g_return_if_fail (GVA_IS_PROCESS (process));

        process->priv->progress++;
        g_object_notify (G_OBJECT (process), "progress");
}

/**
 * gva_process_set_progress:
 * @process: a #GvaProcess
 * @progress: progress value
 *
 * Sets the progress value for @process.  The progress value is just a
 * counter; it is up to the application to establish an upper bound for
 * the value.
 **/
void
gva_process_set_progress (GvaProcess *process,
                          guint progress)
{
        g_return_if_fail (GVA_IS_PROCESS (process));

        process->priv->progress = progress;
        g_object_notify (G_OBJECT (process), "progress");
}

/**
 * gva_process_has_exited:
 * @process: a #GvaProcess
 * @status: return location for the exit status, or %NULL
 *
 * Returns %TRUE if the child process represented by @process has exited
 * and writes the exit status to the location pointed to by @status, if
 * @status is non-%NULL.  There may still be lines available for reading
 * even after the child process has exited.
 *
 * Returns: %TRUE if the child process has exited, %FALSE otherwise
 **/
gboolean
gva_process_has_exited (GvaProcess *process, gint *status)
{
        g_return_val_if_fail (GVA_IS_PROCESS (process), FALSE);

        if (process->priv->exited && status != NULL)
                *status = process->priv->status;

        return process->priv->exited;
}

/**
 * gva_process_kill:
 * @process: a #GvaProcess
 *
 * Kills the child process represented by @process by sending it a
 * "kill" signal.
 **/
void
gva_process_kill (GvaProcess *process)
{
        GPid pid;

        g_return_if_fail (GVA_IS_PROCESS (process));

        /* XXX Using SIGKILL here because xmame appears to ignore SIGINT
         *     and SIGTERM.  A friendlier approach for an arbitrary process
         *     might be to use SIGTERM first.  Then, if it hasn't exited
         *     after a reasonable duration, SIGKILL. */

        pid = process->priv->pid;
        g_message ("Sending KILL signal to process %d.", pid);
        if (kill (pid, SIGKILL) < 0)
                g_warning ("%s", g_strerror (errno));
}

/**
 * gva_process_kill_all:
 *
 * Kills all active child processes represented by #GvaProcess instances
 * by sending them "kill" signals.
 **/
void
gva_process_kill_all (void)
{
        g_slist_foreach (active_list, (GFunc) gva_process_kill, NULL);
}

/**
 * gva_process_get_time_elapsed:
 * @process: a #GvaProcess
 * @time_elapsed: location to put the time elapsed
 *
 * Writes the time elapsed since @process (the #GvaProcess instance, not
 * necessarily the child process it represents) was created to @time_elapsed.
 **/
void
gva_process_get_time_elapsed (GvaProcess *process, GTimeVal *time_elapsed)
{
        g_return_if_fail (GVA_IS_PROCESS (process));
        g_return_if_fail (time_elapsed != NULL);

        gva_get_time_elapsed (&process->priv->start_time, time_elapsed);
}
