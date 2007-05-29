#ifndef GVA_PROCESS_H
#define GVA_PROCESS_H

#include "gva-common.h"

G_BEGIN_DECLS

typedef struct _GvaProcess GvaProcess;

typedef void    (*GvaProcessNotify)             (GvaProcess *process,
                                                 gpointer user_data);

GvaProcess *    gva_process_new                 (GPid pid,
                                                 gint standard_input,
                                                 gint standard_output,
                                                 gint standard_error,
                                                 GvaProcessNotify on_stdout,
                                                 GvaProcessNotify on_stderr,
                                                 GvaProcessNotify on_exit,
                                                 gpointer user_data,
                                                 GError **error);
gboolean        gva_process_write_stdin         (GvaProcess *process,
                                                 const gchar *data,
                                                 gssize length,
                                                 GError **error);
const gchar *   gva_process_peek_stdout         (GvaProcess *process);
const gchar *   gva_process_peek_stderr         (GvaProcess *process);
gchar *         gva_process_read_stdout         (GvaProcess *process);
gchar *         gva_process_read_stderr         (GvaProcess *process);
gboolean        gva_process_has_exited          (GvaProcess *process,
                                                 gint *exit_status);
void            gva_process_free                (GvaProcess *process);


G_END_DECLS

#endif /* GVA_PROCESS_H */
