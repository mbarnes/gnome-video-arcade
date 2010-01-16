/* Copyright 2007-2010 Matthew Barnes
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

#include "gva-error.h"

static GKeyFile *keyfile = NULL;

static void
nplayers_parse (const gchar *string,
                gint *max_alternating,
                gint *max_simultaneous)
{
        gint max_players;
        gboolean parsable;

        /* Sanity check the string. */
        parsable = (strlen (string) >= 2) &&
                g_ascii_isdigit (string[0]) &&
                string[1] == 'P';

        if (!parsable)
                return;

        max_players = string[0] - '0';

        /* "1P" is a special case.  We'll return it as alternating. */
        if (max_alternating != NULL && max_players == 1)
                *max_alternating = max_players;

        if (max_alternating != NULL && g_str_has_suffix (string, " alt"))
                *max_alternating = max_players;

        if (max_simultaneous != NULL && g_str_has_suffix (string, " sim"))
                *max_simultaneous = max_players;
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
                _("This program is not configured to show "
                  "detailed number of players information."));
#endif

        return success;
}

/**
 * gva_nplayers_lookup:
 * @game: the name of a game
 * @max_alternating: return location for the maximum alternating players
 * @max_simultaneous: return location for the maximum simultaneous players
 * @error: return location for a #GError, or %NULL
 *
 * Returns the maximum number of alternating and/or simultaneous players for
 * @game.  If @game only allows alternating players, @max_simultaneous will
 * be zero.  If @game only allows simultaneous players, @max_alternating will
 * be zero.  If @game is listed in the file but the number of players is
 * unknown or cannot be parsed, both @max_alternating and @max_simultaneous
 * will be zero.  In all of these cases the function returns %TRUE.
 *
 * If an error occurs, the function returns %FALSE and sets @error, leaving
 * @max_alternating and @max_simultaneous unaltered.
 *
 * Returns: %TRUE on success, %FALSE if an error occurred
 **/
gboolean
gva_nplayers_lookup (const gchar *game,
                     gint *max_alternating,
                     gint *max_simultaneous,
                     GError **error)
{
        gchar *nplayers;
        gchar *cp;

        nplayers = g_key_file_get_value (keyfile, "NPlayers", game, error);
        if (nplayers == NULL)
                return FALSE;

        if (max_alternating != NULL)
                *max_alternating = 0;

        if (max_simultaneous != NULL)
                *max_simultaneous = 0;

        cp = strchr (nplayers, '/');
        if (cp != NULL)
        {
                *cp++ = '\0';
                nplayers_parse (
                        g_strstrip (cp), max_alternating, max_simultaneous);
        }

        nplayers_parse (
                g_strstrip (nplayers), max_alternating, max_simultaneous);

        return TRUE;
}

/**
 * gva_nplayers_describe:
 * @max_alternating: the maximum alternating players
 * @max_simultaneous: the maximum simultaneous players
 *
 * Returns a human-readable description of the number of players a game
 * supports and whether the players alternate, play simultaneously, or
 * both.  If a description can not be formed, the function returns %NULL.
 *
 * Returns: a human-readable description
 **/
const gchar *
gva_nplayers_describe (gint max_alternating,
                       gint max_simultaneous)
{
        g_return_val_if_fail (max_alternating >= 0, NULL);
        g_return_val_if_fail (max_simultaneous >= 0, NULL);

        /* XXX Known combinations are hard-coded. */

        if (max_alternating == 4 && max_simultaneous == 2)
                return _("Up to four players alternating "
                         "or two players simultaneously");

        if (max_alternating == 8 && max_simultaneous == 2)
                return _("Up to eight players alternating "
                         "or two players simultaneously");

        /* XXX This is a stupid hack, but it works for now and keeps all
         *     the descriptions in one place.  Should only get triggered
         *     if we're getting player information from database queries
         *     instead of an nplayers.ini file. */
        if (max_alternating == max_simultaneous)
        {
                switch (max_alternating)
                {
                        case 0:
                                break;

                        case 1:
                                return _("One player only");

                        case 2:
                                return _("One or two players");

                        case 3:
                                return _("Up to three players");

                        case 4:
                                return _("Up to four players");

                        case 5:
                                return _("Up to five players");

                        case 6:
                                return _("Up to six players");

                        case 7:
                                return _("Up to seven players");

                        case 8:
                                return _("Up to eight players");

                        default:
                                break;
                }
        }

        if (max_alternating != 0 && max_simultaneous != 0)
        {
                g_warning (
                        "Can't describe number of players: "
                        "%dP alt / %dP sim", max_alternating,
                        max_simultaneous);
                return NULL;
        }

        switch (max_alternating)
        {
                case 0:
                        break;

                case 1:
                        return _("One player only");

                case 2:
                        return _("One or two players alternating");

                case 3:
                        return _("Up to three players alternating");

                case 4:
                        return _("Up to four players alternating");

                case 5:
                        return _("Up to five players alternating");

                case 6:
                        return _("Up to six players alternating");

                case 7:
                        return _("Up to seven players alternating");

                case 8:
                        return _("Up to eight players alternating");

                default:
                        g_warning (
                                "Can't describe number of players: "
                                "%dP alt", max_alternating);
                        break;
        }

        switch (max_simultaneous)
        {
                case 0:
                        break;

                case 1:
                        return _("One player only");

                case 2:
                        return _("One or two players simultaneously");

                case 3:
                        return _("Up to three players simultaneously");

                case 4:
                        return _("Up to four players simultaneously");

                case 5:
                        return _("Up to five players simultaneously");

                case 6:
                        return _("Up to six players simultaneously");

                case 7:
                        return _("Up to seven players simultaneously");

                case 8:
                        return _("Up to eight players simultaneously");

                default:
                        g_warning (
                                "Can't describe number of players: "
                                "%dP sim", max_simultaneous);
        }

        return NULL;
}
