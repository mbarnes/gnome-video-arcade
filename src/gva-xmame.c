#include "gva-xmame.h"

#include <stdarg.h>
#include <string.h>
#include <wordexp.h>  /* TODO configure should test for this */

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

gboolean
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
                return FALSE;

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

        return (exit_status == 0);
}

gchar *
gva_xmame_get_version (GError **error)
{
        gchar *version = NULL;
        gchar *strout = NULL;
        gchar **lines;
        guint n_lines, ii;

        /* Execute the command "${xmame} -version". */
        if (!gva_xmame_command ("-version", &strout, NULL, error))
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
        wordexp_t words;

        g_return_val_if_fail (config_key != NULL, NULL);

        /* Execute the command "${xmame} -showconfig". */
        if (!gva_xmame_command ("-showconfig", &strout, NULL, error))
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

        return config_value;
}

GHashTable *
gva_xmame_list_full (GError **error)
{
        GHashTable *hash_table = NULL;
        gchar *strout = NULL;
        gchar **lines;
        guint n_lines, ii;

        /* Execute the command "${xmame} -listfull". */
        if (!gva_xmame_command ("-listfull", &strout, NULL, error))
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

gboolean
gva_xmame_run_game (const gchar *romname, GError **error)
{
        g_return_val_if_fail (romname != NULL, FALSE);

        /* Execute the command "${xmame} ${romname}". */
        return gva_xmame_command (romname, NULL, NULL, error);
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
        success = gva_xmame_command (arguments, NULL, NULL, error);
        g_free (arguments);

        return success;
}

gboolean
gva_xmame_playback_game (const gchar *inpname, GError **error)
{
}
