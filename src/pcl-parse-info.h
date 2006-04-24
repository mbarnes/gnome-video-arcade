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

#ifndef PCL_PARSE_INFO_H
#define PCL_PARSE_INFO_H

#include "pcl-config.h"

G_BEGIN_DECLS

typedef struct _PclParseInfo {
        GNode *parse_tree;
        GMemChunk *node_mem_chunk;
        GAllocator *g_list_allocator;
        GAllocator *g_node_allocator;
        gint error_code;
} PclParseInfo;

PclParseInfo *pcl_parse_info_new (const gchar *source);
PclParseInfo *pcl_parse_info_display (PclParseInfo *parse_info);
PclParseInfo *pcl_parse_info_normalize (PclParseInfo *parse_info);
void pcl_parse_info_destroy (PclParseInfo *parse_info);

G_END_DECLS

#endif /* PCL_PARSE_INFO_H */
