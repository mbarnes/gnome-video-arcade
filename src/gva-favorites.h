#ifndef GVA_FAVORITES_H
#define GVA_FAVORITES_H

#include "gva-common.h"

G_BEGIN_DECLS

GSList *        gva_favorites_copy              (void);
void            gva_favorites_insert            (const gchar *romname);
void            gva_favorites_remove            (const gchar *romname);
gboolean        gva_favorites_contains          (const gchar *romname);

G_END_DECLS

#endif /* GVA_FAVORITES_H */
