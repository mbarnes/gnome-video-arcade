#ifndef GVA_UI_H
#define GVA_UI_H

#include "gva-common.h"
#include "gva-game-store.h"

#define GW(x) (gva_ui_get_glade_widget (x))

/* Main Window */
#define GVA_WIDGET_MAIN_AVAILABLE_BUTTON     GW ("main-available-button")
#define GVA_WIDGET_MAIN_FAVORITES_BUTTON     GW ("main-favorites-button")
#define GVA_WIDGET_MAIN_PROPERTIES_BUTTON    GW ("main-properties-button")
#define GVA_WIDGET_MAIN_RESULTS_BUTTON       GW ("main-results-button")
#define GVA_WIDGET_MAIN_START_GAME_BUTTON    GW ("main-start-game-button")
#define GVA_WIDGET_MAIN_STATUSBAR            GW ("main-statusbar")
#define GVA_WIDGET_MAIN_TREE_VIEW            GW ("main-tree-view")
#define GVA_WIDGET_MAIN_VBOX                 GW ("main-vbox")
#define GVA_WIDGET_MAIN_WINDOW               GW ("main-window")

/* Play Back Window */
#define GVA_WIDGET_PLAY_BACK_BUTTON          GW ("play-back-button")
#define GVA_WIDGET_PLAY_BACK_CLOSE_BUTTON    GW ("play-back-close-button")
#define GVA_WIDGET_PLAY_BACK_DELETE_BUTTON   GW ("play-back-delete-button")
#define GVA_WIDGET_PLAY_BACK_TREE_VIEW       GW ("play-back-tree-view")
#define GVA_WIDGET_PLAY_BACK_WINDOW          GW ("play-back-window")

G_BEGIN_DECLS

GtkWidget *     gva_ui_get_glade_widget         (const gchar *name);
GtkWidget *     gva_ui_get_managed_widget       (const gchar *path);

G_END_DECLS

#endif /* GVA_UI_H */
