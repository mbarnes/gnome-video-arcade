/* Copyright 2007, 2008 Matthew Barnes
 *
 * This file is part of GNOME Video Arcade.
 *
 * GNOME Video Arcade is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * GNOME Video Arcade is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION: gva-ui
 * @short_description: User Interface Support
 *
 * These functions and macros provide easy access to global actions and
 * widgets in the application.
 **/

#ifndef GVA_UI_H
#define GVA_UI_H

#include "gva-common.h"

/* Actions */
#define GVA_ACTION_ABOUT                (gva_ui_get_action ("about"))
#define GVA_ACTION_AUTO_SAVE            (gva_ui_get_action ("auto-save"))
#define GVA_ACTION_CONTENTS             (gva_ui_get_action ("contents"))
#define GVA_ACTION_FULL_SCREEN          (gva_ui_get_action ("full-screen"))
#define GVA_ACTION_INSERT_FAVORITE      (gva_ui_get_action ("insert-favorite"))
#define GVA_ACTION_NEXT_GAME            (gva_ui_get_action ("next-game"))
#define GVA_ACTION_PLAY_BACK            (gva_ui_get_action ("play-back"))
#define GVA_ACTION_PREFERENCES          (gva_ui_get_action ("preferences"))
#define GVA_ACTION_PREVIOUS_GAME        (gva_ui_get_action ("previous-game"))
#define GVA_ACTION_PROPERTIES           (gva_ui_get_action ("properties"))
#define GVA_ACTION_QUIT                 (gva_ui_get_action ("quit"))
#define GVA_ACTION_RECORD               (gva_ui_get_action ("record"))
#define GVA_ACTION_REMOVE_COLUMN        (gva_ui_get_action ("remove-column"))
#define GVA_ACTION_REMOVE_FAVORITE      (gva_ui_get_action ("remove-favorite"))
#define GVA_ACTION_SAVE_ERRORS          (gva_ui_get_action ("save-errors"))
#define GVA_ACTION_SEARCH               (gva_ui_get_action ("search"))
#define GVA_ACTION_SHOW_CLONES          (gva_ui_get_action ("show-clones"))
#define GVA_ACTION_SHOW_PLAY_BACK       (gva_ui_get_action ("show-play-back"))
#define GVA_ACTION_START                (gva_ui_get_action ("start"))
#define GVA_ACTION_VIEW_AVAILABLE       (gva_ui_get_action ("view-available"))
#define GVA_ACTION_VIEW_FAVORITES       (gva_ui_get_action ("view-favorites"))
#define GVA_ACTION_VIEW_RESULTS         (gva_ui_get_action ("view-results"))

/* Audit Window */
#define GVA_WIDGET_AUDIT_SAVE_BUTTON \
        (gva_ui_get_widget ("audit-save-button"))
#define GVA_WIDGET_AUDIT_TREE_VIEW \
        (gva_ui_get_widget ("audit-tree-view"))
#define GVA_WIDGET_AUDIT_WINDOW \
        (gva_ui_get_widget ("audit-window"))

/* Main Window */
#define GVA_WIDGET_MAIN_PROGRESS_BAR \
        (gva_ui_get_widget ("main-progress-bar"))
#define GVA_WIDGET_MAIN_PROPERTIES_BUTTON \
        (gva_ui_get_widget ("main-properties-button"))
#define GVA_WIDGET_MAIN_SEARCH_ENTRY \
        (gva_ui_get_widget ("main-search-entry"))
#define GVA_WIDGET_MAIN_SEARCH_HBOX \
        (gva_ui_get_widget ("main-search-hbox"))
#define GVA_WIDGET_MAIN_START_GAME_BUTTON \
        (gva_ui_get_widget ("main-start-game-button"))
#define GVA_WIDGET_MAIN_STATUSBAR \
        (gva_ui_get_widget ("main-statusbar"))
#define GVA_WIDGET_MAIN_TREE_VIEW \
        (gva_ui_get_widget ("main-tree-view"))
#define GVA_WIDGET_MAIN_VBOX \
        (gva_ui_get_widget ("main-vbox"))
#define GVA_WIDGET_MAIN_VIEW_BUTTON_0 \
        (gva_ui_get_widget ("main-view-button-0"))
#define GVA_WIDGET_MAIN_VIEW_BUTTON_1 \
        (gva_ui_get_widget ("main-view-button-1"))
#define GVA_WIDGET_MAIN_VIEW_BUTTON_2 \
        (gva_ui_get_widget ("main-view-button-2"))
#define GVA_WIDGET_MAIN_WINDOW \
        (gva_ui_get_widget ("main-window"))

/* Play Back Window */
#define GVA_WIDGET_PLAY_BACK_BUTTON \
        (gva_ui_get_widget ("play-back-button"))
#define GVA_WIDGET_PLAY_BACK_CLOSE_BUTTON \
        (gva_ui_get_widget ("play-back-close-button"))
#define GVA_WIDGET_PLAY_BACK_DELETE_BUTTON \
        (gva_ui_get_widget ("play-back-delete-button"))
#define GVA_WIDGET_PLAY_BACK_TREE_VIEW \
        (gva_ui_get_widget ("play-back-tree-view"))
#define GVA_WIDGET_PLAY_BACK_WINDOW \
        (gva_ui_get_widget ("play-back-window"))

/* Preferences Window */
#define GVA_WIDGET_PREFERENCES_AUTO_SAVE \
        (gva_ui_get_widget ("preferences-auto-save"))
#define GVA_WIDGET_PREFERENCES_CLOSE_BUTTON \
        (gva_ui_get_widget ("preferences-close-button"))
#define GVA_WIDGET_PREFERENCES_FULL_SCREEN \
        (gva_ui_get_widget ("preferences-full-screen"))
#define GVA_WIDGET_PREFERENCES_SHOW_CLONES \
        (gva_ui_get_widget ("preferences-show-clones"))
#define GVA_WIDGET_PREFERENCES_WINDOW \
        (gva_ui_get_widget ("preferences-window"))

/* Properties Window */
#define GVA_WIDGET_PROPERTIES_BACK_BUTTON \
        (gva_ui_get_widget ("properties-back-button"))
#define GVA_WIDGET_PROPERTIES_BIOS_LABEL \
        (gva_ui_get_widget ("properties-bios-label"))
#define GVA_WIDGET_PROPERTIES_BIOS_VBOX \
        (gva_ui_get_widget ("properties-bios-vbox"))
#define GVA_WIDGET_PROPERTIES_CLOSE_BUTTON \
        (gva_ui_get_widget ("properties-close-button"))
#define GVA_WIDGET_PROPERTIES_CPU_VBOX \
        (gva_ui_get_widget ("properties-cpu-vbox"))
#define GVA_WIDGET_PROPERTIES_FORWARD_BUTTON \
        (gva_ui_get_widget ("properties-forward-button"))
#define GVA_WIDGET_PROPERTIES_HEADER \
        (gva_ui_get_widget ("properties-header"))
#define GVA_WIDGET_PROPERTIES_HISTORY_TEXT_VIEW \
        (gva_ui_get_widget ("properties-history-text-view"))
#define GVA_WIDGET_PROPERTIES_IMPERFECT_COLOR_LABEL \
        (gva_ui_get_widget ("properties-imperfect-color-label"))
#define GVA_WIDGET_PROPERTIES_IMPERFECT_GRAPHIC_LABEL \
        (gva_ui_get_widget ("properties-imperfect-graphic-label"))
#define GVA_WIDGET_PROPERTIES_IMPERFECT_SOUND_LABEL \
        (gva_ui_get_widget ("properties-imperfect-sound-label"))
#define GVA_WIDGET_PROPERTIES_NOTEBOOK \
        (gva_ui_get_widget ("properties-notebook"))
#define GVA_WIDGET_PROPERTIES_PRELIMINARY_COCKTAIL_LABEL \
        (gva_ui_get_widget ("properties-preliminary-cocktail-label"))
#define GVA_WIDGET_PROPERTIES_PRELIMINARY_COLOR_LABEL \
        (gva_ui_get_widget ("properties-preliminary-color-label"))
#define GVA_WIDGET_PROPERTIES_PRELIMINARY_EMULATION_LABEL \
        (gva_ui_get_widget ("properties-preliminary-emulation-label"))
#define GVA_WIDGET_PROPERTIES_PRELIMINARY_PROTECTION_LABEL \
        (gva_ui_get_widget ("properties-preliminary-protection-label"))
#define GVA_WIDGET_PROPERTIES_PRELIMINARY_SOUND_LABEL \
        (gva_ui_get_widget ("properties-preliminary-sound-label"))
#define GVA_WIDGET_PROPERTIES_SOUND_VBOX \
        (gva_ui_get_widget ("properties-sound-vbox"))
#define GVA_WIDGET_PROPERTIES_STATUS_HBOX \
        (gva_ui_get_widget ("properties-status-hbox"))
#define GVA_WIDGET_PROPERTIES_STATUS_IMAGE \
        (gva_ui_get_widget ("properties-status-image"))
#define GVA_WIDGET_PROPERTIES_VIDEO_VBOX \
        (gva_ui_get_widget ("properties-video-vbox"))
#define GVA_WIDGET_PROPERTIES_WINDOW \
        (gva_ui_get_widget ("properties-window"))

G_BEGIN_DECLS

GtkAction *     gva_ui_get_action               (const gchar *action_name);
GtkWidget *     gva_ui_get_widget               (const gchar *widget_name);
GtkWidget *     gva_ui_get_managed_widget       (const gchar *widget_path);
void            gva_ui_add_column_actions       (GtkTreeView *view);

G_END_DECLS

#endif /* GVA_UI_H */
