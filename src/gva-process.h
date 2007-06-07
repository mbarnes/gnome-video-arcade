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

struct _GvaProcess
{
        GObject parent;
};

struct _GvaProcessClass
{
        GObjectClass parent_class;

        void (*stdout_ready) (GvaProcess *process);
        void (*stderr_ready) (GvaProcess *process);
        void (*exited) (GvaProcess *process);
};

GType           gva_process_get_type            (void);
GvaProcess *    gva_process_new                 (GPid pid,
                                                 gint standard_input,
                                                 gint standard_output,
                                                 gint standard_error);
gboolean        gva_process_write_stdin         (GvaProcess *process,
                                                 const gchar *data,
                                                 gssize length,
                                                 GError **error);
const gchar *   gva_process_peek_stdout         (GvaProcess *process);
const gchar *   gva_process_peek_stderr         (GvaProcess *process);
gchar *         gva_process_read_stdout         (GvaProcess *process);
gchar *         gva_process_read_stderr         (GvaProcess *process);
gboolean        gva_process_has_exited          (GvaProcess *process,
                                                 gint *status);

G_END_DECLS

#endif /* GVA_PROCESS_H */
