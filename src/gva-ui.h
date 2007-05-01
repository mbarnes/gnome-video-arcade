#ifndef GVA_UI_H
#define GVA_UI_H

#include "gva-common.h"
#include "gva-game-store.h"

G_BEGIN_DECLS

void            gva_ui_init                     (GvaGameStore *game_store);
gchar *         gva_ui_get_selected_game        (void);
GtkStatusbar *  gva_ui_get_statusbar            (void);

G_END_DECLS

#endif /* GVA_UI_H */
