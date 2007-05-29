#include "gva-preferences.h"

#include "gva-ui.h"

void
gva_preferences_init (void)
{
        GtkWindow *window;

        window = GTK_WINDOW (GVA_WIDGET_PREFERENCES_WINDOW);

        g_signal_connect (
                GVA_WIDGET_PREFERENCES_WINDOW, "delete-event",
                G_CALLBACK (gtk_widget_hide_on_delete), NULL);
        g_signal_connect_swapped (
                GVA_WIDGET_PREFERENCES_CLOSE_BUTTON, "clicked",
                G_CALLBACK (gtk_widget_hide), window);
}
