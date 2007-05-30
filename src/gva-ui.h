#ifndef GVA_UI_H
#define GVA_UI_H

#include "gva-common.h"

#define GW(x) (gva_ui_get_widget (x))

/* Actions */
#define GVA_ACTION_ABOUT                (gva_ui_get_action ("about"))
#define GVA_ACTION_AUTO_SAVE            (gva_ui_get_action ("auto-save"))
#define GVA_ACTION_CONTENTS             (gva_ui_get_action ("contents"))
#define GVA_ACTION_FULL_SCREEN          (gva_ui_get_action ("full-screen"))
#define GVA_ACTION_INSERT_FAVORITE      (gva_ui_get_action ("insert-favorite"))
#define GVA_ACTION_PLAY_BACK            (gva_ui_get_action ("play-back"))
#define GVA_ACTION_PROPERTIES           (gva_ui_get_action ("properties"))
#define GVA_ACTION_QUIT                 (gva_ui_get_action ("quit"))
#define GVA_ACTION_RECORD               (gva_ui_get_action ("record"))
#define GVA_ACTION_REMOVE_FAVORITE      (gva_ui_get_action ("remove-favorite"))
#define GVA_ACTION_SHOW_PLAY_BACK       (gva_ui_get_action ("show-play-back"))
#define GVA_ACTION_START                (gva_ui_get_action ("start"))
#define GVA_ACTION_VIEW_AVAILABLE       (gva_ui_get_action ("view-available"))
#define GVA_ACTION_VIEW_FAVORITES       (gva_ui_get_action ("view-favorites"))
#define GVA_ACTION_VIEW_RESULTS         (gva_ui_get_action ("view-results"))

/* Main Window */
#define GVA_WIDGET_MAIN_PROPERTIES_BUTTON    GW ("main-properties-button")
#define GVA_WIDGET_MAIN_START_GAME_BUTTON    GW ("main-start-game-button")
#define GVA_WIDGET_MAIN_STATUSBAR            GW ("main-statusbar")
#define GVA_WIDGET_MAIN_TREE_VIEW            GW ("main-tree-view")
#define GVA_WIDGET_MAIN_VBOX                 GW ("main-vbox")
#define GVA_WIDGET_MAIN_VIEW_BUTTON_0        GW ("main-view-button-0")
#define GVA_WIDGET_MAIN_VIEW_BUTTON_1        GW ("main-view-button-1")
#define GVA_WIDGET_MAIN_VIEW_BUTTON_2        GW ("main-view-button-2")
#define GVA_WIDGET_MAIN_WINDOW               GW ("main-window")

/* Play Back Window */
#define GVA_WIDGET_PLAY_BACK_BUTTON          GW ("play-back-button")
#define GVA_WIDGET_PLAY_BACK_CLOSE_BUTTON    GW ("play-back-close-button")
#define GVA_WIDGET_PLAY_BACK_DELETE_BUTTON   GW ("play-back-delete-button")
#define GVA_WIDGET_PLAY_BACK_TREE_VIEW       GW ("play-back-tree-view")
#define GVA_WIDGET_PLAY_BACK_WINDOW          GW ("play-back-window")

/* Preferences Window */
#define GVA_WIDGET_PREFERENCES_AUTO_SAVE     GW ("preferences-auto-save")
#define GVA_WIDGET_PREFERENCES_CLOSE_BUTTON  GW ("preferences-close-button")
#define GVA_WIDGET_PREFERENCES_FULL_SCREEN   GW ("preferences-full-screen")
#define GVA_WIDGET_PREFERENCES_WINDOW        GW ("preferences-window")

G_BEGIN_DECLS

GtkAction *     gva_ui_get_action               (const gchar *action_name);
GtkWidget *     gva_ui_get_widget               (const gchar *widget_name);
GtkWidget *     gva_ui_get_managed_widget       (const gchar *widget_path);

G_END_DECLS

#endif /* GVA_UI_H */
