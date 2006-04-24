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

#ifndef PCL_SYMBOL_TABLE_H
#define PCL_SYMBOL_TABLE_H

#include "pcl-config.h"

G_BEGIN_DECLS

typedef struct _PclSymbolTable PclSymbolTable;

struct _PclSymbolTable {
        GPtrArray *scopes;
        GNode *global_scope;
        GNode *current_scope;
        gboolean ready;
};

PclSymbolTable * pcl_symbol_table_new           (GNode *parse_tree);
void             pcl_symbol_table_destroy       (PclSymbolTable *symbol_table);
void             pcl_symbol_table_enter_scope   (PclSymbolTable *symbol_table,
                                                 GNode *parse_node);
void             pcl_symbol_table_leave_scope   (PclSymbolTable *symbol_table);

G_END_DECLS

#endif /* PCL_SYMBOL_TABLE_H */
