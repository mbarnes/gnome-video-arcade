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

#ifndef PCL_TRACEBACK_H
#define PCL_TRACEBACK_H

#include "pcl-config.h"
#include "pcl-container.h"
#include "pcl-frame.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclTraceback PclTraceback;
typedef struct _PclTracebackClass PclTracebackClass;

struct _PclTraceback {
        PclContainer parent;
        PclTraceback *next;
        PclFrame *frame;
        gint lasti;
        gint lineno;
};

struct _PclTracebackClass {
        PclContainerClass parent_class;
};

GType           pcl_traceback_get_type          (void);
PclObject *     pcl_traceback_get_type_object   (void);
gboolean        pcl_traceback_here              (PclFrame *frame);
gboolean        pcl_traceback_print             (PclObject *self,
                                                 PclObject *file);

/* Standard GObject macros */
#define PCL_TYPE_TRACEBACK \
        (pcl_traceback_get_type ())
#define PCL_TRACEBACK(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_TRACEBACK, PclTraceback))
#define PCL_TRACEBACK_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_TRACEBACK, PclTracebackClass))
#define PCL_IS_TRACEBACK(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_TRACEBACK))
#define PCL_IS_TRACEBACK_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_TRACEBACK))
#define PCL_TRACEBACK_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_TRACEBACK, PclTracebackClass))

G_END_DECLS

#endif /* PCL_TRACEBACK_H */
