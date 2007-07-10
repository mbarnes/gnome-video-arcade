#ifndef GVA_HISTORY_H
#define GVA_HISTORY_H

#include "gva-common.h"

G_BEGIN_DECLS

gboolean        gva_history_init                (GError **error);
gchar *         gva_history_lookup              (const gchar *name,
                                                 GError **error);

G_END_DECLS

#endif /* GVA_HISTORY_H */
