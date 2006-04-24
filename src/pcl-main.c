/* PCL - Predicate Constraint Language
 * Copyright (C) 2006 The Boeing Company
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "pcl.h"

gboolean PCL_OPTION_DEBUG = FALSE;
gboolean PCL_OPTION_PARSE_ONLY = FALSE;

static gchar *command = NULL;
static gboolean show_version = FALSE;
static gint remaining_argc = 0;
static gchar **remaining_argv = NULL;

extern int yydebug;  /* from the parser */

static GOptionEntry entries[] =
{
        { "command", 'c', 0, G_OPTION_ARG_STRING, &command,
                "Execute argument string, then exit", "STRING" },
        { "debug", 'd', 0, G_OPTION_ARG_NONE, &PCL_OPTION_DEBUG,
                "Print debug information to stderr", NULL },
        { "parse-only", 'p', 0, G_OPTION_ARG_NONE, &PCL_OPTION_PARSE_ONLY,
                "Just parse the input, do not execute it", NULL },
        { "version", 'v', 0, G_OPTION_ARG_NONE, &show_version,
                "Print version number, then exit", NULL },
        { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY,
                &remaining_argv, NULL, NULL }, 
        { NULL }
};

static void
run_startup_file (void)
{
        const gchar *startup = g_getenv ("PCLSTARTUP");
        if (startup != NULL && startup[0] != '\0')
        {
                FILE *stream = fopen (startup, "r");
                if (stream != NULL)
                {
                        (void) pcl_run_simple_file_ex_flags (
                                        stream, startup, FALSE, NULL);
                        pcl_error_clear ();
                        fclose (stream);
                }
        }
}

gint
pcl_main (gint argc, gchar **argv)
{
        const gchar *prgname;
        GOptionContext *context;
        gboolean stdin_is_interactive;
        gchar *filename = NULL;
        GError *error = NULL;
        FILE *stream = stdin;
        gint status;

        context = g_option_context_new ("[FILE]");
        g_option_context_add_main_entries (context, entries, NULL);
        g_option_context_parse (context, &argc, &argv, &error);
        g_option_context_free (context);
        prgname = g_get_prgname ();

        if (error != NULL)
        {
                g_printerr ("%s: %s\n", prgname, error->message);
                g_error_free (error);
                return 2;
        }

        if (show_version)
        {
                g_printerr ("%s\n", PACKAGE_STRING);
                return 0;
        }

        if (PCL_OPTION_DEBUG)
        {
                yydebug = 0;  /* non-zero enables parser traces */
                g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL |
                                        G_LOG_LEVEL_WARNING);
        }

        if (remaining_argv != NULL)
                remaining_argc = (gint) g_strv_length (remaining_argv);

        if (remaining_argc > 0)
        {
                filename = remaining_argv[0];
                stream = fopen (filename, "r");
                if (stream == NULL)
                {
                        const gchar *message = g_strerror (errno);
                        g_printerr ("%s: ", prgname);
                        g_printerr ("%s: %s\n", filename, message);
                        return 2;
                }
        }

        stdin_is_interactive = isatty (fileno (stdin));

        pcl_initialize ();

        if (command == NULL && filename == NULL && stdin_is_interactive)
                g_printerr ("%s\n", PACKAGE_STRING);

        /* NOTE: sys.argv[0] becomes the PCL script name */
        pcl_sys_set_argv (remaining_argc, remaining_argv);

        if (command != NULL)
                status = pcl_run_simple_string (command);
        else
        {
                if (filename == NULL && stdin_is_interactive)
                        run_startup_file ();
                status = pcl_run_any_file_ex_flags (stream,
                        filename == NULL ? "<stdin>" : filename,
                        filename != NULL, NULL);
        }

        pcl_finalize ();

        return status;
}
