#include "gva-game-db.h"

#include <string.h>

#include "gva-error.h"
#include "gva-game-store.h"
#include "gva-parser.h"
#include "gva-xmame.h"

static GArray *history_file_offset_array = NULL;
static GHashTable *history_file_offset_table = NULL;

static GIOChannel *
game_db_history_file_open (GError **error)
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
        g_set_error (
                error, GVA_ERROR, GVA_ERROR_CONFIG,
                _("This program is not configured to show "
                  "arcade history information."));
#endif

        return channel;
}

static void
game_db_history_file_mark (const gchar *line, gint64 offset)
{
        gchar **names;
        guint length, ii;

        g_array_append_val (history_file_offset_array, offset);

        /* Skip "$info=" prefix. */
        names = g_strsplit (line + 6, ",", -1);
        length = g_strv_length (names);

        for (ii = 0; ii < length; ii++)
        {
                if (*g_strstrip (names[ii]) == '\0')
                        continue;

                g_hash_table_insert (
                        history_file_offset_table, g_strdup (names[ii]),
                        GUINT_TO_POINTER (history_file_offset_array->len - 1));
        }

        g_strfreev (names);
}

static gboolean
game_db_history_file_scan (GError **error)
{
        GIOChannel *channel;
        GIOStatus status;
        GString *buffer;
        gint64 offset = 0;

        g_return_val_if_fail (history_file_offset_array == NULL, FALSE);
        g_return_val_if_fail (history_file_offset_table == NULL, FALSE);

        channel = game_db_history_file_open (error);
        if (channel == NULL)
                return FALSE;

        history_file_offset_array =
                g_array_new (FALSE, FALSE, sizeof (gint64));

        history_file_offset_table = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) NULL);

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
                        game_db_history_file_mark (buffer->str, offset);
        }

        g_string_free (buffer, TRUE);
        g_io_channel_unref (channel);

        return (status == G_IO_STATUS_EOF);
}

static void
game_db_insert (const gchar *romname)
{
        GtkTreeModel *model;
        GtkTreeIter iter;
        gboolean favorite;

        model = gva_game_db_get_model ();

        favorite = gva_favorites_contains (romname);

        gtk_list_store_append (GTK_LIST_STORE (model), &iter);

        gtk_list_store_set (
                GTK_LIST_STORE (model), &iter,
                GVA_GAME_STORE_COLUMN_NAME, romname,
                GVA_GAME_STORE_COLUMN_FAVORITE, favorite,
                -1);

        gva_game_store_index_insert (GVA_GAME_STORE (model), romname, &iter);
}

static void
game_db_add_sample (const gchar *romname, const gchar *status)
{
        GtkTreePath *path;

        path = gva_game_db_lookup (romname);

        if (path != NULL)
        {
                GtkTreeModel *model;
                GtkTreeIter iter;
                gboolean have_samples;
                gboolean valid;

                model = gva_game_db_get_model ();
                valid = gtk_tree_model_get_iter (model, &iter, path);
                gtk_tree_path_free (path);
                g_assert (valid);

                have_samples = (strcmp (status, "correct") == 0);

                gtk_list_store_set (
                        GTK_LIST_STORE (model), &iter,
                        GVA_GAME_STORE_COLUMN_USES_SAMPLES, TRUE,
                        GVA_GAME_STORE_COLUMN_HAVE_SAMPLES, have_samples,
                        -1);
        }
}

static void
game_db_add_title (const gchar *romname, const gchar *title)
{
        GtkTreePath *path;

        path = gva_game_db_lookup (romname);

        if (path != NULL)
        {
                GtkTreeModel *model;
                GtkTreeIter iter;
                gboolean valid;

                model = gva_game_db_get_model ();
                valid = gtk_tree_model_get_iter (model, &iter, path);
                gtk_tree_path_free (path);
                g_assert (valid);

                gtk_list_store_set (
                        GTK_LIST_STORE (model), &iter,
                        GVA_GAME_STORE_COLUMN_DESCRIPTION, title, -1);
        }
}

gboolean
gva_game_db_init (GError **error)
{
        GvaProcess *process;
        GPtrArray *array;

        if (!game_db_history_file_scan (error))
                return FALSE;

        array = gva_xmame_get_romset_files (error);
        if (array == NULL)
                return FALSE;

        g_ptr_array_foreach (array, (GFunc) game_db_insert, NULL);
        g_ptr_array_foreach (array, (GFunc) g_free, NULL);
        g_ptr_array_free (array, TRUE);

        return TRUE;
}

GtkTreePath *
gva_game_db_lookup (const gchar *romname)
{
        GtkTreeModel *model;

        g_return_val_if_fail (romname != NULL, NULL);

        model = gva_game_db_get_model ();

        return gva_game_store_index_lookup (GVA_GAME_STORE (model), romname);
}

GtkTreeModel *
gva_game_db_get_model (void)
{
        static GtkTreeModel *model = NULL;

        if (G_UNLIKELY (model == NULL))
                model = gva_game_store_new ();

        return model;
}

GvaProcess *
gva_game_db_update_samples (GError **error)
{
        return gva_xmame_verify_sample_sets (
                (GvaXmameCallback) game_db_add_sample, NULL, error);
}

GvaProcess *
gva_game_db_update_titles (GError **error)
{
        return gva_xmame_list_full (
                (GvaXmameCallback) game_db_add_title, NULL, error);
}

gchar *
gva_game_db_get_history (const gchar *romname, GError **error)
{
        GIOChannel *channel;
        GIOStatus status;
        GString *buffer;
        GString *history;
        gboolean free_history;
        gpointer key, value;
        gint64 offset;
        guint index;

        g_return_val_if_fail (romname != NULL, NULL);
        g_return_val_if_fail (history_file_offset_array != NULL, NULL);
        g_return_val_if_fail (history_file_offset_table != NULL, NULL);

        channel = game_db_history_file_open (error);
        if (channel == NULL)
                return NULL;

        buffer = g_string_sized_new (1024);
        history = g_string_sized_new (8096);
        free_history = TRUE;  /* assume failure */

        if (!g_hash_table_lookup_extended (
                history_file_offset_table, romname, &key, &value))
                goto exit;

        index = GPOINTER_TO_UINT (value);
        g_assert (index < history_file_offset_array->len);
        offset = g_array_index (history_file_offset_array, gint64, index);

        status = G_IO_STATUS_AGAIN;
        while (status == G_IO_STATUS_AGAIN)
                status = g_io_channel_seek_position (
                        channel, offset, G_SEEK_SET, error);
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

                if (g_str_has_prefix (buffer->str, "$bio"))
                        continue;

                if (g_str_has_prefix (buffer->str, "$end"))
                        break;

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
