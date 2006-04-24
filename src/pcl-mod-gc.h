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

#ifndef PCL_MOD_GC_H
#define PCL_MOD_GC_H

#include "pcl-config.h"

G_BEGIN_DECLS

typedef struct _PclGCList PclGCList;

typedef enum {
        PCL_GC_DEBUG_STATS              = 1 << 0,
        PCL_GC_DEBUG_COLLECTABLE        = 1 << 1,
        PCL_GC_DEBUG_UNCOLLECTABLE      = 1 << 2,
        PCL_GC_DEBUG_SAVEALL            = 1 << 3,
        PCL_GC_DEBUG_LEAK               = PCL_GC_DEBUG_COLLECTABLE
                                        | PCL_GC_DEBUG_UNCOLLECTABLE
                                        | PCL_GC_DEBUG_SAVEALL
} PclGCDebugFlags;

struct _PclGCList {
        struct _PclGCList *next;
        struct _PclGCList *prev;
        guint ref_state;
        guint ref_count;
        gpointer object;
};

glong           pcl_gc_collect                  (void);
void            pcl_gc_list_insert              (PclGCList *link);
void            pcl_gc_list_remove              (PclGCList *link);

G_END_DECLS

#endif /* PCL_MOD_GC_H */
