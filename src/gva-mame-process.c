#include "gva-mame-process.h"

#include <string.h>
#include <wait.h>

#include "gva-error.h"

static gpointer parent_class = NULL;

static void
mame_process_check_for_error (GvaProcess *process, const gchar *line)
{
        gchar *copy;

        if (process->error != NULL)
                return;

        copy = g_strdup (line);

        if (strlen (g_strstrip (copy)) == 0)
                goto exit;

        if (g_ascii_strncasecmp (copy, "error: ", 7) == 0)
        {
                gchar *error_message;

                error_message = copy + 7;
                *error_message = g_ascii_toupper (*error_message);

                g_set_error (
                        &process->error, GVA_ERROR, GVA_ERROR_SYSTEM,
                        "%s", error_message);
        }

exit:
        g_free (copy);
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
                        &process->error, GVA_ERROR, GVA_ERROR_XMAME,
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
                static const GTypeInfo type_info = {
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

gchar *
gva_mame_process_get_executable (GError **error)
{
        gchar *executable;

        executable = g_find_program_in_path ("xmame");

        if (executable == NULL)
                g_set_error (error, GVA_ERROR, GVA_ERROR_XMAME,
                        _("Could not find xmame executable"));

        return executable;
}

GvaProcess *
gva_mame_process_spawn (const gchar *arguments, GError **error)
{
        GvaProcess *process;
        gchar *command_line;
        gchar *executable;

        g_return_val_if_fail (arguments != NULL, FALSE);

        executable = gva_mame_process_get_executable (error);

        if (executable == NULL)
                return NULL;

        command_line = g_strdup_printf ("%s %s", executable, arguments);

        process = gva_process_spawn (command_line, error);

        g_free (command_line);
        g_free (executable);

        return process;
}
