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

#include "gva-mame-common.h"

#include <errno.h>
#include <string.h>
#include <wait.h>

#ifdef HAVE_WORDEXP_H
#include <wordexp.h>
#endif

#include "gva-error.h"
#include "gva-mame-process.h"
#include "gva-preferences.h"

/*****************************************************************************
 * Private utilities for MAME backends
 *****************************************************************************/

GvaMameAsyncData *
gva_mame_async_data_new (GvaMameCallback callback,
                         gpointer user_data)
{
        GvaMameAsyncData *data;

        data = g_slice_new (GvaMameAsyncData);
        data->callback = callback;
        data->user_data = user_data;

        return data;
}

void
gva_mame_async_data_free (GvaMameAsyncData *data)
{
        g_slice_free (GvaMameAsyncData, data);
}

gint
gva_mame_command (const gchar *arguments,
                  gchar ***stdout_lines,
                  gchar ***stderr_lines,
                  GError **error)
{
        gchar **local_stdout_lines = NULL;
        gchar **local_stderr_lines = NULL;
        GvaProcess *process;
        gint status;

        process = gva_mame_process_spawn (
                arguments, G_PRIORITY_DEFAULT_IDLE, error);
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

/*****************************************************************************
 * Partial implementation of public MAME interface
 *****************************************************************************/

gchar *
gva_mame_get_config_value (const gchar *config_key,
                           GError **error)
{
        gchar *config_value = NULL;
        gchar **lines;
        guint num_lines, ii;
#ifdef HAVE_WORDEXP_H
        wordexp_t words;
#endif

        g_return_val_if_fail (config_key != NULL, NULL);

        /* Execute the command "${mame} -showconfig". */
        if (gva_mame_command ("-showconfig", &lines, NULL, error) != 0)
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

        /* MAME sometimes reports shell variables like $HOME in some of
         * its configuration values, so we need to expand them ourselves. */
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
                        error, GVA_ERROR, GVA_ERROR_MAME,
                        _("%s: No such configuration key"), config_key);

        return config_value;
}

gboolean
gva_mame_has_config_value (const gchar *config_key)
{
        gchar *config_value;
        gboolean result;
        GError *error = NULL;

        config_value = gva_mame_get_config_value (config_key, &error);

        if (config_value != NULL)
        {
                result = TRUE;
                g_free (config_value);
        }
        else
        {
                result = FALSE;
                g_assert (error != NULL);

                /* Suppress warnings about unknown configuration
                 * keys, since that's what we're testing for. */
                if (error->code == GVA_ERROR_MAME)
                        g_clear_error (&error);
                else
                        gva_error_handle (&error);
        }

        return result;
}

GHashTable *
gva_mame_get_input_files (GError **error)
{
        GHashTable *hash_table = NULL;
        const gchar *basename;
        gchar *inppath;
        GDir *dir;

        inppath = gva_mame_get_config_value ("input_directory", error);
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
gva_mame_list_xml (GError **error)
{
        /* Execute the command "${mame} -listxml". */
        return gva_mame_process_spawn (
                "-listxml", G_PRIORITY_DEFAULT_IDLE, error);
}

static void
mame_verify_read (GvaProcess *process,
                  GvaMameAsyncData *data)
{
        gchar *line;
        gchar *name;
        gchar *status = NULL;
        gchar *token;

        /* Output for -verifyroms is as follows:
         *
         * romset puckman is good
         * romset puckmana [puckman] is good
         * romset puckmanf [puckman] is good
         * ...
         *
         * - Ignore lines that do not start with "romset".
         *
         * - Status may be "good", "bad", or "best available".
         *
         * - Older MAMEs used "correct" and "incorrect" instead of
         *   "good" and "bad".  Convert to the newer form if seen.
         *
         * - Similar output for -verifysamples.
         */

        line = g_strchomp (gva_process_stdout_read_line (process));

        if ((token = strtok (line, " ")) == NULL)
                goto exit;

        if (strcmp (token, "romset") != 0 && strcmp (token, "sampleset") != 0)
                goto exit;

        name = strtok (NULL, " ");
        while ((token = strtok (NULL, " ")) != NULL)
                status = token;

        if (name == NULL || status == NULL)
                goto exit;

        /* Normalize the status. */
        if (strcmp (status, "correct") == 0)
                status = "good";
        else if (strcmp (status, "incorrect") == 0)
                status = "bad";

        data->callback (name, status, data->user_data);
        gva_process_inc_progress (process);

exit:
        g_free (line);
}

static void
mame_verify_exit (GvaProcess *process,
                  gint status,
                  GvaMameAsyncData *data)
{
        gva_mame_async_data_free (data);
}

GvaProcess *
gva_mame_verify_roms (GvaMameCallback callback,
                      gpointer user_data,
                      GError **error)
{
        GvaProcess *process;
        GvaMameAsyncData *data;

        g_return_val_if_fail (callback != NULL, NULL);

        /* Execute the command "${mame} -verifyroms". */
        process = gva_mame_process_spawn (
                "-verifyroms", G_PRIORITY_LOW, error);
        if (process == NULL)
                return NULL;

        data = gva_mame_async_data_new (callback, user_data);

        g_signal_connect (
                process, "stdout-ready",
                G_CALLBACK (mame_verify_read), data);
        g_signal_connect (
                process, "exited",
                G_CALLBACK (mame_verify_exit), data);

        return process;
}

GvaProcess *
gva_mame_verify_samples (GvaMameCallback callback,
                         gpointer user_data,
                         GError **error)
{
        GvaProcess *process;
        GvaMameAsyncData *data;

        g_return_val_if_fail (callback != NULL, NULL);

        /* Execute the command "${mame} -verifysamples". */
        process = gva_mame_process_spawn (
                "-verifysamples", G_PRIORITY_LOW, error);
        if (process == NULL)
                return NULL;

        data = gva_mame_async_data_new (callback, user_data);

        g_signal_connect (
                process, "stdout-ready",
                G_CALLBACK (mame_verify_read), data);
        g_signal_connect (
                process, "exited",
                G_CALLBACK (mame_verify_exit), data);

        return process;
}
GvaProcess *
gva_mame_run_game (const gchar *name,
                   GError **error)
{
        GvaProcess *process;
        GString *arguments;

        g_return_val_if_fail (name != NULL, FALSE);

        arguments = g_string_sized_new (64);

        if (gva_mame_supports_auto_save ())
        {
                if (gva_preferences_get_auto_save ())
                        g_string_append (arguments, "-autosave ");
                else
                        g_string_append (arguments, "-noautosave ");
        }

        /* Support for the -fullscreen and -window options appears to be
         * mutually exclusive.  The latest xmame supports -fullscreen but
         * not -window, while the latest sdlmame supports -window but not
         * -fullscreen.  We'll use whichever is available. */

        if (gva_mame_supports_full_screen ())
        {
                if (gva_preferences_get_full_screen ())
                        g_string_append (arguments, "-fullscreen ");
                else
                        g_string_append (arguments, "-nofullscreen ");
        }
        else if (gva_mame_supports_window ())
        {
                if (gva_preferences_get_full_screen ())
                        g_string_append (arguments, "-nowindow ");
                else
                        g_string_append (arguments, "-window ");
        }

        g_string_append_printf (arguments, "%s", name);

        /* Execute the command "${mame} ${name}". */
        process = gva_mame_process_spawn (
                arguments->str, G_PRIORITY_LOW, error);

        g_string_free (arguments, TRUE);

        return process;
}

GvaProcess *
gva_mame_record_game (const gchar *name,
                      const gchar *inpname,
                      GError **error)
{
        GvaProcess *process;
        GString *arguments;

        g_return_val_if_fail (name != NULL, FALSE);

        if (inpname == NULL)
                inpname = name;

        arguments = g_string_sized_new (64);

        if (gva_mame_supports_auto_save ())
                g_string_append (arguments, "-noautosave ");

        /* Support for the -fullscreen and -window options appears to be
         * mutually exclusive.  The latest xmame supports -fullscreen but
         * not -window, while the latest sdlmame supports -window but not
         * -fullscreen.  We'll use whichever is available. */

        if (gva_mame_supports_full_screen ())
        {
                if (gva_preferences_get_full_screen ())
                        g_string_append (arguments, "-fullscreen ");
                else
                        g_string_append (arguments, "-nofullscreen ");
        }
        else if (gva_mame_supports_window ())
        {
                if (gva_preferences_get_full_screen ())
                        g_string_append (arguments, "-nowindow ");
                else
                        g_string_append (arguments, "-window ");
        }

        g_string_append_printf (arguments, "-record %s %s", inpname, name);

        /* Execute the command "${mame} -record ${inpname} ${name}". */
        process = gva_mame_process_spawn (
                arguments->str, G_PRIORITY_LOW, error);

        g_string_free (arguments, TRUE);

        return process;
}

GvaProcess *
gva_mame_playback_game (const gchar *name,
                        const gchar *inpname,
                        GError **error)
{
        GvaProcess *process;
        GString *arguments;

        g_return_val_if_fail (inpname != NULL, FALSE);

        arguments = g_string_sized_new (64);

        if (gva_mame_supports_auto_save ())
                g_string_append (arguments, "-noautosave ");

        /* Support for the -fullscreen and -window options appears to be
         * mutually exclusive.  The latest xmame supports -fullscreen but
         * not -window, while the latest sdlmame supports -window but not
         * -fullscreen.  We'll use whichever is available. */

        if (gva_mame_supports_full_screen ())
        {
                if (gva_preferences_get_full_screen ())
                        g_string_append (arguments, "-fullscreen ");
                else
                        g_string_append (arguments, "-nofullscreen ");
        }
        else if (gva_mame_supports_window ())
        {
                if (gva_preferences_get_full_screen ())
                        g_string_append (arguments, "-nowindow ");
                else
                        g_string_append (arguments, "-window ");
        }

        g_string_append_printf (arguments, "%s -playback %s", name, inpname);

        /* Execute the command "${mame} ${name} -playback ${inpname}". */
        process = gva_mame_process_spawn (
                arguments->str, G_PRIORITY_LOW, error);

        g_string_free (arguments, TRUE);

        if (process == NULL)
                return NULL;

        /* xmame asks the user to press return before it will start playing
         * back the game, so we have to supply the expected keystroke. */
        if (!gva_process_write_stdin (process, "\n", 1, error))
                return NULL;  /* FIXME Abort the process */

        return process;
}

gchar *
gva_mame_get_save_state_file (const gchar *name)
{
        gchar *directory;
        gchar *basename;
        gchar *filename = NULL;
        GError *error = NULL;

        directory = gva_mame_get_config_value ("state_directory", &error);
        gva_error_handle (&error);
        if (directory == NULL)
                goto exit;

        /* As of 0.115, MAME stores save state files as "name/auto.sta". */
        filename = g_build_filename (directory, name, "auto.sta", NULL);

        if (g_file_test (filename, G_FILE_TEST_IS_REGULAR))
                goto exit;
        else
                g_free (filename);

        /* Prior to 0.115, MAME stored save state files as "name.sta". */
        basename = g_strdup_printf ("%s.sta", name);
        filename = g_build_filename (directory, basename, NULL);
        g_free (basename);

        if (g_file_test (filename, G_FILE_TEST_IS_REGULAR))
                goto exit;
        else
                g_free (filename);

        filename = NULL;  /* file not found */

exit:
        g_free (directory);

        return filename;
}

void
gva_mame_delete_save_state (const gchar *name)
{
        gchar *filename;

        filename = gva_mame_get_save_state_file (name);

        if (filename != NULL && g_unlink (filename) < 0)
                g_warning (
                        "Could not delete %s: %s",
                        filename, g_strerror (errno));

        g_free (filename);
}
