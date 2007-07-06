#ifndef GVA_ERROR_H
#define GVA_ERROR_H

#include "gva-common.h"

#define GVA_ERROR               gva_error_quark ()
#define GVA_SQLITE_ERROR        gva_sqlite_error_quark ()

G_BEGIN_DECLS

typedef enum
{
        GVA_ERROR_CONFIG,
        GVA_ERROR_SYSTEM,
        GVA_ERROR_XMAME

} GvaError;

GQuark          gva_error_quark                 (void);
GQuark          gva_sqlite_error_quark          (void);
void            gva_error_handle                (GError **error);

G_END_DECLS

#endif /* GVA_ERROR_H */
