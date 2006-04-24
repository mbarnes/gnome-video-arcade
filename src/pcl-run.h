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

#ifndef PCL_RUN_H
#define PCL_RUN_H

#include "pcl-config.h"

G_BEGIN_DECLS

gint            pcl_run_any_file                (FILE *stream,
                                                 const gchar *filename);
gint            pcl_run_any_file_flags          (FILE *stream,
                                                 const gchar *filename,
                                                 gpointer flags);
gint            pcl_run_any_file_ex             (FILE *stream,
                                                 const gchar *filename,
                                                 gboolean closeit);
gint            pcl_run_any_file_ex_flags       (FILE *stream,
                                                 const gchar *filename,
                                                 gboolean closeit,
                                                 gpointer flags);
PclObject *     pcl_run_file                    (FILE *stream,
                                                 const gchar *filename,
                                                 gint start,
                                                 PclObject *globals,
                                                 PclObject *locals);
PclObject *     pcl_run_file_flags              (FILE *stream,
                                                 const gchar *filename,
                                                 gint start,
                                                 PclObject *globals,
                                                 PclObject *locals,
                                                 gpointer flags);
PclObject *     pcl_run_file_ex                 (FILE *stream,
                                                 const gchar *filename,
                                                 gint start,
                                                 PclObject *globals,
                                                 PclObject *locals,
                                                 gboolean closeit);
PclObject *     pcl_run_file_ex_flags           (FILE *stream,
                                                 const gchar *filename,
                                                 gint start,
                                                 PclObject *globals,
                                                 PclObject *locals,
                                                 gboolean closeit,
                                                 gpointer flags);
gint            pcl_run_interactive_loop        (FILE *stream,
                                                 const gchar *filename);
gint            pcl_run_interactive_loop_flags  (FILE *stream,
                                                 const gchar *filename,
                                                 gpointer flags);
gint            pcl_run_interactive_one         (FILE *stream,
                                                 const gchar *filename);
gint            pcl_run_interactive_one_flags   (FILE *stream,
                                                 const gchar *filename,
                                                 gpointer flags);
gint            pcl_run_simple_file             (FILE *stream,
                                                 const gchar *filename);
gint            pcl_run_simple_file_flags       (FILE *stream,
                                                 const gchar *filename,
                                                 gpointer flags);
gint            pcl_run_simple_file_ex          (FILE *stream,
                                                 const gchar *filename,
                                                 gboolean closeit);
gint            pcl_run_simple_file_ex_flags    (FILE *stream,
                                                 const gchar *filename,
                                                 gboolean closeit,
                                                 gpointer flags);
gint            pcl_run_simple_string           (const gchar *command);
gint            pcl_run_simple_string_flags     (const gchar *command,
                                                 gpointer flags);
PclObject *     pcl_run_string                  (const gchar *string,
                                                 gint start,
                                                 PclObject *globals,
                                                 PclObject *locals);
PclObject *     pcl_run_string_flags            (const gchar *string,
                                                 gint start,
                                                 PclObject *globals,
                                                 PclObject *locals,
                                                 gpointer flags);

/* Parser Error Codes */
#define PCL_ERROR_EOF           (1 << 1)        /* End of file */

G_END_DECLS

#endif /* PCL_RUN_H */
