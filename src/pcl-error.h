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

#ifndef PCL_ERROR_H
#define PCL_ERROR_H

#include "pcl-config.h"

G_BEGIN_DECLS

PclObject *     pcl_error_occurred              (void);
void            pcl_error_fetch                 (PclObject **p_exception,
                                                 PclObject **p_value,
                                                 PclObject **p_traceback);
void            pcl_error_display               (PclObject *exception,
                                                 PclObject *value,
                                                 PclObject *traceback);
void            pcl_error_restore               (PclObject *exception,
                                                 PclObject *value,
                                                 PclObject *traceback);
void            pcl_error_normalize             (PclObject **p_exception,
                                                 PclObject **p_value,
                                                 PclObject **p_traceback);
void            pcl_error_clear                 (void);
void            pcl_error_print                 (void);
void            pcl_error_set_none              (PclObject *exception);
void            pcl_error_set_object            (PclObject *exception,
                                                 PclObject *value);
void            pcl_error_set_string            (PclObject *exception,
                                                 const gchar *string);
void            pcl_error_set_format            (PclObject *exception,
                                                 const gchar *format, ...);
void            pcl_error_set_from_errno        (PclObject *exception);
gboolean        pcl_error_exception_matches     (PclObject *exception);
gboolean        pcl_error_given_exception_matches (PclObject *error,
                                                 PclObject *exception);
gchar *         pcl_error_program_text          (FILE *stream,
                                                 gint lineno);
void            pcl_error_write_unraisable      (PclObject *object);

gboolean        pcl_error_check_signals         (void);
void            pcl_error_set_interrupt         (void);
gboolean        pcl_os_interrupt_occurred       (void);

#define pcl_error_bad_argument() \
        pcl_error_set_string (pcl_exception_type_error (), \
        "bad argument type for built-in operation")

#define pcl_error_bad_internal_call() \
        pcl_error_set_string (pcl_exception_system_error (), \
        G_STRLOC ": bad argument to internal function")

G_END_DECLS

#endif /* PCL_ERROR_H */
