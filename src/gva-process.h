#ifndef GVA_PROCESS_H
#define GVA_PROCESS_H

#include "gva-common.h"

/* Standard GObject macros */
#define GVA_TYPE_PROCESS \
        (gva_process_get_type ())
#define GVA_PROCESS(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), GVA_TYPE_PROCESS, GvaProcess))
#define GVA_PROCESS_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), GVA_TYPE_PROCESS, GvaProcessClass))
#define GVA_IS_PROCESS(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), GVA_TYPE_PROCESS))
#define GVA_IS_PROCESS_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), GVA_TYPE_PROCESS))
#define GVA_PROCESS_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), GVA_TYPE_PROCESS, GvaProcessClass))

G_BEGIN_DECLS

typedef struct _GvaProcess GvaProcess;
typedef struct _GvaProcessClass GvaProcessClass;
typedef struct _GvaProcessPrivate GvaProcessPrivate;

struct _GvaProcess
{
        GObject parent;

        GError *error;
        GvaProcessPrivate *priv;
};

struct _GvaProcessClass
{
        GObjectClass parent_class;

        /* Methods */
        const gchar *   (*stdout_peek_line)     (GvaProcess *process);
        const gchar *   (*stderr_peek_line)     (GvaProcess *process);
        gchar *         (*stdout_read_line)     (GvaProcess *process);
        gchar *         (*stderr_read_line)     (GvaProcess *process);

        /* Signals */
        void            (*stdout_ready)         (GvaProcess *process);
        void            (*stderr_ready)         (GvaProcess *process);
        void            (*exited)               (GvaProcess *process,
                                                 gint status);
};

GType           gva_process_get_type            (void);
GvaProcess *    gva_process_new                 (GPid pid,
                                                 gint standard_input,
                                                 gint standard_output,
                                                 gint standard_error);
GvaProcess *    gva_process_spawn               (const gchar *command_line,
                                                 GError **error);
gboolean        gva_process_write_stdin         (GvaProcess *process,
                                                 const gchar *data,
                                                 gssize length,
                                                 GError **error);
guint           gva_process_stdout_num_lines    (GvaProcess *process);
guint           gva_process_stderr_num_lines    (GvaProcess *process);
const gchar *   gva_process_stdout_peek_line    (GvaProcess *process);
const gchar *   gva_process_stderr_peek_line    (GvaProcess *process);
gchar *         gva_process_stdout_read_line    (GvaProcess *process);
gchar *         gva_process_stderr_read_line    (GvaProcess *process);
gchar **        gva_process_stdout_read_lines   (GvaProcess *process);
gchar **        gva_process_stderr_read_lines   (GvaProcess *process);
gboolean        gva_process_has_exited          (GvaProcess *process,
                                                 gint *status);

G_END_DECLS

#endif /* GVA_PROCESS_H */
