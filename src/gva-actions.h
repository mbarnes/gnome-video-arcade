#ifndef GVA_ACTIONS_H
#define GVA_ACTIONS_H

#include "gva-common.h"

G_BEGIN_DECLS

GtkAction *      gva_get_action                  (const gchar *action_name);
GtkActionGroup * gva_get_action_group            (void);

G_END_DECLS

#endif /* GVA_ACTIONS_H */
