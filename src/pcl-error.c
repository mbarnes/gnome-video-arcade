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

static void
error_handle_system_exit (void)
{
        PclObject *exception, *value, *traceback;
        gint status = 0;

        pcl_error_fetch (&exception, &value, &traceback);
        if (!pcl_flush_line ())
                pcl_error_clear ();
        fflush (stdout);
        if (value == NULL || value == PCL_NONE)
                goto done;
        if (PCL_IS_INT (value))
                status = (gint) pcl_int_as_long (value);
        else
        {
                pcl_object_print (value, stderr, PCL_PRINT_FLAG_RAW);
                pcl_sys_write_stderr ("\n");
                status = 1;
        }

done:
        pcl_error_restore (exception, value, traceback);
        pcl_error_clear ();
        pcl_exit (status);  /* never returns */
}

PclObject *
pcl_error_occurred (void)
{
        PclThreadState *ts = pcl_thread_state_get ();
        return ts->current_exc_type;
}

void
pcl_error_fetch (PclObject **p_exception, PclObject **p_value,
                 PclObject **p_traceback)
{
        PclThreadState *ts = pcl_thread_state_get ();

        *p_exception = ts->current_exc_type;
        *p_value = ts->current_exc_value;
        *p_traceback = ts->current_exc_traceback;

        ts->current_exc_type = NULL;
        ts->current_exc_value = NULL;
        ts->current_exc_traceback = NULL;
}

void
pcl_error_display (PclObject *exception, PclObject *value,
                   PclObject *traceback)
{
        PclObject *file = pcl_sys_get_object ("stderr");
        gboolean success = TRUE;

        if (file == NULL)
        {
                g_printerr ("Lost sys.stderr\n");
                return;
        }

        if (!pcl_flush_line ())
                pcl_error_clear ();
        if (PCL_IS_TRACEBACK (traceback))
                /* FIXME This fails for interactive mode, but keep going. */
                /*success =*/ pcl_traceback_print (traceback, file);
        if (success && exception == pcl_exception_syntax_error ())
        {
                /* FIXME Kludgy, very different from Python */
                GString *buffer = g_string_sized_new (256);
                gchar *filename, *line, *message;
                gint lineno, offset, ii;
                FILE *stream;

                success = pcl_arg_parse_tuple (value, "siis",
                          &filename, &lineno, &offset, &message);
                if (!success)
                {
                        pcl_error_clear ();
                        return;
                }

                line = NULL;
                stream = pcl_open_source (filename);
                if (stream != NULL)
                {
                        line = pcl_error_program_text (stream, lineno);
                        fclose (stream);
                }

                g_string_printf (buffer, "  File \"%s\", line %d\n",
                                 filename, lineno);
                if (line != NULL)
                {
                        gchar *indent = "    ";

                        /* Adjust offset if it extends past EOL. */
                        offset = MIN (offset, strlen (line) - 1);
                        g_string_append (buffer, indent);
                        g_string_append (buffer, line);
                        g_string_append_c (buffer, '\n');
                        g_string_append (buffer, indent);
                        for (ii = 0; ii < offset; ii++)
                        {
                                if (g_ascii_isspace (line[ii]))
                                        g_string_append_c (buffer, line[ii]);
                                else
                                        g_string_append_c (buffer, ' ');
                        }
                        g_string_append (buffer, "^\n");
                }
                pcl_file_write_string (file, buffer->str);

                g_free (line);
                g_string_free (buffer, TRUE);

                /* Re-fetch the error message. */
                value = PCL_TUPLE_GET_ITEM (value, 3);
        }
        if (success)
        {
                success = pcl_file_write_string (
                          file, pcl_exception_get_name (exception));
        }
        if (success)
        {
                if (!PCL_IS_NONE (value))
                {
                        PclObject *str = pcl_object_str (value);
                        if (str == NULL)
                                success = FALSE;
                        else if (pcl_object_measure (str) > 0)
                                success = pcl_file_write_string (file, ": ");
                        if (success)
                                success = pcl_file_write_object (file,
                                                str, PCL_PRINT_FLAG_RAW);
                        if (str != NULL)
                                pcl_object_unref (str);
                }
        }
        if (success)
                success = pcl_file_write_string (file, "\n");
}

void
pcl_error_restore (PclObject *exception, PclObject *value,
                   PclObject *traceback)
{
        PclThreadState *ts = pcl_thread_state_get ();
        PclObject *old_exception, *old_value, *old_traceback;

        old_exception = ts->current_exc_type;
        old_value = ts->current_exc_value;
        old_traceback = ts->current_exc_traceback;

        /* steal the references */
        ts->current_exc_type = exception;
        ts->current_exc_value = value;
        ts->current_exc_traceback = traceback;

        if (old_exception != NULL)
                pcl_object_unref (old_exception);
        if (old_value != NULL)
                pcl_object_unref (old_value);
        if (old_traceback != NULL)
                pcl_object_unref (old_traceback);
}

void
pcl_error_normalize (PclObject **p_exception, PclObject **p_value,
                     PclObject **p_traceback)
{
        if (*p_exception == NULL)
                return;
        if (*p_value == NULL)
                *p_value = pcl_object_ref (PCL_NONE);

        /* XXX Python does more stuff here... */
}

void
pcl_error_clear (void)
{
        pcl_error_restore (NULL, NULL, NULL);
}

void
pcl_error_print (void)
{
        PclObject *system_exit = pcl_exception_system_exit ();
        PclObject *exception, *value, *traceback;
        PclObject *hook;

        if (pcl_error_exception_matches (system_exit))
                error_handle_system_exit ();
        pcl_error_fetch (&exception, &value, &traceback);
        pcl_error_normalize (&exception, &value, &traceback);
        if (exception == NULL)
                return;

        hook = pcl_sys_get_object ("excepthook");
        if (hook != NULL)
        {
                PclObject *args;
                PclObject *result;

                args = pcl_tuple_pack (3, exception,
                       (value != NULL) ? value : PCL_NONE,
                       (traceback != NULL) ? traceback : PCL_NONE);
                result = pcl_eval_call_object (hook, args);
                if (result == NULL)
                {
                        PclObject *exception2, *value2, *traceback2;

                        if (pcl_error_exception_matches (system_exit))
                                error_handle_system_exit ();
                        pcl_error_fetch (&exception2, &value2, &traceback2);
                        pcl_error_normalize (&exception2, &value2, &traceback2);
                        if (!pcl_flush_line ())
                                pcl_error_clear ();
                        fflush (stdout);
                        pcl_sys_write_stderr ("Error in sys.excepthook:\n");
                        pcl_error_display (exception2, value2, traceback2);
                        pcl_sys_write_stderr ("\nOriginal exception was:\n");
                        pcl_error_display (exception, value, traceback);
                        if (exception2 != NULL)
                                pcl_object_unref (exception2);
                        if (value2 != NULL)
                                pcl_object_unref (value2);
                        if (traceback2 != NULL)
                                pcl_object_unref (traceback2);
                }
                if (result != NULL)
                        pcl_object_unref (result);
                if (args != NULL)
                        pcl_object_unref (args);
        }
        else
        {
                pcl_sys_write_stderr ("sys.excepthook is missing\n");
                pcl_error_display (exception, value, traceback);
        }
        if (exception != NULL)
                pcl_object_unref (exception);
        if (value != NULL)
                pcl_object_unref (value);
        if (traceback != NULL)
                pcl_object_unref (traceback);
}

void
pcl_error_set_none (PclObject *exception)
{
        pcl_error_set_object (exception, NULL);
}

void
pcl_error_set_object (PclObject *exception, PclObject *value)
{
        /* pcl_error_restore() steals references */
        if (exception != NULL)
                pcl_object_ref (exception);
        if (value != NULL)
                pcl_object_ref (value);
        pcl_error_restore (exception, value, NULL);
}

void
pcl_error_set_string (PclObject *exception, const gchar *string)
{
        PclObject *value = pcl_string_from_string (string);
        pcl_error_set_object (exception, value);
        pcl_object_unref (value);
}

void
pcl_error_set_format (PclObject *exception, const gchar *format, ...)
{
        va_list va;
        PclObject *value;

        va_start (va, format);
        value = pcl_string_from_format_va (format, va);
        pcl_error_set_object (exception, value);
        pcl_object_unref (value);
        va_end (va);
}

void
pcl_error_set_from_errno (PclObject *exception)
{
        PclObject *value = pcl_string_from_string (g_strerror (errno));
        pcl_error_set_object (exception, value);
}

gboolean
pcl_error_exception_matches (PclObject *exception)
{
        PclObject *error = pcl_error_occurred ();
        return pcl_error_given_exception_matches (error, exception);
}

gboolean
pcl_error_given_exception_matches (PclObject *error, PclObject *exception)
{
        /* XXX While exceptions are just instances, this is trivial. */
        /* XXX Need to check recursively if exception is a tuple. */
        return (error != NULL && exception != NULL && error == exception);
}

gchar *
pcl_error_program_text (FILE *stream, gint lineno)
{
        gchar buffer[2048];
        gchar *last_cp = &buffer[sizeof (buffer) - 2];
        gint ii;

        if (stream == NULL || lineno < 1)
                return NULL;

        for (ii = 0; ii < lineno; ii++)
        {
                {
                        *last_cp = '\0';
                        if (pcl_universal_newline_fgets (
                                        buffer, sizeof (buffer),
                                        stream, NULL) == NULL)
                                break;
                }
                while (*last_cp != '\0' && *last_cp != '\n');
        }

        return g_strdup (g_strstrip (buffer));
}

void
pcl_error_write_unraisable (PclObject *object)
{
        PclObject *exception, *value, *traceback;
        PclObject *file;

        pcl_error_fetch (&exception, &value, &traceback);
        file = pcl_sys_get_object ("stderr");
        if (file != NULL)
        {
                pcl_file_write_string (file, "Exception ");
                if (exception != NULL)
                {
                        pcl_file_write_object (
                                file, exception, PCL_PRINT_FLAG_RAW);
                        if (value != NULL && value != PCL_NONE)
                        {
                                pcl_file_write_string (file, ": ");
                                pcl_file_write_object (file, value, 0);
                        }
                }
                pcl_file_write_string (file, " in ");
                pcl_file_write_object (file, object, 0);
                pcl_file_write_string (file, " ignored\n");
                pcl_error_clear ();  /* just in case */
        }
        if (exception != NULL)
                pcl_object_unref (exception);
        if (value != NULL)
                pcl_object_unref (value);
        if (traceback != NULL)
                pcl_object_unref (traceback);
}

/* pcl_error_check_signals() is implemented in pcl-mod-signal.c */
/* pcl_error_set_interrupt() is implemented in pcl-mod-signal.c */
/* pcl_os_interrupt_occurred() is implemented in pcl-mod-signal.c */
