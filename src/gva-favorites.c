/* Copyright 2007-2015 Matthew Barnes
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
#include "gva-util.h"

static GList *favorites = NULL;
static gboolean initialized = FALSE;

static void
favorites_load (void)
{
        GSettings *settings;
        GVariantIter *iter;
        gchar *string;

        g_return_if_fail (favorites == NULL);

        settings = gva_get_settings ();
        g_settings_get (settings, GVA_SETTING_FAVORITES, "as", &iter);
        while (g_variant_iter_loop (iter, "s", &string))
        {
                const gchar *game = g_intern_string (string);
                favorites = g_list_prepend (favorites, (gpointer) game);
        }
        g_variant_iter_free (iter);

        favorites = g_list_sort (favorites, (GCompareFunc) strcmp);

        initialized = TRUE;
}

static void
favorites_save (void)
{
        GSettings *settings;
        GVariantBuilder builder;
        GVariant *variant;
        GList *iter;

        settings = gva_get_settings ();

        g_variant_builder_init (&builder, (GVariantType *) "as");
        for (iter = favorites; iter != NULL; iter = iter->next)
                g_variant_builder_add (&builder, "s", iter->data);
        variant = g_variant_builder_end (&builder);

        /* This consumes the floating GVariant reference. */
        g_settings_set_value (settings, GVA_SETTING_FAVORITES, variant);
}

/**
 * gva_favorites_copy:
 *
 * Returns a copy of the favorite games list.  The contents of the list
 * must not be freed.  The list itself should be freed with g_list_free().
 *
 * Returns: a copy of the favorite games list
 **/
GList *
gva_favorites_copy (void)
{
        if (G_UNLIKELY (!initialized))
                favorites_load ();

        return g_list_copy (favorites);
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

        if (g_list_find (favorites, game) != NULL)
                return;

        favorites = g_list_insert_sorted (
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

        favorites = g_list_remove_all (favorites, game);

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

        return (g_list_find (favorites, game) != NULL);
}
