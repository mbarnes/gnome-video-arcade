#ifndef GVA_TREE_VIEW_H
#define GVA_TREE_VIEW_H

#include "gva-common.h"
#include "gva-game-store.h"

G_BEGIN_DECLS

void            gva_tree_view_init              (GtkTreeView *tree_view,
                                                 GvaGameStore *game_store);

G_END_DECLS

#endif /* GVA_TREE_VIEW_H */
