#include "gva-ui.h"

#include <glade/glade.h>

#include "gva-actions.h"
#include "gva-util.h"

static GladeXML *xml = NULL;
static GtkUIManager *manager = NULL;
static gboolean initialized = FALSE;

static void
gva_ui_init (void)
{
        GtkWidget *widget;
        gchar *filename;

        filename = gva_find_data_file (PACKAGE ".glade");
        if (filename != NULL)
                xml = glade_xml_new (filename, NULL, NULL);
        g_free (filename);

        filename = gva_find_data_file (PACKAGE ".ui");
        if (filename != NULL)
        {
                manager = gtk_ui_manager_new ();
                gtk_ui_manager_add_ui_from_file (
                        manager, filename, NULL);
                gtk_ui_manager_insert_action_group (
                        manager, gva_get_action_group (), 0);
        }
        g_free (filename);

        if (xml == NULL || manager == NULL)
                g_error ("%s", _("Failed to initialize user interface"));

        initialized = TRUE;
}

GtkWidget *
gva_ui_get_glade_widget (const gchar *name)
{
        GtkWidget *widget;

        g_return_val_if_fail (name != NULL, NULL);

        if (G_UNLIKELY (!initialized))
                gva_ui_init ();

        widget = glade_xml_get_widget (xml, name);
        g_assert (widget != NULL);
        return widget;
}

GtkWidget *
gva_ui_get_managed_widget (const gchar *path)
{
        GtkWidget *widget;

        g_return_val_if_fail (path != NULL, NULL);

        if (G_UNLIKELY (!initialized))
                gva_ui_init ();

        widget = gtk_ui_manager_get_widget (manager, path);
        g_assert (widget != NULL);
        return widget;
}
