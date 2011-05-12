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

#include "gva-categories.h"

#include <string.h>
#include "gva-error.h"

static GKeyFile *keyfile = NULL;

/**
 * gva_categories_init:
 * @error: return location for a #GError, or %NULL
 *
 * Loads the category file.  If an error occurs, it returns %FALSE and
 * sets @error.
 *
 * This function should be called once when the application starts.
 *
 * Returns: %TRUE on success, %FALSE if an error occurred.
 **/
gboolean
gva_categories_init (GError **error)
{
        gboolean success = FALSE;
#ifdef CATEGORY_FILE
        gchar *contents, *cp;
        gsize length;
#endif

        keyfile = g_key_file_new ();

#ifdef CATEGORY_FILE
        if (!g_file_get_contents (CATEGORY_FILE, &contents, &length, error))
                return FALSE;

        /* Convert Windows INI-style comments (lines beginning with ';')
         * to a form recognized by the GKeyFile parser. */
        cp = contents;
        while (cp != NULL && *cp != '\0')
        {
                if (*cp == ';')
                        *cp = '#';
                /* Find the next line. */
                cp = strchr (cp, '\n');
                if (cp != NULL)
                        cp++;
        }

        success = g_key_file_load_from_data (
                keyfile, contents, length, G_KEY_FILE_NONE, error);

        g_free (contents);
#else
        g_message (
                "This program is not configured "
                "to show category information.");
#endif

        return success;
}

/**
 * gva_categories_lookup:
 * @game: the name of a game
 * @error: return location for a #GError, or %NULL
 *
 * Returns the category for @game.  If an error occurs, it returns
 * %NULL and sets @error.
 *
 * Returns: category for @game, or %NULL if an error occurred
 **/
gchar *
gva_categories_lookup (const gchar *game,
                       GError **error)
{
        return g_key_file_get_value (keyfile, "Category", game, error);
}

/**
 * gva_mame_version_lookup:
 * @game: the name of a game
 * @error: return location for a #GError, or %NULL
 *
 * Returns the initial MAME version for @game.  If an error occurs, it
 * returns %NULL and sets @error.
 *
 * Returns: initial MAME version for @game, or %NULL if an error occurred
 **/
gchar *
gva_mame_version_lookup (const gchar *game,
                         GError **error)
{
        return g_key_file_get_value (keyfile, "VerAdded", game, error);
}
