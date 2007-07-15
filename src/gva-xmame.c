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

#include "gva-xmame.h"

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#ifdef HAVE_WORDEXP_H
#include <wordexp.h>
#endif

#include "gva-error.h"
#include "gva-mame-process.h"
#include "gva-preferences.h"

typedef struct
{
        GvaXmameCallback callback;
        gpointer user_data;
        gint lineno;

} XmameAsyncData;

static XmameAsyncData *
xmame_async_data_new (GvaXmameCallback callback,
                      gpointer user_data)
{
        XmameAsyncData *data;

        data = g_slice_new (XmameAsyncData);
        data->callback = callback;
        data->user_data = user_data;
        data->lineno = 0;

        return data;
}

static void
xmame_async_data_free (XmameAsyncData *data)
{
        g_slice_free (XmameAsyncData, data);
}

gint
gva_xmame_command (const gchar *arguments,
                   gchar ***stdout_lines,
                   gchar ***stderr_lines,
                   GError **error)
{
        gchar **local_stdout_lines = NULL;
        gchar **local_stderr_lines = NULL;
        GvaProcess *process;
        gint status;

        process = gva_mame_process_spawn (arguments, error);
        if (process == NULL)
                return -1;

        /* Wait for the process to exit. */
        while (!gva_process_has_exited (process, &status))
                g_main_context_iteration (NULL, TRUE);

        local_stdout_lines = gva_process_stdout_read_lines (process);
        local_stderr_lines = gva_process_stderr_read_lines (process);

        if (process->error != NULL)
        {
                g_propagate_error (error, process->error);
                status = -1;
                goto fail;
        }

        if (stdout_lines != NULL)
                *stdout_lines = local_stdout_lines;
        else
                g_strfreev (local_stdout_lines);

        if (stderr_lines != NULL)
                *stderr_lines = local_stderr_lines;
        else
                g_strfreev (local_stderr_lines);

        g_assert (WIFEXITED (status));
        status = WEXITSTATUS (status);

fail:
        g_object_unref (process);

        return status;
}

gchar *
gva_xmame_get_version (GError **error)
{
        gchar *version = NULL;
        gchar **lines;
        guint num_lines, ii;

        /* Execute the command "${xmame} -version". */
        if (gva_xmame_command ("-version", &lines, NULL, error) != 0)
                return NULL;

        /* Output is as follows:
         *
         * xmame (backend) version n.nnn (Mmm dd yyyy)
         */

        num_lines = g_strv_length (lines);

        for (ii = 0; ii < num_lines && version == NULL; ii++)
        {
                if (strstr (lines[ii], "xmame") == NULL)
                        continue;
                if (strstr (lines[ii], "version") == NULL)
                        continue;
                version = g_strdup (g_strstrip (lines[ii]));
        }

        g_strfreev (lines);

        return version;
}

guint
gva_xmame_get_total_supported (GError **error)
{
        gchar **lines;
        guint num_lines, ii;
        guint total_supported = 0;

        /* Execute the command "${xmame} -list". */
        if (gva_xmame_command ("-list", &lines, NULL, error) != 0)
                return 0;

        /* Output is as follows:
         *
         * xmame currently supports:
         * romname   romname   romname   romname   romname   ...
         * romname   romname   romname   romname   romname   ...
         * romname   romname   romname   romname   romname   ...
         *
         * Total Supported: nnnn
         */

        num_lines = g_strv_length (lines);

        for (ii = 0; ii < num_lines; ii++)
        {
                if (g_str_has_prefix (lines[ii], "Total Supported:"))
                        total_supported = strtoul (lines[ii] + 16, NULL, 10);
        }

        g_strfreev (lines);

        return total_supported;
}

gchar *
gva_xmame_get_config_value (const gchar *config_key, GError **error)
{
        gchar *config_value = NULL;
        gchar **lines;
        guint num_lines, ii;
#ifdef HAVE_WORDEXP_H
        wordexp_t words;
#endif

        g_return_val_if_fail (config_key != NULL, NULL);

        /* Execute the command "${xmame} -showconfig". */
        if (gva_xmame_command ("-showconfig", &lines, NULL, error) != 0)
                return NULL;

        /* Output is as follows:
         *
         * # Lines that start with '#' are comments.
         * config_key           config_value
         * config_key           config_value
         * ...
         */

        num_lines = g_strv_length (lines);

        for (ii = 0; ii < num_lines; ii++)
        {
                if (g_str_has_prefix (lines[ii], config_key))
                {
                        gchar *cp = lines[ii] + strlen (config_key);
                        config_value = g_strdup (g_strstrip (cp));
                        break;
                }
        }

        g_strfreev (lines);

#ifdef HAVE_WORDEXP_H
        if (config_value == NULL)
                goto exit;

        /* xmame reports shell variables like $HOME in some of its
         * configuration values, so we need to expand them ourselves. */
        if (wordexp (config_value, &words, 0) == 0)
        {
                GString *buffer;
                gsize ii;

                buffer = g_string_sized_new (strlen (config_value));
                for (ii = 0; ii < words.we_wordc; ii++)
                {
                        if (ii > 0)
                                g_string_append_c (buffer, ' ');
                        g_string_append (buffer, words.we_wordv[ii]);
                }

                g_free (config_value);
                config_value = g_string_free (buffer, FALSE);

                wordfree (&words);
        }
#endif

exit:
        if (config_value == NULL)
                g_set_error (
                        error, GVA_ERROR, GVA_ERROR_XMAME,
                        _("%s: No such configuration key"), config_key);

        return config_value;
}

gboolean
gva_xmame_has_config_value (const gchar *config_key)
{
        gchar *config_value;
        GError *error = NULL;

        config_value = gva_xmame_get_config_value (config_key, &error);
        gva_error_handle (&error);

        if (config_value != NULL)
        {
                g_free (config_value);
                return TRUE;
        }

        return FALSE;
}

GHashTable *
gva_xmame_get_input_files (GError **error)
{
        GHashTable *hash_table = NULL;
        const gchar *basename;
        gchar *inppath;
        GDir *dir;

        inppath = gva_xmame_get_config_value ("input_directory", error);
        dir = (inppath != NULL) ? g_dir_open (inppath, 0, error) : NULL;

        if (dir == NULL)
                goto exit;

        hash_table = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) g_free);

        while ((basename = g_dir_read_name (dir)) != NULL)
        {
                gchar *filename;
                gchar buffer[16];
                GIOChannel *channel;
                GError *local_error = NULL;

                filename = g_build_filename (inppath, basename, NULL);

                channel = g_io_channel_new_file (filename, "r", &local_error);

                if (channel != NULL)
                {
                        gchar name[16];
                        GIOStatus status;

                        status = g_io_channel_read_chars (
                                channel, name, sizeof (name),
                                NULL, &local_error);
                        if (status == G_IO_STATUS_NORMAL)
                                g_hash_table_insert (
                                        hash_table, filename,
                                        g_strdup (name));
                        g_io_channel_unref (channel);
                }

                if (local_error != NULL)
                {
                        g_free (filename);
                        g_hash_table_destroy (hash_table);
                        g_propagate_error (error, local_error);
                        hash_table = NULL;
                        break;
                }
        }

        g_dir_close (dir);

exit:
        g_free (inppath);

        return hash_table;
}

GvaProcess *
gva_xmame_list_xml (GError **error)
{
        /* Execute the command "${xmame} -listxml". */
        return gva_mame_process_spawn ("-listxml", error);
}

static void
xmame_verify_read (GvaProcess *process,
                   XmameAsyncData *data)
{
        gchar *line;

        /* Output is as follows:
         *
         * name      result
         * --------  ------
         * romname   correct|incorrect|not found
         * romname   correct|incorrect|not found
         * ...
         *
         *
         * Total Supported: nnnn
         * Displayed: nnnn ...
         * Found: nnnn ...
         * Not found: nnnn
         */

        line = gva_process_stdout_read_line (process);

        /* Stop parsing when we see a blank line. */
        if (data->lineno < 0 || *g_strchomp (line) == '\0')
                data->lineno = -1;
        else
                data->lineno++;

        if (data->lineno > 2)
        {
                gchar **tokens;

                tokens = g_strsplit_set (line, " ", 2);
                if (g_strv_length (tokens) == 2)
                {
                        gchar *name, *status;

                        name = g_strstrip (tokens[0]);
                        status = g_strstrip (tokens[1]);

                        data->callback (name, status, data->user_data);
                }
                g_strfreev (tokens);
        }

        g_free (line);
}

static void
xmame_verify_exit (GvaProcess *process,
                   gint status,
                   XmameAsyncData *data)
{
        xmame_async_data_free (data);
}

GvaProcess *
gva_xmame_verify_romsets (GvaXmameCallback callback,
                          gpointer user_data,
                          GError **error)
{
        GvaProcess *process;
        XmameAsyncData *data;

        g_return_val_if_fail (callback != NULL, NULL);

        /* Execute the command "${xmame} -verifyromsets". */
        process = gva_mame_process_spawn ("-verifyromsets", error);
        if (process == NULL)
                return NULL;

        data = xmame_async_data_new (callback, user_data);

        g_signal_connect (
                process, "stdout-ready",
                G_CALLBACK (xmame_verify_read), data);
        g_signal_connect (
                process, "exited",
                G_CALLBACK (xmame_verify_exit), data);

        return process;
}

GvaProcess *
gva_xmame_verify_samplesets (GvaXmameCallback callback,
                             gpointer user_data,
                             GError **error)
{
        GvaProcess *process;
        XmameAsyncData *data;

        g_return_val_if_fail (callback != NULL, NULL);

        /* Execute the command "${xmame} -verifysamplesets". */
        process = gva_mame_process_spawn ("-verifysamplesets", error);
        if (process == NULL)
                return NULL;

        data = xmame_async_data_new (callback, user_data);

        g_signal_connect (
                process, "stdout-ready",
                G_CALLBACK (xmame_verify_read), data);
        g_signal_connect (
                process, "exited",
                G_CALLBACK (xmame_verify_exit), data);

        return process;
}

GvaProcess *
gva_xmame_run_game (const gchar *name, GError **error)
{
        GvaProcess *process;
        GString *arguments;

        g_return_val_if_fail (name != NULL, FALSE);

        arguments = g_string_sized_new (64);

        if (gva_xmame_supports_auto_save ())
        {
                if (gva_preferences_get_auto_save ())
                        g_string_append (arguments, "-autosave ");
                else
                        g_string_append (arguments, "-noautosave ");
        }

        if (gva_xmame_supports_full_screen ())
        {
                if (gva_preferences_get_full_screen ())
                        g_string_append (arguments, "-fullscreen ");
                else
                        g_string_append (arguments, "-nofullscreen ");
        }

        g_string_append_printf (arguments, "%s", name);

        /* Execute the command "${xmame} ${name}". */
        process = gva_mame_process_spawn (arguments->str, error);

        g_string_free (arguments, TRUE);

        return process;
}

GvaProcess *
gva_xmame_record_game (const gchar *name, const gchar *inpname,
                       GError **error)
{
        GvaProcess *process;
        GString *arguments;

        g_return_val_if_fail (name != NULL, FALSE);

        if (inpname == NULL)
                inpname = name;

        arguments = g_string_sized_new (64);

        if (gva_xmame_supports_auto_save ())
                g_string_append (arguments, "-noautosave ");

        if (gva_xmame_supports_full_screen ())
        {
                if (gva_preferences_get_full_screen ())
                        g_string_append (arguments, "-fullscreen ");
                else
                        g_string_append (arguments, "-nofullscreen ");
        }

        g_string_append_printf (arguments, "-record %s %s", inpname, name);

        /* Execute the command "${xmame} -record ${inpname} ${name}". */
        process = gva_mame_process_spawn (arguments->str, error);

        g_string_free (arguments, TRUE);

        return process;
}

GvaProcess *
gva_xmame_playback_game (const gchar *name, const gchar *inpname,
                         GError **error)
{
        GvaProcess *process;
        GString *arguments;

        g_return_val_if_fail (inpname != NULL, FALSE);

        arguments = g_string_sized_new (64);

        if (gva_xmame_supports_auto_save ())
                g_string_append (arguments, "-noautosave ");

        if (gva_xmame_supports_full_screen ())
        {
                if (gva_preferences_get_full_screen ())
                        g_string_append (arguments, "-fullscreen ");
                else
                        g_string_append (arguments, "-nofullscreen ");
        }

        g_string_append_printf (arguments, "-playback %s", inpname);

        /* Execute the command "${xmame} -playback ${inpname}". */
        process = gva_mame_process_spawn (arguments->str, error);

        g_string_free (arguments, TRUE);

        if (process == NULL)
                return NULL;

        /* xmame asks the user to press return before it will start playing
         * back the game, so we have to supply the expected keystroke. */
        if (!gva_process_write_stdin (process, "\n", 1, error))
                return NULL;  /* FIXME Abort the process */

        return process;
}

gboolean
gva_xmame_clear_state (const gchar *name, GError **error)
{
        gchar *basename;
        gchar *directory;
        gchar *filename;
        gboolean success = TRUE;

        directory = gva_xmame_get_config_value ("state_directory", error);
        if (directory == NULL)
                return FALSE;

        basename = g_strdup_printf ("%s.sta", name);
        filename = g_build_filename (directory, basename, NULL);

        if (g_file_test (filename, G_FILE_TEST_IS_REGULAR))
        {
                if (g_unlink (filename) < 0)
                {
                        g_set_error (
                                error, GVA_ERROR, GVA_ERROR_SYSTEM,
                                "Unable to delete %s: %s",
                                filename, g_strerror (errno));
                        success = FALSE;
                }
        }

        g_free (filename);
        g_free (basename);
        g_free (directory);

        return success;
}

gboolean
gva_xmame_supports_auto_save (void)
{
        return gva_xmame_has_config_value ("autosave");
}

gboolean
gva_xmame_supports_full_screen (void)
{
        return gva_xmame_has_config_value ("fullscreen");
}
