#ifndef GVA_GAME_DB_H
#define GVA_GAME_DB_H

#include "gva-common.h"
#include "gva-process.h"

G_BEGIN_DECLS

gboolean        gva_game_db_init                (GError **error);
GtkTreePath *   gva_game_db_lookup              (const gchar *romname);
GtkTreeModel *  gva_game_db_get_model           (void);
GvaProcess *    gva_game_db_update_samples      (GError **error);
GvaProcess *    gva_game_db_update_titles       (GError **error);

G_END_DECLS

#endif /* GVA_GAME_DB_H */
