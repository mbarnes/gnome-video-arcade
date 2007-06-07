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
        GString *stdout_buffer;
        GString *stderr_buffer;

        guint child_source_id;
        guint stdout_source_id;
        guint stderr_source_id;

        gboolean exited;
        gint status;
};

static gpointer parent_class = NULL;
static guint signals[LAST_SIGNAL] = { 0 };

static void
process_exited (GPid pid,
                gint status,
                GvaProcess *process)
{
        GvaProcessPrivate *priv = GVA_PROCESS_GET_PRIVATE (process);
        GIOStatus io_status;
        gchar *data;
        gsize length;
        GError *error = NULL;

        if (priv->stdout_source_id != 0)
        {
                g_source_remove (priv->stdout_source_id);
                priv->stdout_source_id = 0;
        }

        if (priv->stderr_source_id != 0)
        {
                g_source_remove (priv->stderr_source_id);
                priv->stderr_source_id = 0;
        }

        io_status = G_IO_STATUS_AGAIN;
        while (io_status == G_IO_STATUS_AGAIN)
                io_status = g_io_channel_read_to_end (
                        priv->stdout_channel, &data, &length, &error);
        if (io_status == G_IO_STATUS_NORMAL)
        {
                g_string_append_len (priv->stdout_buffer, data, length);
                g_free (data);
                g_signal_emit (process, signals[STDOUT_READY], 0);
        }
        else  /* G_IO_STATUS_ERROR */
        {
                g_assert (error != NULL);
                g_warning ("%s", error->message);
                g_clear_error (&error);
        }

        io_status = G_IO_STATUS_AGAIN;
        while (io_status == G_IO_STATUS_AGAIN)
                io_status = g_io_channel_read_to_end (
                        priv->stderr_channel, &data, &length, &error);
        if (io_status == G_IO_STATUS_NORMAL)
        {
                g_string_append_len (priv->stderr_buffer, data, length);
                g_free (data);
                g_signal_emit (process, signals[STDERR_READY], 0);
        }
        else  /* G_IO_STATUS_ERROR */
        {
                g_assert (error != NULL);
                g_warning ("%s", error->message);
                g_clear_error (&error);
        }

        priv->exited = TRUE;
        priv->status = status;
        g_spawn_close_pid (pid);

        g_signal_emit (process, signals[EXITED], 0);
}

static gboolean
process_data_ready (GvaProcess *process,
                    GIOChannel *source,
                    GIOCondition condition,
                    GString *string,
                    guint *source_id,
                    guint signal_id)
{
        static gchar read_buffer[READ_BUFFER_SIZE];

        if (condition & G_IO_IN)
        {
                GIOStatus status = G_IO_STATUS_AGAIN;
                gsize bytes_read;
                GError *error = NULL;

                while (status == G_IO_STATUS_AGAIN)
                        status = g_io_channel_read_chars (
                                source, read_buffer,
                                sizeof (read_buffer),
                                &bytes_read, &error);
                switch (status)
                {
                        case G_IO_STATUS_NORMAL:
                                g_string_append_len (
                                        string, read_buffer, bytes_read);
                                g_signal_emit (process, signal_id, 0);
                                break;

                        case G_IO_STATUS_ERROR:
                                g_assert (error != NULL);
                                g_warning ("%s", error->message);
                                g_clear_error (&error);
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
        GvaProcessPrivate *priv = GVA_PROCESS_GET_PRIVATE (process);

        return process_data_ready (
                process, source, condition, priv->stdout_buffer,
                &priv->stdout_source_id, signals[STDOUT_READY]);
}

static gboolean
process_stderr_ready (GIOChannel *source,
                      GIOCondition condition,
                      GvaProcess *process)
{
        GvaProcessPrivate *priv = GVA_PROCESS_GET_PRIVATE (process);

        return process_data_ready (
                process, source, condition, priv->stderr_buffer,
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
                        if (priv->child_source_id > 0)
                                g_source_remove (priv->child_source_id);
                        priv->child_source_id  = g_child_watch_add (
                                priv->pid, (GChildWatchFunc)
                                process_exited, object);
                        return;

                case PROP_STDIN:
                        if (priv->stdin_channel != NULL)
                        {
                                g_io_channel_unref (priv->stdin_channel);
                                priv->stdin_channel = NULL;
                        }
                        if ((fd = g_value_get_int (value)) < 0)
                                return;
                        channel = g_io_channel_unix_new (fd);
                        g_io_channel_set_close_on_unref (channel, TRUE);
                        g_io_channel_set_encoding (channel, NULL, &error);
                        CHECK_FOR_ERROR;
                        priv->stdin_channel = channel;
                        return;

                case PROP_STDOUT:
                        if (priv->stdout_channel != NULL)
                        {
                                g_io_channel_unref (priv->stdout_channel);
                                priv->stdout_channel = NULL;
                        }
                        if (priv->stdout_source_id > 0)
                        {
                                g_source_remove (priv->stdout_source_id);
                                priv->stdout_source_id = 0;
                        }
                        if ((fd = g_value_get_int (value)) < 0)
                                return;
                        channel = g_io_channel_unix_new (fd);
                        g_io_channel_set_close_on_unref (channel, TRUE);
                        g_io_channel_set_encoding (channel, NULL, &error);
                        CHECK_FOR_ERROR;
                        g_io_channel_set_flags (
                                channel, G_IO_FLAG_NONBLOCK, &error);
                        CHECK_FOR_ERROR;
                        priv->stdout_source_id = g_io_add_watch (
                                channel, G_IO_IN | G_IO_ERR | G_IO_HUP,
                                (GIOFunc) process_stdout_ready, object);
                        priv->stdout_channel = channel;
                        return;

                case PROP_STDERR:
                        if (priv->stderr_channel != NULL)
                        {
                                g_io_channel_unref (priv->stderr_channel);
                                priv->stderr_channel = NULL;
                        }
                        if (priv->stderr_source_id > 0)
                        {
                                g_source_remove (priv->stderr_source_id);
                                priv->stderr_source_id = 0;
                        }
                        if ((fd = g_value_get_int (value)) < 0)
                                return;
                        channel = g_io_channel_unix_new (fd);
                        g_io_channel_set_close_on_unref (channel, TRUE);
                        g_io_channel_set_encoding (channel, NULL, &error);
                        CHECK_FOR_ERROR;
                        g_io_channel_set_flags (
                                channel, G_IO_FLAG_NONBLOCK, &error);
                        CHECK_FOR_ERROR;
                        priv->stderr_source_id = g_io_add_watch (
                                channel, G_IO_IN | G_IO_ERR | G_IO_HUP,
                                (GIOFunc) process_stderr_ready, object);
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

        g_string_free (priv->stdout_buffer, TRUE);
        g_string_free (priv->stderr_buffer, TRUE);

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

        priv->stdout_buffer = g_string_sized_new (READ_BUFFER_SIZE);
        priv->stderr_buffer = g_string_sized_new (READ_BUFFER_SIZE);
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

const gchar *
gva_process_peek_stdout (GvaProcess *process)
{
        GvaProcessPrivate *priv;

        g_return_val_if_fail (GVA_IS_PROCESS (process), NULL);

        priv = GVA_PROCESS_GET_PRIVATE (process);

        return priv->stdout_buffer->str;
}

const gchar *
gva_process_peek_stderr (GvaProcess *process)
{
        GvaProcessPrivate *priv;

        g_return_val_if_fail (GVA_IS_PROCESS (process), NULL);

        priv = GVA_PROCESS_GET_PRIVATE (process);

        return priv->stderr_buffer->str;
}

gchar *
gva_process_read_stdout (GvaProcess *process)
{
        GvaProcessPrivate *priv;
        gchar *data;

        g_return_val_if_fail (GVA_IS_PROCESS (process), NULL);

        priv = GVA_PROCESS_GET_PRIVATE (process);

        data = g_strdup (gva_process_peek_stdout (process));
        g_string_truncate (priv->stdout_buffer, 0);

        return data;
}

gchar *
gva_process_read_stderr (GvaProcess *process)
{
        GvaProcessPrivate *priv;
        gchar *data;

        g_return_val_if_fail (GVA_IS_PROCESS (process), NULL);

        priv = GVA_PROCESS_GET_PRIVATE (process);

        data = g_strdup (gva_process_peek_stderr (process));
        g_string_truncate (priv->stderr_buffer, 0);

        return data;
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
