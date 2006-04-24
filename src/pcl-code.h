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

#ifndef PCL_CODE_H
#define PCL_CODE_H

#include "pcl-config.h"
#include "pcl-container.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclCode PclCode;
typedef struct _PclCodeClass PclCodeClass;

/**
 * PclCodeFlags:
 * @PCL_CODE_FLAG_NEWLOCALS:
 *      Code object defines its own local namespace (as opposed to sharing the
 *      global namespace).
 * @PCL_CODE_FLAG_GENERATOR:
 *      Code object is a generator expression or a function containing a
 *      @yield statement.
 * @PCL_CODE_FLAG_PREDICATE:
 *      Code object is a predicate function.
 * @PCL_CODE_FLAG_CONJUNCTION:
 *      Code object is a conjunction within a predicate function.
 * @PCL_CODE_FLAG_NOFREE:
 *      Code object contains no free variables in its own namespace or in any
 *      of its nested namespaces.
 * @PCL_CODE_FLAG_VARARGS:
 *      Code object accepts variable-length arguments.
 * @PCL_CODE_FLAG_VARKWDS:
 *      Code object accepts variable-length keyword arguments.
 *
 * Used to check or determine characteristics of a #PclCode object.
 */
typedef enum {
        PCL_CODE_FLAG_NEWLOCALS         = 1 << 0,
        PCL_CODE_FLAG_GENERATOR         = 1 << 1,
        PCL_CODE_FLAG_PREDICATE         = 1 << 2,
        PCL_CODE_FLAG_CONJUNCTION       = 1 << 3,
        PCL_CODE_FLAG_NOFREE            = 1 << 4,
        PCL_CODE_FLAG_VARARGS           = 1 << 5,
        PCL_CODE_FLAG_VARKWDS           = 1 << 6
} PclCodeFlags;

struct _PclCode {
        PclContainer parent;
        guint argument_count;      /* # of required arguments */
        guint variable_count;      /* # of arguments + locals */
        gulong stack_size;         /* # of entries req'd for value stack */
        PclObject *instructions;   /* string: instruction opcodes */
        PclObject *lineno_table;   /* string: address / lineno mapping */
        PclObject *constants;      /* tuple: constants used */
        PclObject *names;          /* tuple: variable names used */
        PclObject *varnames;       /* tuple: local variable names */
        PclObject *freevars;       /* tuple: free variable names */
        PclObject *cellvars;       /* tuple: cell variable names */
        PclObject *filename;       /* string: where it was loaded from */
        PclObject *name;           /* string: name, for reference */
        gint first_lineno;         /* first source line number */
        PclCodeFlags flags;
};

struct _PclCodeClass {
        PclContainerClass parent_class;
};

GType           pcl_code_get_type               (void);
PclObject *     pcl_code_get_type_object        (void);
PclCode *       pcl_code_new                    (guint argument_count,
                                                 guint variable_count,
                                                 gulong stack_size,
                                                 PclObject *instructions,
                                                 PclObject *lineno_table,
                                                 PclObject *constants,
                                                 PclObject *names,
                                                 PclObject *varnames,
                                                 PclObject *freevars,
                                                 PclObject *cellvars,
                                                 PclObject *filename,
                                                 PclObject *name,
                                                 gint first_lineno,
                                                 PclCodeFlags flags);
gint            pcl_code_addr_to_line           (PclCode *code,
                                                 gint addrq);

#define PCL_CODE_GET_NUM_FREE(obj) \
        (pcl_object_measure (PCL_CODE (obj)->freevars))

/* Standard GObject macros */
#define PCL_TYPE_CODE \
        (pcl_code_get_type ())
#define PCL_CODE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_CODE, PclCode))
#define PCL_CODE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_CODE, PclCodeClass))
#define PCL_IS_CODE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_CODE))
#define PCL_IS_CODE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_CODE))
#define PCL_CODE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_CODE, PclCodeClass))

G_END_DECLS

#endif /* PCL_CODE_H */
