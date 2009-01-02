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

#include "gva-favorites.h"

#include <string.h>

#include "gva-error.h"

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
        gva_error_handle (&error);

        for (iter = favorites; iter != NULL; iter = iter->next)
        {
                gchar *game = iter->data;
                iter->data = (gchar *) g_intern_string (game);
                g_free (game);
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
        gva_error_handle (&error);
}

/**
 * gva_favorites_copy:
 *
 * Returns a copy of the favorite games list.  The contents of the list
 * must not be freed.  The list itself should be freed with g_slist_free().
 *
 * Returns: a copy of the favorite games list
 **/
GSList *
gva_favorites_copy (void)
{
        if (G_UNLIKELY (!initialized))
                favorites_load ();

        return g_slist_copy (favorites);
}

/**
 * gva_favorites_insert:
 * @game: the name of a game
 *
 * Inserts @game into the favorite games list.
 **/
void
gva_favorites_insert (const gchar *game)
{
        if (G_UNLIKELY (!initialized))
                favorites_load ();

        g_return_if_fail (game != NULL);

        game = g_intern_string (game);

        if (g_slist_find (favorites, game) != NULL)
                return;

        favorites = g_slist_insert_sorted (
                favorites, (gchar *) game, (GCompareFunc) strcmp);

        favorites_save ();
}

/**
 * gva_favorites_remove:
 * @game: the name of a game
 *
 * Removes @game from the favorite games list.
 **/
void
gva_favorites_remove (const gchar *game)
{
        if (G_UNLIKELY (!initialized))
                favorites_load ();

        g_return_if_fail (game != NULL);

        game = g_intern_string (game);

        favorites = g_slist_remove_all (favorites, game);

        favorites_save ();
}

/**
 * gva_favorites_contains:
 * @game: the name of a game
 *
 * Returns %TRUE if the favorite games list contains @game.
 *
 * Returns: %TRUE if @game is a favorite, otherwise %FALSE
 **/
gboolean
gva_favorites_contains (const gchar *game)
{
        if (G_UNLIKELY (!initialized))
                favorites_load ();

        g_return_val_if_fail (game != NULL, FALSE);

        game = g_intern_string (game);

        return (g_slist_find (favorites, game) != NULL);
}
