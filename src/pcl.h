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

#ifndef PCL_H
#define PCL_H

#include "pcl-config.h"

/* Interfaces */
#include "pcl-mapping.h"
#include "pcl-number.h"
#include "pcl-sequence.h"

/* Abstract Types */
#include "pcl-container.h"
#include "pcl-descriptor.h"
#include "pcl-iterator.h"

/* Standard Types */
#include "pcl-attr-getter.h"
#include "pcl-bool.h"
#include "pcl-callable-iterator.h"
#include "pcl-class-method.h"
#include "pcl-complex.h"
#include "pcl-dict.h"
#include "pcl-ellipsis.h"
#include "pcl-enumerate.h"
#include "pcl-exception.h"
#include "pcl-file.h"
#include "pcl-float.h"
#include "pcl-free.h"
#include "pcl-generator.h"
#include "pcl-instance-method.h"
#include "pcl-int.h"
#include "pcl-item-getter.h"
#include "pcl-list.h"
#include "pcl-long.h"
#include "pcl-method.h"
#include "pcl-module.h"
#include "pcl-none.h"
#include "pcl-not-implemented.h"
#include "pcl-object.h"
#include "pcl-property.h"
#include "pcl-range.h"
#include "pcl-reversed.h"
#include "pcl-sequence-iterator.h"
#include "pcl-set.h"
#include "pcl-static-method.h"
#include "pcl-string.h"
#include "pcl-super.h"
#include "pcl-table.h"
#include "pcl-tablerecord.h"
#include "pcl-traceback.h"
#include "pcl-tuple.h"
#include "pcl-type.h"
#include "pcl-zip.h"

/* Extending & Embedding API */
#include "pcl-arg.h"
#include "pcl-cell.h"
#include "pcl-code.h"
#include "pcl-compiler.h"
#include "pcl-dict-proxy.h"
#include "pcl-error.h"
#include "pcl-eval.h"
#include "pcl-getset-descriptor.h"
#include "pcl-factory.h"
#include "pcl-frame.h"
#include "pcl-function.h"
#include "pcl-import.h"
#include "pcl-main.h"
#include "pcl-member-descriptor.h"
#include "pcl-member.h"
#include "pcl-method-descriptor.h"
#include "pcl-parse-info.h"
#include "pcl-run.h"
#include "pcl-slice.h"
#include "pcl-state.h"
#include "pcl-util.h"
#include "pcl-weak-ref.h"

/* Built-in Modules */
#include "pcl-mod-gc.h"
#include "pcl-mod-sys.h"

G_BEGIN_DECLS

void            pcl_initialize                  (void);
void            pcl_finalize                    (void);
void            pcl_exit                        (gint status);
gboolean        pcl_at_exit                     (GVoidFunc func);

PclParseInfo *  pcl_parse_input_file            (FILE *stream,
                                                 const gchar *filename,
                                                 gint start);
PclParseInfo *  pcl_parse_input_file_with_prompts (
                                                 FILE *stream,
                                                 const gchar *filename,
                                                 gint start,
                                                 const gchar *ps1,
                                                 const gchar *ps2);
PclParseInfo *  pcl_parse_input_string          (const gchar *string,
                                                 gint start);

void            pcl_register_singleton          (const gchar *name,
                                                 gpointer *p_object);

/* XXX These definitions at one time enabled some debugging features that
 *     have since been ripped out.  Now they just alias their GObject
 *     counterpart.  At this point it would be a major undertaking to
 *     remove these definitions entirely. */
#define pcl_object_new          g_object_new
#define pcl_object_ref          g_object_ref
#define pcl_object_unref        g_object_unref

#define PCL_CLEAR(obj) \
        G_STMT_START { \
                if ((obj) != NULL) \
                { \
                        gpointer tmp = (obj); \
                        (obj) = NULL; \
                        pcl_object_unref (tmp); \
                } \
        } G_STMT_END

/* Parser Start States */
extern const gint PCL_EVAL_INPUT;
extern const gint PCL_FILE_INPUT;
extern const gint PCL_SINGLE_INPUT;

/* Macros for returning a singleton object from a function. */
#define PCL_RETURN_NONE         return pcl_object_ref (PCL_NONE)
#define PCL_RETURN_TRUE         return pcl_object_ref (PCL_TRUE)
#define PCL_RETURN_FALSE        return pcl_object_ref (PCL_FALSE)

G_END_DECLS

#endif /* PCL_H */
