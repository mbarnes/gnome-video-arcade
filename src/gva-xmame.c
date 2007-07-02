#include "gva-xmame.h"

#include <errno.h>
#include <stdarg.h>
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

        while (!gva_process_has_exited (process, &status))
                g_main_context_iteration (g_main_context_default (), TRUE);

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
                version = g_strdup (lines[ii]);
        }

        g_strfreev (lines);

        return version;
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

GPtrArray *
gva_xmame_get_romset_files (GError **error)
{
        gchar *romname;
        gchar *rompath;
        GPtrArray *array;
        GDir *dir;

        rompath = gva_xmame_get_config_value ("rompath", error);
        dir = (rompath != NULL) ? g_dir_open (rompath, 0, error) : NULL;
        g_free (rompath);

        if (dir == NULL)
                return NULL;

        array = g_ptr_array_new ();

        while ((romname = g_strdup (g_dir_read_name (dir))) != NULL)
        {
                g_strdelimit (romname, ".", '\0');
                g_ptr_array_add (array, romname);
        }

        g_dir_close (dir);

        return array;
}

static void
xmame_list_full_read (GvaProcess *process,
                      XmameAsyncData *data)
{
        gchar *line;

        /* Output is as follows:
         *
         * name      description
         * --------  -----------
         * romname   "Full Game Title"
         * romname   "Full Game Title"
         * ...
         *
         * Total Supported: nnnn
         */

        line = gva_process_stdout_read_line (process);
        data->lineno++;

        if (data->lineno > 2)
        {
                gchar *cp;

                cp = strchr (line, '"');
                if (cp != NULL && strchr (cp + 1, '"') != NULL)
                {
                        gchar *romname, *title;

                        title = g_strdelimit (cp, "\"", '\0') + 1;
                        romname = g_strchomp (line);

                        data->callback (romname, title, data->user_data);
                }
        }

        g_free (line);
}

static void
xmame_list_full_exit (GvaProcess *process,
                      gint status,
                      XmameAsyncData *data)
{
        xmame_async_data_free (data);
}

GvaProcess *
gva_xmame_list_full (GvaXmameCallback callback,
                     gpointer user_data,
                     GError **error)
{
        GvaProcess *process;
        XmameAsyncData *data;

        g_return_val_if_fail (callback != NULL, NULL);

        /* Execute the command "${xmame} -listfull". */
        process = gva_mame_process_spawn ("-listfull", error);
        if (process == NULL)
                return NULL;

        data = xmame_async_data_new (callback, user_data);

        g_signal_connect (
                process, "stdout-ready",
                G_CALLBACK (xmame_list_full_read), data);
        g_signal_connect (
                process, "exited",
                G_CALLBACK (xmame_list_full_exit), data);

        return process;
}

GvaProcess *
gva_xmame_list_xml (GError **error)
{
        /* Execute the command "${xmame} -listxml". */
        return gva_mame_process_spawn ("-listxml", error);
}

static void
xmame_verify_rom_sets_on_exit (GvaProcess *process,
                               gint status,
                               XmameAsyncData *data)
{
        gchar **lines;
        guint num_lines, ii;

        lines = gva_process_stdout_read_lines (process);

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

        num_lines = g_strv_length (lines);
        g_assert (num_lines > 4);

        for (ii = 2; ii < num_lines - 3; ii++)
        {
                gchar **tokens;

                tokens = g_strsplit_set (lines[ii], " ", 2);
                if (g_strv_length (tokens) == 2)
                        data->callback (
                                g_intern_string (g_strstrip (tokens[0])),
                                g_intern_string (g_strstrip (tokens[1])),
                                data->user_data);
                g_strfreev (tokens);
        }

        g_strfreev (lines);

        xmame_async_data_free (data);
}

GvaProcess *
gva_xmame_verify_rom_sets (GvaXmameCallback callback,
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
                process, "exited",
                G_CALLBACK (xmame_verify_rom_sets_on_exit), data);

        return process;
}

static void
xmame_verify_sample_sets_read (GvaProcess *process,
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
        data->lineno++;

        if (data->lineno > 2)
        {
                gchar **tokens;

                tokens = g_strsplit_set (line, " ", 2);
                if (g_strv_length (tokens) == 2)
                {
                        gchar *romname, *status;

                        romname = g_strstrip (tokens[0]);
                        status = g_strstrip (tokens[1]);

                        data->callback (romname, status, data->user_data);
                }
                g_strfreev (tokens);
        }

        g_free (line);
}

static void
xmame_verify_sample_sets_exit (GvaProcess *process,
                               gint status,
                               XmameAsyncData *data)
{
        xmame_async_data_free (data);
}

GvaProcess *
gva_xmame_verify_sample_sets (GvaXmameCallback callback,
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
                G_CALLBACK (xmame_verify_sample_sets_read), data);
        g_signal_connect (
                process, "exited",
                G_CALLBACK (xmame_verify_sample_sets_exit), data);

        return process;
}

GvaProcess *
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
        process = gva_mame_process_spawn (arguments->str, error);

        g_string_free (arguments, TRUE);

        return process;
}

GvaProcess *
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
        process = gva_mame_process_spawn (arguments->str, error);

        g_string_free (arguments, TRUE);

        return process;
}

GvaProcess *
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
