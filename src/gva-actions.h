#ifndef GVA_ACTIONS_H
#define GVA_ACTIONS_H

#include "gva-common.h"

#define GVA_ACTION_ABOUT                (gva_get_action ("about"))
#define GVA_ACTION_CONTENTS             (gva_get_action ("contents"))
#define GVA_ACTION_PLAY_BACK            (gva_get_action ("play-back"))
#define GVA_ACTION_PROPERTIES           (gva_get_action ("properties"))
#define GVA_ACTION_QUIT                 (gva_get_action ("quit"))
#define GVA_ACTION_RECORD               (gva_get_action ("record"))
#define GVA_ACTION_START                (gva_get_action ("start"))
#define GVA_ACTION_VIEW_AVAILABLE       (gva_get_action ("view-available"))
#define GVA_ACTION_VIEW_FAVORITES       (gva_get_action ("view-favorites"))
#define GVA_ACTION_VIEW_RESULTS         (gva_get_action ("view-results"))

G_BEGIN_DECLS

GtkAction *      gva_get_action                  (const gchar *action_name);
GtkActionGroup * gva_get_action_group            (void);

G_END_DECLS

#endif /* GVA_ACTIONS_H */
