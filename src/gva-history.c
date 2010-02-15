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

#include "gva-history.h"

#include <stdlib.h>

#include "gva-error.h"

#define BASE_URI "http://www.arcade-history.com/"

typedef struct _HistoryEntry HistoryEntry;

struct _HistoryEntry {
        guint id;
        goffset offset;
};

static GPtrArray *history_file_array = NULL;
static GHashTable *history_file_table = NULL;

static GIOChannel *
history_file_open (GError **error)
{
        GIOChannel *channel = NULL;

#ifdef HISTORY_FILE
        channel = g_io_channel_new_file (HISTORY_FILE, "r", error);

        if (channel != NULL)
        {
                GIOStatus status;

                status = g_io_channel_set_encoding (channel, NULL, NULL);
                g_assert (status == G_IO_STATUS_NORMAL);
        }
#else
        g_message (
                _("This program is not configured "
                  "to show history information."));
#endif

        return channel;
}

static void
history_process_info (HistoryEntry *entry,
                      const gchar *line)
{
        gchar **games;
        guint length, ii;

        games = g_strsplit (line, ",", -1);
        length = g_strv_length (games);

        for (ii = 0; ii < length; ii++)
                if (*g_strstrip (games[ii]) != '\0')
                        g_hash_table_insert (
                                history_file_table,
                                g_strdup (games[ii]), entry);

        g_strfreev (games);
}

static void
history_process_link (HistoryEntry *entry,
                      const gchar *line)
{
        if ((line = strstr (line, "&id=")) != NULL)
                entry->id = (guint) strtol (line + 4, NULL, 10);
}

/**
 * gva_history_init:
 * @error: return location for a #GError, or %NULL
 *
 * Scans the arcade history file and creates an index of games.  If an
 * error occurs, it returns %FALSE and sets @error.
 *
 * This function should be called once when the application starts.
 *
 * Returns: %TRUE on success, %FALSE if an error occurred
 **/
gboolean
gva_history_init (GError **error)
{
        HistoryEntry *entry = NULL;
        GIOChannel *channel;
        GIOStatus status;
        GString *buffer;
        goffset offset = 0;

        g_return_val_if_fail (history_file_array == NULL, FALSE);
        g_return_val_if_fail (history_file_table == NULL, FALSE);

        history_file_array = g_ptr_array_new ();

        history_file_table = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) NULL);

        channel = history_file_open (error);
        if (channel == NULL)
                return FALSE;

        buffer = g_string_sized_new (1024);

        while (TRUE)
        {
                status = G_IO_STATUS_AGAIN;
                while (status == G_IO_STATUS_AGAIN)
                        status = g_io_channel_read_line_string (
                                channel, buffer, NULL, error);

                if (status != G_IO_STATUS_NORMAL)
                        break;

                offset += buffer->len;

                if (g_str_has_prefix (buffer->str, "$info="))
                {
                        entry = g_slice_new0 (HistoryEntry);
                        g_ptr_array_add (history_file_array, entry);
                        history_process_info (entry, buffer->str + 6);
                }
                else if (g_str_has_prefix (buffer->str, "$<a"))
                {
                        history_process_link (entry, buffer->str + 1);
                }
                else if (g_str_has_prefix (buffer->str, "$bio"))
                {
                        entry->offset = offset;
                }
        }

        g_string_free (buffer, TRUE);
        g_io_channel_unref (channel);

        return (status == G_IO_STATUS_EOF);
}

/**
 * gva_history_lookup:
 * @game: the name of a game
 * @error: return location for a #GError, or %NULL
 *
 * Returns arcade history information for @game.  If an error occurs,
 * it returns %NULL and sets @error.
 *
 * Returns: history for @game, or %NULL if an error occurred
 **/
gchar *
gva_history_lookup (const gchar *game,
                    GError **error)
{
        HistoryEntry *entry;
        GIOChannel *channel;
        GIOStatus status;
        GString *buffer;
        GString *history;
        gboolean free_history;

        g_return_val_if_fail (game != NULL, NULL);
        g_return_val_if_fail (history_file_array != NULL, NULL);
        g_return_val_if_fail (history_file_table != NULL, NULL);

        channel = history_file_open (error);
        if (channel == NULL)
                return NULL;

        buffer = g_string_sized_new (1024);
        history = g_string_sized_new (8096);
        free_history = TRUE;  /* assume failure */

        entry = g_hash_table_lookup (history_file_table, game);
        if (entry == NULL || entry->offset == 0)
                goto exit;

        status = G_IO_STATUS_AGAIN;
        while (status == G_IO_STATUS_AGAIN)
                status = g_io_channel_seek_position (
                        channel, entry->offset, G_SEEK_SET, error);
        if (status == G_IO_STATUS_ERROR)
                goto exit;

        while (TRUE)
        {
                gchar *utf8;
                gsize bytes_written;

                status = G_IO_STATUS_AGAIN;
                while (status == G_IO_STATUS_AGAIN)
                        status = g_io_channel_read_line_string (
                                channel, buffer, NULL, error);

                if (status == G_IO_STATUS_ERROR)
                        goto exit;

                if (status == G_IO_STATUS_EOF)
                {
                        g_warning ("Unexpected end of history file");
                        break;
                }

                if (g_str_has_prefix (buffer->str, "$info="))
                {
                        g_warning ("Unexpected $info line in history file");
                        g_warning ("\"%s\"", buffer->str);
                        goto exit;
                }

                if (g_str_has_prefix (buffer->str, "$end"))
                        break;

                /* Ignore anything else that starts with '$'. */
                if (g_str_has_prefix (buffer->str, "$"))
                        continue;

                /* Be forgiving of the input.  If a conversion
                 * error occurs, skip the line and move on. */
                utf8 = g_locale_to_utf8 (
                        buffer->str, buffer->len, NULL, &bytes_written, NULL);
                if (utf8 != NULL)
                        g_string_append_len (history, utf8, bytes_written);
                g_free (utf8);
        }

        free_history = FALSE;

exit:
        g_io_channel_unref (channel);
        g_string_free (buffer, TRUE);

        return g_string_free (history, free_history);
}

/**
 * gva_history_lookup_id:
 * @game: the name of a game
 *
 * Returns the numeric ID for @game at http://www.arcade-history.com/.
 * This is used to help locate game-specific resources on the website.
 *
 * Returns: ID for @game, or zero if unknown
 **/
guint
gva_history_lookup_id (const gchar *game)
{
        HistoryEntry *entry;

        g_return_val_if_fail (game != NULL, 0);

        entry = g_hash_table_lookup (history_file_table, game);

        return (entry != NULL) ? entry->id : 0;
}
