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

#ifndef PCL_STATE_H
#define PCL_STATE_H

#include "pcl-config.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclInterpreterState PclInterpreterState;
typedef struct _PclThreadState PclThreadState;

/* State shared between threads */

struct _PclInterpreterState {
        GSList *ts_list;
        PclObject *modules;
        PclObject *sysdict;
        PclObject *builtins;
};

/* State unique per thread */

struct _PclFrame;  /* Avoid including pcl-frame.h */

typedef enum {
        PCL_TRACE_CALL,
        PCL_TRACE_EXCEPTION,
        PCL_TRACE_LINE,
        PCL_TRACE_RETURN,
        PCL_TRACE_C_CALL,
        PCL_TRACE_C_EXCEPTION,
        PCL_TRACE_C_LINE
} PclTraceType;

typedef gboolean (*PclTraceFunc) (PclObject *trace_object,
                                  struct _PclFrame *frame,
                                  PclTraceType type,
                                  PclObject *argument);

struct _PclThreadState {
        PclInterpreterState *is;
        struct _PclFrame *frame;
        gint recursion_depth;
        gint tracing;
        gboolean use_tracing;

        PclTraceFunc profile_function;
        PclTraceFunc trace_function;
        PclObject *profile_object;
        PclObject *trace_object;

        PclObject *exc_type;
        PclObject *exc_value;
        PclObject *exc_traceback;

        PclObject *current_exc_type;
        PclObject *current_exc_value;
        PclObject *current_exc_traceback;

        PclObject *dict;

        GHashTable *cache;
        GThread *thread;
};

PclInterpreterState *pcl_interpreter_state_new (void);
void pcl_interpreter_state_clear (PclInterpreterState *is);
void pcl_interpreter_state_destroy (PclInterpreterState *is);

PclThreadState *pcl_thread_state_new (PclInterpreterState *is);
PclThreadState *pcl_thread_state_swap (PclThreadState *ts);
void pcl_thread_state_clear (PclThreadState *ts);
void pcl_thread_state_destroy (PclThreadState *ts);

PclThreadState *pcl_thread_state_get (void);
PclObject *pcl_thread_state_get_dict (void);
struct _PclFrame *pcl_thread_state_get_frame (PclThreadState *ts);

G_END_DECLS

#endif /* PCL_STATE_H */
