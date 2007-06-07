#include "gva-xmame.h"

#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#ifdef HAVE_WORDEXP_H
#include <wordexp.h>
#endif

#include "gva-preferences.h"

static void
xmame_post_game_analysis (GvaProcess *process)
{
        gint exit_status;
        GError *error = NULL;

        exit_status = gva_xmame_wait_for_exit (process, &error);

        if (error == NULL && exit_status != 0)
                g_set_error (
                        &error, GVA_ERROR, GVA_ERROR_XMAME,
                        _("Child process exited with status (%d)"),
                        exit_status);

        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_clear_error (&error);
        }

        g_object_unref (process);
}

static gboolean
xmame_scan_for_error (const gchar *string, GError **error)
{
        gchar **lines;
        guint n_lines, ii;

        if (string == NULL)
                return FALSE;

        lines = g_strsplit_set (string, "\n", -1);
        n_lines = g_strv_length (lines);

        for (ii = 0; ii < n_lines; ii++)
        {
                if (strlen (g_strstrip (lines[ii])) == 0)
                        continue;

                if (g_ascii_strncasecmp (lines[ii], "error: ", 7) == 0)
                {
                        gchar *error_message;

                        error_message = lines[ii] + 7;
                        *error_message = g_ascii_toupper (*error_message);

                        g_set_error (
                                error, GVA_ERROR, GVA_ERROR_XMAME,
                                "%s", error_message);

                        break;
                }
        }

        g_strfreev (lines);

        return (ii < n_lines);
}

GQuark
gva_error_quark (void)
{
        static GQuark quark = 0;

        if (G_UNLIKELY (quark == 0))
                quark = g_quark_from_static_string ("gva-error-quark");

        return quark;
}

gchar *
gva_xmame_get_executable (GError **error)
{
        gchar *executable;

        executable = g_find_program_in_path ("xmame");

        if (executable == NULL)
                g_set_error (error, GVA_ERROR, GVA_ERROR_XMAME,
                        _("Could not find xmame executable"));

        return executable;
}

GvaProcess *
gva_xmame_async_command (const gchar *arguments,
                         GError **error)
{
        GvaProcess *process;
        gchar *command_line;
        gchar *executable;
        gchar **argv;
        gint standard_input;
        gint standard_output;
        gint standard_error;
        GPid child_pid;
        gboolean success;

        g_return_val_if_fail (arguments != NULL, FALSE);

        if ((executable = gva_xmame_get_executable (error)) == NULL)
                return NULL;

        command_line = g_strdup_printf ("%s %s", executable, arguments);
        success = g_shell_parse_argv (command_line, NULL, &argv, error);
        g_free (command_line);
        g_free (executable);

        if (!success)
                return NULL;

        success = gdk_spawn_on_screen_with_pipes (
                gdk_screen_get_default (), NULL, argv, NULL,
                G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &child_pid,
                &standard_input, &standard_output, &standard_error,
                error);

        g_strfreev (argv);

        if (!success)
                return NULL;

        return gva_process_new (
                child_pid, standard_input, standard_output, standard_error);
}

gint
gva_xmame_command (const gchar *arguments,
                   gchar **standard_output,
                   gchar **standard_error,
                   GError **error)
{
        GvaProcess *process;
        gint exit_status;

        process = gva_xmame_async_command (arguments, error);
        if (process == NULL)
                return -1;

        exit_status = gva_xmame_wait_for_exit (process, error);

        if (exit_status >= 0 && standard_output != NULL)
                *standard_output = gva_process_read_stdout (process);

        if (exit_status >= 0 && standard_error != NULL)
                *standard_error = gva_process_read_stderr (process);

        g_object_unref (process);

        return exit_status;
}

gint
gva_xmame_wait_for_exit (GvaProcess *process, GError **error)
{
        gint exit_status;

        g_return_val_if_fail (process != NULL, FALSE);

        while (!gva_process_has_exited (process, &exit_status))
                g_main_context_iteration (g_main_context_default (), TRUE);

        if (xmame_scan_for_error (gva_process_peek_stderr (process), error))
                return -1;

        if (xmame_scan_for_error (gva_process_peek_stdout (process), error))
                return -1;

        if (WIFSIGNALED (exit_status))
        {
                g_set_error (
                        error, GVA_ERROR, GVA_ERROR_XMAME,
                        "Child process terminated: %s",
                        g_strsignal (WTERMSIG (exit_status)));
                return -1;
        }

        g_assert (WIFEXITED (exit_status));
        return WEXITSTATUS (exit_status);
}

gchar *
gva_xmame_get_version (GError **error)
{
        gchar *version = NULL;
        gchar *output;
        gchar **lines;
        guint n_lines, ii;

        /* Execute the command "${xmame} -version". */
        if (gva_xmame_command ("-version", &output, NULL, error) != 0)
                return NULL;

        /* Output is as follows:
         *
         * xmame (backend) version n.nnn (Mmm dd yyyy)
         */

        lines = g_strsplit_set (output, "\n", -1);
        n_lines = g_strv_length (lines);
        g_free (output);

        for (ii = 0; ii < n_lines && version == NULL; ii++)
        {
                if (strstr (lines[ii], "xmame") == NULL)
                        continue;
                if (strstr (lines[ii], "version") == NULL)
                        continue;
                version = g_strdup (lines[ii]);
        }

        g_strfreev (lines);

        return version;
}

gchar *
gva_xmame_get_config_value (const gchar *config_key, GError **error)
{
        gchar *config_value = NULL;
        gchar *output;
        gchar **lines;
        guint n_lines, ii;
#ifdef HAVE_WORDEXP_H
        wordexp_t words;
#endif

        g_return_val_if_fail (config_key != NULL, NULL);

        /* Execute the command "${xmame} -showconfig". */
        if (gva_xmame_command ("-showconfig", &output, NULL, error) != 0)
                return NULL;

        /* Output is as follows:
         *
         * # Lines that start with '#' are comments.
         * config_key           config_value
         * config_key           config_value
         * ...
         */

        lines = g_strsplit_set (output, "\n", -1);
        n_lines = g_strv_length (lines);
        g_free (output);

        for (ii = 0; ii < n_lines; ii++)
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
                return NULL;

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

        return config_value;
}

gboolean
gva_xmame_has_config_value (const gchar *config_key)
{
        gchar *config_value;
        GError *error = NULL;

        config_value = gva_xmame_get_config_value (config_key, &error);
        if (config_value != NULL)
        {
                g_free (config_value);
                return TRUE;
        }
        else if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_error_free (error);
        }

        return FALSE;
}

GList *
gva_xmame_get_available (GError **error)
{
        gchar *romname;
        gchar *rompath;
        GList *list = NULL;
        GDir *dir;

        rompath = gva_xmame_get_config_value ("rompath", error);
        dir = (rompath != NULL) ? g_dir_open (rompath, 0, error) : NULL;
        g_free (rompath);

        if (dir == NULL)
                return NULL;

        while ((romname = g_strdup (g_dir_read_name (dir))) != NULL)
        {
                g_strdelimit (romname, ".", '\0');
                list = g_list_prepend (list, romname);
        }

        g_dir_close (dir);

        return g_list_reverse (list);
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
                        gchar romname[16];
                        GIOStatus status;

                        status = g_io_channel_read_chars (
                                channel, romname, sizeof (romname),
                                NULL, &local_error);
                        if (status == G_IO_STATUS_NORMAL)
                                g_hash_table_insert (
                                        hash_table, filename,
                                        g_strdup (romname));
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

gchar *
gva_xmame_get_rompath (GError **error)
{
        return gva_xmame_get_config_value ("rompath", error);
}

GHashTable *
gva_xmame_list_full (GError **error)
{
        GHashTable *hash_table = NULL;
        gchar *output;
        gchar **lines;
        guint n_lines, ii;

        /* Execute the command "${xmame} -listfull". */
        if (gva_xmame_command ("-listfull", &output, NULL, error) != 0)
                return NULL;

        /* Output is as follows:
         *
         * name      description
         * --------  -----------
         * romname   "Full Game Title"
         * romname   "Full Game Title"
         * ...
         *
         *
         * Total Supported: nnnn
         */

        hash_table = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) g_free);

        lines = g_strsplit_set (output, "\n", -1);
        n_lines = g_strv_length (lines);
        g_assert (n_lines > 0);
        g_free (output);

        for (ii = 0; ii < n_lines; ii++)
        {
                gchar *key, *value, *cp;

                cp = strchr (lines[ii], '\"');
                if (cp == NULL || strchr (cp + 1, '\"') == NULL)
                        continue;
                value = g_strdup (g_strdelimit (cp, "\"", '\0') + 1);
                key = g_strdup (g_strchomp (lines[ii]));

                g_hash_table_insert (hash_table, key, value);
        }

        g_strfreev (lines);

        return hash_table;
}

typedef struct
{
        GvaXmameVerifyFunc func;
        gpointer user_data;

} XmameVerifyData;

static void
xmame_verify_rom_sets_on_exit (GvaProcess *process, XmameVerifyData *data)
{
        gchar *output;
        gchar **lines;
        guint n_lines, ii;

        output = gva_process_read_stdout (process);

        /* Output is as follows:
         *
         * name      result
         * --------  ------
         * romname   best available|correct|incorrect|not found
         * romname   best available|correct|incorrect|not found
         * ...
         *
         * Total Supported: nnnn
         * Found: nnnn ...
         * Not found: nnnn
         */

        lines = g_strsplit_set (output, "\n", -1);
        n_lines = g_strv_length (lines);
        g_assert (n_lines > 4);
        g_free (output);

        for (ii = 2; ii < n_lines - 3; ii++)
        {
                gchar **tokens;

                tokens = g_strsplit_set (lines[ii], " ", 2);
                if (g_strv_length (tokens) == 2)
                        data->func (
                                g_intern_string (g_strstrip (tokens[0])),
                                g_intern_string (g_strstrip (tokens[1])),
                                data->user_data);
                g_strfreev (tokens);
        }

        g_strfreev (lines);

        g_slice_free (XmameVerifyData, data);
}

GvaProcess *
gva_xmame_verify_rom_sets (GvaXmameVerifyFunc func,
                           gpointer user_data,
                           GError **error)
{
        GvaProcess *process;
        XmameVerifyData *data;

        g_return_val_if_fail (func != NULL, NULL);

        data = g_slice_new (XmameVerifyData);
        data->func = func;
        data->user_data = user_data;

        /* Execute the command "${xmame} -verifyromsets". */
        process = gva_xmame_async_command ("-verifyromsets", error);

        if (process == NULL)
                g_slice_free (XmameVerifyData, data);
        else
                g_signal_connect (
                        process, "exited",
                        G_CALLBACK (xmame_verify_rom_sets_on_exit), NULL);

        return process;
}

GHashTable *
gva_xmame_verify_sample_sets (GError **error)
{
        GHashTable *hash_table = NULL;
        gchar *output;
        gchar **lines;
        guint n_lines, ii;

        /* Execute the command "${xmame} -verifysamplesets". */
        /* XXX What are the exit codes for this command? */
        if (gva_xmame_command ("-verifysamplesets", &output, NULL, error) < 0)
                return NULL;

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

        hash_table = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) g_free);

        lines = g_strsplit_set (output, "\n", -1);
        n_lines = g_strv_length (lines);
        g_assert (n_lines > 4);
        g_free (output);

        for (ii = 2; ii < n_lines - 4; ii++)
        {
                gchar **tokens;

                tokens = g_strsplit_set (lines[ii], " ", 2);
                if (g_strv_length (tokens) == 2)
                {
                        gchar *key, *value;

                        key = g_strdup (g_strstrip (tokens[0]));
                        value = g_strdup (g_strstrip (tokens[1]));
                        g_hash_table_insert (hash_table, key, value);
                }
                g_strfreev (tokens);
        }

        g_strfreev (lines);

        return hash_table;
}

gboolean
gva_xmame_run_game (const gchar *romname, GError **error)
{
        GvaProcess *process;
        GString *arguments;

        g_return_val_if_fail (romname != NULL, FALSE);

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

        g_string_append_printf (arguments, "%s", romname);

        /* Execute the command "${xmame} ${romname}". */
        process = gva_xmame_async_command (arguments->str, error);

        if (process != NULL)
                g_signal_connect (
                        process, "exited",
                        G_CALLBACK (xmame_post_game_analysis), NULL);

        g_string_free (arguments, TRUE);

        return (process != NULL);
}

gboolean
gva_xmame_record_game (const gchar *romname, const gchar *inpname,
                       GError **error)
{
        GvaProcess *process;
        GString *arguments;

        g_return_val_if_fail (romname != NULL, FALSE);

        if (inpname == NULL)
                inpname = romname;

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

        g_string_append_printf (arguments, "-record %s %s", inpname, romname);

        /* Execute the command "${xmame} -record ${inpname} ${romname}". */
        process = gva_xmame_async_command (arguments->str, error);

        if (process != NULL)
                g_signal_connect (
                        process, "exited",
                        G_CALLBACK (xmame_post_game_analysis), NULL);

        g_string_free (arguments, TRUE);

        return (process != NULL);
}

gboolean
gva_xmame_playback_game (const gchar *romname, const gchar *inpname,
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
        process = gva_xmame_async_command (arguments->str, error);

        if (process != NULL)
                g_signal_connect (
                        process, "exited",
                        G_CALLBACK (xmame_post_game_analysis), NULL);

        g_string_free (arguments, TRUE);

        if (process == NULL)
                return FALSE;

        /* xmame asks the user to press return before it will start playing
         * back the game, so we have to supply the expected keystroke. */
        return gva_process_write_stdin (process, "\n", 1, error);
}

gboolean
gva_xmame_clear_state (const gchar *romname, GError **error)
{
        gchar *basename;
        gchar *directory;
        gchar *filename;
        gboolean success = TRUE;

        directory = gva_xmame_get_config_value ("state_directory", error);
        if (directory == NULL)
                return FALSE;

        basename = g_strdup_printf ("%s.sta", romname);
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
