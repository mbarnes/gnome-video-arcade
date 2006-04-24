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

#include "pcl.h"
#include "pcl-node.h"

static gboolean
parse_info_display_node (GNode *node)
{
        const gchar *indent = ".  ";
        guint depth = g_node_depth (node);
        guint length = depth * strlen (indent) + 1;
        gchar *prefix, *contents, *cp;

        prefix = cp = g_new0 (gchar, length);
        while (depth-- > 1)
                cp = g_stpcpy (cp, indent);
        contents = pcl_node_contents (PCL_NODE (node));
        g_debug ("%s%s", prefix, contents);
        g_free (contents);
        g_free (prefix);
        return FALSE;
}

static gboolean
parse_info_normalize_lineno (GNode *node, gint *p_lineno)
{
        if (PCL_NODE_LINENO (node) >= *p_lineno)
                *p_lineno = PCL_NODE_LINENO (node);
        else
        {
                PCL_NODE_LINENO (node) = *p_lineno;
                PCL_NODE_OFFSET (node) = -1;  /* invalid */
        }
        return FALSE;
}

PclParseInfo *
pcl_parse_info_new (const gchar *source)
{
        PclParseInfo *parse_info = g_new (PclParseInfo, 1);
        gchar *name;

        name = g_strconcat (source, ":g-list-allocator", NULL);
        parse_info->g_list_allocator = g_allocator_new (name, 128);
        g_list_push_allocator (parse_info->g_list_allocator);
        g_free (name);

        name = g_strconcat (source, ":g-node-allocator", NULL);
        parse_info->g_node_allocator = g_allocator_new (name, 128);
        g_node_push_allocator (parse_info->g_node_allocator);
        g_free (name);

        return parse_info;
}

PclParseInfo *
pcl_parse_info_display (PclParseInfo *parse_info)
{
        g_return_val_if_fail (parse_info != NULL, NULL);
        g_debug ("<<< BEGIN PARSE TREE >>>");
        g_node_traverse (
                parse_info->parse_tree, G_PRE_ORDER, G_TRAVERSE_ALL, -1,
                (GNodeTraverseFunc) parse_info_display_node, NULL);
        g_debug ("<<<  END PARSE TREE  >>>");
        return parse_info;
}

PclParseInfo *
pcl_parse_info_normalize (PclParseInfo *parse_info)
{
        gint lineno = 1;
        g_return_val_if_fail (parse_info != NULL, NULL);
        g_node_traverse (
                parse_info->parse_tree, G_PRE_ORDER, G_TRAVERSE_ALL, -1,
                (GNodeTraverseFunc) parse_info_normalize_lineno, &lineno);
        return parse_info;
}

void
pcl_parse_info_destroy (PclParseInfo *parse_info)
{
        g_return_if_fail (parse_info != NULL);
        g_list_pop_allocator ();
        g_allocator_free (parse_info->g_list_allocator);
        g_node_pop_allocator ();
        g_allocator_free (parse_info->g_node_allocator);
        g_free (parse_info);
}
