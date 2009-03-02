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

#include "gva-nplayers.h"

static GKeyFile *keyfile = NULL;

static void
nplayers_parse (const gchar *string,
                guint *max_alt,
                guint *max_sim)
{
        guint max_players;
        gboolean parsable;

        /* Sanity check the string. */
        parsable =
                (strlen (string) > 2) &&
                g_ascii_isdigit (string[0]) &&
                string[1] == 'P';

        if (!parsable)
                return;

        max_players = string[0] - '0';

        if (max_alt != NULL && g_str_has_suffix (string, " alt"))
                *max_alt = max_players;

        if (max_sim != NULL && g_str_has_suffix (string, " sim"))
                *max_sim = max_players;
}

/**
 * gva_nplayers_init:
 * @error: return location for a #GError, or %NULL
 *
 * Loads the number of players file.  If an error occurs, it returns
 * %FALSE and sets @error.
 *
 * This function should be called once when the application starts.
 *
 * Returns: %TRUE on success, %FALSE if an error occurred
 **/
gboolean
gva_nplayers_init (GError **error)
{
        gboolean success = FALSE;
#ifdef NPLAYERS_FILE
        gchar *contents, *cp;
        gsize length;
#endif

        keyfile = g_key_file_new ();

#ifdef NPLAYERS_FILE
        if (!g_file_get_contents (NPLAYERS_FILE, &contents, &length, error))
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
                _("This program is not configured to show"
                  "detailed number of players information."));
#endif

        return success;
}

/**
 * gva_nplayers_lookup:
 * @game: the name of a game
 * @max_alt: return location for the maximum number of alternating players
 * @max_sim: return location for the maximum number of simultaneous players
 * @error: return location for a #GError, or %NULL
 *
 * Returns the maximum number of alternating and/or simultaneous players
 * for @game.  If @game only allows alternating players, @max_sim will be
 * zero.  If @game only allows simultaneous players, @max_alt will be zero.
 * If @game is listed in the file but the number of players is unknown or
 * cannot be parsed, both @max_alt and @max_sim will be zero.  In all of
 * these cases the function returns %TRUE.
 *
 * If an error occurs, the function returns %FALSE and sets @error, leaving
 * @max_alt and @max_sim unaltered.
 *
 * Returns: %TRUE on success, %FALSE if an error occurred
 **/
gboolean
gva_nplayers_lookup (const gchar *game,
                     guint *max_alt,
                     guint *max_sim,
                     GError **error)
{
        gchar *nplayers;
        gchar *cp;

        nplayers = g_key_file_get_value (keyfile, "NPlayers", game, error);
        if (nplayers == NULL)
                return FALSE;

        if (max_alt != NULL)
                *max_alt = 0;

        if (max_sim != NULL)
                *max_sim = 0;

        cp = strchr (nplayers, '/');
        if (cp != NULL)
        {
                *cp++ = '\0';
                nplayers_parse (g_strstrip (cp), max_alt, max_sim);
        }

        nplayers_parse (g_strstrip (nplayers), max_alt, max_sim);

        return TRUE;
}
