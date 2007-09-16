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

#include "gva-search.h"

#include <string.h>

#include "gva-error.h"
#include "gva-tree-view.h"
#include "gva-ui.h"

static void
search_window_visible_notify_cb (GtkWindow *window,
                                 GParamSpec *pspec,
                                 GtkEntry *entry)
{
        if (GTK_WIDGET_VISIBLE (window))
                return;

        gtk_widget_grab_focus (GTK_WIDGET (entry));
        gtk_editable_select_region (GTK_EDITABLE (entry), 0, -1);
}

static void
search_entry_changed_cb (GtkEntry *entry,
                         GtkWidget *widget)
{
        gchar *text;

        text = g_strstrip (g_strdup (gtk_entry_get_text (entry)));
        gtk_widget_set_sensitive (widget, strlen (text) > 0);
        g_free (text);
}

/**
 * gva_search_init:
 *
 * Initializes the Search window.
 *
 * This function should be called once when the application starts.
 **/
void
gva_search_init (void)
{
        gchar *text;

        g_signal_connect (
                GVA_WIDGET_SEARCH_WINDOW, "notify::visible",
                G_CALLBACK (search_window_visible_notify_cb),
                GVA_WIDGET_SEARCH_ENTRY);

        g_signal_connect (
                GVA_WIDGET_SEARCH_ENTRY, "changed",
                G_CALLBACK (search_entry_changed_cb),
                GVA_WIDGET_SEARCH_FIND_BUTTON);

        text = g_strstrip (gva_search_get_last_search ());
        gtk_entry_set_text (GTK_ENTRY (GVA_WIDGET_SEARCH_ENTRY), text);
	g_object_notify (G_OBJECT (GVA_WIDGET_SEARCH_WINDOW), "visible");
        g_free (text);
}

/**
 * gva_search_get_last_search:
 *
 * Returns the criteria of the most recent search in either the current
 * or the previous session of GNOME Video Arcade.
 *
 * Returns: the criteria of the most recent search
 **/
gchar *
gva_search_get_last_search (void)
{
        GConfClient *client;
        gchar *text;
        GError *error = NULL;

        client = gconf_client_get_default ();
        text = gconf_client_get_string (client, GVA_GCONF_SEARCH_KEY, &error);
        gva_error_handle (&error);
        g_object_unref (client);

        return (text != NULL) ? g_strstrip (text) : g_strdup ("");
}

/**
 * gva_search_set_last_search:
 * @text: the search text
 *
 * Writes @text to GConf key
 * <filename>/apps/gnome-video-arcade/search</filename>.
 *
 * This is used to remember the search text from the previous session of
 * GNOME Video Arcade so that the same text can be preset in the Search
 * window at startup.
 **/
void
gva_search_set_last_search (const gchar *text)
{
        GConfClient *client;
        GError *error = NULL;

        g_return_if_fail (text != NULL);

        client = gconf_client_get_default ();
        gconf_client_set_string (client, GVA_GCONF_SEARCH_KEY, text, &error);
        gva_error_handle (&error);
        g_object_unref (client);
}

/**
 * gva_search_find_clicked_cb:
 * @entry: the search entry
 * @button: the "Find" button
 *
 * Handler for #GtkButton::clicked signals to the "Find" button.
 *
 * Save the contents of the search entry and switches the main window to
 * the "Search Results" view.
 **/
void
gva_search_find_clicked_cb (GtkEntry *entry,
                            GtkButton *button)
{
        gva_search_set_last_search (gtk_entry_get_text (entry));

        /* Force a tree view update. */
        if (gva_tree_view_get_selected_view () != 2)
                gva_tree_view_set_selected_view (2);
        else
        {
                GError *error = NULL;

                gva_tree_view_update (&error);
                gva_error_handle (&error);
        }
}
