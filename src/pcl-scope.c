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

#include "pcl-scope.h"
#include "pcl-node.h"

/* Keep this list in sync with the PclScopeType enumeration. */
static const gchar *scope_type_names[] = {
        /* 0 */  "<invalid-type>",
        /* 1 */  "CLASS",
        /* 2 */  "FUNCTION",
        /* 3 */  "MODULE"
};

static void
scope_collect_frees (gchar *name, gpointer p_flags, GSList **p_list)
{
        if (pcl_symbol_is_free (GPOINTER_TO_INT (p_flags)))
                *p_list = g_slist_prepend (*p_list, name);
}

static void
scope_collect_names (gchar *name, gpointer p_flags, GSList **p_list)
{
        *p_list = g_slist_prepend (*p_list, name);
}

static void
scope_inherit_from_parent (PclScope *scope, PclScope *parent)
{
        GSList *names, *link;
        PclSymbolFlags flags;

        g_assert (parent != NULL);
        names = pcl_scope_get_names (parent);
        for (link = names; link != NULL; link = link->next)
        {
                flags = pcl_scope_get_flags (parent, link->data);

                if (flags & PCL_SYMBOL_FLAG_STATIC)
                        flags = PCL_SYMBOL_FLAG_STATIC;

                /* inherit parameters and globals as locals */
                if ((flags & PCL_SYMBOL_FLAG_DEF_GLOBAL) ||
                        (flags & PCL_SYMBOL_FLAG_DEF_PARAM))
                {
                        flags &= ~(PCL_SYMBOL_FLAG_DEF_GLOBAL |
                                PCL_SYMBOL_FLAG_DEF_PARAM |
                                PCL_SYMBOL_FLAG_DEF_STAR |
                                PCL_SYMBOL_FLAG_DEF_DOUBLE_STAR);
                        flags |= PCL_SYMBOL_FLAG_DEF_LOCAL;
                }

                pcl_scope_set_flags (scope, link->data, flags);
        }
        g_slist_free (names);
}

const gchar *
pcl_scope_type_name (PclScopeType scope_type)
{
        if (0 <= scope_type && scope_type < G_N_ELEMENTS (scope_type_names))
                return scope_type_names[scope_type];
        return scope_type_names[PCL_SCOPE_TYPE_INVALID];
}

PclScope *
pcl_scope_new (GNode *node, PclScope *parent)
{
        PclScope *scope;
        PclScopeType type;
        GHashTable *symbols;

        g_return_val_if_fail (node != NULL, NULL);

        /* determine scope type from parse node type */
        switch (PCL_NODE_TYPE (node))
        {
                case PCL_NODE_TYPE_CLASS:
                        type = PCL_SCOPE_TYPE_CLASS;
                        break;
                case PCL_NODE_TYPE_CONJUNCTION:
                case PCL_NODE_TYPE_FUNCTION:
                case PCL_NODE_TYPE_GENERATOR:
                case PCL_NODE_TYPE_PREDICATE:
                        type = PCL_SCOPE_TYPE_FUNCTION;
                        break;
                case PCL_NODE_TYPE_EVAL_INPUT:
                case PCL_NODE_TYPE_FILE_INPUT:
                case PCL_NODE_TYPE_SINGLE_INPUT:
                        type = PCL_SCOPE_TYPE_MODULE;
                        break;
                default:
                        type = PCL_SCOPE_TYPE_INVALID;
                        break;
        }

        symbols = g_hash_table_new (g_str_hash, g_str_equal);

        scope = g_new (PclScope, 1);
        scope->type = type;
        scope->symbols = symbols;
        scope->params = NULL;
        scope->node = node;
        scope->tmpname = 0;

        /* determine whether scope is nested */
        if (parent == NULL)
                scope->flags = 0;
        else if (parent->flags & PCL_SCOPE_FLAG_NESTED)
                scope->flags = PCL_SCOPE_FLAG_NESTED;
        else if (parent->type == PCL_SCOPE_TYPE_FUNCTION)
                scope->flags = PCL_SCOPE_FLAG_NESTED;
        else
                scope->flags = 0;

        /* conjunctions inherit symbols from parent */
        if (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_CONJUNCTION)
                scope_inherit_from_parent (scope, parent);

        return scope;
}

void
pcl_scope_destroy (PclScope *scope)
{
        g_return_if_fail (scope != NULL);
        g_hash_table_destroy (scope->symbols);
        g_slist_free (scope->params);
        g_free (scope);
}

void
pcl_scope_add_param (PclScope *scope, const gchar *name)
{
        g_return_if_fail (scope != NULL);
        scope->params = g_slist_append (scope->params, (gpointer) name);
}

PclSymbolFlags
pcl_scope_get_flags (PclScope *scope, const gchar *name)
{
        g_return_val_if_fail (scope != NULL, 0);
        return GPOINTER_TO_INT (g_hash_table_lookup (scope->symbols, name));
}

void
pcl_scope_set_flags (PclScope *scope, const gchar *name, PclSymbolFlags flags)
{
        g_return_if_fail (scope != NULL);
        g_hash_table_insert (scope->symbols,
                (gpointer) name, GINT_TO_POINTER (flags));
}

GSList *
pcl_scope_get_frees (PclScope *scope)
{
        GSList *list = NULL;
        g_return_val_if_fail (scope != NULL, NULL);
        g_hash_table_foreach (scope->symbols,
                (GHFunc) scope_collect_frees, &list);
        return list;
}

GSList *
pcl_scope_get_names (PclScope *scope)
{
        GSList *list = NULL;
        g_return_val_if_fail (scope != NULL, NULL);
        g_hash_table_foreach (scope->symbols,
                (GHFunc) scope_collect_names, &list);
        return list;
}

gboolean
pcl_symbol_is_free (PclSymbolFlags flags)
{
        if ((flags & (PCL_SYMBOL_FLAG_USE | PCL_SYMBOL_FLAG_DEF_FREE)) &&
                !(flags & (PCL_SYMBOL_FLAG_DEF_GLOBAL |
                           PCL_SYMBOL_FLAG_DEF_LOCAL |
                           PCL_SYMBOL_FLAG_DEF_PARAM)))
                return TRUE;
        if (flags & PCL_SYMBOL_FLAG_DEF_FREE_CLASS)
                return TRUE;
        return FALSE;
}
