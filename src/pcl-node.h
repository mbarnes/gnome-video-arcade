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

#ifndef PCL_NODE_H
#define PCL_NODE_H

#include "pcl-config.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef enum {
        PCL_NODE_TYPE_INVALID,
        PCL_NODE_TYPE_EVAL_INPUT,
        PCL_NODE_TYPE_FILE_INPUT,
        PCL_NODE_TYPE_SINGLE_INPUT,
        PCL_NODE_TYPE_NAME,
        PCL_NODE_TYPE_FLOAT,
        PCL_NODE_TYPE_IMAGINARY,
        PCL_NODE_TYPE_INTEGER,
        PCL_NODE_TYPE_STRING,
        PCL_NODE_TYPE_ASSIGN,
        PCL_NODE_TYPE_AUG_ASSIGN,
        PCL_NODE_TYPE_COMPARISON,
        PCL_NODE_TYPE_UNARY_OPER,
        PCL_NODE_TYPE_BINARY_OPER,
        PCL_NODE_TYPE_TEST_AND,
        PCL_NODE_TYPE_TEST_NOT,
        PCL_NODE_TYPE_TEST_OR,
        PCL_NODE_TYPE_ARGUMENTS,
        PCL_NODE_TYPE_ASSERT,
        PCL_NODE_TYPE_ATTRIBUTE,
        PCL_NODE_TYPE_BREAK,
        PCL_NODE_TYPE_CALL,
        PCL_NODE_TYPE_CLASS,
        PCL_NODE_TYPE_COMPREHENSION,
        PCL_NODE_TYPE_CONDITIONAL,
        PCL_NODE_TYPE_CONJUNCTION,
        PCL_NODE_TYPE_CONTINUE,
        PCL_NODE_TYPE_CUT,
        PCL_NODE_TYPE_DECORATORS,
        PCL_NODE_TYPE_DEL,
        PCL_NODE_TYPE_DICT,
        PCL_NODE_TYPE_DISJUNCTION,
        PCL_NODE_TYPE_EXCEPT,
        PCL_NODE_TYPE_EXEC,
        PCL_NODE_TYPE_EXPRESSION,
        PCL_NODE_TYPE_FOR,
        PCL_NODE_TYPE_FUNCTION,
        PCL_NODE_TYPE_GENERATOR,
        PCL_NODE_TYPE_GLOBAL,
        PCL_NODE_TYPE_IF,
        PCL_NODE_TYPE_IMPORT,
        PCL_NODE_TYPE_LIST,
        PCL_NODE_TYPE_MODULE,
        PCL_NODE_TYPE_NONE,
        PCL_NODE_TYPE_PARAMETERS,
        PCL_NODE_TYPE_PASS,
        PCL_NODE_TYPE_PREDICATE,
        PCL_NODE_TYPE_PRINT,
        PCL_NODE_TYPE_RAISE,
        PCL_NODE_TYPE_RETURN,
        PCL_NODE_TYPE_SLICING,
        PCL_NODE_TYPE_STATIC,
        PCL_NODE_TYPE_SUBSCRIPT,
        PCL_NODE_TYPE_SUITE,
        PCL_NODE_TYPE_TRY_EXCEPT,
        PCL_NODE_TYPE_TRY_FINALLY,
        PCL_NODE_TYPE_TUPLE,
        PCL_NODE_TYPE_WHILE,
        PCL_NODE_TYPE_YIELD
} PclNodeType;

typedef enum {
        PCL_NODE_FLAG_COMMA     = 1 << 0,  /* Trailing comma */
        PCL_NODE_FLAG_INTERNED  = 1 << 1,  /* String is interned */
        PCL_NODE_FLAG_PAIRS     = 1 << 2   /* Children are in pairs */
} PclNodeFlags;

typedef struct {
        PclNodeType type;
        const gchar *string;
        gint lineno;
        gint offset;
        gint opcode;
        PclNodeFlags flags;
} PclNode;

#define PCL_NODE(g_node_p)              ((PclNode *) (g_node_p)->data)
#define PCL_NODE_TYPE(g_node_p)         (PCL_NODE (g_node_p)->type)
#define PCL_NODE_STRING(g_node_p)       (PCL_NODE (g_node_p)->string)
#define PCL_NODE_LINENO(g_node_p)       (PCL_NODE (g_node_p)->lineno)
#define PCL_NODE_OFFSET(g_node_p)       (PCL_NODE (g_node_p)->offset)
#define PCL_NODE_OPCODE(g_node_p)       (PCL_NODE (g_node_p)->opcode)
#define PCL_NODE_FLAGS(g_node_p)        (PCL_NODE (g_node_p)->flags)

const gchar *   pcl_node_type_name              (PclNodeType node_type);
PclNode *       pcl_node_new                    (PclNodeType node_type,
                                                 gint lineno,
                                                 gint offset);
PclNode *       pcl_node_copy                   (PclNode *node);
void            pcl_node_destroy                (PclNode *node);
gchar *         pcl_node_contents               (PclNode *node);

G_END_DECLS

#endif /* PCL_NODE_H */
