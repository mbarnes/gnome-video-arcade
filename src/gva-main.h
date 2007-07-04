#ifndef GVA_MAIN_H
#define GVA_MAIN_H

#include "gva-common.h"

G_BEGIN_DECLS

void          gva_main_init                     (void);
void          gva_main_connect_proxy_cb         (GtkUIManager *manager,
                                                 GtkAction *action,
                                                 GtkWidget *proxy);
guint         gva_main_statusbar_get_context_id (const gchar *context);
guint         gva_main_statusbar_push           (guint context_id,
                                                 const gchar *format,
                                                 ...);
void          gva_main_statusbar_pop            (guint context_id);
void          gva_main_statusbar_remove         (guint context_id,
                                                 guint message_id);

G_END_DECLS

#endif /* GVA_MAIN_H */
