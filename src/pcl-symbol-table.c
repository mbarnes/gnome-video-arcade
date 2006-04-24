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

#include "pcl-symbol-table.h"
#include "pcl-node.h"
#include "pcl-scope.h"

#define FOR_EACH(link, init, term) \
        for ((link) = (init); (link) != (term); (link) = (link)->next)

#define symbol_table_current_scope(symbol_table) \
        ((PclScope *) (symbol_table)->current_scope->data)

#define symbol_table_add_def(symbol_table, name, flags) \
        (symbol_table_add_def_ext ((symbol_table), \
        symbol_table_current_scope (symbol_table), (name), (flags)))

#define symbol_table_add_use(symbol_table, name) \
        (symbol_table_add_def ((symbol_table), (name), PCL_SYMBOL_FLAG_USE))

#define symbol_table_add_flag(symbol_table, flag) \
        (symbol_table_current_scope (symbol_table)->flags |= (flag))

#define symbol_table_tmpname_borrow(symbol_table) \
        (g_strdup_printf ("_[%d]", \
        ++symbol_table_current_scope (symbol_table)->tmpname))

#define symbol_table_tmpname_return(symbol_table) \
        (symbol_table_current_scope (symbol_table)->tmpname--)

/* Forward Declarations */
static void symbol_table_node (PclSymbolTable *symbol_table, GNode *node);

static void
symbol_table_add_def_ext (PclSymbolTable *symbol_table, PclScope *scope,
                          const gchar *name, gint flags)
{
        gint combined_flags;

        combined_flags = pcl_scope_get_flags (scope, name) | flags;
        pcl_scope_set_flags (scope, name, combined_flags);

        if (flags & PCL_SYMBOL_FLAG_DEF_PARAM)
                pcl_scope_add_param (scope, name);
        if (flags & PCL_SYMBOL_FLAG_DEF_GLOBAL)
        {
                scope = symbol_table->global_scope->data;
                combined_flags = pcl_scope_get_flags (scope, name) | flags;
                pcl_scope_set_flags (scope, name, combined_flags);
        }
}

static void
symbol_table_add_target (PclSymbolTable *symbol_table, GNode *node, gint flags)
{
        GNode *child;

        switch (PCL_NODE_TYPE (node))
        {
                case PCL_NODE_TYPE_NAME:
                {
                        /* XXX Kludge to get make statics work (kinda). */
                        gint existing_flags;
                        existing_flags = pcl_scope_get_flags (
                                symbol_table_current_scope (symbol_table),
                                PCL_NODE_STRING (node));
                        if (!(existing_flags & PCL_SYMBOL_FLAG_STATIC))
                        {
                                flags |= PCL_SYMBOL_FLAG_DEF_LOCAL;
                                symbol_table_add_def (
                                        symbol_table,
                                        PCL_NODE_STRING (node), flags);
                        }
                        break;
                }

                case PCL_NODE_TYPE_ATTRIBUTE:
                        symbol_table_node (symbol_table, node);
                        break;

                case PCL_NODE_TYPE_LIST:
                case PCL_NODE_TYPE_TUPLE:
                        FOR_EACH (child, node->children, NULL)
                                symbol_table_add_target (
                                        symbol_table, child, flags);
                        break;

               case PCL_NODE_TYPE_SLICING:
               case PCL_NODE_TYPE_SUBSCRIPT:
                        FOR_EACH (child, node->children, NULL)
                                symbol_table_node (symbol_table, child);
                        break;

                default:
                        g_assert_not_reached ();
        }
}

static void
symbol_table_arguments (PclSymbolTable *symbol_table, GNode *node)
{
        GNode *child;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_ARGUMENTS);

        FOR_EACH (child, node->children, NULL)
        {
                /* The PAIRS flag indicates that these are keyword
                 * arguments (keyword=value).  Skip the keyword. */
                if (PCL_NODE_FLAGS (node) & PCL_NODE_FLAG_PAIRS)
                        child = child->next;

                symbol_table_node (symbol_table, child);
        }
}

static void
symbol_table_assign (PclSymbolTable *symbol_table, GNode *node)
{
        GNode *child, *last_child;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_ASSIGN);

        last_child = g_node_last_child (node);
        FOR_EACH (child, node->children, last_child)
                symbol_table_add_target (symbol_table, child, 0);
        symbol_table_node (symbol_table, last_child);
}

static void
symbol_table_class (PclSymbolTable *symbol_table, GNode *node)
{
        GNode *child, *last_child;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_CLASS);

        symbol_table_add_def (
                symbol_table,
                PCL_NODE_STRING (node),
                PCL_SYMBOL_FLAG_DEF_LOCAL);

        last_child = g_node_last_child (node);
        FOR_EACH (child, node->children, last_child)
                symbol_table_node (symbol_table, child);
        pcl_symbol_table_enter_scope (symbol_table, node);
        symbol_table_node (symbol_table, last_child);
        pcl_symbol_table_leave_scope (symbol_table);
}

static void
symbol_table_comprehension (PclSymbolTable *symbol_table, GNode *node)
{
        GNode *child;
        gchar *tmpname;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_COMPREHENSION);

        tmpname = symbol_table_tmpname_borrow (symbol_table);

        symbol_table_add_def (
                symbol_table, tmpname,
                PCL_SYMBOL_FLAG_DEF_LOCAL);

        child = node->children;
        symbol_table_node (symbol_table, child);

        FOR_EACH (child, child->next, NULL)
        {
                if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_FOR)
                        symbol_table_add_target (
                                symbol_table, child->children, 0);
                else
                        g_assert (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_IF);
                symbol_table_node (symbol_table, g_node_last_child (child));
        }

        /* Embed a hidden variable name. */
        PCL_NODE_STRING (node) = tmpname;

        symbol_table_tmpname_return (symbol_table);
}

static void
symbol_table_conjunction (PclSymbolTable *symbol_table, GNode *node)
{
        GNode *child;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_CONJUNCTION);

        pcl_symbol_table_enter_scope (symbol_table, node);

        FOR_EACH (child, node->children, NULL)
                symbol_table_node (symbol_table, child);

        pcl_symbol_table_leave_scope (symbol_table);
}

static void
symbol_table_except (PclSymbolTable *symbol_table, GNode *node)
{
        GNode *child, *last_child;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_EXCEPT);

        child = node->children;
        last_child = g_node_last_child (node);

        if (child != last_child)
        {
                symbol_table_node (symbol_table, child);
                child = child->next;
        }
        if (child != last_child)
        {
                symbol_table_add_target (symbol_table, child, 0);
                child = child->next;
        }

        g_assert (child == last_child);
        symbol_table_node (symbol_table, last_child);
}

static void
symbol_table_for (PclSymbolTable *symbol_table, GNode *node)
{
        GNode *child;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_FOR);

        child = node->children;
        symbol_table_add_target (symbol_table, child, 0);

        FOR_EACH (child, child->next, NULL)
                symbol_table_node (symbol_table, child);
}

static void
symbol_table_function (PclSymbolTable *symbol_table, GNode *node)
{
        GNode *child;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_FUNCTION);

        symbol_table_add_def (
                symbol_table,
                PCL_NODE_STRING (node),
                PCL_SYMBOL_FLAG_DEF_LOCAL);

        /* add decorators */
        child = node->children;
        if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_DECORATORS)
                symbol_table_node (symbol_table, child);

        /* add default parameter values */
        child = g_node_nth_child (node, 2);
        if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_PARAMETERS)
        {
                g_assert (PCL_NODE_FLAGS (child) & PCL_NODE_FLAG_PAIRS);
                FOR_EACH (child, child->children, NULL)
                {
                        child = child->next;
                        symbol_table_node (symbol_table, child);
                }
        }

        pcl_symbol_table_enter_scope (symbol_table, node);

        child = g_node_nth_child (node, 1);
        if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_PARAMETERS)
                symbol_table_node (symbol_table, child);

        child = child->next;
        if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_PARAMETERS)
                symbol_table_node (symbol_table, child);

        child = child->next;
        if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_NAME)
                symbol_table_add_def (
                        symbol_table,
                        PCL_NODE_STRING (child),
                        PCL_SYMBOL_FLAG_DEF_PARAM |
                        PCL_SYMBOL_FLAG_DEF_STAR);

        child = child->next;
        if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_NAME)
                symbol_table_add_def (
                        symbol_table,
                        PCL_NODE_STRING (child),
                        PCL_SYMBOL_FLAG_DEF_PARAM |
                        PCL_SYMBOL_FLAG_DEF_DOUBLE_STAR);

        child = child->next;
        symbol_table_node (symbol_table, child);

        pcl_symbol_table_leave_scope (symbol_table);
}

static void
symbol_table_generator (PclSymbolTable *symbol_table, GNode *node)
{
        GNode *expression, *child;
        gboolean is_outmost = TRUE;
        const gchar *outmost;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_GENERATOR);

        pcl_symbol_table_enter_scope (symbol_table, node);

        /* XXX Python assigns the constant GENERATOR_EXPRESSION (2) here,
         *     but doesn't do any special check for it versus a normal
         *     generator function.  So we can keep ours a flag. */
        symbol_table_add_flag (symbol_table, PCL_SCOPE_FLAG_GENERATOR);

        outmost = "[outmost-iterable]";
        symbol_table_add_def (
                symbol_table, outmost,
                PCL_SYMBOL_FLAG_DEF_PARAM);

        expression = node->children;
        FOR_EACH (child, expression->next, NULL)
        {
                if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_FOR)
                {
                        symbol_table_add_target (
                                symbol_table, child->children, 0);
                        if (is_outmost)
                                symbol_table_add_use (symbol_table, outmost);
                        else
                                symbol_table_node (
                                        symbol_table,
                                        g_node_last_child (child));
                        is_outmost = FALSE;
                }
                else
                {
                        g_assert (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_IF);
                        symbol_table_node (
                                symbol_table,
                                g_node_last_child (child));
                }
        }

        symbol_table_node (symbol_table, expression);

        pcl_symbol_table_leave_scope (symbol_table);

        /* For outmost iterable precomputation. */
        child = expression->next;
        symbol_table_node (symbol_table, g_node_last_child (child));

        /* Embed a hidden variable name. */
        PCL_NODE_STRING (node) = g_strdup (outmost);
}

static void
symbol_table_global (PclSymbolTable *symbol_table, GNode *node)
{
        GNode *child;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_GLOBAL);

        FOR_EACH (child, node->children, NULL)
        {
                const gchar *name = PCL_NODE_STRING (child);
                PclScope *current = symbol_table_current_scope (symbol_table);
                gint flags = pcl_scope_get_flags (current, name);
                g_assert (flags == 0 || flags == PCL_SYMBOL_FLAG_DEF_GLOBAL);
                symbol_table_add_def (
                        symbol_table, name,
                        PCL_SYMBOL_FLAG_DEF_GLOBAL);
        }
}

static void
symbol_table_import (PclSymbolTable *symbol_table, GNode *node)
{
        GNode *child;
        GNode *target;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_IMPORT);

        if (g_node_n_children (node) % 2 == 0)
        {
                FOR_EACH (child, node->children, NULL)
                {
                        child = child->next;
                        g_assert (child != NULL);

                        target = child;  /* assume 'as NAME' */
                        if (PCL_NODE_TYPE (target) != PCL_NODE_TYPE_NAME)
                                target = target->prev->children;
                        symbol_table_add_target (
                                symbol_table, target,
                                PCL_SYMBOL_FLAG_DEF_IMPORT);
                }
        }
        else
        {
                child = node->children;
                g_assert (child != NULL);

                if (child->next == NULL)
                {
                        PclScope *scope;
                        scope = symbol_table_current_scope (symbol_table);
                        if (scope->type != PCL_SCOPE_TYPE_MODULE)
                        {
                                /* WARNING: import * only allowed at
                                 *          module level */
                                g_assert_not_reached ();
                        }
                }
                else
                {
                        FOR_EACH (child, child->next, NULL)
                        {
                                child = child->next;
                                g_assert (child != NULL);

                                target = child;  /* assume 'as NAME' */
                                if (PCL_NODE_TYPE (target) !=
                                                PCL_NODE_TYPE_NAME)
                                        target = target->prev;
                                symbol_table_add_target (
                                        symbol_table, target,
                                        PCL_SYMBOL_FLAG_DEF_IMPORT);
                        }
                }
        }
}

static void
symbol_table_parameters (PclSymbolTable *symbol_table, GNode *node)
{
        GNode *child;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_PARAMETERS);

        FOR_EACH (child, node->children, NULL)
        {
                symbol_table_add_def (
                        symbol_table,
                        PCL_NODE_STRING (child),
                        PCL_SYMBOL_FLAG_DEF_PARAM);

                /* The PAIRS flags indicates that these are default
                 * parameters (name=default).  Skip the default. */
                if (PCL_NODE_FLAGS (node) & PCL_NODE_FLAG_PAIRS)
                        child = child->next;
        }
}

static void
symbol_table_predicate (PclSymbolTable *symbol_table, GNode *node)
{
        GNode *child;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_PREDICATE);

        symbol_table_add_def (
                symbol_table,
                PCL_NODE_STRING (node),
                PCL_SYMBOL_FLAG_DEF_LOCAL);

        /* add decorators */
        child = node->children;
        if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_DECORATORS)
                symbol_table_node (symbol_table, child);

        /* add default parameter values */
        child = g_node_nth_child (node, 2);
        if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_PARAMETERS)
        {
                g_assert (PCL_NODE_FLAGS (child) & PCL_NODE_FLAG_PAIRS);
                FOR_EACH (child, child->children, NULL)
                {
                        child = child->next;
                        symbol_table_node (symbol_table, child);
                }
        }

        /* add initial return value */
        child = g_node_nth_child (node, 5);
        if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_ASSIGN)
        {
                child = g_node_nth_child (child, 1);
                symbol_table_node (symbol_table, child);
        }

        pcl_symbol_table_enter_scope (symbol_table, node);

        child = g_node_nth_child (node, 1);
        if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_PARAMETERS)
                symbol_table_node (symbol_table, child);

        child = child->next;
        if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_PARAMETERS)
                symbol_table_node (symbol_table, child);

        child = child->next;
        if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_NAME)
                symbol_table_add_def (
                        symbol_table,
                        PCL_NODE_STRING (child),
                        PCL_SYMBOL_FLAG_DEF_PARAM |
                        PCL_SYMBOL_FLAG_DEF_STAR);

        child = child->next;
        if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_NAME)
                symbol_table_add_def (
                        symbol_table,
                        PCL_NODE_STRING (child),
                        PCL_SYMBOL_FLAG_DEF_PARAM |
                        PCL_SYMBOL_FLAG_DEF_DOUBLE_STAR);

        child = child->next;
        if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_ASSIGN)
        {
                /* NAME '=' expression */
                symbol_table_add_def (
                        symbol_table,
                        PCL_NODE_STRING (g_node_first_child (child)),
                        PCL_SYMBOL_FLAG_DEF_LOCAL |
                        PCL_SYMBOL_FLAG_STATIC);
                symbol_table_node (
                        symbol_table,
                        g_node_last_child (child));
        }

        child = child->next;
        symbol_table_node (symbol_table, child);

        pcl_symbol_table_leave_scope (symbol_table);
}

static void
symbol_table_static (PclSymbolTable *symbol_table, GNode *node)
{
        GNode *child;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_STATIC);

        FOR_EACH (child, node->children, NULL)
                symbol_table_add_def (
                        symbol_table,
                        PCL_NODE_STRING (child),
                        PCL_SYMBOL_FLAG_STATIC |
                        PCL_SYMBOL_FLAG_USE);
}

static void
symbol_table_yield (PclSymbolTable *symbol_table, GNode *node)
{
        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_YIELD);

        symbol_table_add_flag (symbol_table, PCL_SCOPE_FLAG_GENERATOR);
        symbol_table_node (symbol_table, node->children);
}

static void
symbol_table_node (PclSymbolTable *symbol_table, GNode *node)
{
        GNode *child;

        switch (PCL_NODE_TYPE (node))
        {
                case PCL_NODE_TYPE_NAME:
                        symbol_table_add_use (
                                symbol_table,
                                PCL_NODE_STRING (node));
                        break;

                case PCL_NODE_TYPE_ARGUMENTS:
                        symbol_table_arguments (symbol_table, node);
                        break;

                case PCL_NODE_TYPE_ASSIGN:
                        symbol_table_assign (symbol_table, node);
                        break;

                case PCL_NODE_TYPE_ATTRIBUTE:
                        symbol_table_node (symbol_table, node->children);
                        break;

                case PCL_NODE_TYPE_CLASS:
                        symbol_table_class (symbol_table, node);
                        break;

                case PCL_NODE_TYPE_COMPREHENSION:
                        symbol_table_comprehension (symbol_table, node);
                        break;

                case PCL_NODE_TYPE_CONJUNCTION:
                        symbol_table_conjunction (symbol_table, node);
                        break;

                case PCL_NODE_TYPE_EXCEPT:
                        symbol_table_except (symbol_table, node);
                        break;

                case PCL_NODE_TYPE_FOR:
                        symbol_table_for (symbol_table, node);
                        break;

                case PCL_NODE_TYPE_FUNCTION:
                        symbol_table_function (symbol_table, node);
                        break;

                case PCL_NODE_TYPE_GENERATOR:
                        symbol_table_generator (symbol_table, node);
                        break;

                case PCL_NODE_TYPE_GLOBAL:
                        symbol_table_global (symbol_table, node);
                        break;

                case PCL_NODE_TYPE_IMPORT:
                        symbol_table_import (symbol_table, node);
                        break;

                case PCL_NODE_TYPE_PARAMETERS:
                        symbol_table_parameters (symbol_table, node);
                        break;

                case PCL_NODE_TYPE_PREDICATE:
                        symbol_table_predicate (symbol_table, node);
                        break;

                case PCL_NODE_TYPE_STATIC:
                        symbol_table_static (symbol_table, node);
                        break;

                case PCL_NODE_TYPE_YIELD:
                        symbol_table_yield (symbol_table, node);
                        break;

                default:
                        FOR_EACH (child, node->children, NULL)
                                symbol_table_node (symbol_table, child);
        }
}

/* helper for symbol_table_update_free_vars() */
static void
symbol_table_undo_free (PclSymbolTable *symbol_table, GNode *node,
                        const gchar *name)
{
        gint flags;

        flags = pcl_scope_get_flags (node->data, name);

        if (pcl_symbol_is_free (flags))
        {
                GNode *child;

                symbol_table_add_def_ext (
                        symbol_table, node->data, name,
                        PCL_SYMBOL_FLAG_DEF_FREE_GLOBAL);
                FOR_EACH (child, node->children, NULL)
                        symbol_table_undo_free (symbol_table, child, name);
        }
}

/* helper for symbol_table_update_free_vars() */
static void
symbol_table_check_global (PclSymbolTable *symbol_table, GNode *node,
                           const gchar *name)
{
        PclScope *current;
        gint flags;

        current = symbol_table_current_scope (symbol_table);
        flags = pcl_scope_get_flags (current, name);

        if (flags == 0 || current->type == PCL_SCOPE_TYPE_CLASS) 
                symbol_table_undo_free (symbol_table, node, name);
        else if (pcl_symbol_is_free (flags))
                symbol_table_undo_free (symbol_table, node, name);
        else if (flags & PCL_SYMBOL_FLAG_DEF_GLOBAL)
                symbol_table_undo_free (symbol_table, node, name);
        else
                symbol_table_add_def_ext (
                        symbol_table, current, name,
                        PCL_SYMBOL_FLAG_DEF_FREE);
}

static void
symbol_table_update_free_vars (PclSymbolTable *symbol_table, GNode *node)
{
        GSList *frees, *link;
        PclScope *current;
        gint def, flags;

        current = symbol_table_current_scope (symbol_table);
        if (current->type == PCL_SCOPE_TYPE_CLASS)
                def = PCL_SYMBOL_FLAG_DEF_FREE_CLASS;
        else
                def = PCL_SYMBOL_FLAG_DEF_FREE;

        frees = pcl_scope_get_frees (node->data);
        if (frees != NULL)
                current->flags |= PCL_SCOPE_FLAG_CHILD_FREE;

        FOR_EACH (link, frees, NULL)
        {
                const gchar *name = link->data;
                flags = pcl_scope_get_flags (current, name);

                /* If a name N is declared global in scope A and
                 * referenced in scope B contained (perhaps indirectly)
                 * in A and there are no scopes with bindings for N
                 * between B and A, then N is global in B.  Unless A
                 * is a class scope, because class scopes are not
                 * considered for nested scopes. */
                if (current->type != PCL_SCOPE_TYPE_CLASS &&
                                (flags & PCL_SYMBOL_FLAG_DEF_GLOBAL))
                        symbol_table_undo_free (symbol_table, node, name);
                else if (current->flags & PCL_SCOPE_FLAG_NESTED)
                        symbol_table_add_def_ext (
                                symbol_table, current, name, def);
                else
                        symbol_table_check_global (symbol_table, node, name);
        }

        g_slist_free (frees);
}

static gboolean
symbol_table_display_scope (GNode *node)
{
        const gchar *indent = ".  ";
        PclScope *scope = node->data;
        guint depth = g_node_depth (node);
        guint length = depth * strlen (indent) + 1;
        GSList *names, *link;
        gchar *prefix, *cp;
        gchar *name = NULL;
        gint flags;

        prefix = cp = g_new0 (gchar, length);
        while (depth-- > 1)
                cp = g_stpcpy (cp, indent);

#define DISPLAY_FLAG(name, text) \
        ((flags & PCL_SCOPE_FLAG_##name) ? " [" text "]" : "")

        flags = scope->flags;
        if (PCL_NODE_STRING (scope->node) != NULL)
        {
                const gchar *tmp;
                tmp = PCL_NODE_STRING (scope->node);
                name = g_strdup_printf (" ('%s')", tmp);
        }
        g_debug ("%s%d: %s-SCOPE%s%s%s%s",
                prefix, PCL_NODE_LINENO (scope->node),
                pcl_scope_type_name (scope->type),
                (name != NULL) ? name : "",
                DISPLAY_FLAG (CHILD_FREE, "child-free"),
                DISPLAY_FLAG (GENERATOR, "generator"),
                DISPLAY_FLAG (NESTED, "nested"));
        g_free (name);

#undef DISPLAY_FLAG

#define DISPLAY_FLAG(name, text) \
        ((flags & PCL_SYMBOL_FLAG_##name) ? " [" text "]" : "")

        names = pcl_scope_get_names (scope);
        names = g_slist_sort (names, (GCompareFunc) strcmp);
        for (link = names; link != NULL; link = link->next)
        {
                const gchar *name = link->data;
                flags = pcl_scope_get_flags (scope, name);
                g_debug ("%s%sNAME ('%s')%s%s%s%s%s%s%s%s%s%s%s",
                        prefix, indent, name,
                        DISPLAY_FLAG (DEF_GLOBAL, "def-global"),
                        DISPLAY_FLAG (DEF_IMPORT, "def-import"),
                        DISPLAY_FLAG (DEF_LOCAL, "def-local"),
                        DISPLAY_FLAG (DEF_PARAM, "def-param"),
                        DISPLAY_FLAG (DEF_FREE, "def-free"),
                        DISPLAY_FLAG (DEF_FREE_CLASS, "def-free-class"),
                        DISPLAY_FLAG (DEF_FREE_GLOBAL, "def-free-global"),
                        DISPLAY_FLAG (DEF_STAR, "*"),
                        DISPLAY_FLAG (DEF_DOUBLE_STAR, "**"),
                        DISPLAY_FLAG (STATIC, "static"),
                        DISPLAY_FLAG (USE, "use"));
        }
        g_slist_free (names);

#undef DISPLAY_FLAG

        g_free (prefix);
        return FALSE;
}

void
symbol_table_display (PclSymbolTable *symbol_table)
{
        g_return_if_fail (symbol_table != NULL);
        g_debug ("<<< BEGIN SYMBOL TABLE >>>");
        g_node_traverse (
                symbol_table->global_scope, G_PRE_ORDER, G_TRAVERSE_ALL, -1,
                (GNodeTraverseFunc) symbol_table_display_scope, NULL);
        g_debug ("<<<  END SYMBOL TABLE  >>>");
}

PclSymbolTable *
pcl_symbol_table_new (GNode *parse_tree)
{
        PclSymbolTable *symbol_table;
        PclScope *new_scope;

        symbol_table = g_new (PclSymbolTable, 1);
        new_scope = pcl_scope_new (parse_tree, NULL);

        symbol_table->scopes = g_ptr_array_sized_new (32);
        g_ptr_array_add (symbol_table->scopes, new_scope);
        symbol_table->global_scope = g_node_new (new_scope);
        symbol_table->current_scope = symbol_table->global_scope;

        symbol_table->ready = FALSE;
        symbol_table_node (symbol_table, parse_tree);
        symbol_table->ready = TRUE;

        if (PCL_OPTION_DEBUG)
                symbol_table_display (symbol_table);

        return symbol_table;
}

void
pcl_symbol_table_destroy (PclSymbolTable *symbol_table)
{
        g_return_if_fail (symbol_table != NULL);
        g_ptr_array_foreach (
                symbol_table->scopes,
                (GFunc) pcl_scope_destroy, NULL);
        g_node_destroy (symbol_table->global_scope);
        g_free (symbol_table);
}

void
pcl_symbol_table_enter_scope (PclSymbolTable *symbol_table, GNode *parse_node)
{
        g_return_if_fail (symbol_table != NULL);
        if (parse_node != NULL)
        {
                PclScope *new_scope;
                new_scope = pcl_scope_new (parse_node,
                        symbol_table_current_scope (symbol_table));
                g_node_prepend_data (symbol_table->current_scope, new_scope);
                g_ptr_array_add (symbol_table->scopes, new_scope);
        }
        symbol_table->current_scope = symbol_table->current_scope->children;
}

void
pcl_symbol_table_leave_scope (PclSymbolTable *symbol_table)
{
        GNode *node, *previous;

        g_return_if_fail (symbol_table != NULL);
        if (!symbol_table->ready)
                FOR_EACH (node, symbol_table->current_scope->children, NULL)
                        symbol_table_update_free_vars (symbol_table, node);
        previous = symbol_table->current_scope;
        symbol_table->current_scope = previous->parent;
        if (previous->next != NULL)
        {
                g_node_unlink (previous);
                g_node_append (symbol_table->current_scope, previous);
        }
}
