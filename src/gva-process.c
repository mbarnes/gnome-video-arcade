#include "gva-process.h"

#include <string.h>
#include <wait.h>

struct _GvaProcess
{
        GIOChannel *stdin_channel;
        GIOChannel *stdout_channel;
        GIOChannel *stderr_channel;
        GString *stdout_buffer;
        GString *stderr_buffer;
        guint stdout_source_id;
        guint stderr_source_id;

        GvaProcessDataNotify on_stdout;
        GvaProcessDataNotify on_stderr;
        GvaProcessExitNotify on_exit;
        gpointer user_data;

        gboolean exited;
        gint exit_status;
};

static void
process_exited (GPid pid,
                gint exit_status,
                GvaProcess *process)
{
        GIOStatus status;
        gchar *data;
        gsize length;
        GError *error = NULL;

        if (process->stdout_source_id != 0)
        {
                g_source_remove (process->stdout_source_id);
                process->stdout_source_id = 0;
        }

        if (process->stderr_source_id != 0)
        {
                g_source_remove (process->stderr_source_id);
                process->stderr_source_id = 0;
        }

        status = G_IO_STATUS_AGAIN;
        while (status == G_IO_STATUS_AGAIN)
                status = g_io_channel_read_to_end (
                        process->stdout_channel, &data, &length, &error);
        if (status == G_IO_STATUS_NORMAL)
        {
                g_string_append_len (process->stdout_buffer, data, length);
                g_free (data);

                if (process->on_stdout != NULL)
                        process->on_stdout (process, process->user_data);
        }
        else  /* G_IO_STATUS_ERROR */
        {
                g_assert (error != NULL);
                g_warning ("%s", error->message);
                g_clear_error (&error);
        }

        status = G_IO_STATUS_AGAIN;
        while (status == G_IO_STATUS_AGAIN)
                status = g_io_channel_read_to_end (
                        process->stderr_channel, &data, &length, &error);
        if (status == G_IO_STATUS_NORMAL)
        {
                g_string_append_len (process->stderr_buffer, data, length);
                g_free (data);

                if (process->on_stderr != NULL)
                        process->on_stderr (process, process->user_data);
        }
        else  /* G_IO_STATUS_ERROR */
        {
                g_assert (error != NULL);
                g_warning ("%s", error->message);
                g_clear_error (&error);
        }

        process->exited = TRUE;
        process->exit_status = exit_status;
        g_spawn_close_pid (pid);

        if (process->on_exit != NULL)
                process->on_exit (process, process->user_data);
}

static gboolean
process_data_ready (GvaProcess *process,
                    GIOChannel *source,
                    GIOCondition condition,
                    GString *string,
                    guint *source_id,
                    GvaProcessDataNotify notify,
                    gpointer user_data)
{
        if (condition & G_IO_IN)
        {
                GIOStatus status = G_IO_STATUS_AGAIN;
                gchar *line;
                gsize length;
                GError *error = NULL;

                while (status == G_IO_STATUS_AGAIN)
                        status = g_io_channel_read_line (
                                source, &line, &length, NULL, &error);
                switch (status)
                {
                        case G_IO_STATUS_NORMAL:
                                g_string_append_len (string, line, length);
                                g_free (line);
                                if (notify != NULL)
                                        notify (process, user_data);
                                break;

                        case G_IO_STATUS_ERROR:
                                g_assert (error != NULL);
                                g_warning ("%s", error->message);
                                g_error_free (error);
                                break;

                        case G_IO_STATUS_EOF:
                                g_assert (condition & G_IO_HUP);
                                break;

                        default:
                                g_assert_not_reached ();
                }
        }

        if (condition & G_IO_HUP)
                *source_id = 0;

        /* XXX What to do if G_IO_ERR is set? */

        return (*source_id != 0);
}

static gboolean
process_stdout_ready (GIOChannel *source,
                      GIOCondition condition,
                      GvaProcess *process)
{
        return process_data_ready (
                process, source, condition, process->stdout_buffer,
                &process->stdout_source_id, process->on_stdout,
                process->user_data);
}

static gboolean
process_stderr_ready (GIOChannel *source,
                      GIOCondition condition,
                      GvaProcess *process)
{
        return process_data_ready (
                process, source, condition, process->stderr_buffer,
                &process->stderr_source_id, process->on_stderr,
                process->user_data);
}

GvaProcess *
gva_process_new (GPid pid,
                 gint standard_input,
                 gint standard_output,
                 gint standard_error,
                 GvaProcessDataNotify on_stdout,
                 GvaProcessDataNotify on_stderr,
                 GvaProcessExitNotify on_exit,
                 gpointer user_data)
{
        GvaProcess *process;

        process = g_slice_new (GvaProcess);
        process->stdin_channel = g_io_channel_unix_new (standard_input);
        process->stdout_channel = g_io_channel_unix_new (standard_output);
        process->stderr_channel = g_io_channel_unix_new (standard_error);
        process->stdout_buffer = g_string_sized_new (1024);
        process->stderr_buffer = g_string_sized_new (1024);
        process->on_stdout = on_stdout;
        process->on_stderr = on_stderr;
        process->on_exit = on_exit;
        process->user_data = user_data;
        process->exited = FALSE;
        process->exit_status = -1;

        g_io_channel_set_close_on_unref (process->stdin_channel, TRUE);
        g_io_channel_set_close_on_unref (process->stdout_channel, TRUE);
        g_io_channel_set_close_on_unref (process->stderr_channel, TRUE);

        g_child_watch_add (pid, (GChildWatchFunc) process_exited, process);

        process->stdout_source_id = g_io_add_watch (
                process->stdout_channel, G_IO_IN | G_IO_ERR | G_IO_HUP,
                (GIOFunc) process_stdout_ready, process);
        process->stderr_source_id = g_io_add_watch (
                process->stderr_channel, G_IO_IN | G_IO_ERR | G_IO_HUP,
                (GIOFunc) process_stderr_ready, process);

        return process;
}

gboolean
gva_process_write_stdin (GvaProcess *process,
                         const gchar *data,
                         gssize length,
                         GError **error)
{
        GIOStatus status = G_IO_STATUS_AGAIN;
        gsize bytes_written;

        while (status == G_IO_STATUS_AGAIN)
                status = g_io_channel_write_chars (
                        process->stdin_channel, data,
                        length, &bytes_written, error);
        if (status == G_IO_STATUS_NORMAL)
        {
                if (length == -1)
                        length = strlen (data);
                g_assert (bytes_written == length);

                status = G_IO_STATUS_AGAIN;
                while (status == G_IO_STATUS_AGAIN)
                        status = g_io_channel_flush (
                                process->stdin_channel, error);
        }
        g_assert (status != G_IO_STATUS_EOF);

        return (status == G_IO_STATUS_NORMAL);
}

const gchar *
gva_process_peek_stdout (GvaProcess *process)
{
        g_return_val_if_fail (process != NULL, NULL);

        return process->stdout_buffer->str;
}

const gchar *
gva_process_peek_stderr (GvaProcess *process)
{
        g_return_val_if_fail (process != NULL, NULL);

        return process->stderr_buffer->str;
}

gchar *
gva_process_read_stdout (GvaProcess *process)
{
        gchar *data;

        g_return_val_if_fail (process != NULL, NULL);

        data = g_strdup (gva_process_peek_stdout (process));
        g_string_truncate (process->stdout_buffer, 0);

        return data;
}

gchar *
gva_process_read_stderr (GvaProcess *process)
{
        gchar *data;

        g_return_val_if_fail (process != NULL, NULL);

        data = g_strdup (gva_process_peek_stderr (process));
        g_string_truncate (process->stderr_buffer, 0);

        return data;
}

gboolean
gva_process_has_exited (GvaProcess *process, gint *exit_status)
{
        g_return_val_if_fail (process != NULL, FALSE);

        if (process->exited && exit_status != NULL)
                *exit_status = process->exit_status;

        return process->exited;
}

void
gva_process_free (GvaProcess *process)
{
        g_return_if_fail (process != NULL);

        g_io_channel_unref (process->stdin_channel);
        g_io_channel_unref (process->stdout_channel);
        g_io_channel_unref (process->stderr_channel);
        g_string_free (process->stdout_buffer, TRUE);
        g_string_free (process->stderr_buffer, TRUE);

        g_slice_free (GvaProcess, process);
}
