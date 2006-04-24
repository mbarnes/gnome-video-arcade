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

/* Keep this list in sync with the PclNodeType enumeration. */
static const gchar *node_type_names[] = {
        /*  0 */  "<invalid-type>",
        /*  1 */  "EVAL-INPUT",
        /*  2 */  "FILE-INPUT",
        /*  3 */  "SINGLE-INPUT",
        /*  4 */  "NAME",
        /*  5 */  "FLOAT",
        /*  6 */  "IMAGINARY",
        /*  7 */  "INTEGER",
        /*  8 */  "STRING",
        /*  9 */  "ASSIGN",
        /* 10 */  "AUG-ASSIGN",
        /* 11 */  "COMPARISON",
        /* 12 */  "UNARY-OPER",
        /* 13 */  "BINARY-OPER",
        /* 14 */  "TEST-AND",
        /* 15 */  "TEST-NOT",
        /* 16 */  "TEST-OR",
        /* 17 */  "ARGUMENTS",
        /* 18 */  "ASSERT",
        /* 19 */  "ATTRIBUTE",
        /* 20 */  "BREAK",
        /* 21 */  "CALL",
        /* 22 */  "CLASS",
        /* 23 */  "COMPREHENSION",
        /* 24 */  "CONDITIONAL",
        /* 25 */  "CONJUNCTION",
        /* 26 */  "CONTINUE",
        /* 27 */  "CUT",
        /* 28 */  "DECORATORS",
        /* 29 */  "DEL",
        /* 30 */  "DICT",
        /* 31 */  "DISJUNCTION",
        /* 32 */  "EXCEPT",
        /* 33 */  "EXEC",
        /* 34 */  "EXPRESSION",
        /* 35 */  "FOR",
        /* 36 */  "FUNCTION",
        /* 37 */  "GENERATOR",
        /* 38 */  "GLOBAL",
        /* 39 */  "IF",
        /* 40 */  "IMPORT",
        /* 41 */  "LIST",
        /* 42 */  "MODULE",
        /* 43 */  "NONE",
        /* 44 */  "PARAMETERS",
        /* 45 */  "PASS",
        /* 46 */  "PREDICATE",
        /* 47 */  "PRINT",
        /* 48 */  "RAISE",
        /* 49 */  "RETURN",
        /* 50 */  "SLICING",
        /* 51 */  "STATIC",
        /* 52 */  "SUBSCRIPT",
        /* 53 */  "SUITE",
        /* 54 */  "TRY-EXCEPT",
        /* 55 */  "TRY-FINALLY",
        /* 56 */  "TUPLE",
        /* 57 */  "WHILE",
        /* 58 */  "YIELD"
};

const gchar *
pcl_node_type_name (PclNodeType node_type)
{
        if (0 <= node_type && node_type < G_N_ELEMENTS (node_type_names))
                return node_type_names[node_type];
        return node_type_names[PCL_NODE_TYPE_INVALID];
}

PclNode *
pcl_node_new (PclNodeType node_type, gint lineno, gint offset)
{
        PclNode *node = g_new (PclNode, 1);
        node->type = node_type;
        node->string = NULL;
        node->lineno = lineno;
        node->offset = offset;
        node->opcode = 0;
        node->flags = 0;
        return node;
}

PclNode *
pcl_node_copy (PclNode *node)
{
        PclNode *copy = g_new (PclNode, 1);
        copy->type = node->type;
        if (node->flags & PCL_NODE_FLAG_INTERNED)
                copy->string = node->string;
        else
                copy->string = g_strdup (node->string);
        copy->lineno = node->lineno;
        copy->offset = node->offset;
        copy->opcode = node->opcode;
        copy->flags = node->flags;
        return copy;
}

void
pcl_node_destroy (PclNode *node)
{
        g_return_if_fail (node != NULL);
        if (!(node->flags & PCL_NODE_FLAG_INTERNED))
                g_free ((gchar *) node->string);  /* discard const */
        g_free (node);
}

gchar *
pcl_node_contents (PclNode *node)
{
        gchar *contents;
        gchar *opcode = NULL;
        gchar *string = NULL;

        g_return_val_if_fail (node != NULL, NULL);

#define DISPLAY_FLAG(name, text) \
        ((node->flags & PCL_NODE_FLAG_##name) ? " [" text "]" : "")

        if (node->string != NULL)
                string = g_strdup_printf (" ('%s')", node->string);
        if (node->opcode != 0)
                opcode = g_strdup_printf (" [op=0x%x]", node->opcode);
        contents = g_strdup_printf ("%d: %s%s%s%s%s%s",
                node->lineno,
                pcl_node_type_name (node->type),
                (string != NULL) ? string : "",
                (opcode != NULL) ? opcode : "",
                DISPLAY_FLAG (COMMA, "comma"),
                DISPLAY_FLAG (INTERNED, "interned"),
                DISPLAY_FLAG (PAIRS, "pairs"));
        g_free (opcode);
        g_free (string);

#undef DISPLAY_FLAG

        return contents;
}
