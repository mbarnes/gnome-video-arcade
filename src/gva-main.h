#ifndef GVA_MAIN_H
#define GVA_MAIN_H

#include "gva-common.h"

G_BEGIN_DECLS

void            gva_main_init                   (void);
gchar *         gva_main_get_selected_game      (void);
void            gva_main_tree_view_select_path  (GtkTreePath *path);

G_END_DECLS

#endif /* GVA_MAIN_H */
