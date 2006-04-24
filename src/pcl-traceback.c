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

#define TRACEBACK_FORMAT "  File \"%s\", line %d, in %s\n"
#define TRACEBACK_HEADER "Traceback (most recent call last):\n"

static gpointer traceback_parent_class = NULL;

static gboolean
traceback_display_line (PclObject *file, const gchar *filename,
                        gint lineno, const gchar *name)
{
        FILE *stream;
        gboolean success = FALSE;
        gchar *line;

        if (filename == NULL || name == NULL)
                return FALSE;

        line = g_strdup_printf (TRACEBACK_FORMAT, filename, lineno, name);
        success = pcl_file_write_string (file, line);
        g_free (line);

        /* Note, we may return TRUE even if we can't open a stream. */
        stream = pcl_open_source (filename);
        if (success && stream != NULL)
        {
                line = pcl_error_program_text (stream, lineno);
                if (line == NULL)
                        line = g_strdup ("<error extracting line>");
                success = pcl_file_write_string (file, "    ") &&
                        pcl_file_write_string (file, line) &&
                        pcl_file_write_string (file, "\n");
                g_free (line);
                fclose (stream);
        }

        return success;
}

PclObject *
traceback_new (PclObject *next, PclFrame *frame)
{
        PclTraceback *traceback;

        if ((next != NULL && !PCL_IS_TRACEBACK (next)) ||
                !PCL_IS_FRAME (frame))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        traceback = pcl_object_new (PCL_TYPE_TRACEBACK, NULL);
        if (next != NULL)
                pcl_object_ref (next);
        traceback->next = PCL_TRACEBACK (next);
        traceback->frame = pcl_object_ref (frame);
        traceback->lasti = frame->lasti;
        traceback->lineno = pcl_code_addr_to_line (frame->code, frame->lasti);
        return PCL_OBJECT (traceback);
}

static void
traceback_dispose (GObject *g_object)
{
        PclTraceback *self = PCL_TRACEBACK (g_object);

        PCL_CLEAR (self->next);
        PCL_CLEAR (self->frame);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (traceback_parent_class)->dispose (g_object);
}

static gboolean
traceback_traverse (PclContainer *container, PclTraverseFunc func,
                    gpointer user_data)
{
        PclTraceback *self = PCL_TRACEBACK (container);

        if (self->next != NULL)
                if (!func (self->next, user_data))
                        return FALSE;
        if (self->frame != NULL)
                if (!func (self->frame, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (traceback_parent_class)->
                traverse (container, func, user_data);
}

static void
traceback_class_init (PclTracebackClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        traceback_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = traceback_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_traceback_get_type_object;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = traceback_dispose;
}

GType
pcl_traceback_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclTracebackClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) traceback_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclTraceback),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclTraceback", &type_info, 0);
        }
        return type;
}

/**
 * pcl_traceback_get_type_object:
 *
 * Returns the type object for #PclTraceback.
 *
 * Returns: a borrowed reference to the type object for #PclTraceback
 */
PclObject *
pcl_traceback_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_TRACEBACK, "traceback");
                pcl_register_singleton ("<type 'traceback'>", &object);
        }
        return object;
}

gboolean
pcl_traceback_here (PclFrame *frame)
{
        PclThreadState *ts = frame->ts;
        PclObject *old_tb, *new_tb;
        old_tb = ts->current_exc_traceback;
        new_tb = traceback_new (old_tb, frame);
        if (new_tb == NULL)
                return FALSE;
        ts->current_exc_traceback = new_tb;
        if (old_tb != NULL)
                pcl_object_unref (old_tb);
        return TRUE;
}

gboolean
pcl_traceback_print (PclObject *self, PclObject *file)
{
        PclTraceback *traceback;
        PclTraceback *tb_iter;
        PclObject *object;
        gboolean success = TRUE;
        glong depth = 0;
        glong depth_limit = 1000;

        if (self == NULL)
                return TRUE;
        if (!PCL_IS_TRACEBACK (self))
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        traceback = tb_iter = PCL_TRACEBACK (self);

        while (tb_iter != NULL)
        {
                depth++;
                tb_iter = tb_iter->next;
        }

        object = pcl_sys_get_object ("tracebacklimit");
        if (PCL_IS_INT (object))
        {
                depth_limit = pcl_int_as_long (object);
                if (depth_limit <= 0)
                        return TRUE;
        }

        if (!pcl_file_write_string (file, TRACEBACK_HEADER))
                return FALSE;

        while (traceback != NULL && success)
        {
                if (depth <= depth_limit)
                {
                        gchar *filename;

                        filename = pcl_string_as_string (
                                        traceback->frame->code->filename);
                        success = traceback_display_line (file, filename,
                                  traceback->lineno, pcl_string_as_string (
                                  traceback->frame->code->name));
                }
                depth--;
                traceback = traceback->next;
                if (success)
                        success = pcl_error_check_signals ();
        }
        return success;
}
