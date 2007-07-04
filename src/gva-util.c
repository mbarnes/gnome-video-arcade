#include "gva-util.h"

#include "gva-error.h"
#include "gva-xmame.h"

#define GCONF_MONOSPACE_FONT_NAME_KEY \
        "/desktop/gnome/interface/monospace_font_name"

static gboolean
inpname_exists (const gchar *inppath, const gchar *inpname)
{
        gchar *inpfile;
        gchar *filename;
        gboolean exists;

        inpfile = g_strdup_printf ("%s.inp", inpname);
        filename = g_build_filename (inppath, inpfile, NULL);
        exists = g_file_test (filename, G_FILE_TEST_EXISTS);
        g_free (filename);
        g_free (inpfile);

        return exists;
}

gchar *
gva_choose_inpname (const gchar *romname)
{
        gchar *inpname;
        gchar *inppath;
        gint nn = 1;
        GError *error = NULL;

        g_return_val_if_fail (romname != NULL, NULL);

        inppath = gva_xmame_get_config_value ("input_directory", &error);

        if (inppath == NULL || !inpname_exists (inppath, romname))
        {
                gva_error_handle (&error);
                inpname = g_strdup (romname);
        }
        else while (TRUE)
        {
                inpname = g_strdup_printf ("%s-%d", romname, nn++);
                if (!inpname_exists (inppath, inpname))
                        break;
                g_free (inpname);
        }

        g_free (inppath);

        return inpname;
}

gchar *
gva_find_data_file (const gchar *basename)
{
        const gchar * const *datadirs;
        gchar *filename;

        g_return_val_if_fail (basename != NULL, NULL);

        /* Support running directly from the source tree. */
        filename = g_build_filename ("..", "data", basename, NULL);
        if (g_file_test (filename, G_FILE_TEST_EXISTS))
                return filename;
        g_free (filename);

        datadirs = g_get_system_data_dirs ();
        while (*datadirs != NULL)
        {
                filename = g_build_filename (
                        *datadirs++, PACKAGE, basename, NULL);
                if (g_file_test (filename, G_FILE_TEST_EXISTS))
                        return filename;
                g_free (filename);
        }

        return NULL;
}

gchar *
gva_get_monospace_font_name (void)
{
        GConfClient *client;
        PangoFontDescription *desc;
        gchar *font_name;
        GError *error = NULL;

        client = gconf_client_get_default ();
        font_name = gconf_client_get_string (
                client, GCONF_MONOSPACE_FONT_NAME_KEY, &error);
        gva_error_handle (&error);
        g_object_unref (client);

        return font_name;
}
