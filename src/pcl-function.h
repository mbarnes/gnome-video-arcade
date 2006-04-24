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

#ifndef PCL_FUNCTION_H
#define PCL_FUNCTION_H

#include "pcl-config.h"
#include "pcl-code.h"
#include "pcl-container.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclFunction PclFunction;
typedef struct _PclFunctionClass PclFunctionClass;

struct _PclFunction {
        PclContainer parent;
        PclCode *code;
        PclObject *globals;
        PclObject *defaults;
        PclObject *closure;
        PclObject *doc;
        PclObject *name;
        PclObject *dict;
        PclObject *module;
};

struct _PclFunctionClass {
        PclContainerClass parent_class;
};

GType           pcl_function_get_type           (void);
PclObject *     pcl_function_get_type_object    (void);
PclObject *     pcl_function_new                (PclCode *code,
                                                 PclObject *globals);
PclCode *       pcl_function_get_code           (PclObject *self);
PclObject *     pcl_function_get_globals        (PclObject *self);
PclObject *     pcl_function_get_defaults       (PclObject *self);
gboolean        pcl_function_set_defaults       (PclObject *self,
                                                 PclObject *defaults);
PclObject *     pcl_function_get_closure        (PclObject *self);
gboolean        pcl_function_set_closure        (PclObject *self,
                                                 PclObject *closure);
PclObject *     pcl_function_get_module         (PclObject *self);

/* Standard GObject macros */
#define PCL_TYPE_FUNCTION \
        (pcl_function_get_type ())
#define PCL_FUNCTION(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_FUNCTION, PclFunction))
#define PCL_FUNCTION_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_FUNCTION, PclFunctionClass))
#define PCL_IS_FUNCTION(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_FUNCTION))
#define PCL_IS_FUNCTION_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_FUNCTION))
#define PCL_FUNCTION_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_FUNCTION, PclFunctionClass))

/* Fast access macros (use carefully) */
#define PCL_FUNCTION_GET_CODE(obj) \
        (PCL_FUNCTION (obj)->code)
#define PCL_FUNCTION_GET_GLOBALS(obj) \
        (PCL_FUNCTION (obj)->globals)
#define PCL_FUNCTION_GET_DEFAULTS(obj) \
        (PCL_FUNCTION (obj)->defaults)
#define PCL_FUNCTION_GET_CLOSURE(obj) \
        (PCL_FUNCTION (obj)->closure)
#define PCL_FUNCTION_GET_NAME(obj) \
        (PCL_FUNCTION (obj)->name)
#define PCL_FUNCTION_GET_MODULE(obj) \
        (PCL_FUNCTION (obj)->module)

G_END_DECLS

#endif /* PCL_FUNCTION_H */
