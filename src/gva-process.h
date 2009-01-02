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

/**
 * SECTION: gva-process
 * @short_description: An abstraction of a UNIX process
 *
 * A #GvaProcess provides a convenient interface for communicating with a
 * child process through pipes.
 **/

#ifndef GVA_PROCESS_H
#define GVA_PROCESS_H

#include "gva-common.h"

/* Standard GObject macros */
#define GVA_TYPE_PROCESS \
        (gva_process_get_type ())
#define GVA_PROCESS(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), GVA_TYPE_PROCESS, GvaProcess))
#define GVA_PROCESS_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), GVA_TYPE_PROCESS, GvaProcessClass))
#define GVA_IS_PROCESS(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), GVA_TYPE_PROCESS))
#define GVA_IS_PROCESS_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), GVA_TYPE_PROCESS))
#define GVA_PROCESS_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), GVA_TYPE_PROCESS, GvaProcessClass))

G_BEGIN_DECLS

typedef struct _GvaProcess GvaProcess;
typedef struct _GvaProcessClass GvaProcessClass;
typedef struct _GvaProcessPrivate GvaProcessPrivate;

/**
 * GvaProcess:
 *
 * Contains only private data that should be read and manipulated using the
 * functions below.
 **/
struct _GvaProcess
{
        GObject parent;

        GError *error;
        GvaProcessPrivate *priv;
};

struct _GvaProcessClass
{
        GObjectClass parent_class;

        /* Methods */
        gchar *         (*stdout_read_line)     (GvaProcess *process);
        gchar *         (*stderr_read_line)     (GvaProcess *process);

        /* Signals */
        void            (*stdout_ready)         (GvaProcess *process);
        void            (*stderr_ready)         (GvaProcess *process);
        void            (*exited)               (GvaProcess *process,
                                                 gint status);
};

GType           gva_process_get_type            (void);
GvaProcess *    gva_process_new                 (GPid pid,
                                                 gint priority,
                                                 gint standard_input,
                                                 gint standard_output,
                                                 gint standard_error);
GvaProcess *    gva_process_spawn               (const gchar *command_line,
                                                 gint priority,
                                                 GError **error);
GPid            gva_process_get_pid             (GvaProcess *process);
gboolean        gva_process_write_stdin         (GvaProcess *process,
                                                 const gchar *data,
                                                 gssize length,
                                                 GError **error);
guint           gva_process_stdout_num_lines    (GvaProcess *process);
guint           gva_process_stderr_num_lines    (GvaProcess *process);
gchar *         gva_process_stdout_read_line    (GvaProcess *process);
gchar *         gva_process_stderr_read_line    (GvaProcess *process);
gchar **        gva_process_stdout_read_lines   (GvaProcess *process);
gchar **        gva_process_stderr_read_lines   (GvaProcess *process);
guint           gva_process_get_progress        (GvaProcess *process);
void            gva_process_inc_progress        (GvaProcess *process);
void            gva_process_set_progress        (GvaProcess *process,
                                                 guint progress);
gboolean        gva_process_has_exited          (GvaProcess *process,
                                                 gint *status);
void            gva_process_kill                (GvaProcess *process);
void            gva_process_kill_all            (void);
void            gva_process_get_time_elapsed    (GvaProcess *process,
                                                 GTimeVal *time_elapsed);

G_END_DECLS

#endif /* GVA_PROCESS_H */
