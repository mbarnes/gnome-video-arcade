#include "gva-preferences.h"

#include "gva-ui.h"
#include "gva-xmame.h"

void
gva_preferences_init (void)
{
        GtkWindow *window;
        gboolean active;

        window = GTK_WINDOW (GVA_WIDGET_PREFERENCES_WINDOW);

        gtk_action_connect_proxy (
                GVA_ACTION_AUTO_SAVE,
                GVA_WIDGET_PREFERENCES_AUTO_SAVE);

        gtk_action_connect_proxy (
                GVA_ACTION_FULL_SCREEN,
                GVA_WIDGET_PREFERENCES_FULL_SCREEN);

        g_signal_connect (
                GVA_WIDGET_PREFERENCES_WINDOW, "delete-event",
                G_CALLBACK (gtk_widget_hide_on_delete), NULL);

        g_signal_connect_swapped (
                GVA_WIDGET_PREFERENCES_CLOSE_BUTTON, "clicked",
                G_CALLBACK (gtk_widget_hide), window);

        gtk_toggle_action_set_active (
                GTK_TOGGLE_ACTION (GVA_ACTION_AUTO_SAVE),
                gva_preferences_get_auto_save ());

        gtk_action_set_sensitive (
                GVA_ACTION_AUTO_SAVE,
                gva_xmame_supports_auto_save ());

        gtk_toggle_action_set_active (
                GTK_TOGGLE_ACTION (GVA_ACTION_FULL_SCREEN),
                gva_preferences_get_full_screen ());

        gtk_action_set_sensitive (
                GVA_ACTION_FULL_SCREEN,
                gva_xmame_supports_full_screen ());
}

gboolean
gva_preferences_get_auto_save (void)
{
        GConfClient *client;
        gboolean auto_save;
        GError *error = NULL;

        client = gconf_client_get_default ();
        auto_save = gconf_client_get_bool (
                client, GVA_GCONF_AUTO_SAVE_KEY, &error);
        g_object_unref (client);

        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_clear_error (&error);
        }

        return auto_save;
}

void
gva_preferences_set_auto_save (gboolean auto_save)
{
        GConfClient *client;
        GError *error = NULL;

        client = gconf_client_get_default ();
        gconf_client_set_bool (
                client, GVA_GCONF_AUTO_SAVE_KEY, auto_save, &error);
        g_object_unref (client);

        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_clear_error (&error);
        }
}

gboolean
gva_preferences_get_full_screen (void)
{
        GConfClient *client;
        gboolean full_screen;
        GError *error = NULL;

        client = gconf_client_get_default ();
        full_screen = gconf_client_get_bool (
                client, GVA_GCONF_FULL_SCREEN_KEY, &error);
        g_object_unref (client);

        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_clear_error (&error);
        }

        return full_screen;
}

void
gva_preferences_set_full_screen (gboolean full_screen)
{
        GConfClient *client;
        GError *error = NULL;

        client = gconf_client_get_default ();
        gconf_client_set_bool (
                client, GVA_GCONF_FULL_SCREEN_KEY, full_screen, &error);
        g_object_unref (client);

        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_clear_error (&error);
        }
}
