#ifndef GVA_TREE_VIEW_H
#define GVA_TREE_VIEW_H

#include "gva-common.h"

G_BEGIN_DECLS

void            gva_tree_view_init              (void);
gchar *         gva_tree_view_get_selected_game (void);
void            gva_tree_view_set_selected_game (const gchar *romname);
void            gva_tree_view_refresh_favorites (void);

G_END_DECLS

#endif /* GVA_TREE_VIEW_H */
