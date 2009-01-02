/* Copyright 2007-2009 Matthew Barnes
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
        /* Auto Save */

        gtk_action_connect_proxy (
                GVA_ACTION_AUTO_SAVE,
                GVA_WIDGET_PREFERENCES_AUTO_SAVE);

        gtk_action_set_sensitive (
                GVA_ACTION_AUTO_SAVE,
                gva_mame_supports_auto_save ());

        gconf_bridge_bind_property (
                gconf_bridge_get (), GVA_GCONF_AUTO_SAVE_KEY,
                G_OBJECT (GVA_ACTION_AUTO_SAVE), "active");

        /* Full Screen */

        gtk_action_connect_proxy (
                GVA_ACTION_FULL_SCREEN,
                GVA_WIDGET_PREFERENCES_FULL_SCREEN);

        gtk_action_set_sensitive (
                GVA_ACTION_FULL_SCREEN,
                gva_mame_supports_full_screen () ||
                gva_mame_supports_window ());

        gconf_bridge_bind_property (
                gconf_bridge_get (), GVA_GCONF_FULL_SCREEN_KEY,
                G_OBJECT (GVA_ACTION_FULL_SCREEN), "active");

        /* Show Clones */

        gtk_action_connect_proxy (
                GVA_ACTION_SHOW_CLONES,
                GVA_WIDGET_PREFERENCES_SHOW_CLONES);

        gconf_bridge_bind_property (
                gconf_bridge_get (), GVA_GCONF_SHOW_CLONES_KEY,
                G_OBJECT (GVA_ACTION_SHOW_CLONES), "active");
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
        return gtk_toggle_action_get_active (
                GTK_TOGGLE_ACTION (GVA_ACTION_AUTO_SAVE));
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
        gtk_toggle_action_set_active (
                GTK_TOGGLE_ACTION (GVA_ACTION_AUTO_SAVE), auto_save);
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
        return gtk_toggle_action_get_active (
                GTK_TOGGLE_ACTION (GVA_ACTION_FULL_SCREEN));
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
        gtk_toggle_action_set_active (
                GTK_TOGGLE_ACTION (GVA_ACTION_FULL_SCREEN), full_screen);
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
        return gtk_toggle_action_get_active (
                GTK_TOGGLE_ACTION (GVA_ACTION_SHOW_CLONES));
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
        gtk_toggle_action_set_active (
                GTK_TOGGLE_ACTION (GVA_ACTION_SHOW_CLONES), show_clones);
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
