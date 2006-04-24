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

#ifndef PCL_METHOD_H
#define PCL_METHOD_H

#include "pcl-config.h"
#include "pcl-container.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef PclObject *(*PclCFunction)
        (PclObject *, PclObject *);
typedef PclObject *(*PclCFunctionWithKeywords)
        (PclObject *, PclObject *, PclObject *);

typedef struct _PclMethod PclMethod;
typedef struct _PclMethodClass PclMethodClass;
typedef struct _PclMethodDef PclMethodDef;

/**
 * PclMethodFlags:
 * @PCL_METHOD_FLAG_VARARGS:
 *      Accept variable-length arguments.
 * @PCL_METHOD_FLAG_KEYWORDS:
 *      Accept keyword arguments.
 * @PCL_METHOD_FLAG_ONEARG:
 *      Accept exactly one argument.
 * @PCL_METHOD_FLAG_NOARGS:
 *      Do not accept any arguments.
 * @PCL_METHOD_FLAG_CLASS:
 *      This is a class method.
 *
 * Used to check or determine characteristics of a #PclMethodDef struct.
 */
typedef enum {
        PCL_METHOD_FLAG_VARARGS         = 1 << 0,
        PCL_METHOD_FLAG_KEYWORDS        = 1 << 1,
        PCL_METHOD_FLAG_ONEARG          = 1 << 2,
        PCL_METHOD_FLAG_NOARGS          = 1 << 3,
        PCL_METHOD_FLAG_CLASS           = 1 << 4
} PclMethodFlags;

struct _PclMethod {
        PclContainer parent;
        PclMethodDef *def;
        PclObject *self;
        PclObject *module;
};

struct _PclMethodClass {
        PclContainerClass parent_class;
};

struct _PclMethodDef {
        const gchar *name;
        PclCFunction func;
        PclMethodFlags flags;
        const gchar *doc;
        glong offset;
};

GType           pcl_method_get_type             (void);
PclObject *     pcl_method_get_type_object      (void);
PclObject *     pcl_method_new                  (PclMethodDef *def,
                                                 PclObject *self,
                                                 PclObject *module);

/* Standard GObject macros */
#define PCL_TYPE_METHOD \
        (pcl_method_get_type ())
#define PCL_METHOD(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_METHOD, PclMethod))
#define PCL_METHOD_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_METHOD, PclMethodClass))
#define PCL_IS_METHOD(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_METHOD))
#define PCL_IS_METHOD_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_METHOD))
#define PCL_METHOD_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_METHOD, PclMethodClass))

/* Fast access macros (use carefully) */
#define PCL_METHOD_GET_FLAGS(obj) \
        (PCL_METHOD (obj)->def->flags)
#define PCL_METHOD_GET_FUNCTION(obj) \
        (PCL_METHOD (obj)->def->func)
#define PCL_METHOD_GET_NAME(obj) \
        (PCL_METHOD (obj)->def->name)
#define PCL_METHOD_GET_SELF(obj) \
        (PCL_METHOD (obj)->self)

G_END_DECLS

#endif /* PCL_METHOD_H */
