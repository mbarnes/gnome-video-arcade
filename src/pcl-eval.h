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

#ifndef PCL_EVAL_H
#define PCL_EVAL_H

#include "pcl-config.h"
#include "pcl-code.h"
#include "pcl-frame.h"
#include "pcl-function.h"
#include "pcl-object.h"

G_BEGIN_DECLS

gboolean        pcl_add_pending_call            (gboolean (*func) (gpointer),
                                                 gpointer arg);
gboolean        pcl_make_pending_calls          (void);
gint            pcl_get_recursion_limit         (void);
void            pcl_set_recursion_limit         (gint limit);
gboolean        pcl_enter_recursive_call        (void);
void            pcl_leave_recursive_call        (void);

PclObject *     pcl_eval_frame                  (PclFrame *frame);
PclObject *     pcl_eval_code                   (PclCode *code,
                                                 PclObject *globals,
                                                 PclObject *locals);
PclObject *     pcl_eval_code_ex                (PclCode *code,
                                                 PclObject *globals,
                                                 PclObject *locals,
                                                 PclObject **arguments,
                                                 glong n_arguments_given,
                                                 PclObject **keywords,
                                                 glong n_keywords_given,
                                                 PclObject **defaults,
                                                 glong n_defaults_given,
                                                 PclObject *closure);

PclFrame *      pcl_eval_get_frame              (void);
PclObject *     pcl_eval_get_builtins           (void);
PclObject *     pcl_eval_get_globals            (void);
PclObject *     pcl_eval_get_locals             (void);
PclObject *     pcl_eval_call_object_with_keywords
                                                (PclObject *object,
                                                 PclObject *arguments,
                                                 PclObject *keywords);
PclObject *     pcl_eval_call_function          (PclFunction *function,
                                                 const gchar *format,
                                                 ...);
const gchar *   pcl_eval_get_func_name          (PclObject *func);
const gchar *   pcl_eval_get_func_desc          (PclObject *func);
void            pcl_eval_set_profile            (PclTraceFunc function,
                                                 PclObject *object);
void            pcl_eval_set_trace              (PclTraceFunc function,
                                                 PclObject *object);

#define pcl_eval_call_object(object, arguments) \
        pcl_eval_call_object_with_keywords (object, arguments, NULL);

G_END_DECLS

#endif /* PCL_EVAL_H */
