#include "gva-favorites.h"

#include <string.h>

static GSList *favorites = NULL;
static GConfClient *client = NULL;
static gboolean initialized = FALSE;

static void
favorites_load (void)
{
        GSList *iter;
        GError *error = NULL;

        client = gconf_client_get_default ();

        favorites = gconf_client_get_list (
                client, GVA_GCONF_FAVORITES_KEY,
                GCONF_VALUE_STRING, &error);
        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_clear_error (&error);
        }

        for (iter = favorites; iter != NULL; iter = iter->next)
        {
                gchar *romname = iter->data;
                iter->data = (gchar *) g_intern_string (romname);
                g_free (romname);
        }

        favorites = g_slist_sort (favorites, (GCompareFunc) strcmp);

        initialized = TRUE;
}

static void
favorites_save (void)
{
        GError *error = NULL;

        gconf_client_set_list (
                client, GVA_GCONF_FAVORITES_KEY,
                GCONF_VALUE_STRING, favorites, &error);
        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_clear_error (&error);
        }
}

GSList *
gva_favorites_copy (void)
{
        if (G_UNLIKELY (!initialized))
                favorites_load ();

        return g_slist_copy (favorites);
}

void
gva_favorites_insert (const gchar *romname)
{
        if (G_UNLIKELY (!initialized))
                favorites_load ();

        g_return_if_fail (romname != NULL);

        romname = g_intern_string (romname);

        if (g_slist_find (favorites, romname) != NULL)
                return;

        favorites = g_slist_insert_sorted (
                favorites, (gchar *) romname, (GCompareFunc) strcmp);

        favorites_save ();
}

void
gva_favorites_remove (const gchar *romname)
{
        if (G_UNLIKELY (!initialized))
                favorites_load ();

        g_return_if_fail (romname != NULL);

        romname = g_intern_string (romname);

        favorites = g_slist_remove_all (favorites, romname);

        favorites_save ();
}

gboolean
gva_favorites_contains (const gchar *romname)
{
        if (G_UNLIKELY (!initialized))
                favorites_load ();

        g_return_val_if_fail (romname != NULL, FALSE);

        romname = g_intern_string (romname);

        return (g_slist_find (favorites, romname) != NULL);
}
