/* Copyright 2007-2011 Matthew Barnes
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

#include "gva-preferences.h"

#include "gva-column-manager.h"
#include "gva-error.h"
#include "gva-mame.h"
#include "gva-ui.h"

/**
 * gva_preferences_init:
 *
 * Initializes the Preferences window.
 *
 * This function should be called once when the application starts.
 **/
void
gva_preferences_init (void)
{
        /* Auto Play */

        /* This actually appears in the Properties window,
         * but it's still a preference so we manage it here. */

        gtk_activatable_set_related_action (
                GTK_ACTIVATABLE (GVA_WIDGET_PROPERTIES_MUSIC_AUTO_PLAY),
                GVA_ACTION_AUTO_PLAY);

        gconf_bridge_bind_property (
                gconf_bridge_get (), GVA_GCONF_AUTO_PLAY_KEY,
                G_OBJECT (GVA_ACTION_AUTO_PLAY), "active");

        /* Auto Save */

        gtk_activatable_set_related_action (
                GTK_ACTIVATABLE (GVA_WIDGET_PREFERENCES_AUTO_SAVE),
                GVA_ACTION_AUTO_SAVE);

        gtk_action_set_sensitive (
                GVA_ACTION_AUTO_SAVE,
                gva_mame_supports_auto_save ());

        gconf_bridge_bind_property (
                gconf_bridge_get (), GVA_GCONF_AUTO_SAVE_KEY,
                G_OBJECT (GVA_ACTION_AUTO_SAVE), "active");

        /* Full Screen */

        gtk_activatable_set_related_action (
                GTK_ACTIVATABLE (GVA_WIDGET_PREFERENCES_FULL_SCREEN),
                GVA_ACTION_FULL_SCREEN);

        gtk_action_set_sensitive (
                GVA_ACTION_FULL_SCREEN,
                gva_mame_supports_full_screen () ||
                gva_mame_supports_window ());

        gconf_bridge_bind_property (
                gconf_bridge_get (), GVA_GCONF_FULL_SCREEN_KEY,
                G_OBJECT (GVA_ACTION_FULL_SCREEN), "active");

        /* Show Clones */

        gtk_activatable_set_related_action (
                GTK_ACTIVATABLE (GVA_WIDGET_PREFERENCES_SHOW_CLONES),
                GVA_ACTION_SHOW_CLONES);

        gconf_bridge_bind_property (
                gconf_bridge_get (), GVA_GCONF_SHOW_CLONES_KEY,
                G_OBJECT (GVA_ACTION_SHOW_CLONES), "active");
}

/**
 * gva_preferences_get_auto_play:
 *
 * Returns the user's preference for whether to automatically
 * play a music clip of the selected game when the user opens
 * the Properties window or when the user selects a different
 * game while the Properties window is visible.
 *
 * Returns: %TRUE to automatically play music clips
 **/
gboolean
gva_preferences_get_auto_play (void)
{
        GtkToggleAction *toggle_action;

        toggle_action = GTK_TOGGLE_ACTION (GVA_ACTION_AUTO_PLAY);

        return gtk_toggle_action_get_active (toggle_action);
}

/**
 * gva_preferences_set_auto_play:
 * @auto_play: the user's preference
 *
 * Accepts the user's preference for whether to automatically
 * play a music clip of the selected game when the user opens
 * the Properties window or when the user selects a different
 * game while the Properties window is visible.
 *
 * The preference is stored in GConf key
 * <filename>/apps/gnome-video-arcade/auto-play</filename>.
 **/
void
gva_preferences_set_auto_play (gboolean auto_play)
{
        GtkToggleAction *toggle_action;

        toggle_action = GTK_TOGGLE_ACTION (GVA_ACTION_AUTO_PLAY);

        gtk_toggle_action_set_active (toggle_action, auto_play);
}

/**
 * gva_preferences_get_auto_save:
 *
 * Returns the user's preference for whether to restore the emulated
 * machine's previous state when starting a game.
 *
 * Returns: %TRUE to restore state, %FALSE to cold boot
 **/
gboolean
gva_preferences_get_auto_save (void)
{
        GtkToggleAction *toggle_action;

        toggle_action = GTK_TOGGLE_ACTION (GVA_ACTION_AUTO_SAVE);

        return gtk_toggle_action_get_active (toggle_action);
}

/**
 * gva_preferences_set_auto_save:
 * @auto_save: the user's preference
 *
 * Accepts the user's preference for whether to restore the emulated
 * machine's previous state when starting a game.
 *
 * The preference is stored in GConf key
 * <filename>/apps/gnome-video-arcade/auto-save</filename>.
 **/
void
gva_preferences_set_auto_save (gboolean auto_save)
{
        GtkToggleAction *toggle_action;

        toggle_action = GTK_TOGGLE_ACTION (GVA_ACTION_AUTO_SAVE);

        gtk_toggle_action_set_active (toggle_action, auto_save);
}

/**
 * gva_preferences_get_full_screen:
 *
 * Returns the user's preference for whether to start games in full
 * screen mode.
 *
 * Returns: %TRUE to start in full screen mode, %FALSE to start in a window
 **/
gboolean
gva_preferences_get_full_screen (void)
{
        GtkToggleAction *toggle_action;

        toggle_action = GTK_TOGGLE_ACTION (GVA_ACTION_FULL_SCREEN);

        return gtk_toggle_action_get_active (toggle_action);
}

/**
 * gva_preferences_set_full_screen:
 * @full_screen: the user's preference
 *
 * Accepts the user's preference for whether to start games in full
 * screen mode.
 *
 * The preference is stored in GConf key
 * <filename>/apps/gnome-video-arcade/full-screen</filename>.
 **/
void
gva_preferences_set_full_screen (gboolean full_screen)
{
        GtkToggleAction *toggle_action;

        toggle_action = GTK_TOGGLE_ACTION (GVA_ACTION_FULL_SCREEN);

        gtk_toggle_action_set_active (toggle_action, full_screen);
}

/**
 * gva_preferences_get_show_clones:
 *
 * Returns the user's preference for whether to show cloned games in
 * the main window.
 *
 * Returns: %TRUE to show cloned games, %FALSE to only show original games
 **/
gboolean
gva_preferences_get_show_clones (void)
{
        GtkToggleAction *toggle_action;

        toggle_action = GTK_TOGGLE_ACTION (GVA_ACTION_SHOW_CLONES);

        return gtk_toggle_action_get_active (toggle_action);
}

/**
 * gva_preferences_set_show_clones:
 * @show_clones: the user's preference
 *
 * Accepts the user's preference for whether to show cloned games in
 * the main window.
 *
 * The preference is stored in GConf key
 * <filename>/apps/gnome-video-arcade/show-clones</filename>.
 **/
void
gva_preferences_set_show_clones (gboolean show_clones)
{
        GtkToggleAction *toggle_action;

        toggle_action = GTK_TOGGLE_ACTION (GVA_ACTION_SHOW_CLONES);

        gtk_toggle_action_set_active (toggle_action, show_clones);
}

/**
 * gva_preferences_close_clicked_cb:
 * @window: the "Preferences" window
 * @button: the "Close" button
 *
 * Handler for #GtkButton::clicked signals to the "Close" button.
 *
 * Hides @window.
 **/
void
gva_preferences_close_clicked_cb (GtkWindow *window,
                                  GtkButton *button)
{
        gtk_widget_hide (GTK_WIDGET (window));
}
