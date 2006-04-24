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

#ifndef PCL_SCOPE_H
#define PCL_SCOPE_H

#include "pcl-config.h"

G_BEGIN_DECLS

/**
 * PclScopeType:
 * @PCL_SCOPE_TYPE_INVALID:
 *      Invalid scope.
 * @PCL_SCOPE_TYPE_CLASS:
 *      Scope is a class definition.
 * @PCL_SCOPE_TYPE_FUNCTION:
 *      Scope is a function definition.  This includes generators, predicates,
 *      and conjunctions.
 * @PCL_SCOPE_TYPE_MODULE:
 *      Scope is a module, interactive input, or an eval() string.
 *
 * Used to classify a #PclScope struct.
 */
typedef enum {
        PCL_SCOPE_TYPE_INVALID,
        PCL_SCOPE_TYPE_CLASS,
        PCL_SCOPE_TYPE_FUNCTION,
        PCL_SCOPE_TYPE_MODULE
} PclScopeType;

/**
 * PclScopeFlags:
 * @PCL_SCOPE_FLAG_CHILD_FREE:
 *      A descendant of this scope has free variables.
 * @PCL_SCOPE_FLAG_GENERATOR:
 *      This scope is a generator expression or a function containing a @yield
 *      statement.
 * @PCL_SCOPE_FLAG_NESTED:
 *      This is a nested scope (a function within a function).
 *
 * Used to check or determine characteristics of a #PclScope struct.
 */
typedef enum {
        PCL_SCOPE_FLAG_CHILD_FREE       = 1 << 0,
        PCL_SCOPE_FLAG_GENERATOR        = 1 << 1,
        PCL_SCOPE_FLAG_NESTED           = 1 << 2
} PclScopeFlags;

/**
 * PclSymbolFlags:
 * @PCL_SYMBOL_FLAG_DEF_GLOBAL:
 *      Symbol is defined by a @global statement.
 * @PCL_SYMBOL_FLAG_DEF_IMPORT:
 *      Symbol is defined by an @import statement.
 * @PCL_SYMBOL_FLAG_DEF_LOCAL:
 *      Symbol is defined by an assignment statement.
 * @PCL_SYMBOL_FLAG_DEF_PARAM:
 *      Symbol is defined as a parameter to a function.
 * @PCL_SYMBOL_FLAG_DEF_FREE:
 *      Symbol is free in the local scope.
 * @PCL_SYMBOL_FLAG_DEF_FREE_CLASS:
 *      Symbol is free in the local scope, and the local scope is nested
 *      within a class definition.
 * @PCL_SYMBOL_FLAG_DEF_FREE_GLOBAL:
 *      Symbol is free in the local scope, but is defined by a @global
 *      statement in an ancestor scope.
 * @PCL_SYMBOL_FLAG_DEF_STAR:
 *      Symbol is a parameter with a star (@*) prefix.
 * @PCL_SYMBOL_FLAG_DEF_DOUBLE_STAR:
 *      Symbol is a parameter with a double-star (@**) prefix.
 * @PCL_SYMBOL_FLAG_STATIC:
 *      Symbol is static (only used in predicates).
 * @PCL_SYMBOL_FLAG_USE:
 *      Symbol is used in the local scope.
 *
 * Used to check or determine characteristics of a #PclScope entry.
 */
typedef enum {
        PCL_SYMBOL_FLAG_DEF_GLOBAL      = 1 << 0,
        PCL_SYMBOL_FLAG_DEF_IMPORT      = 1 << 1,
        PCL_SYMBOL_FLAG_DEF_LOCAL       = 1 << 2,
        PCL_SYMBOL_FLAG_DEF_PARAM       = 1 << 3,
        PCL_SYMBOL_FLAG_DEF_FREE        = 1 << 4,
        PCL_SYMBOL_FLAG_DEF_FREE_CLASS  = 1 << 5,
        PCL_SYMBOL_FLAG_DEF_FREE_GLOBAL = 1 << 6,
        PCL_SYMBOL_FLAG_DEF_STAR        = 1 << 7,
        PCL_SYMBOL_FLAG_DEF_DOUBLE_STAR = 1 << 8,
        PCL_SYMBOL_FLAG_STATIC          = 1 << 9,
        PCL_SYMBOL_FLAG_USE             = 1 << 10
} PclSymbolFlags;

typedef struct {
        PclScopeType type;
        GHashTable *symbols;
        GSList *params;
        GNode *node;
        gint tmpname;
        PclScopeFlags flags;
} PclScope;

const gchar *   pcl_scope_type_name             (PclScopeType type);
PclScope *      pcl_scope_new                   (GNode *node,
                                                 PclScope *parent);
void            pcl_scope_destroy               (PclScope *scope);
void            pcl_scope_add_param             (PclScope *scope,
                                                 const gchar *name);
PclSymbolFlags  pcl_scope_get_flags             (PclScope *scope,
                                                 const gchar *name);
void            pcl_scope_set_flags             (PclScope *scope,
                                                 const gchar *name,
                                                 PclSymbolFlags flags);
GSList *        pcl_scope_get_frees             (PclScope *scope);
GSList *        pcl_scope_get_names             (PclScope *scope);
gboolean        pcl_symbol_is_free              (PclSymbolFlags flags);

G_END_DECLS

#endif /* PCL_SCOPE_H */
