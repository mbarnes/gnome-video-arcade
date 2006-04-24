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

#ifndef PCL_FRAME_H
#define PCL_FRAME_H

#include "pcl-config.h"
#include "pcl-container.h"
#include "pcl-code.h"
#include "pcl-object.h"
#include "pcl-state.h"

G_BEGIN_DECLS

typedef struct _PclTryBlock PclTryBlock;
typedef struct _PclFrame PclFrame;
typedef struct _PclFrameClass PclFrameClass;

struct _PclTryBlock {
        guint type;                /* what kind of block this is */
        guint handler;             /* where to jump to find handler */
        guint level;               /* value stack level to pop to */
};

struct _PclFrame {
        PclContainer parent;
        PclFrame *previous;        /* previous frame or NULL */
        PclCode *code;             /* code object for this frame */
        PclObject *builtins;       /* symbol table: name -> object */
        PclObject *globals;        /* symbol table: name -> object */
        PclObject *locals;         /* symbol table: name -> object */
        PclObject **slots;         /* slots for local and free variables */
        PclObject **stack;         /* value stack (see code->stack_size) */
        PclObject **tos;           /* top of value stack */
        PclObject *exc_type;       /* temporary exception storage */
        PclObject *exc_value;      /* temporary exception storage */
        PclObject *exc_traceback;  /* temporary exception storage */
	PclObject *result;         /* predicate result list */
        PclThreadState *ts;
        guint cell_count;          /* length of code->cellvars */
        guint free_count;          /* length of code->freevars */
        guint slot_count;          /* length of slots array */
        guint stack_size;          /* length of stack array */
        gint lasti;                /* last instruction if called */
        PclTryBlock block[PCL_MAX_BLOCKS];
        gint block_count;
        gboolean cut;
};

struct _PclFrameClass {
        PclContainerClass parent_class;
        void (*block_setup) (PclFrame *frame, guint type,
                             guint handler, guint level);
        PclTryBlock *(*block_pop) (PclFrame *frame);
        void (*fast_to_locals) (PclFrame *frame);
        void (*locals_to_fast) (PclFrame *frame, gboolean clear);
};

GType           pcl_frame_get_type              (void);
PclObject *     pcl_frame_get_type_object       (void);
PclFrame *      pcl_frame_new                   (PclThreadState *ts,
                                                 PclCode *code,
                                                 PclObject *globals,
                                                 PclObject *locals);
void            pcl_frame_block_setup           (PclFrame *frame,
                                                 guint type,
                                                 guint handler,
                                                 guint level);
PclTryBlock *   pcl_frame_block_pop             (PclFrame *frame);
void            pcl_frame_fast_to_locals        (PclFrame *frame);
void            pcl_frame_locals_to_fast        (PclFrame *frame,
                                                 gboolean clear);

/* Standard GObject macros */
#define PCL_TYPE_FRAME \
        (pcl_frame_get_type ())
#define PCL_FRAME(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_FRAME, PclFrame))
#define PCL_FRAME_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_FRAME, PclFrameClass))
#define PCL_IS_FRAME(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_FRAME))
#define PCL_IS_FRAME_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_FRAME))
#define PCL_FRAME_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_FRAME, PclFrameClass))

G_END_DECLS

#endif /* PCL_FRAME_H */
