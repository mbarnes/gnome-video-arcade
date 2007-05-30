#ifndef GVA_PREFERENCES_H
#define GVA_PREFERENCES_H

#include "gva-common.h"

G_BEGIN_DECLS

void            gva_preferences_init            (void);
gboolean        gva_preferences_get_auto_save   (void);
void            gva_preferences_set_auto_save   (gboolean auto_save);
gboolean        gva_preferences_get_full_screen (void);
void            gva_preferences_set_full_screen (gboolean full_screen);

G_END_DECLS

#endif /* GVA_PREFERENCES_H */
