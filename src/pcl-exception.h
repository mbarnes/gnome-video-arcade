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

#ifndef PCL_EXCEPTION_H
#define PCL_EXCEPTION_H

#include "pcl-config.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclException PclException;
typedef struct _PclExceptionClass PclExceptionClass;

struct _PclException {
        PclObject parent;
        gchar *name;
};

struct _PclExceptionClass {
        PclObjectClass parent_class;
};

GType           pcl_exception_get_type          (void);
PclObject *     pcl_exception_get_type_object   (void);
const gchar *   pcl_exception_get_name          (PclObject *self);

/* Standard Exceptions */
PclObject *     pcl_exception_assertion_error           (void);
PclObject *     pcl_exception_attribute_error           (void);
PclObject *     pcl_exception_eof_error                 (void);
PclObject *     pcl_exception_floating_point_error      (void);
PclObject *     pcl_exception_io_error                  (void);
PclObject *     pcl_exception_import_error              (void);
PclObject *     pcl_exception_index_error               (void);
PclObject *     pcl_exception_key_error                 (void);
PclObject *     pcl_exception_keyboard_interrupt        (void);
PclObject *     pcl_exception_memory_error              (void);
PclObject *     pcl_exception_name_error                (void);
PclObject *     pcl_exception_not_implemented_error     (void);
PclObject *     pcl_exception_os_error                  (void);
PclObject *     pcl_exception_overflow_error            (void);
PclObject *     pcl_exception_reference_error           (void);
PclObject *     pcl_exception_runtime_error             (void);
PclObject *     pcl_exception_stop_iteration            (void);
PclObject *     pcl_exception_syntax_error              (void);
PclObject *     pcl_exception_system_error              (void);
PclObject *     pcl_exception_system_exit               (void);
PclObject *     pcl_exception_type_error                (void);
PclObject *     pcl_exception_unbound_local_error       (void);
PclObject *     pcl_exception_value_error               (void);
PclObject *     pcl_exception_zero_division_error       (void);

/* Standard GObject macros */
#define PCL_TYPE_EXCEPTION \
        (pcl_exception_get_type ())
#define PCL_EXCEPTION(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_EXCEPTION, PclException))
#define PCL_EXCEPTION_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_EXCEPTION, PclExceptionClass))
#define PCL_IS_EXCEPTION(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_EXCEPTION))
#define PCL_IS_EXCEPTION_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_EXCEPTION))
#define PCL_EXCEPTION_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_EXCEPTION, PclExceptionClass))

G_END_DECLS

#endif /* PCL_EXCEPTION_H */
