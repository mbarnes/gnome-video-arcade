#ifndef GVA_TREE_VIEW_H
#define GVA_TREE_VIEW_H

#include "gva-common.h"

G_BEGIN_DECLS

void            gva_tree_view_init                   (void);
void            gva_tree_view_update                 (void);
const gchar *   gva_tree_view_get_selected_game      (void);
void            gva_tree_view_set_selected_game      (const gchar *romname);
gint            gva_tree_view_get_selected_view      (void);
void            gva_tree_view_set_selected_view      (gint view);
const gchar *   gva_tree_view_get_last_selected_game (void);
void            gva_tree_view_set_last_selected_game (const gchar *romname);
gint            gva_tree_view_get_last_selected_view (void);
void            gva_tree_view_set_last_selected_view (gint view);

G_END_DECLS

#endif /* GVA_TREE_VIEW_H */
