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
#include "pcl-node.h"

static PclObject *
run_parse_tree (PclParseInfo *parse_info, const gchar *filename,
                PclObject *globals, PclObject *locals, gpointer flags)
{
        PclCode *code;
        PclObject *result;
        GNode *parse_tree;

        if (parse_info == NULL)
                return NULL;
        parse_tree = parse_info->parse_tree;
        if (PCL_OPTION_DEBUG)
                pcl_parse_info_display (parse_info);
        if (PCL_OPTION_PARSE_ONLY)
        {
                pcl_parse_info_destroy (parse_info);
                return pcl_object_ref (PCL_NONE);
        }
        code = pcl_compile (parse_tree, filename, NULL);
        pcl_parse_info_destroy (parse_info);
        if (code == NULL)
                return NULL;
        result = pcl_eval_code (code, globals, locals);
        pcl_object_unref (code);
        return result;
}

gint
pcl_run_any_file (FILE *stream, const gchar *filename)
{
        return pcl_run_any_file_ex_flags (stream, filename, FALSE, NULL);
}

gint
pcl_run_any_file_flags (FILE *stream, const gchar *filename, gpointer flags)
{
        return pcl_run_any_file_ex_flags (stream, filename, FALSE, flags);
}

gint
pcl_run_any_file_ex (FILE *stream, const gchar *filename, gboolean closeit)
{
        return pcl_run_any_file_ex_flags (stream, filename, closeit, NULL);
}

gint
pcl_run_any_file_ex_flags (FILE *stream, const gchar *filename,
                           gboolean closeit, gpointer flags)
{
        gint status;
        if (filename == NULL)
                filename = "???";
        if (isatty (fileno (stream)))
        {
                status = pcl_run_interactive_loop_flags (
                         stream, filename, flags);
                if (closeit)
                        fclose (stream);
        }
        else
                status = pcl_run_simple_file_ex_flags (
                         stream, filename, closeit, flags);
        return status;
}

PclObject *
pcl_run_file (FILE *stream, const gchar *filename, gint start,
              PclObject *globals, PclObject *locals)
{
        return pcl_run_file_ex_flags (stream, filename, start,
                                      globals, locals, FALSE, NULL);
}

PclObject *
pcl_run_file_flags (FILE *stream, const gchar *filename, gint start,
                    PclObject *globals, PclObject *locals, gpointer flags)
{
        return pcl_run_file_ex_flags (stream, filename, start,
                                      globals, locals, FALSE, flags);
}

PclObject *
pcl_run_file_ex (FILE *stream, const gchar *filename, gint start,
                 PclObject *globals, PclObject *locals, gboolean closeit)
{
        return pcl_run_file_ex_flags (stream, filename, start,
                                      globals, locals, closeit, NULL);
}

PclObject *
pcl_run_file_ex_flags (FILE *stream, const gchar *filename, gint start,
                       PclObject *globals, PclObject *locals,
                       gboolean closeit, gpointer flags)
{
        PclParseInfo *parse_info;
        parse_info = pcl_parse_input_file (stream, filename, start);
        if (closeit)
                fclose (stream);
        return run_parse_tree (parse_info, filename, globals, locals, flags);
}

gint
pcl_run_interactive_loop (FILE *stream, const gchar *filename)
{
        return pcl_run_interactive_loop_flags (stream, filename, NULL);
}

gint
pcl_run_interactive_loop_flags (FILE *stream, const gchar *filename,
                                gpointer flags)
{
        PclObject *string;
        gint error_code = 0;

        /* XXX if (flags == NULL)
         * XXX         flags = &local_flags;
         */
        if (pcl_sys_get_object ("ps1") == NULL)
        {
                string = pcl_string_from_string (">>> ");
                pcl_sys_set_object ("ps1", string);
                pcl_object_unref (string);
        }

        if (pcl_sys_get_object ("ps2") == NULL)
        {
                string = pcl_string_from_string ("... ");
                pcl_sys_set_object ("ps2", string);
                pcl_object_unref (string);
        }

        while (error_code != PCL_ERROR_EOF)
                error_code = pcl_run_interactive_one_flags (
                                stream, filename, flags);

        return 0;
}

gint
pcl_run_interactive_one (FILE *stream, const gchar *filename)
{
        return pcl_run_interactive_one_flags (stream, filename, NULL);
}

gint
pcl_run_interactive_one_flags (FILE *stream, const gchar *filename,
                               gpointer flags)
{
        PclParseInfo *parse_info;
        PclObject *globals;
        PclObject *locals;
        PclObject *module;
        PclObject *object;
        PclObject *result;
        gchar *ps1 = "";
        gchar *ps2 = "";

        object = pcl_sys_get_object ("ps1");
        if (object != NULL)
        {
                object = pcl_object_str (object);
                if (object != NULL)
                {
                        if (PCL_IS_STRING (object))
                                ps1 = pcl_string_as_string (object);
                        pcl_object_unref (object);
                }
                else
                        pcl_error_clear ();
        }

        object = pcl_sys_get_object ("ps2");
        if (object != NULL)
        {
                object = pcl_object_str (object);
                if (object != NULL)
                {
                        if (PCL_IS_STRING (object))
                                ps2 = pcl_string_as_string (object);
                        pcl_object_unref (object);
                }
                else
                        pcl_error_clear ();
        }

        parse_info = pcl_parse_input_file_with_prompts (
                        stream, filename, PCL_SINGLE_INPUT, ps1, ps2);
        if (parse_info == NULL)
        {
                pcl_error_print ();
                return -1;
        }
        if (parse_info->error_code != 0)
        {
                gint error_code;
                error_code = parse_info->error_code;
                pcl_parse_info_destroy (parse_info);
                return error_code;
        }
        module = pcl_import_add_module ("__main__");
        if (module == NULL)
                return -1;
        locals = globals = pcl_module_get_dict (module);
        result = run_parse_tree (parse_info, filename, globals, locals, flags);
        if (result == NULL)
        {
                pcl_error_print ();
                return -1;
        }
        pcl_object_unref (result);
        if (!pcl_flush_line ())
                pcl_error_clear ();
        return 0;
}

gint
pcl_run_simple_file (FILE *stream, const gchar *filename)
{
        return pcl_run_simple_file_ex_flags (stream, filename, FALSE, NULL);
}

gint
pcl_run_simple_file_flags (FILE *stream, const gchar *filename, gpointer flags)
{
        return pcl_run_simple_file_ex_flags (stream, filename, FALSE, flags);
}

gint
pcl_run_simple_file_ex (FILE *stream, const gchar *filename, gboolean closeit)
{
        return pcl_run_simple_file_ex_flags (stream, filename, closeit, NULL);
}

gint
pcl_run_simple_file_ex_flags (FILE *stream, const gchar *filename,
                              gboolean closeit, gpointer flags)
{
        PclObject *globals;
        PclObject *locals;
        PclObject *module;
        PclObject *result;

        if ((module = pcl_import_add_module ("__main__")) == NULL)
                return -1;
        locals = globals = pcl_module_get_dict (module);
        if (pcl_dict_get_item_string (globals, "__file__") == NULL)
        {
                PclObject *file;
                if ((file = pcl_string_from_string (filename)) == NULL)
                        return -1;
                if (!pcl_dict_set_item_string (globals, "__file__", file))
                {
                        pcl_object_unref (file);
                        return -1;
                }
                pcl_object_unref (file);
        }
        result = pcl_run_file_ex_flags (stream, filename, PCL_FILE_INPUT,
                                        globals, locals, closeit, flags);
        if (result == NULL)
        {
                pcl_error_print ();
                return -1;
        }
        pcl_object_unref (result);
        if (!pcl_flush_line ())
                pcl_error_clear ();
        return 0;
}

gint
pcl_run_simple_string (const gchar *command)
{
        return pcl_run_simple_string_flags (command, NULL);
}

gint
pcl_run_simple_string_flags (const gchar *command, gpointer flags)
{
        PclObject *globals;
        PclObject *locals;
        PclObject *module;
        PclObject *result;

        if ((module = pcl_import_add_module ("__main__")) == NULL)
                return -1;
        locals = globals = pcl_module_get_dict (module);
        result = pcl_run_string_flags (command, PCL_FILE_INPUT,
                                       globals, locals, flags);
        if (result == NULL)
        {
                pcl_error_print ();
                return -1;
        }
        pcl_object_unref (result);
        if (!pcl_flush_line ())
                pcl_error_clear ();
        return 0;
}

PclObject *
pcl_run_string (const gchar *string, gint start,
                PclObject *globals, PclObject *locals)
{
        PclParseInfo *parse_info;
        parse_info = pcl_parse_input_string (string, start);
        return run_parse_tree (parse_info, "<string>", globals, locals, NULL);
}

PclObject *
pcl_run_string_flags (const gchar *string, gint start,
                      PclObject *globals, PclObject *locals, gpointer flags)
{
        PclParseInfo *parse_info;
        parse_info = pcl_parse_input_string (string, start);
        return run_parse_tree (parse_info, "<string>", globals, locals, flags);
}
