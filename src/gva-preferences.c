/* Copyright 2007 Matthew Barnes
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

static void
preferences_init_general (void)
{
        /* Auto Save */

        gtk_action_connect_proxy (
                GVA_ACTION_AUTO_SAVE,
                GVA_WIDGET_PREFERENCES_AUTO_SAVE);

        gtk_action_set_sensitive (
                GVA_ACTION_AUTO_SAVE,
                gva_mame_supports_auto_save ());

        gtk_toggle_action_set_active (
                GTK_TOGGLE_ACTION (GVA_ACTION_AUTO_SAVE),
                gva_preferences_get_auto_save ());

        /* Full Screen */

        gtk_action_connect_proxy (
                GVA_ACTION_FULL_SCREEN,
                GVA_WIDGET_PREFERENCES_FULL_SCREEN);

        gtk_action_set_sensitive (
                GVA_ACTION_FULL_SCREEN,
                gva_mame_supports_full_screen () ||
                gva_mame_supports_window ());

        gtk_toggle_action_set_active (
                GTK_TOGGLE_ACTION (GVA_ACTION_FULL_SCREEN),
                gva_preferences_get_full_screen ());
}

static void
preferences_init_columns (void)
{
        GtkWidget *column_manager;
        GtkTreeView *view;

        view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);
        column_manager = gva_column_manager_new (view);

        gtk_box_pack_start (
                GTK_BOX (GVA_WIDGET_PREFERENCES_COLUMNS_INNER_VBOX),
                column_manager, TRUE, TRUE, 0);
}

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
        preferences_init_general ();
        preferences_init_columns ();
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
        GConfClient *client;
        gboolean auto_save;
        GError *error = NULL;

        client = gconf_client_get_default ();
        auto_save = gconf_client_get_bool (
                client, GVA_GCONF_AUTO_SAVE_KEY, &error);
        gva_error_handle (&error);
        g_object_unref (client);

        return auto_save;
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
        GConfClient *client;
        GError *error = NULL;

        client = gconf_client_get_default ();
        gconf_client_set_bool (
                client, GVA_GCONF_AUTO_SAVE_KEY, auto_save, &error);
        gva_error_handle (&error);
        g_object_unref (client);
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
        GConfClient *client;
        gboolean full_screen;
        GError *error = NULL;

        client = gconf_client_get_default ();
        full_screen = gconf_client_get_bool (
                client, GVA_GCONF_FULL_SCREEN_KEY, &error);
        gva_error_handle (&error);
        g_object_unref (client);

        return full_screen;
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
        GConfClient *client;
        GError *error = NULL;

        client = gconf_client_get_default ();
        gconf_client_set_bool (
                client, GVA_GCONF_FULL_SCREEN_KEY, full_screen, &error);
        gva_error_handle (&error);
        g_object_unref (client);
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
