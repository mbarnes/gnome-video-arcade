#include "gva-process.h"

#include <string.h>
#include <wait.h>

#define READ_BUFFER_SIZE (65536) /* 16K */

#define GVA_PROCESS_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE \
        ((obj), GVA_TYPE_PROCESS, GvaProcessPrivate))

enum {
        PROP_0,
        PROP_PID,
        PROP_STDIN,
        PROP_STDOUT,
        PROP_STDERR,
        PROP_EXITED,
        PROP_STATUS
};

enum {
        STDOUT_READY,
        STDERR_READY,
        EXITED,
        LAST_SIGNAL
};

typedef struct _GvaProcessPrivate GvaProcessPrivate;

struct _GvaProcessPrivate
{
        GPid pid;  /* XXX assume this is a gint, not portable */
        GIOChannel *stdin_channel;
        GIOChannel *stdout_channel;
        GIOChannel *stderr_channel;
        GQueue *stdout_lines;
        GQueue *stderr_lines;

        guint child_source_id;
        guint stdout_source_id;
        guint stderr_source_id;

        gboolean exited;
        gint status;
};

static gpointer parent_class = NULL;
static guint signals[LAST_SIGNAL] = { 0 };

static void
process_source_removed (GvaProcess *process)
{
        GvaProcessPrivate *priv = GVA_PROCESS_GET_PRIVATE (process);
        gint n_active_sources = 0;

        g_assert (!priv->exited);

        n_active_sources += (priv->child_source_id > 0);
        n_active_sources += (priv->stdout_source_id > 0);
        n_active_sources += (priv->stderr_source_id > 0);

        if (n_active_sources == 0)
        {
                priv->exited = TRUE;
                g_signal_emit (process, signals[EXITED], 0);
        }
}

static void
process_exited (GPid pid,
                gint status,
                GvaProcess *process)
{
        GvaProcessPrivate *priv = GVA_PROCESS_GET_PRIVATE (process);

        priv->child_source_id = 0;
        priv->status = status;

        g_spawn_close_pid (pid);
}

static gboolean
process_data_ready (GvaProcess *process,
                    GIOChannel *channel,
                    GIOCondition condition,
                    GQueue *queue,
                    guint *source_id,
                    guint signal_id)
{
        while (condition & G_IO_IN || condition & G_IO_HUP)
        {
                GIOStatus status;
                gchar *line;
		gsize terminator_pos;
                GError *error = NULL;

                status = g_io_channel_read_line (
                        channel, &line, NULL, &terminator_pos, &error);
                switch (status)
                {
                        case G_IO_STATUS_NORMAL:
                                g_assert (line != NULL);
				line[terminator_pos] = '\0';
                                g_queue_push_tail (queue, line);
                                g_signal_emit (process, signal_id, 0);
                                break;

                        case G_IO_STATUS_ERROR:
                                g_assert (error != NULL);
                                g_warning ("%s", error->message);
                                g_clear_error (&error);
                                break;

                        case G_IO_STATUS_EOF:
				g_assert (condition & G_IO_HUP);
				condition &= ~G_IO_HUP;  /* clear HUP */
				*source_id = 0;
				break;

                        case G_IO_STATUS_AGAIN:
                                break;
                }

		condition &= ~G_IO_IN;  /* clear IN */
		condition |= g_io_channel_get_buffer_condition (channel);
        }

        /* XXX What to do if G_IO_ERR is set? */

        return (*source_id != 0);
}

static gboolean
process_stdout_ready (GIOChannel *source,
                      GIOCondition condition,
                      GvaProcess *process)
{
        GvaProcessPrivate *priv = GVA_PROCESS_GET_PRIVATE (process);

        return process_data_ready (
                process, source, condition, priv->stdout_lines,
                &priv->stdout_source_id, signals[STDOUT_READY]);
}

static gboolean
process_stderr_ready (GIOChannel *source,
                      GIOCondition condition,
                      GvaProcess *process)
{
        GvaProcessPrivate *priv = GVA_PROCESS_GET_PRIVATE (process);

        return process_data_ready (
                process, source, condition, priv->stderr_lines,
                &priv->stderr_source_id, signals[STDERR_READY]);
}

static void
process_set_property (GObject *object,
                      guint property_id,
                      const GValue *value,
                      GParamSpec *pspec)
{
        GvaProcessPrivate *priv = GVA_PROCESS_GET_PRIVATE (object);
        GIOChannel *channel;
        gint fd;
        GError *error = NULL;

#define CHECK_FOR_ERROR \
        if (error != NULL) g_error ("%s: %s", G_STRLOC, error->message)

        switch (property_id)
        {
                case PROP_PID:
                        /* XXX Not portable */
                        priv->pid = g_value_get_uint (value);
                        g_assert (priv->child_source_id == 0);
                        priv->child_source_id = g_child_watch_add_full (
                                G_PRIORITY_DEFAULT, priv->pid,
                                (GChildWatchFunc) process_exited, object,
                                (GDestroyNotify) process_source_removed);
                        return;

                case PROP_STDIN:
                        if ((fd = g_value_get_int (value)) < 0)
                                return;
                        channel = g_io_channel_unix_new (fd);
                        g_io_channel_set_close_on_unref (channel, TRUE);
                        g_io_channel_set_encoding (channel, NULL, &error);
                        CHECK_FOR_ERROR;
                        g_assert (priv->stdin_channel == NULL);
                        priv->stdin_channel = channel;
                        return;

                case PROP_STDOUT:
                        if ((fd = g_value_get_int (value)) < 0)
                                return;
                        channel = g_io_channel_unix_new (fd);
                        g_io_channel_set_close_on_unref (channel, TRUE);
                        g_io_channel_set_encoding (channel, NULL, &error);
                        CHECK_FOR_ERROR;
                        g_assert (priv->stdout_source_id == 0);
                        priv->stdout_source_id = g_io_add_watch_full (
                                channel, G_PRIORITY_DEFAULT,
                                G_IO_IN | G_IO_ERR | G_IO_HUP,
                                (GIOFunc) process_stdout_ready, object,
                                (GDestroyNotify) process_source_removed);
                        g_assert (priv->stdout_channel == NULL);
                        priv->stdout_channel = channel;
                        return;

                case PROP_STDERR:
                        if ((fd = g_value_get_int (value)) < 0)
                                return;
                        channel = g_io_channel_unix_new (fd);
                        g_io_channel_set_close_on_unref (channel, TRUE);
                        g_io_channel_set_encoding (channel, NULL, &error);
                        CHECK_FOR_ERROR;
                        g_assert (priv->stderr_source_id == 0);
                        priv->stderr_source_id = g_io_add_watch_full (
                                channel, G_PRIORITY_DEFAULT,
                                G_IO_IN | G_IO_ERR | G_IO_HUP,
                                (GIOFunc) process_stderr_ready, object,
                                (GDestroyNotify) process_source_removed);
                        g_assert (priv->stderr_channel == NULL);
                        priv->stderr_channel = channel;
                        return;
        }

        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);

#undef CHECK_FOR_ERROR
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

                case PROP_EXITED:
                        g_value_set_boolean (value, priv->exited);
                        return;

                case PROP_STATUS:
                        g_value_set_int (value, priv->status);
                        return;
        }

        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
process_finalize (GObject *object)
{
        GvaProcessPrivate *priv = GVA_PROCESS_GET_PRIVATE (object);

        if (priv->stdin_channel != NULL)
                g_io_channel_unref (priv->stdin_channel);
        if (priv->stdout_channel != NULL)
                g_io_channel_unref (priv->stdout_channel);
        if (priv->stderr_channel != NULL)
                g_io_channel_unref (priv->stderr_channel);

        while (!g_queue_is_empty (priv->stdout_lines))
                g_free (g_queue_pop_head (priv->stdout_lines));

        while (!g_queue_is_empty (priv->stderr_lines))
                g_free (g_queue_pop_head (priv->stderr_lines));

        g_queue_free (priv->stdout_lines);
        g_queue_free (priv->stderr_lines);

        /* Chain up to parent's finalize() method. */
        G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
process_class_init (GvaProcessClass *class)
{
        GObjectClass *object_class;

        parent_class = g_type_class_peek_parent (class);
        g_type_class_add_private (class, sizeof (GvaProcessPrivate));

        object_class = G_OBJECT_CLASS (class);
        object_class->set_property = process_set_property;
        object_class->get_property = process_get_property;
        object_class->finalize = process_finalize;

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

        g_object_class_install_property (
                object_class,
                PROP_EXITED,
                g_param_spec_boolean (
                        "exited",
                        NULL,
                        NULL,
                        FALSE,
                        G_PARAM_READABLE));

        g_object_class_install_property (
                object_class,
                PROP_STATUS,
                g_param_spec_int (
                        "status",
                        NULL,
                        NULL,
                        G_MININT, G_MAXINT, -1,
                        G_PARAM_READABLE));

        signals[STDOUT_READY] = g_signal_new (
                "stdout-ready",
                G_TYPE_FROM_CLASS (class),
                G_SIGNAL_RUN_LAST,
                G_STRUCT_OFFSET (GvaProcessClass, stdout_ready),
                NULL, NULL,
                g_cclosure_marshal_VOID__VOID,
                G_TYPE_NONE,
                0,
                NULL);

        signals[STDERR_READY] = g_signal_new (
                "stderr-ready",
                G_TYPE_FROM_CLASS (class),
                G_SIGNAL_RUN_LAST,
                G_STRUCT_OFFSET (GvaProcessClass, stderr_ready),
                NULL, NULL,
                g_cclosure_marshal_VOID__VOID,
                G_TYPE_NONE,
                0,
                NULL);

        signals[EXITED] = g_signal_new (
                "exited",
                G_TYPE_FROM_CLASS (class),
                G_SIGNAL_RUN_LAST,
                G_STRUCT_OFFSET (GvaProcessClass, stderr_ready),
                NULL, NULL,
                g_cclosure_marshal_VOID__VOID,
                G_TYPE_NONE,
                0,
                NULL);
}

static void
process_init (GvaProcess *process)
{
        GvaProcessPrivate *priv = GVA_PROCESS_GET_PRIVATE (process);

        priv->stdout_lines = g_queue_new ();
        priv->stderr_lines = g_queue_new ();
}

GType
gva_process_get_type (void)
{
        static GType type = 0;

        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (GvaProcessClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) process_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (GvaProcess),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) process_init,
                        NULL
                };

                type = g_type_register_static (
                        G_TYPE_OBJECT, "GvaProcess", &type_info, 0);
        }

        return type;
}

GvaProcess *
gva_process_new (GPid pid,
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
                "stdin", standard_input,
                "stdout", standard_output,
                "stderr", standard_error,
                NULL);
}

gboolean
gva_process_write_stdin (GvaProcess *process,
                         const gchar *data,
                         gssize length,
                         GError **error)
{
        GvaProcessPrivate *priv;
        GIOStatus status = G_IO_STATUS_AGAIN;
        gsize bytes_written;

        g_return_val_if_fail (GVA_IS_PROCESS (process), FALSE);
        g_return_val_if_fail (data != NULL, FALSE);

        priv = GVA_PROCESS_GET_PRIVATE (process);

        while (status == G_IO_STATUS_AGAIN)
                status = g_io_channel_write_chars (
                        priv->stdin_channel, data,
                        length, &bytes_written, error);
        if (status == G_IO_STATUS_NORMAL)
        {
                if (length == -1)
                        length = strlen (data);
                g_assert (bytes_written == length);

                status = G_IO_STATUS_AGAIN;
                while (status == G_IO_STATUS_AGAIN)
                        status = g_io_channel_flush (
                                priv->stdin_channel, error);
        }
        g_assert (status != G_IO_STATUS_EOF);

        return (status == G_IO_STATUS_NORMAL);
}

guint
gva_process_stdout_num_lines (GvaProcess *process)
{
        GvaProcessPrivate *priv;

        g_return_val_if_fail (GVA_IS_PROCESS (process), 0);

        priv = GVA_PROCESS_GET_PRIVATE (process);

        return g_queue_get_length (priv->stdout_lines);
}

guint
gva_process_stderr_num_lines (GvaProcess *process)
{
        GvaProcessPrivate *priv;

        g_return_val_if_fail (GVA_IS_PROCESS (process), 0);

        priv = GVA_PROCESS_GET_PRIVATE (process);

        return g_queue_get_length (priv->stderr_lines);
}

const gchar *
gva_process_stdout_peek_line (GvaProcess *process)
{
        GvaProcessPrivate *priv;

        g_return_val_if_fail (GVA_IS_PROCESS (process), NULL);

        priv = GVA_PROCESS_GET_PRIVATE (process);

        return g_queue_peek_head (priv->stdout_lines);
}

const gchar *
gva_process_stderr_peek_line (GvaProcess *process)
{
        GvaProcessPrivate *priv;

        g_return_val_if_fail (GVA_IS_PROCESS (process), NULL);

        priv = GVA_PROCESS_GET_PRIVATE (process);

        return g_queue_peek_head (priv->stderr_lines);
}

gchar *
gva_process_stdout_read_line (GvaProcess *process)
{
        GvaProcessPrivate *priv;

        g_return_val_if_fail (GVA_IS_PROCESS (process), NULL);

        priv = GVA_PROCESS_GET_PRIVATE (process);

        return g_queue_pop_head (priv->stdout_lines);
}

gchar *
gva_process_stderr_read_line (GvaProcess *process)
{
        GvaProcessPrivate *priv;

        g_return_val_if_fail (GVA_IS_PROCESS (process), NULL);

        priv = GVA_PROCESS_GET_PRIVATE (process);

        return g_queue_pop_head (priv->stderr_lines);
}

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

gboolean
gva_process_has_exited (GvaProcess *process, gint *status)
{
        GvaProcessPrivate *priv;

        g_return_val_if_fail (GVA_IS_PROCESS (process), FALSE);

        priv = GVA_PROCESS_GET_PRIVATE (process);

        if (priv->exited && status != NULL)
                *status = priv->status;

        return priv->exited;
}
