#ifndef GVA_MAIN_H
#define GVA_MAIN_H

#include "gva-common.h"

G_BEGIN_DECLS

void            gva_main_init                   (void);
void            gva_main_connect_proxy_cb       (GtkUIManager *manager,
                                                 GtkAction *action,
                                                 GtkWidget *proxy);

G_END_DECLS

#endif /* GVA_MAIN_H */
