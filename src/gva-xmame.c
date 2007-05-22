#include "gva-xmame.h"

#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_WORDEXP_H
#include <wordexp.h>
#endif

#define GVA_GCONF_PREFIX        "/apps/" PACKAGE

static GHashTable *xmame_config = NULL;

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

gboolean
gva_xmame_scan_for_error (const gchar *xmame_output, GError **error)
{
        gchar **lines;
        guint n_lines, ii;

        if (xmame_output == NULL)
                return FALSE;

        lines = g_strsplit_set (xmame_output, "\n", -1);
        n_lines = g_strv_length (lines);

        for (ii = 0; ii < n_lines; ii++)
        {
                if (strlen (g_strstrip (lines[ii])) == 0)
                        continue;

                if (g_ascii_strncasecmp (lines[ii], "error: ", 7))
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

gint
gva_xmame_command (const gchar *arguments, gchar **standard_output,
                   gchar **standard_error, GError **error)
{
        gchar *local_standard_output = NULL;
        gchar *local_standard_error = NULL;
        gchar *executable;
        gchar *command_line;
        gint exit_status = -1;
        gboolean success;

        g_return_val_if_fail (arguments != NULL, FALSE);

        if ((executable = gva_xmame_get_executable (error)) == NULL)
                return -1;

        command_line = g_strdup_printf ("%s %s", executable, arguments);

        success = g_spawn_command_line_sync (
                command_line, &local_standard_output,
                &local_standard_error, &exit_status, error);

        g_free (command_line);
        g_free (executable);

        if (!success || exit_status == 0 || error == NULL)
                goto exit;

        if (gva_xmame_scan_for_error (local_standard_error, error))
                goto exit;

        if (gva_xmame_scan_for_error (local_standard_output, error))
                goto exit;

        g_set_error (
                error, GVA_ERROR, GVA_ERROR_XMAME,
                "xmame exited with status (%d)", exit_status);

exit:
        if (standard_output != NULL)
                *standard_output = local_standard_output;
        else
                g_free (local_standard_output);

        if (standard_error != NULL)
                *standard_error = local_standard_error;
        else
                g_free (local_standard_error);

        return exit_status;
}

gchar *
gva_xmame_get_version (GError **error)
{
        gchar *version = NULL;
        gchar *strout = NULL;
        gchar **lines;
        guint n_lines, ii;

        /* Execute the command "${xmame} -version". */
        if (gva_xmame_command ("-version", &strout, NULL, error) != 0)
                goto exit;

        /* Output is as follows:
         *
         * xmame (backend) version n.nnn (Mmm dd yyyy)
         */

        lines = g_strsplit_set (strout, "\n", -1);
        n_lines = g_strv_length (lines);

        for (ii = 0; ii < n_lines && version == NULL; ii++)
        {
                if (strstr (lines[ii], "xmame") == NULL)
                        continue;
                if (strstr (lines[ii], "version") == NULL)
                        continue;
                version = g_strdup (lines[ii]);
        }

        g_strfreev (lines);

exit:
        g_free (strout);
        return version;
}

gchar *
gva_xmame_get_config_value (const gchar *config_key, GError **error)
{
        gchar *config_value = NULL;
        gchar *strout = NULL;
        gchar **lines;
        guint n_lines, ii;
#ifdef HAVE_WORDEXP_H
        wordexp_t words;
#endif

        g_return_val_if_fail (config_key != NULL, NULL);

        /* Execute the command "${xmame} -showconfig". */
        if (gva_xmame_command ("-showconfig", &strout, NULL, error) != 0)
                return NULL;

        /* Output is as follows:
         *
         * # Lines that start with '#' are comments.
         * config_key           config_value
         * config_key           config_value
         * ...
         */

        lines = g_strsplit_set (strout, "\n", -1);
        n_lines = g_strv_length (lines);

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
        g_free (strout);

        if (config_value == NULL)
                return NULL;

#ifdef HAVE_WORDEXP_H
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

GHashTable *
gva_xmame_list_full (GError **error)
{
        GHashTable *hash_table = NULL;
        gchar *strout = NULL;
        gchar **lines;
        guint n_lines, ii;

        /* Execute the command "${xmame} -listfull". */
        if (gva_xmame_command ("-listfull", &strout, NULL, error) != 0)
                goto exit;

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

        lines = g_strsplit_set (strout, "\n", -1);
        n_lines = g_strv_length (lines);
        g_assert (n_lines > 0);

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

exit:
        g_free (strout);
        return hash_table;
}

GHashTable *
gva_xmame_verify_sample_sets (GError **error)
{
        GHashTable *hash_table = NULL;
        gchar *strout = NULL;
        gchar **lines;
        guint n_lines, ii;

        /* Execute the command "${xmame} -verifysamplesets". */
        /* XXX What are the exit codes for this command? */
        if (gva_xmame_command ("-verifysamplesets", &strout, NULL, error) < 0)
                goto exit;

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

        lines = g_strsplit_set (strout, "\n", -1);
        n_lines = g_strv_length (lines);
        g_assert (n_lines > 4);

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

exit:
        g_free (strout);
        return hash_table;
}

gboolean
gva_xmame_run_game (const gchar *romname, GError **error)
{
        g_return_val_if_fail (romname != NULL, FALSE);

        /* Execute the command "${xmame} ${romname}". */
        return (gva_xmame_command (romname, NULL, NULL, error) == 0);
}

gboolean
gva_xmame_record_game (const gchar *romname, const gchar *inpname,
                       GError **error)
{
        gchar *arguments;
        gboolean success;

        g_return_val_if_fail (romname != NULL, FALSE);

        if (inpname == NULL)
                inpname = romname;

        /* Execute the command "${xmame} -record ${inpname} ${romname}". */
        arguments = g_strdup_printf ("-record %s %s", inpname, romname);
        success = (gva_xmame_command (arguments, NULL, NULL, error) == 0);
        g_free (arguments);

        return success;
}

static void
xmame_exited (GPid pid, gint exit_status, gint *exit_status_out)
{
        *exit_status_out = exit_status;
        g_spawn_close_pid (pid);
}

gboolean
gva_xmame_playback_game (const gchar *inpname, GError **error)
{
        GIOChannel *channel;
        gchar *executable;
        gchar *argv[4];
        gchar *buffer = NULL;
        gint standard_input = -1;
        gint standard_output = -1;
        gint standard_error = -1;
        gint exit_status = ~0;
        gboolean success;
        GIOStatus status;
        GPid pid;

        /* xmame asks the user to press return before it will start playing
         * back the recorded game, so we have to go through extra trouble
         * to supply the expected keystroke. */

        g_return_val_if_fail (inpname != NULL, FALSE);

        if ((executable = gva_xmame_get_executable (error)) == NULL)
                return FALSE;

        argv[0] = executable;
        argv[1] = "-playback";
        argv[2] = (gchar *) inpname;
        argv[3] = NULL;

        success = g_spawn_async_with_pipes (
                NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &pid,
                &standard_input, &standard_output, &standard_error, error);

        g_free (executable);

        if (!success)
                goto exit;

        /* All for this!
         * FIXME Being lazy about error checking. */
        write (standard_input, "\n", 1);

        g_child_watch_add (pid, (GChildWatchFunc) xmame_exited, &exit_status);
        while (exit_status == ~0)
                g_main_context_iteration (NULL, TRUE);
        if (exit_status == 0)
                goto exit;

        /* XXX Portability issue? */
        channel = g_io_channel_unix_new (standard_error);
        status = G_IO_STATUS_AGAIN;
        while (status == G_IO_STATUS_AGAIN)
                status = g_io_channel_read_to_end (
                        channel, &buffer, NULL, error);
        g_io_channel_unref (channel);
        if (status == G_IO_STATUS_ERROR)
                goto exit;
        if (gva_xmame_scan_for_error (buffer, error))
                goto exit;
        g_free (buffer);
        buffer = NULL;

        /* XXX Portability issue? */
        channel = g_io_channel_unix_new (standard_output);
        status = G_IO_STATUS_AGAIN;
        while (status == G_IO_STATUS_AGAIN)
                status = g_io_channel_read_to_end (
                        channel, &buffer, NULL, error);
        g_io_channel_unref (channel);
        if (status == G_IO_STATUS_ERROR)
                goto exit;
        if (gva_xmame_scan_for_error (buffer, error))
                goto exit;
        g_free (buffer);
        buffer = NULL;

        g_set_error (
                error, GVA_ERROR, GVA_ERROR_XMAME,
                "xmame exited with status (%d)", exit_status);

exit:
        if (standard_input >= 0)
                close (standard_input);
        if (standard_output >= 0)
                close (standard_output);
        if (standard_error >= 0)
                close (standard_error);

        g_free (buffer);

        return (exit_status == 0);
}
