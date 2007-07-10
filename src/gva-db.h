#ifndef GVA_DB_H
#define GVA_DB_H

#include "gva-common.h"
#include "gva-process.h"

G_BEGIN_DECLS

gboolean        gva_db_init                     (GError **error);
GvaProcess *    gva_db_build                    (GError **error);
gboolean        gva_db_execute                  (const gchar *sql,
                                                 GError **error);
gboolean        gva_db_prepare                  (const gchar *sql,
                                                 sqlite3_stmt **stmt,
                                                 GError **error);
gboolean        gva_db_get_build                (gchar **build,
                                                 GError **error);
const gchar *   gva_db_get_filename             (void);
void            gva_db_set_error                (GError **error,
                                                 gint code,
                                                 const gchar *message);

G_END_DECLS

#endif /* GVA_DB_H */
