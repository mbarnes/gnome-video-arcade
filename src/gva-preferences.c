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
#include "gva-util.h"

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
        GSettings *settings;

        settings = gva_get_settings ();

        /* Auto Save */

        gtk_action_set_sensitive (
                GVA_ACTION_AUTO_SAVE,
                gva_mame_supports_auto_save ());

        g_settings_bind (
                settings, GVA_SETTING_AUTO_SAVE,
                GVA_ACTION_AUTO_SAVE, "active",
                G_SETTINGS_BIND_DEFAULT |
                G_SETTINGS_BIND_NO_SENSITIVITY);

        /* Full Screen */

        gtk_action_set_sensitive (
                GVA_ACTION_FULL_SCREEN,
                gva_mame_supports_window ());

        g_settings_bind (
                settings, GVA_SETTING_FULL_SCREEN,
                GVA_ACTION_FULL_SCREEN, "active",
                G_SETTINGS_BIND_DEFAULT |
                G_SETTINGS_BIND_NO_SENSITIVITY);

        /* Show Clones */

        g_settings_bind (
                settings, GVA_SETTING_SHOW_CLONES,
                GVA_ACTION_SHOW_CLONES, "active",
                G_SETTINGS_BIND_DEFAULT);
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
 * The preference is stored in GSettings key <filename>auto-save</filename>.
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
 * The preference is stored in GSettings key <filename>full-screen</filename>.
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
 * The preference is stored in GSettings key <filename>show-clones</filename>.
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
