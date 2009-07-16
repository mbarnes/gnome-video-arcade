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

#include "gva-mame-common.h"

#include <errno.h>
#include <string.h>
#include <sys/wait.h>

#ifdef HAVE_WORDEXP_H
#include <wordexp.h>
#endif

#include "gva-error.h"
#include "gva-input-file.h"
#include "gva-mame-process.h"
#include "gva-mute-button.h"
#include "gva-preferences.h"
#include "gva-ui.h"

/*****************************************************************************
 * Private utilities for MAME backends
 *****************************************************************************/

/**
 * gva_mame_async_data_new:
 * @callback: callback function
 * @user_data: user data to pass to the callback function
 *
 * Creates a new #GvaMameAsyncData structure and populates it with @callback
 * and @user_data.  Call gva_mame_async_data_free() to free it.
 *
 * Returns: a new #GvaMameAsyncData
 **/
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

/**
 * gva_mame_async_data_free:
 * @data: a #GvaMameAsyncData
 *
 * Frees @data.
 **/
void
gva_mame_async_data_free (GvaMameAsyncData *data)
{
        g_slice_free (GvaMameAsyncData, data);
}

/**
 * gva_mame_command:
 * @arguments: command line arguments
 * @stdout_lines: return location for stdout lines, or %NULL
 * @stderr_lines: return location for stderr lines, or %NULL
 * @error: return locations for a #GError, or %NULL
 *
 * Spawns MAME with @arguments and blocks until the child process exits.
 * The line-based output from the stdout and stderr pipes are written to
 * @stdout_lines and @stderr_lines, respectively, as %NULL-terminated
 * string arrays.  The function returns the exit status of the child
 * process, or -1 if an error occurred while spawning the process.
 *
 * Returns: exit status of the child process or -1 if an error occurred
 **/
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
                process->error = NULL;
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

/**
 * gva_mame_get_config_value:
 * @config_key: a configuration key
 * @error: return location for a #GError, or %NULL
 *
 * Runs "MAME -showconfig" and extracts from the output the value of
 * @config_key.  If an error occurs, or if @config_key is not found in
 * MAME's configuration, the function returns %NULL and sets @error.
 *
 * Returns: the value of @config_key, or %NULL
 **/
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

        num_lines = (lines != NULL) ? g_strv_length (lines) : 0;

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

/**
 * gva_mame_has_config_value:
 * @config_key: a configuration key
 *
 * Returns %TRUE if the MAME configuration has a configuration value for
 * @config_key.  The function does not report errors that occur in the
 * course of spawning MAME, so false negatives are possible.
 *
 * Returns: %TRUE if a value for @config_key exists, %FALSE if no such
 *          value exists or if an error occurred
 **/
gboolean
gva_mame_has_config_value (const gchar *config_key)
{
        gchar *config_value;
        gboolean result;
        GError *error = NULL;

        config_value = gva_mame_get_config_value (config_key, &error);
        result = (config_value != NULL && *config_value != '\0');
        g_free (config_value);

        if (error != NULL)
        {
                /* Suppress warnings about unknown configuration
                 * keys, since that's what we're testing for. */
                if (error->code == GVA_ERROR_MAME)
                        g_clear_error (&error);
                else
                        gva_error_handle (&error);
        }

        return result;
}

/**
 * gva_mame_get_search_paths:
 * @config_key: a configuration key
 * @error: return location for a #GError, or %NULL
 *
 * Returns the value of @config_key as an ordered list of search paths.
 * This is only appropriate for configuration keys containing a list of
 * search paths, such as "rompath" or "samplepath".  If an error occurs,
 * or if @config_key is not found in MAME's configuration, the function
 * returns %NULL and sets @error.
 *
 * Returns: a newly-allocated %NULL-terminated array of paths, or %NULL.
 *          Use g_strfreev() to free it.
 **/
gchar **
gva_mame_get_search_paths (const gchar *config_key,
                           GError **error)
{
        gchar *config_value;

        config_value = gva_mame_get_config_value (config_key, error);
        if (config_value == NULL)
                return NULL;

        return g_strsplit (config_value, gva_mame_get_path_sep (), -1);
}

/**
 * gva_mame_get_input_files:
 * @error: return location for a @GError, or %NULL
 *
 * Returns a list of #GvaInputFile instances corresponding to files in the
 * input directory.  If an error occurs, it returns %NULL and sets @error.
 *
 * Returns: a list of #GvaInputFile instances, or %NULL
 **/
GList *
gva_mame_get_input_files (GError **error)
{
        GHashTable *hash_table;
        const gchar *basename;
        const gchar *directory;
        GList *list = NULL;
        GDir *dir;

        directory = gva_mame_get_input_directory (error);

        dir = g_dir_open (directory, 0, error);
        if (dir == NULL)
                return NULL;

        hash_table = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) g_free);

        while ((basename = g_dir_read_name (dir)) != NULL)
        {
                gchar *filename;
                GvaInputFile *input_file;

                filename = g_build_filename (directory, basename, NULL);
                input_file = gva_input_file_new (filename);
                list = g_list_prepend (list, input_file);
                g_free (filename);
        }

        g_dir_close (dir);

        return g_list_reverse (list);
}

/**
 * gva_mame_list_xml:
 * @error: return location for a #GError, or %NULL
 *
 * Spawns a "MAME -listxml" child process and returns a #GvaProcess so the
 * output can be read asynchronously.  If an error occurs while spawning,
 * it returns %NULL and sets @error.
 *
 * <note>
 *   <para>
 *     Beware, this command spews many megabytes of XML data!
 *   </para>
 * </note>
 *
 * Returns: a new #GvaProcess, or %NULL
 **/
GvaProcess *
gva_mame_list_xml (GError **error)
{
        /* Execute the command "${mame} -listxml". */
        return gva_mame_process_spawn (
                "-listxml", G_PRIORITY_DEFAULT_IDLE, error);
}

/**
 * gva_mame_verify_roms:
 * @name: the name of a ROM set
 * @error: return location for a #GError, or %NULL
 *
 * Verifies the contents of the ROM set @name and returns the status, which
 * may be "good", "bad", "best available", "not found", or "not supported".
 * If an error occurs, it returns %NULL and sets @error.
 *
 * Returns: verification status, or %NULL
 **/
gchar *
gva_mame_verify_roms (const gchar *name,
                      GError **error)
{
        gchar *arguments;
        gchar **stdout_lines;
        gchar **stderr_lines;
        gchar *status = NULL;
        gint exit_status;
        gint ii;

        g_return_val_if_fail (name != NULL, NULL);

        /* Execute the command "${mame} -verifyroms %{name}". */
        arguments = g_strdup_printf ("-verifyroms %s", name);
        exit_status = gva_mame_command (
                arguments, &stdout_lines, &stderr_lines, error);
        g_free (arguments);

        if (exit_status < 0)
                return NULL;

        /* First try to extract a status from standard output. */
        for (ii = 0; status == NULL && stdout_lines[ii] != NULL; ii++)
                gva_mame_verify_parse (stdout_lines[ii], NULL, &status);

        /* If that fails, try to extract a status from standard error. */
        for (ii = 0; status == NULL && stderr_lines[ii] != NULL; ii++)
                gva_mame_verify_parse (stderr_lines[ii], NULL, &status);

        g_strfreev (stdout_lines);
        g_strfreev (stderr_lines);

        return status;
}

/**
 * gva_mame_verify_samples:
 * @name: the name of a sample set
 * @error: return location for a #GError, or %NULL
 *
 * Verifies the contents of the sample set @name and returns the status,
 * which may be "good", "bad", "best available", "not found", or "not
 * supported".  If an error occurs, it returns %NULL and sets @error.
 *
 * Returns: verification status, or %NULL
 **/
gchar *
gva_mame_verify_samples (const gchar *name,
                         GError **error)
{
        gchar *arguments;
        gchar **stdout_lines;
        gchar **stderr_lines;
        gchar *status = NULL;
        gint exit_status;
        gint ii;

        g_return_val_if_fail (name != NULL, NULL);

        /* Execute the command "${mame} -verifysamples %{name}". */
        arguments = g_strdup_printf ("-verifysamples %s", name);
        exit_status = gva_mame_command (
                arguments, &stdout_lines, &stderr_lines, error);
        g_free (arguments);

        if (exit_status < 0)
                return NULL;

        /* First try to extract a status from standard output. */
        for (ii = 0; status == NULL && stdout_lines[ii] != NULL; ii++)
                gva_mame_verify_parse (stdout_lines[ii], NULL, &status);

        /* If that fails, try to extract a status from standard error. */
        for (ii = 0; status == NULL && stderr_lines[ii] != NULL; ii++)
                gva_mame_verify_parse (stderr_lines[ii], NULL, &status);

        g_strfreev (stdout_lines);
        g_strfreev (stderr_lines);

        return status;
}

/**
 * gva_mame_verify_all_roms:
 * @error: return location for a #GError, or %NULL
 *
 * Spawns a "MAME -verifyroms" child process and returns a #GvaProcess so
 * the output can be read asynchronously.  If an error occurs while spawning,
 * it returns %NULL and sets @error.
 *
 * Returns: a new #GvaProcess, or %NULL
 **/
GvaProcess *
gva_mame_verify_all_roms (GError **error)
{
        /* Execute the command "${mame} -verifyroms". */
        return gva_mame_process_spawn (
                "-verifyroms", G_PRIORITY_DEFAULT_IDLE, error);
}

/**
 * gva_mame_verify_all_samples:
 * @error: return location for a #GError, or %NULL
 *
 * Spawns a "MAME -verifysamples" child process and returns a #GvaProcess so
 * the output can be read asynchronously.  If an error occurs while spawning,
 * it returns %NULL and sets @error.
 *
 * Returns: a new #GvaProcess, or %NULL
 **/
GvaProcess *
gva_mame_verify_all_samples (GError **error)
{
        /* Execute the command "${mame} -verifysamples". */
        return gva_mame_process_spawn (
                "-verifysamples", G_PRIORITY_DEFAULT_IDLE, error);
}

/**
 * gva_mame_verify_parse:
 * @line: output line from a MAME process
 * @out_name: return location for the game name, or %NULL
 * @out_status: return location for the status, or %NULL
 *
 * Attempts to extract a status from a line of output generated by a
 * "MAME -verifyroms" or "MAME -verifysamples" child process.  If successful,
 * it sets @out_name to a newly-allocated string containing the game name and
 * @out_status to a newly-allocated string containing the verification status
 * and returns %TRUE.
 *
 * Known status values are "good", "bad", "best available", "not found" and
 * "not supported".
 *
 * Returns: %TRUE if the parse was successful, %FALSE otherwise
 **/
gboolean
gva_mame_verify_parse (const gchar *line,
                       gchar **out_name,
                       gchar **out_status)
{
        gchar *copy;
        const gchar *name = NULL;
        const gchar *sep = " \"[]:!";
        const gchar *status = NULL;
        const gchar *token;

        g_return_val_if_fail (line != NULL, FALSE);

        /* Output for -verifyroms is as follows:
         *
         * romset puckman is good
         * romset puckmana [puckman] is good
         * romset puckmanf [puckman] is good
         * ...
         * digdug  : 51xx.bin (1024 bytes) - NOT FOUND
         * digdug  : 53xx.bin (1024 bytes) - NOT FOUND
         * romset digdug is bad
         * ...
         *
         * - Status may be "good", "bad", or "best available".
         *
         * - When checking individual sets, status may also be
         *   "not found" or "not supported".
         *
         * - Older MAMEs used "correct" and "incorrect" instead of
         *   "good" and "bad".  Convert to the newer form if seen.
         *
         * - Similar output for -verifysamples.
         */

        copy = g_strchomp (g_ascii_strdown (line, -1));

        if ((token = strtok (copy, sep)) == NULL)
                goto exit;

        if (strcmp (token, "error") == 0)
                if ((token = strtok (NULL, sep)) == NULL)
                        goto exit;

        if (strcmp (token, "romset") != 0)
                if (strcmp (token, "sampleset") != 0)
                        goto exit;

        name = strtok (NULL, sep);
        while ((token = strtok (NULL, sep)) != NULL)
                status = token;

        if (name == NULL || status == NULL)
                goto exit;

        /* Normalize the status. */
        if (strcmp (status, "available") == 0)
                status = "best available";
        else if (strcmp (status, "found") == 0)
                status = "not found";
        else if (strcmp (status, "supported") == 0)
                status = "not supported";
        else if (strcmp (status, "correct") == 0)
                status = "good";
        else if (strcmp (status, "incorrect") == 0)
                status = "bad";

        if (out_name != NULL)
                *out_name = g_strdup (name);

        if (out_status != NULL)
                *out_status = g_strdup (status);

exit:
        g_free (copy);

        return (name != NULL && status != NULL);
}

/**
 * gva_mame_run_game:
 * @name: the name of the game to run
 * @error: return location for a #GError, or %NULL
 *
 * Spawns a "MAME @name" child process (with some additional user preferences)
 * and returns a #GvaProcess to track it.  If an error occurs while spawning,
 * it returns %NULL and sets @error.
 *
 * Returns: a new #GvaProcess, or %NULL
 **/
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

#ifdef HAVE_WNCK
        if (gva_mame_supports_maximize ())
                g_string_append (arguments, "-nomaximize ");
#endif

        if (gva_mame_supports_sound ())
        {
                GvaMuteButton *mute_button;

                mute_button = GVA_MUTE_BUTTON (GVA_WIDGET_MAIN_MUTE_BUTTON);
                if (gva_mute_button_get_muted (mute_button))
                        g_string_append (arguments, "-nosound ");
                else
                        g_string_append (arguments, "-sound ");
        }

        g_string_append_printf (arguments, "%s", name);

        /* Execute the command "${mame} ${name}". */
        process = gva_mame_process_spawn (
                arguments->str, G_PRIORITY_LOW, error);

        g_string_free (arguments, TRUE);

        return process;
}

/**
 * gva_mame_record_game:
 * @name: the name of the game to run
 * @inpname: the name of a file to record keypresses to
 * @error: return location for a #GError, or %NULL
 *
 * Spawns a "MAME -record @inpname @name" child process (with some additional
 * user preferences) and returns a #GvaProcess to track it.  If an error
 * occurs while spawning, it returns %NULL and sets @error.
 *
 * Returns: a new #GvaProcess, or %NULL
 **/
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

#ifdef HAVE_WNCK
        if (gva_mame_supports_maximize ())
                g_string_append (arguments, "-nomaximize ");
#endif

        if (gva_mame_supports_sound ())
        {
                GvaMuteButton *mute_button;

                mute_button = GVA_MUTE_BUTTON (GVA_WIDGET_MAIN_MUTE_BUTTON);
                if (gva_mute_button_get_muted (mute_button))
                        g_string_append (arguments, "-nosound ");
                else
                        g_string_append (arguments, "-sound ");
        }

        g_string_append_printf (arguments, "-record %s %s", inpname, name);

        /* Execute the command "${mame} -record ${inpname} ${name}". */
        process = gva_mame_process_spawn (
                arguments->str, G_PRIORITY_LOW, error);

        g_string_free (arguments, TRUE);

        return process;
}

/**
 * gva_mame_playback_game:
 * @name: the name of the game to play back
 * @inpname: the name of a file containing keypresses for @name
 * @error: return location for a #GError, or %NULL
 *
 * Spawns a "MAME -playback @inpname @name" child process (with some
 * additional user preferences) and returns a #GvaProcess to track it.
 * If an error occurs while spawning, it returns %NULL and sets @error.
 *
 * Returns: a new #GvaProcess, or %NULL
 **/
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

#ifdef HAVE_WNCK
        if (gva_mame_supports_maximize ())
                g_string_append (arguments, "-nomaximize ");
#endif

        if (gva_mame_supports_sound ())
        {
                GvaMuteButton *mute_button;

                mute_button = GVA_MUTE_BUTTON (GVA_WIDGET_MAIN_MUTE_BUTTON);
                if (gva_mute_button_get_muted (mute_button))
                        g_string_append (arguments, "-nosound ");
                else
                        g_string_append (arguments, "-sound ");
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

/**
 * gva_mame_get_save_state_file:
 * @name: the name of a game
 *
 * Returns the name of the automatic save state file for @name.
 *
 * Returns: the name of the save state file
 **/
gchar *
gva_mame_get_save_state_file (const gchar *name)
{
        const gchar *directory;
        gchar *basename;
        gchar *filename = NULL;
        GError *error = NULL;

        directory = gva_mame_get_state_directory (&error);
        gva_error_handle (&error);
        if (directory == NULL)
                return NULL;

        /* As of 0.115, MAME stores save state files as "name/auto.sta". */
        filename = g_build_filename (directory, name, "auto.sta", NULL);

        if (g_file_test (filename, G_FILE_TEST_IS_REGULAR))
                return filename;
        else
                g_free (filename);

        /* Prior to 0.115, MAME stored save state files as "name.sta". */
        basename = g_strdup_printf ("%s.sta", name);
        filename = g_build_filename (directory, basename, NULL);
        g_free (basename);

        if (g_file_test (filename, G_FILE_TEST_IS_REGULAR))
                return filename;
        else
                g_free (filename);

        return NULL;  /* file not found */
}

/**
 * gva_mame_delete_save_state:
 * @name: the name of a game
 *
 * Deletes the automatic save state file for @name, if it exists.
 **/
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

/**
 * gva_mame_get_input_directory:
 * @error: return location for a #GError, or %NULL
 *
 * Returns the value of the "input_directory" configuration key.  The
 * function caches the result to avoid excessive MAME invocations.  If
 * an error occurs, the function returns %NULL and sets @error.
 *
 * Returns: the directory containing MAME input files, or %NULL
 **/
const gchar *
gva_mame_get_input_directory (GError **error)
{
        static gchar *directory = NULL;

        if (G_UNLIKELY (directory == NULL)) {
                const gchar *config_key = "input_directory";
                directory = gva_mame_get_config_value (config_key, error);
        }

        return directory;
}

/**
 * gva_mame_get_snapshot_directory:
 * @error: return location for a #GError, or %NULL
 *
 * Returns the value of the "snapshot_directory" configuration key.  The
 * function caches the result to avoid excessive MAME invocations.  If an
 * error occurs, the function returns %NULL and sets @error.
 *
 * Returns: the directory containing MAME snapshot files, or %NULL
 **/
const gchar *
gva_mame_get_snapshot_directory (GError **error)
{
        static gchar *directory = NULL;

        if (G_UNLIKELY (directory == NULL)) {
                const gchar *config_key = "snapshot_directory";
                directory = gva_mame_get_config_value (config_key, error);
        }

        return directory;
}

/**
 * gva_mame_get_state_directory:
 * @error: return location for a #GError, or %NULL
 *
 * Returns the value of the "state_directory" configuration key.  The
 * function caches the result to avoid excessive MAME invocations.  If
 * an error occurs, the function returns %NULL and sets @error.
 *
 * Returns: the directory containing MAME save state files, or %NULL
 **/
const gchar *
gva_mame_get_state_directory (GError **error)
{
        static gchar *directory = NULL;

        if (G_UNLIKELY (directory == NULL)) {
                const gchar *config_key = "state_directory";
                directory = gva_mame_get_config_value (config_key, error);
        }

        return directory;
}

/*****************************************************************************
 * Documentation for the rest of the public MAME interface
 *****************************************************************************/

/**
 * gva_mame_get_path_sep:
 *
 * Returns the search path delimiter string.
 * XMAME uses UNIX-style ':' whereas SDLMAME uses Window-style ';'.
 *
 * Returns: search path delimiter string
 **/

/**
 * gva_mame_get_version:
 * @error: return location for a #GError, or %NULL
 *
 * Returns the version of the MAME executable that
 * <emphasis>GNOME Video Arcade</emphasis> is configured to use.  If an
 * error occurs, it returns %NULL and sets @error.
 *
 * Returns: the MAME version, or %NULL
 **/

/**
 * gva_mame_get_total_supported:
 * @error: return location for a #GError, or %NULL
 *
 * Returns the number of games supported by the MAME executable that
 * <emphasis>GNOME Video Arcade</emphasis> is configured to use.  If an
 * error occurs, it returns zero and sets @error.
 *
 * Returns: number of supported games, or zero
 **/
