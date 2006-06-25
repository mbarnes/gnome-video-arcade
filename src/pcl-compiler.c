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
#include "pcl-opcode.h"
#include "pcl-scope.h"
#include "pcl-symbol-table.h"

#define REF_CELL             0
#define REF_FREE             1
#define REF_LOCAL            2
#define REF_GLOBAL_EXPLICIT  3
#define REF_GLOBAL_IMPLICIT  4

#define SOURCE_LOCAL    0
#define SOURCE_GLOBAL   1
#define SOURCE_DEFAULT  2
#define SOURCE_CLOSURE  3

#define ACTION_LOAD    0
#define ACTION_STORE   1
#define ACTION_DELETE  2

/* Error Messages */
#define DEL_CLOSURE_ERROR_MSG \
        "can not delete variable '%s' referenced in nested scope"

struct _PclCompilerState
{
        PclObject *instructions;        /* instruction opcodes */
        PclObject *constants;           /* list of objects */
        PclObject *const_dict;          /* inverse of constants */
        PclObject *names;               /* list of strings (names) */
        PclObject *name_dict;           /* inverse of names */
        PclObject *globals;             /* value=None or True */
        PclObject *locals;              /* value=varnames index */
        PclObject *varnames;            /* variable names */
        PclObject *freevars;            /* free variables */
        PclObject *cellvars;            /* cell variables */
        const gchar *filename;          /* filename of current node */
        const gchar *name;              /* name of object */
        guint next_instruction;
        guint argument_count;
        guint variable_count;
        guint error_count;
        guint interactive;
        gint stack_level;
        gint max_stack_level;
        gboolean in_function;
        gboolean nested;
        guint loop_begin;               /* beginning of current loop */
        guint loop_count;               /* counts nested loops */
        guint block[PCL_MAX_BLOCKS];    /* stack of block types */
        gint block_count;               /* current block stack level */
        gint flags;
        PclSymbolTable *symbol_table;

        /* Line Number Tracking */
        PclObject *lineno_table;        /* table mapping address to lineno */
        gboolean lineno_frozen;         /* do not update lineno when true */
        gint lineno_table_next;         /* current length of table */
        gint lineno_table_last;         /* start of last record added */
        gint first_lineno;              /* first source lineno */
        gint last_lineno;               /* last lineno seen and recorded */
        gint last_address;              /* last address seen and recorded */
        gint lineno;                    /* current lineno */
        gint offset;                    /* current offset */ 
};

struct symbol_info {
        guint nlocals;
        guint ncells;
        guint nfrees;
        guint nimplicit;
};

/* Peephole Optimization */
extern PclObject *pcl_optimize (
        PclObject *bytes, PclObject *constants,
        PclObject *names, PclObject *lineno_table);

/* Forward Declaration */
static void compiler_node (PclCompilerState *cs, GNode *node);

static void
compiler_error (PclCompilerState *cs, PclObject *exception,
               const gchar *format, ...)
{
        va_list va;
        PclObject *message;

        va_start (va, format);
        message = pcl_string_from_format_va (format, va);

        cs->error_count++;
        if (cs->lineno < 1 || cs->interactive > 0)
        {
                /* unknown line number or interactive input */
                pcl_error_set_object (exception, message);
                goto exit;
        }
        if (exception == pcl_exception_syntax_error ())
        {
                PclObject *tuple;

                tuple = pcl_build_value ("(siiO)",
                        cs->filename, cs->lineno, cs->offset, message);
                if (tuple != NULL)
                {
                        pcl_error_set_object (exception, tuple);
                        pcl_object_unref (tuple);
                }
        }
        else
        {
                pcl_error_set_object (exception, message);
        }

exit:
        pcl_object_unref (message);
        va_end (va);
}

static gint
compiler_get_ref_type (PclCompilerState *cs, PclObject *name)
{
        PclObject *object;

        g_assert (PCL_IS_STRING (name));

        /* Do not change the order of lookup (see Python's get_ref_type) */
        if (pcl_dict_get_item (cs->cellvars, name) != NULL)
                return REF_CELL;
        if (pcl_dict_get_item (cs->locals, name) != NULL)
                return REF_LOCAL;
        if (pcl_dict_get_item (cs->freevars, name) != NULL)
                return REF_FREE;

        object = pcl_dict_get_item (cs->globals, name);
        if (object != NULL)
        {
                if (object == PCL_NONE)
                        return REF_GLOBAL_EXPLICIT;
                else
                        return REF_GLOBAL_IMPLICIT;
        }

        g_error ("unknown scope for %s in %s(%p) in %s\n"
                 "locals: %s\nglobals: %s\n",
                 pcl_string_as_string (name), cs->name,
                 cs->symbol_table->current_scope->data, cs->filename,
                 pcl_string_as_string (pcl_object_repr (cs->locals)),
                 pcl_string_as_string (pcl_object_repr (cs->globals)));

        return -1;
}

static void
compiler_done (PclCompilerState *cs)
{
        if (cs->instructions != NULL)
                pcl_string_resize (cs->instructions, cs->next_instruction);
        if (cs->lineno_table != NULL)
                pcl_string_resize (cs->lineno_table, cs->lineno_table_next);
}

static void
compiler_check_size (PclObject *string, glong offset)
{
        glong length = pcl_object_measure (string);
        if (offset >= length)
                pcl_string_resize (string, length * 2);
}

static void
compiler_add_byte (PclCompilerState *cs, guint byte)
{
        g_assert (byte <= 255);
        g_assert (cs->instructions != NULL);
        compiler_check_size (cs->instructions, cs->next_instruction);
        pcl_string_as_string (cs->instructions)[cs->next_instruction++] = byte;
}

static void
compiler_add_integer (PclCompilerState *cs, guint value)
{
        g_assert (value <= 65535);
        compiler_add_byte (cs, (value >> 0) & 0xFF);
        compiler_add_byte (cs, (value >> 8) & 0xFF);
}

static void
compiler_add_lineno_table (PclCompilerState *cs, gint addr, gint line)
{
        gchar *cp;

        if (cs->lineno_table == NULL)
                return;
        compiler_check_size (cs->lineno_table, cs->lineno_table_next + 2);

        /* Append address and line (as characters) to lineno table. */
        cp = pcl_string_as_string (cs->lineno_table) + cs->lineno_table_next;
        *cp++ = (guchar) addr, cs->lineno_table_next++;
        *cp++ = (guchar) line, cs->lineno_table_next++;
}

static void
compiler_set_lineno (PclCompilerState *cs, GNode *node)
{
        if (!cs->lineno_frozen)
        {
                cs->lineno = PCL_NODE_LINENO (node);
                cs->offset = PCL_NODE_OFFSET (node);
        }
        if (cs->first_lineno == 0)
                cs->first_lineno = cs->last_lineno = cs->lineno;
        else
        {
                gint incr_addr = cs->next_instruction - cs->last_address;
                gint incr_line = cs->lineno - cs->last_lineno;
                g_assert (incr_addr >= 0 && incr_line >= 0);
                cs->lineno_table_last = cs->lineno_table_next;
                while (incr_addr > 255)
                {
                        compiler_add_lineno_table (cs, 255, 0);
                        incr_addr -= 255;
                }
                while (incr_line > 255)
                {
                        compiler_add_lineno_table (cs, incr_addr, 255);
                        incr_line -= 255;
                        incr_addr = 0;
                }
                if (incr_addr > 0 || incr_line > 0)
                        compiler_add_lineno_table (cs, incr_addr, incr_line);
                cs->last_address = cs->next_instruction;
                cs->last_lineno = cs->lineno;
        }
}

static void
compiler_strip_lineno_table (PclCompilerState *cs)
{
        if (cs->next_instruction == cs->last_address)
                if (cs->lineno_table_last > 0)
                        cs->lineno_table_next = cs->lineno_table_last;
}

static void
compiler_add_oparg (PclCompilerState *cs, guint opcode, guint argument)
{
        guint extended_argument = argument >> 16;
        if (extended_argument != 0)
        {
                compiler_add_byte (cs, PCL_OPCODE_EXTENDED_ARG);
                compiler_add_integer (cs, extended_argument);
                argument &= 0xFFFF;
        }
        compiler_add_byte (cs, opcode);
        compiler_add_integer (cs, argument);
}

static void
compiler_add_fwref (PclCompilerState *cs, gint opcode, guint *p_anchor)
{
        /* Compile a forward reference for backpatching. */
        guint here;
        guint anchor;
        guint link;
        compiler_add_byte (cs, opcode);
        here = cs->next_instruction;
        anchor = *p_anchor;
        *p_anchor = here;
        link = (anchor == 0) ? 0 : here - anchor;
        compiler_add_integer (cs, link);
}

static void
compiler_backpatch (PclCompilerState *cs, guint anchor)
{
        guchar *code = (guchar *) pcl_string_as_string (cs->instructions);
        guint target = cs->next_instruction;
        guint previous;
        guint distance;

        while (TRUE)
        {
                /* Make the JUMP instruction at anchor point to target. */
                previous = code[anchor] + (code[anchor + 1] << 8);
                distance = target - (anchor + 2);
                code[anchor] = distance & 0xFF;
                distance >>= 8;
                code[anchor + 1] = distance & 0xFF;
                distance >>= 8;
                if (distance != 0)
                {
                        compiler_error (cs,
                                pcl_exception_system_error (),
                                "%s: offset too large", G_STRFUNC);
                        break;
                }
                if (previous == 0)
                        break;
                anchor -= previous;
        }
}

static guint
compiler_add (PclCompilerState *cs, PclObject *list,
             PclObject *dict, PclObject *object)
{
        /* Pseudocode:
         *     tuple = (object, type(object))
         *     if tuple in dict:
         *         n = dict[tuple] 
         *     else:
         *         n = len (list)
         *         list.append (object)
         *         dict[tuple] = n
         *     return n
         */
        PclObject *index = NULL;
        PclObject *tuple = NULL;
        PclObject *type;
        PclObject *value;
        glong n;

        /* We pack the type object into the key to ensure that no implicit
         * type coersion takes place when performing dictionary lookups. */
        type = pcl_object_type (object);
        if (type == NULL)
                goto fail;
        tuple = pcl_tuple_pack (2, object, type);
        pcl_object_unref (type);
        if (tuple == NULL)
                goto fail;
        value = pcl_dict_get_item (dict, tuple);
        if (value != NULL)
                n = pcl_int_as_long (value);
        else
        {
                n = pcl_object_measure (list);
                index = pcl_int_from_long (n);
                if (index == NULL)
                        goto fail;
                if (!pcl_list_append (list, object))
                        goto fail;
                if (!pcl_dict_set_item (dict, tuple, index))
                        goto fail;
                pcl_object_unref (index);
        }
        pcl_object_unref (tuple);
        return n;

fail:
        if (index != NULL)
                pcl_object_unref (index);
        if (tuple != NULL)
                pcl_object_unref (tuple);
        cs->error_count++;
        return 0;
}

static guint
compiler_add_const (PclCompilerState *cs, PclObject *object)
{
        return compiler_add (cs, cs->constants, cs->const_dict, object);
}

static guint
compiler_add_name (PclCompilerState *cs, PclObject *object)
{
        return compiler_add (cs, cs->names, cs->name_dict, object);
}

static glong
compiler_lookup_index (PclObject *dict, PclObject *name)
{
        PclObject *object;

        g_assert (PCL_IS_STRING (name));
        object = pcl_dict_get_item (dict, name);
        if (object == NULL)
                return 0;
        return pcl_int_as_long (object);
}

static void
compiler_add_variable (PclCompilerState *cs, gint action, const gchar *name)
{
        gint source = SOURCE_DEFAULT;
        gint opcode = PCL_OPCODE_STOP;
        PclObject *object;
        gint reftype;
        glong index;

        object = pcl_string_intern_from_string (name);

        reftype = compiler_get_ref_type (cs, object);
        switch (reftype)
        {
                case REF_LOCAL:
                {
                        PclScope *scope;
                        scope = cs->symbol_table->current_scope->data;
                        if (scope->type == PCL_SCOPE_TYPE_FUNCTION)
                                source = SOURCE_LOCAL;
                        break;
                }

                case REF_GLOBAL_EXPLICIT:
                case REF_GLOBAL_IMPLICIT:
                        source = SOURCE_GLOBAL;
                        break;

                case REF_FREE:
                case REF_CELL:
                        source = SOURCE_CLOSURE;
                        break;
        }

        index = compiler_add_name (cs, object);
        if (source == SOURCE_LOCAL)
                index = compiler_lookup_index (cs->locals, object);
        else if (reftype == REF_FREE)
                index = compiler_lookup_index (cs->freevars, object);
        else if (reftype == REF_CELL)
                index = compiler_lookup_index (cs->cellvars, object);

        pcl_object_unref (object);

        switch (action)
        {
                case ACTION_LOAD:
                        switch (source)
                        {
                                case SOURCE_LOCAL:
                                        opcode = PCL_OPCODE_LOAD_FAST;
                                        break;
                                case SOURCE_GLOBAL:
                                        opcode = PCL_OPCODE_LOAD_GLOBAL;
                                        break;
                                case SOURCE_DEFAULT:
                                        opcode = PCL_OPCODE_LOAD_LOCAL;
                                        break;
                                case SOURCE_CLOSURE:
                                        opcode = PCL_OPCODE_LOAD_DEREF;
                                        break;
                        }
                        break;

                case ACTION_STORE:
                        switch (source)
                        {
                                case SOURCE_LOCAL:
                                        opcode = PCL_OPCODE_STORE_FAST;
                                        break;
                                case SOURCE_GLOBAL:
                                        opcode = PCL_OPCODE_STORE_GLOBAL;
                                        break;
                                case SOURCE_DEFAULT:
                                        opcode = PCL_OPCODE_STORE_LOCAL;
                                        break;
                                case SOURCE_CLOSURE:
                                        opcode = PCL_OPCODE_STORE_DEREF;
                                        break;
                        }
                        break;

                case ACTION_DELETE:
                        switch (source)
                        {
                                case SOURCE_LOCAL:
                                        opcode = PCL_OPCODE_DELETE_FAST;
                                        break;
                                case SOURCE_GLOBAL:
                                        opcode = PCL_OPCODE_DELETE_GLOBAL;
                                        break;
                                case SOURCE_DEFAULT:
                                        opcode = PCL_OPCODE_DELETE_LOCAL;
                                        break;
                                case SOURCE_CLOSURE:
                                        compiler_error (cs,
                                                pcl_exception_syntax_error (),
                                                DEL_CLOSURE_ERROR_MSG, name);
                                        break;
                        }
                        break;
        }

        compiler_add_oparg (cs, opcode, index);
}

static void
compiler_stack_push (PclCompilerState *cs, gint n)
{
        cs->stack_level += n;
        if (cs->stack_level > cs->max_stack_level)
                cs->max_stack_level = cs->stack_level;
}

static void
compiler_stack_pop (PclCompilerState *cs, gint n)
{
        if (cs->stack_level < n)
                cs->stack_level = 0;
        else
                cs->stack_level -= n;
}

static void
compiler_block_push (PclCompilerState *cs, guint type)
{
        if (cs->block_count < PCL_MAX_BLOCKS)
                cs->block[cs->block_count++] = type;
        else
                compiler_error (cs,
                        pcl_exception_system_error (),
                        "too many statically nested blocks");
}

static void
compiler_block_pop (PclCompilerState *cs, guint type)
{
        if (cs->block_count > 0)
                cs->block_count--;
        if (cs->block[cs->block_count] != type && cs->error_count == 0)
                compiler_error (cs,
                        pcl_exception_system_error (),
                        "bad block pop");
}

static gboolean
compiler_make_closure (PclCompilerState *cs, PclCode *code)
{
        glong ii, length;

        length = PCL_CODE_GET_NUM_FREE (code);
        if (length == 0)
                return FALSE;

        for (ii = 0; ii < length; ++ii)
        {
                PclObject *name;
                gint reftype;
                glong index;

                name = PCL_TUPLE_GET_ITEM (code->freevars, ii);
                reftype = compiler_get_ref_type (cs, name);
                if (reftype == REF_CELL)
                        index = compiler_lookup_index (cs->cellvars, name);
                else  /* (reftype == REF_FREE) */
                        index = compiler_lookup_index (cs->freevars, name);

                compiler_add_oparg (cs, PCL_OPCODE_LOAD_CLOSURE, index);
        }
        compiler_stack_push (cs, length);
        return TRUE;
}

/*** Begin parse tree node handler functions ***/

static void
compiler_name (PclCompilerState *cs, GNode *node)
{
        const gchar *name;

        name = PCL_NODE_STRING (node);
        compiler_add_variable (cs, ACTION_LOAD, name);
        compiler_stack_push (cs, 1);
}

static void
compiler_none (PclCompilerState *cs)
{
        guint index;

        index = compiler_add_const (cs, PCL_NONE);
        compiler_add_oparg (cs, PCL_OPCODE_LOAD_CONST, index);
        compiler_stack_push (cs, 1);
}

static void
compiler_float (PclCompilerState *cs, GNode *node)
{
        const gchar *string;
        PclObject *object;
        guint index;

        string = PCL_NODE_STRING (node);
        object = pcl_float_from_string (string, NULL);
        if (object == NULL)
        {
                cs->error_count++;
                return;
        }

        index = compiler_add_const (cs, object);
        pcl_object_unref (object);

        compiler_add_oparg (cs, PCL_OPCODE_LOAD_CONST, index);
        compiler_stack_push (cs, 1);
}

static void
compiler_imaginary (PclCompilerState *cs, GNode *node)
{
        const gchar *string;
        PclObject *object;
        guint index;

        string = PCL_NODE_STRING (node);
        object = pcl_complex_from_string (string, NULL);
        if (object == NULL)
        {
                cs->error_count++;
                return;
        }

        index = compiler_add_const (cs, object);
        pcl_object_unref (object);

        compiler_add_oparg (cs, PCL_OPCODE_LOAD_CONST, index);
        compiler_stack_push (cs, 1);
}

static void
compiler_integer (PclCompilerState *cs, GNode *node)
{
        const gchar *string;
        PclObject *object;
        guint index;

        string = PCL_NODE_STRING (node);
        object = pcl_int_from_string (string, NULL, 0);
        if (object == NULL)
        {
                cs->error_count++;
                return;
        }

        index = compiler_add_const (cs, object);
        pcl_object_unref (object);

        compiler_add_oparg (cs, PCL_OPCODE_LOAD_CONST, index);
        compiler_stack_push (cs, 1);
}

static void
compiler_string (PclCompilerState *cs, GNode *node)
{
        const gchar *string;
        PclObject *object;
        guint index;

        string = PCL_NODE_STRING (node);
        object = pcl_string_from_string (string);
        if (object == NULL)
        {
                cs->error_count++;
                return;
        }

        index = compiler_add_const (cs, object);
        pcl_object_unref (object);

        compiler_add_oparg (cs, PCL_OPCODE_LOAD_CONST, index);
        compiler_stack_push (cs, 1);
}

static void
compiler_attribute (PclCompilerState *cs, GNode *node, gint action)
{
        GNode *child = g_node_first_child (node);
        GNode *last_child = g_node_last_child (node);
        const gchar *name;
        PclObject *object;
        guint index;

        /* primary (name)+ */
        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_ATTRIBUTE);

        /* compile primary */
        compiler_node (cs, child);
        child = g_node_next_sibling (child);

        /* load intermediate attributes */
        while (child != last_child)
        {
                name = PCL_NODE_STRING (child);
                object = pcl_string_from_string (name);
                index = compiler_add_name (cs, object);
                compiler_add_oparg (cs, PCL_OPCODE_LOAD_ATTR, index);
                pcl_object_unref (object);

                child = g_node_next_sibling (child);
        }

        /* compile final attribute */
        name = PCL_NODE_STRING (child);
        object = pcl_string_from_string (name);
        index = compiler_add_name (cs, object);
        pcl_object_unref (object);
        switch (action)
        {
                case ACTION_LOAD:
                        compiler_add_oparg (cs, PCL_OPCODE_LOAD_ATTR, index);
                        break;

                case ACTION_STORE:
                        compiler_add_oparg (cs, PCL_OPCODE_STORE_ATTR, index);
                        compiler_stack_pop (cs, 2);
                        break;

                case ACTION_DELETE:
                        compiler_add_oparg (cs, PCL_OPCODE_DELETE_ATTR, index);
                        compiler_stack_pop (cs, 1);
                        break;

                default:
                        g_assert_not_reached ();
        }
}

static void
compiler_slicing (PclCompilerState *cs, GNode *node, gint action)
{
        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_SLICING);

        /* primary, lower_bound, upper_bound, stride */
        compiler_node (cs, g_node_nth_child (node, 0));
        compiler_node (cs, g_node_nth_child (node, 1));
        compiler_node (cs, g_node_nth_child (node, 2));
        compiler_node (cs, g_node_nth_child (node, 3));

        /* XXX Argument is unnecessary at present. */
        compiler_add_oparg (cs, PCL_OPCODE_BUILD_SLICE, 3);
        compiler_stack_pop (cs, 2);

        switch (action)
        {
                case ACTION_LOAD:
                        compiler_add_byte (cs, PCL_OPCODE_LOAD_SUBSCRIPT);
                        compiler_stack_pop (cs, 1);
                        break;

                case ACTION_STORE:
                        compiler_add_byte (cs, PCL_OPCODE_STORE_SUBSCRIPT);
                        compiler_stack_pop (cs, 3);
                        break;

                case ACTION_DELETE:
                        compiler_add_byte (cs, PCL_OPCODE_DELETE_SUBSCRIPT);
                        compiler_stack_pop (cs, 2);
                        break;

                default:
                        g_assert_not_reached ();
        }
}

static void
compiler_subscript (PclCompilerState *cs, GNode *node, gint action)
{
        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_SUBSCRIPT);

        /* primary, expression */
        compiler_node (cs, g_node_nth_child (node, 0));
        compiler_node (cs, g_node_nth_child (node, 1));

        switch (action)
        {
                case ACTION_LOAD:
                        compiler_add_byte (cs, PCL_OPCODE_LOAD_SUBSCRIPT);
                        compiler_stack_pop (cs, 1);
                        break;

                case ACTION_STORE:
                        compiler_add_byte (cs, PCL_OPCODE_STORE_SUBSCRIPT);
                        compiler_stack_pop (cs, 3);
                        break;

                case ACTION_DELETE:
                        compiler_add_byte (cs, PCL_OPCODE_DELETE_SUBSCRIPT);
                        compiler_stack_pop (cs, 2);
                        break;

                default:
                        g_assert_not_reached ();
        }
}

static void
compiler_assign_target (PclCompilerState *cs, GNode *target, gint action)
{
        g_assert (action == ACTION_STORE || action == ACTION_DELETE);

        switch (PCL_NODE_TYPE (target))
        {
                case PCL_NODE_TYPE_ATTRIBUTE:
                        compiler_attribute (cs, target, action);
                        break;

                case PCL_NODE_TYPE_NAME:
                {
                        const gchar *name = PCL_NODE_STRING (target);
                        compiler_add_variable (cs, action, name);
                        compiler_stack_pop (cs, 1);
                        break;
                }

                case PCL_NODE_TYPE_LIST:
                case PCL_NODE_TYPE_TUPLE:
                {
                        GNode *child = g_node_first_child (target);
                        g_assert (child != NULL);  /* guaranteed by parser */
                        if (action == ACTION_STORE)
                        {
                                gint opcode = PCL_OPCODE_UNPACK_SEQUENCE;
                                guint n_children = g_node_n_children (target);
                                compiler_add_oparg (cs, opcode, n_children);
                                compiler_stack_push (cs, n_children - 1);
                        }
                        while (child != NULL)
                        {
                                compiler_assign_target (cs, child, action);
                                child = g_node_next_sibling (child);
                        }
                        break;
                }

                case PCL_NODE_TYPE_SLICING:
                        compiler_slicing (cs, target, action);
                        break;

                case PCL_NODE_TYPE_SUBSCRIPT:
                        compiler_subscript (cs, target, action);
                        break;

                default:
                        g_assert_not_reached ();
        }
}

static void
compiler_assign (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);
        GNode *last_child = g_node_last_child (node);
        gboolean saved_frozen = cs->lineno_frozen;

        /* (target '=')+ expression */
        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_ASSIGN);

        /* We need to freeze the line number because we compile the right
         * side of the assignment before the left side.  If the right side
         * spans multiple lines then the line number table will try to write
         * a negative line number increment when we process the left side.
         * This will cause an assertion failure in the compiler. */
        cs->lineno_frozen = TRUE;
        compiler_node (cs, last_child);
        cs->lineno_frozen = saved_frozen;
        while (child != last_child)
        {
                if (g_node_next_sibling (child) != last_child)
                {
                        compiler_add_byte (cs, PCL_OPCODE_DUP_TOP);
                        compiler_stack_push (cs, 1);
                }
                compiler_assign_target (cs, child, ACTION_STORE);
                child = g_node_next_sibling (child);
        }
}

static void
compiler_aug_assign (PclCompilerState *cs, GNode *node)
{
        GNode *target = g_node_nth_child (node, 0);
        GNode *expression = g_node_nth_child (node, 1);
        gboolean saved_frozen = cs->lineno_frozen;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_AUG_ASSIGN);

        /* XXX: slicings are not yet supported */
        g_assert (PCL_NODE_TYPE (target) != PCL_NODE_TYPE_SLICING);

        /* We need to freeze the line number because we compile the right
         * side of the assignment before the left side.  If the right side
         * spans multiple lines then the line number table will try to write
         * a negative line number increment when we process the left side.
         * This will cause an assertion failure in the compiler. */
        cs->lineno_frozen = TRUE;
        compiler_node (cs, target);
        compiler_node (cs, expression);
        cs->lineno_frozen = saved_frozen;
        compiler_add_byte (cs, PCL_NODE_OPCODE (expression));
        compiler_stack_pop (cs, 1);
        compiler_assign_target (cs, target, ACTION_STORE);
}

static void
compiler_comparison (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);
        GNode *last_child = g_node_last_child (node);
        guint anchor = 0;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_COMPARISON);

        compiler_node (cs, child);
        child = g_node_next_sibling (child);

        /* The following code is generated for all but the last
         * comparison in a chain:
         *
         * label:       on stack:       opcode:         jump to:
         *
         *              a               <code to load b>
         *              a, b            DUP_TOP
         *              a, b, b         ROT_THREE
         *              b, a, b         <compare op>
         *              b, 0-or-1       JUMP_IF_FALSE   L1
         *              b, 1            POP_TOP
         *              b
         *
         * We are now ready to repeat this sequence for the next
         * comparison in the chain.
         *
         * For the last we generate:
         *
         *              b               <code to load c>
         *              b, c            <compare op>
         *              0-or-1
         *
         * If there were any jumps to L1 (i.e., there was more than one
         * comparison), we generate:
         *
         *              0-or-1          JUMP_FORWARD    L2
         * L1:          b, 0            ROT_TWO
         *              0, b            POP_TOP
         *              0
         * L2:          0-or-1
         */

        while (child != NULL)
        {
                compiler_node (cs, child);
                if (child != last_child)
                {
                        compiler_add_byte (cs, PCL_OPCODE_DUP_TOP);
                        compiler_stack_push (cs, 1);
                        compiler_add_byte (cs, PCL_OPCODE_ROTATE_3);
                }
                compiler_add_byte (cs, PCL_NODE_OPCODE (child));
                compiler_stack_pop (cs, 1);
                if (child != last_child)
                {
                        compiler_add_fwref (cs,
                                PCL_OPCODE_JUMP_IF_FALSE, &anchor);
                        compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                        compiler_stack_pop (cs, 1);
                }
                child = g_node_next_sibling (child);
        }

        if (anchor != 0)
        {
                guint anchor2 = 0;
                compiler_add_fwref (cs, PCL_OPCODE_JUMP_FORWARD, &anchor2);
                compiler_backpatch (cs, anchor);
                compiler_add_byte (cs, PCL_OPCODE_ROTATE_2);
                compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                compiler_backpatch (cs, anchor2);
        }
}

static void
compiler_unary_oper (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_UNARY_OPER);

        compiler_node (cs, child);
        compiler_add_byte (cs, PCL_NODE_OPCODE (child));
}

static void
compiler_binary_oper (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_BINARY_OPER);

        compiler_node (cs, child);
        child = g_node_next_sibling (child);

        while (child != NULL)
        {
                compiler_node (cs, child);
                compiler_add_byte (cs, PCL_NODE_OPCODE (child));
                compiler_stack_pop (cs, 1);
                child = g_node_next_sibling (child);
        }
}

static void
compiler_test_and (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);
        GNode *last_child = g_node_last_child (node);
        guint anchor = 0;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_TEST_AND); 

        compiler_node (cs, child);
        while (child != last_child)
        {
                compiler_add_fwref (cs, PCL_OPCODE_JUMP_IF_FALSE, &anchor);
                compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                compiler_stack_pop (cs, 1);
                child = g_node_next_sibling (child);
                compiler_node (cs, child);
        }

        if (anchor != 0)
                compiler_backpatch (cs, anchor);
}

static void
compiler_test_not (PclCompilerState *cs, GNode *node)
{
        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_TEST_NOT);

        compiler_node (cs, g_node_first_child (node));
        compiler_add_byte (cs, PCL_OPCODE_UNARY_NOT);
}

static void
compiler_test_or (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);
        GNode *last_child = g_node_last_child (node);
        guint anchor = 0;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_TEST_OR);

        compiler_node (cs, child);
        while (child != last_child)
        {
                compiler_add_fwref (cs, PCL_OPCODE_JUMP_IF_TRUE, &anchor);
                compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                compiler_stack_pop (cs, 1);
                child = g_node_next_sibling (child);
                compiler_node (cs, child);
        }

        if (anchor != 0)
                compiler_backpatch (cs, anchor);
}

static void
compiler_arguments (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_ARGUMENTS);

        /* The PAIRS flag indicates that these are keyword arguments. */
        if (PCL_NODE_FLAGS (node) & PCL_NODE_FLAG_PAIRS)
                g_assert (g_node_n_children (node) % 2 == 0);

        while (child != NULL)
        {
                if (PCL_NODE_FLAGS (node) & PCL_NODE_FLAG_PAIRS)
                {
                        /* compile keyword */
                        compiler_string (cs, child);
                        child = g_node_next_sibling (child);
                }

                /* compile expression */
                compiler_node (cs, child);
                child = g_node_next_sibling (child);
        }
}

static void
compiler_assert (PclCompilerState *cs, GNode *node)
{
        guint anchor = 0;
        guint index;
        PclObject *name;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_ASSERT);

        name = pcl_string_intern_from_string ("AssertionError");
        index = compiler_add_name (cs, name);
        pcl_object_unref (name);

        compiler_node (cs, g_node_nth_child (node, 0));
        compiler_add_fwref (cs, PCL_OPCODE_JUMP_IF_TRUE, &anchor);
        compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
        compiler_stack_pop (cs, 1);
        compiler_add_oparg (cs, PCL_OPCODE_LOAD_GLOBAL, index);
        compiler_stack_push (cs, 1);
        compiler_node (cs, g_node_nth_child (node, 1));  /* expr | None */
        compiler_node (cs, g_node_nth_child (node, 2));  /* always None */
        compiler_add_byte (cs, PCL_OPCODE_RAISE_EXCEPTION);
        compiler_stack_pop (cs, 3);
        /* The interpreter does not fall through. */
        compiler_backpatch (cs, anchor);
        compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
}

static void
compiler_break (PclCompilerState *cs, GNode *node)
{
        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_BREAK);

        if (cs->loop_count > 0)
                compiler_add_byte (cs, PCL_OPCODE_BREAK_LOOP);
        else
                compiler_error (cs,
                        pcl_exception_syntax_error (),
                        "'break' outside loop");
}

static void
compiler_call (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);
        guint n_children = g_node_n_children (node);

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_CALL);

        if (n_children == 1)
        {
                /* primary '(' ')' */
                compiler_node (cs, child);
                compiler_add_oparg (cs, PCL_OPCODE_CALL_FUNCTION, 0);
        }
        else if (n_children == 2)
        {
                /* primary generator_expression */
                compiler_node (cs, child);
                compiler_node (cs, g_node_next_sibling (child));
                compiler_add_oparg (cs, PCL_OPCODE_CALL_FUNCTION, 1);
                compiler_stack_pop (cs, 1);
        }
        else
        {
                gboolean epa, eka;
                guint npa, nka;
                gint opcode;

                g_assert (n_children == 5);

                /* compile primary */
                compiler_node (cs, child);

                /* compile positional arguments */
                child = g_node_next_sibling (child);
                if ((npa = g_node_n_children (child)) > 0)
                        compiler_node (cs, child);

                /* compile keyword arguments */
                child = g_node_next_sibling (child);
                if ((nka = g_node_n_children (child)) > 0)
                        compiler_node (cs, child);

                /* compile excess positional arguments */
                child = g_node_next_sibling (child);
                epa = (PCL_NODE_TYPE (child) != PCL_NODE_TYPE_NONE);
                if (epa) compiler_node (cs, child);

                /* compile excess keyword arguments */
                child = g_node_next_sibling (child);
                eka = (PCL_NODE_TYPE (child) != PCL_NODE_TYPE_NONE);
                if (eka) compiler_node (cs, child);

                if (epa)
                        opcode = eka ? PCL_OPCODE_CALL_FUNCTION_VA_KW
                                     : PCL_OPCODE_CALL_FUNCTION_VA;
                else
                        opcode = eka ? PCL_OPCODE_CALL_FUNCTION_KW
                                     : PCL_OPCODE_CALL_FUNCTION;
                compiler_add_oparg (cs, opcode, npa | ((nka / 2) << 8));
                compiler_stack_pop (cs, npa + nka + epa + eka);
        }
}

static void
compiler_class (PclCompilerState *cs, GNode *node)
{
        PclCode *code;
        gboolean closure;
        const gchar *name;
        guint index;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_CLASS);

        name = PCL_NODE_STRING (node);

        compiler_string (cs, node);  /* load class name */
        compiler_node (cs, g_node_nth_child (node, 0));

        pcl_symbol_table_enter_scope (cs->symbol_table, node);
        code = pcl_compile (node, cs->filename, cs);
        pcl_symbol_table_leave_scope (cs->symbol_table);

        if (code == NULL)
        {
                cs->error_count++;
                return;
        }

        closure = compiler_make_closure (cs, code);
        index = compiler_add_const (cs, PCL_OBJECT (code));
        compiler_add_oparg (cs, PCL_OPCODE_LOAD_CONST, index);
        compiler_stack_push (cs, 1);

        if (closure)
        {
                compiler_add_oparg (cs, PCL_OPCODE_MAKE_CLOSURE, 0);
                compiler_stack_pop (cs, PCL_CODE_GET_NUM_FREE (code));
        }
        else
                compiler_add_oparg (cs, PCL_OPCODE_MAKE_FUNCTION, 0);

        compiler_add_oparg (cs, PCL_OPCODE_CALL_FUNCTION, 0);
        compiler_add_byte (cs, PCL_OPCODE_BUILD_CLASS);
        compiler_stack_pop (cs, 2);
        compiler_add_variable (cs, ACTION_STORE, name);
        compiler_stack_pop (cs, 1);

        pcl_object_unref (code);
}

/* helper for compiler_comprehension */
static void
compiler_list_clause (PclCompilerState *cs, GNode *node, GNode *clause)
{
        /* sanity check */
        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_COMPREHENSION);
        g_assert (clause == NULL || g_node_is_ancestor (node, clause));

        if (clause == NULL)
        {
                const gchar *name = PCL_NODE_STRING (node);
                compiler_add_variable (cs, ACTION_LOAD, name);
                compiler_stack_push (cs, 1);
                compiler_node (cs, g_node_first_child (node));
                compiler_add_byte (cs, PCL_OPCODE_LIST_APPEND);
                compiler_stack_pop (cs, 2);
                return;
        }
        else if (PCL_NODE_TYPE (clause) == PCL_NODE_TYPE_FOR)
        {
                guint anchor = 0;
                guint save_begin = cs->loop_begin;
                GNode *target = g_node_nth_child (clause, 0);
                GNode *source = g_node_nth_child (clause, 1);
                compiler_node (cs, source);
                compiler_add_byte (cs, PCL_OPCODE_GET_ITER);
                cs->loop_begin = cs->next_instruction;
                compiler_add_fwref (cs, PCL_OPCODE_FOR_ITER, &anchor);
                compiler_stack_push (cs, 1);
                compiler_assign_target (cs, target, ACTION_STORE);
                cs->loop_count++;
                compiler_list_clause (cs, node, g_node_next_sibling (clause));
                cs->loop_count--;
                compiler_add_oparg (cs, PCL_OPCODE_JUMP_ABSOLUTE,
                                   cs->loop_begin);
                cs->loop_begin = save_begin;
                compiler_backpatch (cs, anchor);
                compiler_stack_pop (cs, 1);
                return;
        }
        else if (PCL_NODE_TYPE (clause) == PCL_NODE_TYPE_IF)
        {
                guint anchor1 = 0;
                guint anchor2 = 0;
                compiler_node (cs, g_node_first_child (clause));
                compiler_add_fwref (cs, PCL_OPCODE_JUMP_IF_FALSE, &anchor2);
                compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                compiler_stack_pop (cs, 1);
                compiler_list_clause (cs, node, g_node_next_sibling (clause));
                compiler_add_fwref (cs, PCL_OPCODE_JUMP_FORWARD, &anchor1);
                compiler_backpatch (cs, anchor2);
                compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                compiler_backpatch (cs, anchor1);
                return;
        }

        compiler_error (cs,
                pcl_exception_system_error (),
                "invalid list comprehension node type (%s)",
                pcl_node_type_name (PCL_NODE_TYPE (clause)));
}

static void
compiler_comprehension (PclCompilerState *cs, GNode *node)
{
        const gchar *name;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_COMPREHENSION);

        name = PCL_NODE_STRING (node);
        compiler_add_oparg (cs, PCL_OPCODE_BUILD_LIST, 0);
        compiler_add_byte (cs, PCL_OPCODE_DUP_TOP);
        compiler_stack_push (cs, 2);
        compiler_add_variable (cs, ACTION_STORE, name);
        compiler_stack_pop (cs, 1);
        compiler_list_clause (cs, node, g_node_nth_child (node, 1));
        compiler_add_variable (cs, ACTION_DELETE, name);
}

static void
compiler_conditional (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);
        guint anchor1 = 0;
        guint anchor2 = 0;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_CONDITIONAL);

        /* evaluate condition */
        compiler_node (cs, child);
        compiler_add_fwref (cs, PCL_OPCODE_JUMP_IF_FALSE, &anchor1);
        child = g_node_next_sibling (child);

        /* condition is true */
        compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
        compiler_stack_pop (cs, 1);
        compiler_node (cs, child);
        compiler_add_fwref (cs, PCL_OPCODE_JUMP_FORWARD, &anchor2);
        child = g_node_next_sibling (child);

        /* condition is false */
        compiler_backpatch (cs, anchor1);
        compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
        /* stack size is already adjusted from previous POP_TOP */
        compiler_node (cs, child);
        /* only one expression will be pushed on the stack */
        compiler_stack_pop (cs, 1);
        compiler_backpatch (cs, anchor2);
}

static void
compiler_conjunction (PclCompilerState *cs, GNode *node)
{
        PclCode *code;
        gboolean closure;
        guint index;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_CONJUNCTION);

        pcl_symbol_table_enter_scope (cs->symbol_table, NULL);
        code = pcl_compile (node, cs->filename, cs);
        pcl_symbol_table_leave_scope (cs->symbol_table);

        if (code == NULL)
        {
                cs->error_count++;
                return;
        }

        closure = compiler_make_closure (cs, code);
        index = compiler_add_const (cs, PCL_OBJECT (code));
        compiler_add_oparg (cs, PCL_OPCODE_LOAD_CONST, index);
        compiler_stack_push (cs, 1);

        if (closure)
        {
                compiler_add_oparg (cs, PCL_OPCODE_MAKE_CLOSURE, 0);
                compiler_stack_pop (cs, PCL_CODE_GET_NUM_FREE (code));
        }
        else
                compiler_add_oparg (cs, PCL_OPCODE_MAKE_FUNCTION, 0);

        compiler_add_oparg (cs, PCL_OPCODE_CALL_FUNCTION, 0);
        compiler_add_oparg (cs, PCL_OPCODE_UNPACK_SEQUENCE, 2);
        compiler_stack_push (cs, 1);

        pcl_object_unref (code);
}

static void
compiler_continue (PclCompilerState *cs, GNode *node)
{
        gint index;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_CONTINUE);

        /* TODO: Try to move some of these errors into the parser. */

        if (cs->block_count == 0)
        {
                compiler_error (cs,
                        pcl_exception_syntax_error (),
                        "'continue' not properly in loop");
                return;
        }

        index = cs->block_count - 1;
        if (cs->block[index] == PCL_OPCODE_SETUP_LOOP)
                compiler_add_oparg (cs, PCL_OPCODE_JUMP_ABSOLUTE,
                        cs->loop_begin);
        else
        {
                gint setup_loop;
                for (setup_loop = index - 1; setup_loop >= 0; setup_loop--)
                        if (cs->block[setup_loop] == PCL_OPCODE_SETUP_LOOP)
                                break;
                if (setup_loop < 0)
                {
                        compiler_error (cs,
                                pcl_exception_syntax_error (),
                                "'continue' not properly in loop");
                        return;
                }
                while (index > setup_loop)
                {
                        if (cs->block[index] == PCL_OPCODE_SETUP_EXCEPT ||
                                cs->block[index] == PCL_OPCODE_SETUP_FINALLY)
                        {
                                compiler_add_oparg (cs,
                                        PCL_OPCODE_CONTINUE_LOOP,
                                        cs->loop_begin);
                                return;
                        }
                        if (cs->block[index] == PCL_OPCODE_END_FINALLY)
                        {
                                compiler_error (cs,
                                        pcl_exception_syntax_error (),
                                        "'continue' not supported inside "
                                        "'finally' clause");
                                return;
                        }
                        index--;
                }
        }
}

static void
compiler_cut (PclCompilerState *cs, GNode *node)
{
        guint index;
        guint depth;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_CUT);

        index = compiler_add_const (cs, PCL_TRUE);
        depth = (guint) PCL_NODE_OPCODE (node);
        compiler_add_oparg (cs, PCL_OPCODE_LOAD_CONST, index);
        compiler_stack_push (cs, 1);
        compiler_add_oparg (cs, PCL_OPCODE_STORE_CUT, depth);
        compiler_stack_pop (cs, 1);
}

static void
compiler_decorators (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_DECORATORS);

        while (child != NULL)
        {
                compiler_node (cs, child);
                child = g_node_next_sibling (child);
        }
}

static void
compiler_del (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_DEL);

        while (child != NULL)
        {
                compiler_assign_target (cs, child, ACTION_DELETE);
                child = g_node_next_sibling (child);
        }
}

static void
compiler_dict (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);
        guint n_children = g_node_n_children (node);

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_DICT);
        g_assert (n_children % 2 == 0);

        while (child != NULL)
        {
                compiler_node (cs, child);  /* key */
                child = g_node_next_sibling (child);
                compiler_node (cs, child);  /* datum */
                child = g_node_next_sibling (child);
        }

        compiler_add_oparg (cs, PCL_OPCODE_BUILD_DICT, n_children / 2);
        if (n_children > 0)
                compiler_stack_pop (cs, n_children - 1);
        else
                compiler_stack_push (cs, 1);
}

static void
compiler_disjunction (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);
        guint anchor1 = 0;
        guint anchor2 = 0;
        guint index;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_DISJUNCTION);

        /* setup accumulator */
        index = compiler_add_const (cs, PCL_FALSE);
        compiler_add_oparg (cs, PCL_OPCODE_LOAD_CONST, index);
        compiler_stack_push (cs, 1);

        while (child != NULL)
        {
                PclNodeType type = PCL_NODE_TYPE (child);

                compiler_node (cs, child);
                child = g_node_next_sibling (child);

                if (type == PCL_NODE_TYPE_CONJUNCTION)
                {
                        /* merge conjunction's accumulator */
                        /* XXX this is ugly, find a better way:
                         *                      [ACCUM, CUT, SUCCESS]
                         *     ROTATE_3         [SUCCESS, ACCUM, CUT]
                         *     ROTATE_3         [CUT, SUCCESS, ACCUM]
                         *     DISJUNCT         [CUT, ACCUM]
                         *     ROTATE_2         [ACCUM, CUT]
                         */
                        compiler_add_byte (cs, PCL_OPCODE_ROTATE_3);
                        compiler_add_byte (cs, PCL_OPCODE_ROTATE_3);
                        compiler_add_byte (cs, PCL_OPCODE_DISJUNCT);
                        compiler_stack_pop (cs, 1);
                        compiler_add_byte (cs, PCL_OPCODE_ROTATE_2);

                        /* jump if sibling cut seen */
                        compiler_add_fwref (cs,
                                PCL_OPCODE_JUMP_IF_TRUE, &anchor1);
                        compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                        compiler_stack_pop (cs, 1);
                }
        }

        compiler_add_fwref (cs, PCL_OPCODE_JUMP_FORWARD, &anchor2);
        compiler_backpatch (cs, anchor1);
        compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
        compiler_backpatch (cs, anchor2);
}

static void
compiler_exec (PclCompilerState *cs, GNode *node)
{
        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_EXEC);

        compiler_node (cs, g_node_nth_child (node, 0));
        compiler_node (cs, g_node_nth_child (node, 1));
        if (g_node_n_children (node) == 3)
                compiler_node (cs, g_node_nth_child (node, 2));
        else
        {
                compiler_add_byte (cs, PCL_OPCODE_DUP_TOP);
                compiler_stack_push (cs, 1);
        }
        compiler_add_byte (cs, PCL_OPCODE_EXEC_STMT);
        compiler_stack_pop (cs, 3);
}

static void
compiler_expression (PclCompilerState *cs, GNode *node)
{
        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_EXPRESSION);

        compiler_node (cs, g_node_first_child (node));

        /* The value left on the stack after an expression statement has
         * special meaning in conjunctions, so leave it alone. */
        if (cs->flags & PCL_CODE_FLAG_CONJUNCTION)
                return;

        if (cs->interactive > 0)
                compiler_add_byte (cs, PCL_OPCODE_PRINT_EXPR);
        else
                compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
        compiler_stack_pop (cs, 1);
}

static void
compiler_for (PclCompilerState *cs, GNode *node)
{
        guint break_anchor = 0;
        guint anchor = 0;
        guint save_begin = cs->loop_begin;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_FOR);

        /* KW_FOR target_list KW_IN expression_list suite [else_clause] */

        /* setup looping structure */
        compiler_add_fwref (cs, PCL_OPCODE_SETUP_LOOP, &break_anchor);
        compiler_block_push (cs, PCL_OPCODE_SETUP_LOOP);
        compiler_node (cs, g_node_nth_child (node, 1));
        compiler_add_byte (cs, PCL_OPCODE_GET_ITER);
        cs->loop_begin = cs->next_instruction;

        /* assign next item in sequence */
        compiler_add_fwref (cs, PCL_OPCODE_FOR_ITER, &anchor);
        compiler_stack_push (cs, 1);
        compiler_assign_target (cs, g_node_nth_child (node, 0), ACTION_STORE);

        /* execute loop body */
        cs->loop_count++;
        compiler_node (cs, g_node_nth_child (node, 2));
        cs->loop_count--;

        if (cs->flags & PCL_CODE_FLAG_CONJUNCTION)
        {
                /* discard conjunction's accumulator */
                compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                compiler_stack_pop (cs, 1);

                /* remember if sibling cut has been seen */
                compiler_add_byte (cs, PCL_OPCODE_LOAD_CUT);
                compiler_stack_push (cs, 1);
                compiler_add_byte (cs, PCL_OPCODE_DISJUNCT);
                compiler_stack_pop (cs, 1);
                compiler_add_oparg (cs, PCL_OPCODE_STORE_CUT, 0);
                compiler_stack_pop (cs, 1);
        }

        /* next iteration */
        compiler_add_oparg (cs, PCL_OPCODE_JUMP_ABSOLUTE, cs->loop_begin);

        /* iteration complete */
        cs->loop_begin = save_begin;
        compiler_backpatch (cs, anchor);
        compiler_stack_pop (cs, 1);  /* FOR_ITER has popped this */
        compiler_add_byte (cs, PCL_OPCODE_POP_BLOCK);
        compiler_block_pop (cs, PCL_OPCODE_SETUP_LOOP);

        /* handle else clause */
        if (g_node_n_children (node) == 4)
                compiler_node (cs, g_node_nth_child (node, 3));

        compiler_backpatch (cs, break_anchor);
}

static void
compiler_function (PclCompilerState *cs, GNode *node)
{
        PclCode *code;
        GNode *decorators;
        GNode *defaults;
        guint n_decorators;
        guint n_defaults;
        const gchar *name;
        gboolean closure;
        guint index;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_FUNCTION ||
                  PCL_NODE_TYPE (node) == PCL_NODE_TYPE_PREDICATE);

        name = PCL_NODE_STRING (node);

        decorators = g_node_first_child (node);
        n_decorators = g_node_n_children (decorators);
        if (PCL_NODE_TYPE (decorators) == PCL_NODE_TYPE_DECORATORS)
        {
                GNode *child = g_node_first_child (decorators);
                while (child != NULL)
                {
                        compiler_node (cs, child);
                        child = g_node_next_sibling (child);
                }
        }

        defaults = g_node_nth_child (node, 2);
        n_defaults = g_node_n_children (defaults) / 2;
        if (PCL_NODE_TYPE (defaults) == PCL_NODE_TYPE_PARAMETERS)
        {
                GNode *child = g_node_first_child (defaults);
                while (child != NULL)
                {
                        child = g_node_next_sibling (child);
                        compiler_node (cs, child);
                        child = g_node_next_sibling (child);
                }
        }

        pcl_symbol_table_enter_scope (cs->symbol_table, NULL);
        code = pcl_compile (node, cs->filename, cs);
        pcl_symbol_table_leave_scope (cs->symbol_table);

        if (code == NULL)
        {
                cs->error_count++;
                return;
        }

        closure = compiler_make_closure (cs, code);
        index = compiler_add_const (cs, PCL_OBJECT (code));
        compiler_add_oparg (cs, PCL_OPCODE_LOAD_CONST, index);
        compiler_stack_push (cs, 1);

        if (closure)
        {
                compiler_add_oparg (cs, PCL_OPCODE_MAKE_CLOSURE, n_defaults);
                compiler_stack_pop (cs, PCL_CODE_GET_NUM_FREE (code));
        }
        else
                compiler_add_oparg (cs, PCL_OPCODE_MAKE_FUNCTION, n_defaults);
        compiler_stack_pop (cs, n_defaults);

        while (n_decorators > 0)
        {
                compiler_add_oparg (cs, PCL_OPCODE_CALL_FUNCTION, 1);
                compiler_stack_pop (cs, 1);
                n_decorators--;
        }

        compiler_add_variable (cs, ACTION_STORE, name);
        compiler_stack_pop (cs, 1);

        pcl_object_unref (code);
}

static void
compiler_generator (PclCompilerState *cs, GNode *node)
{
        PclCode *code;
        GNode *outmost;
        gboolean closure;
        guint index;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_GENERATOR);

        pcl_symbol_table_enter_scope (cs->symbol_table, NULL);
        code = pcl_compile (node, cs->filename, cs);
        pcl_symbol_table_leave_scope (cs->symbol_table);

        if (code == NULL)
        {
                cs->error_count++;
                return;
        }

        closure = compiler_make_closure (cs, code);
        index = compiler_add_const (cs, PCL_OBJECT (code));
        compiler_add_oparg (cs, PCL_OPCODE_LOAD_CONST, index);
        compiler_stack_push (cs, 1);

        if (closure)
        {
                compiler_add_oparg (cs, PCL_OPCODE_MAKE_CLOSURE, 0);
                compiler_stack_pop (cs, PCL_CODE_GET_NUM_FREE (code));
        }
        else
                compiler_add_oparg (cs, PCL_OPCODE_MAKE_FUNCTION, 0);

        outmost = g_node_nth_child (node, 1);
        compiler_node (cs, g_node_last_child (outmost));
        compiler_add_byte (cs, PCL_OPCODE_GET_ITER);
        compiler_add_oparg (cs, PCL_OPCODE_CALL_FUNCTION, 1);
        compiler_stack_pop (cs, 1);

        pcl_object_unref (code);
}

static void
compiler_if (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);
        guint anchor1 = 0;
        guint anchor2 = 0;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_IF);

        /* KW_IF '(' expression ')' suite [alternative_clause] */

        /* evaluate condition */
        compiler_node (cs, child);
        compiler_add_fwref (cs, PCL_OPCODE_JUMP_IF_FALSE, &anchor1);
        child = g_node_next_sibling (child);

        /* condition is true */
        compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
        compiler_stack_pop (cs, 1);
        compiler_node (cs, child);
        if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_CONJUNCTION)
        {
                /* discard conjunction's accumulator */
                compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                compiler_stack_pop (cs, 1);

                /* remember if sibling cut has been seen */
                compiler_add_byte (cs, PCL_OPCODE_LOAD_CUT);
                compiler_stack_push (cs, 1);
                compiler_add_byte (cs, PCL_OPCODE_DISJUNCT);
                compiler_stack_pop (cs, 1);
                compiler_add_oparg (cs, PCL_OPCODE_STORE_CUT, 0);
                compiler_stack_pop (cs, 1);
        }
        compiler_add_fwref (cs, PCL_OPCODE_JUMP_FORWARD, &anchor2);
        child = g_node_next_sibling (child);

        /* condition is false */
        compiler_backpatch (cs, anchor1);
        compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
        /* stack size is already adjusted from previous POP_TOP */
        if (child != NULL)
        {
                compiler_node (cs, g_node_nth_child (node, 2));
                if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_CONJUNCTION)
                {
                        /* discard conjunction's accumulator */
                        compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                        compiler_stack_pop (cs, 1);

                        /* remember if sibling cut has been seen */
                        compiler_add_byte (cs, PCL_OPCODE_LOAD_CUT);
                        compiler_stack_push (cs, 1);
                        compiler_add_byte (cs, PCL_OPCODE_DISJUNCT);
                        compiler_stack_pop (cs, 1);
                        compiler_add_oparg (cs, PCL_OPCODE_STORE_CUT, 0);
                        compiler_stack_pop (cs, 1);
                }
        }

        compiler_backpatch (cs, anchor2);
}

static void
compiler_import (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);
        guint n_children = g_node_n_children (node);
        const gchar *name;
        PclObject *object;
        guint index;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_IMPORT);

        if (n_children % 2 == 0)
                goto import_name;

        /* from ... import ... */

        /* create a tuple of names to define */
        if (n_children == 1)
                object = pcl_build_value ("(s)", "*");
        else
        {
                glong ii, length = (n_children - 1) / 2;
                object = pcl_tuple_new (length);
                child = g_node_next_sibling (child);
                for (ii = 0; ii < length; ii++)
                {
                        PclObject *item;
                        name = PCL_NODE_STRING (child);
                        item = pcl_string_from_string (name);
                        if (item == NULL)
                        {
                                PCL_CLEAR (object);
                                break;
                        }
                        PCL_TUPLE_SET_ITEM (object, ii, item);
                        child = g_node_next_sibling (child);
                        child = g_node_next_sibling (child);
                }
                child = g_node_first_child (node);  /* reset child */
        }
        g_assert (object != NULL);  /* XXX */
        index = compiler_add_const (cs, object);
        compiler_add_oparg (cs, PCL_OPCODE_LOAD_CONST, index);
        compiler_stack_push (cs, 1);
        pcl_object_unref (object);

        /* import module */
        name = PCL_NODE_STRING (g_node_first_child (child));
        object = pcl_string_from_string (name);
        index = compiler_add_name (cs, object);
        compiler_add_oparg (cs, PCL_OPCODE_IMPORT_NAME, index);
        pcl_object_unref (object);

        if (n_children == 1)
                compiler_add_byte (cs, PCL_OPCODE_IMPORT_STAR);
        else
        {
                child = g_node_next_sibling (child);
                while (child != NULL)
                {
                        name = PCL_NODE_STRING (child);
                        object = pcl_string_from_string (name);
                        index = compiler_add_name (cs, object);
                        compiler_add_oparg (cs, PCL_OPCODE_IMPORT_FROM, index);
                        compiler_stack_push (cs, 1);
                        pcl_object_unref (object);

                        name = PCL_NODE_STRING (child);
                        child = g_node_next_sibling (child);
                        if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_NAME)
                                name = PCL_NODE_STRING (child);
                        object = pcl_string_from_string (name);
                        index = compiler_add_name (cs, object);
                        compiler_add_oparg (cs, PCL_OPCODE_STORE_LOCAL, index);
                        compiler_stack_pop (cs, 1);
                        pcl_object_unref (object);

                        child = g_node_next_sibling (child);
                }
                compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
        }
        compiler_stack_pop (cs, 1);
        return;

import_name:

        while (child != NULL)
        {
                /* The children are in pairs: (MODULE, (NAME | NONE)) */
                GNode *alias, *module = child;
                GNode *package = g_node_first_child (module);

                alias = child = g_node_next_sibling (child);

                compiler_none (cs);

                name = PCL_NODE_STRING (package);
                object = pcl_string_from_string (name);
                index = compiler_add_name (cs, object);
                compiler_add_oparg (cs, PCL_OPCODE_IMPORT_NAME, index);
                pcl_object_unref (object);

                if (PCL_NODE_TYPE (alias) == PCL_NODE_TYPE_NAME)
                {
                        GNode *submodule = g_node_next_sibling (package);
                        while (submodule != NULL)
                        {
                                name = PCL_NODE_STRING (submodule);
                                object = pcl_string_from_string (name);
                                index = compiler_add_name (cs, object);
                                pcl_object_unref (object);
                                compiler_add_oparg (
                                        cs, PCL_OPCODE_LOAD_ATTR, index);
                                submodule = g_node_next_sibling (submodule);
                        }
                        name = PCL_NODE_STRING (alias);
                }
                else
                        name = PCL_NODE_STRING (package);

                compiler_add_variable (cs, ACTION_STORE, name);
                compiler_stack_pop (cs, 1);

                child = g_node_next_sibling (child);
        }
}

static void
compiler_list (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);
        guint n_children = g_node_n_children (node);

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_LIST);

        while (child != NULL)
        {
                compiler_node (cs, child);
                child = g_node_next_sibling (child);
        }

        compiler_add_oparg (cs, PCL_OPCODE_BUILD_LIST, n_children);
        if (n_children > 0)
                compiler_stack_pop (cs, n_children - 1);
        else
                compiler_stack_push (cs, 1);
}

static void
compiler_print (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);
        gboolean extended_form = FALSE;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_PRINT);

        if (PCL_NODE_TYPE (child) != PCL_NODE_TYPE_NONE)
        {
                /* push file object */
                extended_form = TRUE;
                compiler_node (cs, child);
        }
        child = g_node_next_sibling (child);

        while (child != NULL)
        {
                if (extended_form)
                {
                        compiler_add_byte (cs, PCL_OPCODE_DUP_TOP);
                        compiler_stack_push (cs, 1);
                        compiler_node (cs, child);
                        compiler_add_byte (cs, PCL_OPCODE_ROTATE_2);
                        compiler_add_byte (cs, PCL_OPCODE_PRINT_ITEM_TO);
                        compiler_stack_pop (cs, 2);
                }
                else
                {
                        compiler_node (cs, child);
                        compiler_add_byte (cs, PCL_OPCODE_PRINT_ITEM);
                        compiler_stack_pop (cs, 1);
                }
                child = g_node_next_sibling (child);
        }

        if (PCL_NODE_FLAGS (node) & PCL_NODE_FLAG_COMMA)
        {
                if (extended_form)
                {
                        /* pop file object */
                        compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                        compiler_stack_pop (cs, 1);
                }
        }
        else
        {
                if (extended_form)
                {
                        /* consumes file object */
                        compiler_add_byte (cs, PCL_OPCODE_PRINT_NEWLINE_TO);
                        compiler_stack_pop (cs, 1);
                }
                else
                {
                        compiler_add_byte (cs, PCL_OPCODE_PRINT_NEWLINE);
                }
        }
}

static void
compiler_raise (PclCompilerState *cs, GNode *node)
{
        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_RAISE);

        compiler_node (cs, g_node_nth_child (node, 0));
        compiler_node (cs, g_node_nth_child (node, 1));
        compiler_node (cs, g_node_nth_child (node, 2));
        compiler_add_byte (cs, PCL_OPCODE_RAISE_EXCEPTION);
        compiler_stack_pop (cs, 3);
}

static void
compiler_return (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_RETURN);

        if (!cs->in_function)
                compiler_error (cs,
                        pcl_exception_syntax_error (),
                        "'return' outside function");
        if (cs->flags & PCL_CODE_FLAG_GENERATOR)
        {
                if (PCL_NODE_TYPE (child) != PCL_NODE_TYPE_NONE)
                        compiler_error (cs,
                                pcl_exception_syntax_error (),
                                "'return' with argument inside generator");
        }

        compiler_node (cs, child);
        compiler_add_byte (cs, PCL_OPCODE_RETURN_VALUE);
        compiler_stack_pop (cs, 1);
}

static void
compiler_suite (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_SUITE); 

        while (child != NULL)
        {
                compiler_node (cs, child);
                child = g_node_next_sibling (child);
        }
}

static void
compiler_try_except (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);
        guint except_anchor = 0;
        guint else_anchor = 0;
        guint end_anchor = 0;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_TRY_EXCEPT);

        compiler_add_fwref (cs, PCL_OPCODE_SETUP_EXCEPT, &except_anchor);
        compiler_block_push (cs, PCL_OPCODE_SETUP_EXCEPT);
        compiler_node (cs, child);
        child = g_node_next_sibling (child);
        compiler_add_byte (cs, PCL_OPCODE_POP_BLOCK);
        compiler_block_pop (cs, PCL_OPCODE_SETUP_EXCEPT);
        compiler_add_fwref (cs, PCL_OPCODE_JUMP_FORWARD, &else_anchor);
        compiler_backpatch (cs, except_anchor);
        while (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_EXCEPT)
        {
                guint n_children = g_node_n_children (child);

                except_anchor = 0;
                compiler_stack_push (cs, 3);  /* pushed by exception */
                if (n_children > 1)
                {
                        compiler_add_byte (cs, PCL_OPCODE_DUP_TOP);
                        compiler_stack_push (cs, 1);
                        compiler_node (cs, g_node_first_child (child));
                        compiler_add_byte (cs, PCL_OPCODE_COMPARISON_EXCEPT);
                        compiler_stack_pop (cs, 1);
                        compiler_add_fwref (cs,
                                           PCL_OPCODE_JUMP_IF_FALSE,
                                           &except_anchor);
                        compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                        compiler_stack_pop (cs, 1);
                }
                compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                compiler_stack_pop (cs, 1);
                if (n_children == 3)
                {
                        GNode *target = g_node_nth_child (child, 1);
                        compiler_assign_target (cs, target, ACTION_STORE);
                }
                else
                {
                        compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                        compiler_stack_pop (cs, 1);
                }
                compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                compiler_stack_pop (cs, 1);
                compiler_node (cs, g_node_last_child (child));
                compiler_add_fwref (cs, PCL_OPCODE_JUMP_FORWARD, &end_anchor);
                if (except_anchor != 0)
                {
                        compiler_backpatch (cs, except_anchor);
                        /* We come in with [tb, val, exc, False] on the stack;
                         * one pop and it's the same as expected at the start
                         * of the loop. */
                        compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                }
                child = g_node_next_sibling (child);
        }
        /* We actually come in here with [tb, val, exc], but the END_FINALLY
         * will zap those and jump around.  The stack level does not reflect
         * them so we need not pop anything. */
        compiler_add_byte (cs, PCL_OPCODE_END_FINALLY);
        compiler_backpatch (cs, else_anchor);
        if (PCL_NODE_TYPE (child) != PCL_NODE_TYPE_NONE)
                compiler_node (cs, child);  /* else */
        child = g_node_next_sibling (child);
        compiler_backpatch (cs, end_anchor);
        if (PCL_NODE_TYPE (child) != PCL_NODE_TYPE_NONE)
                compiler_node (cs, child);  /* finally */
}

static void
compiler_try_finally (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);
        guint finally_anchor = 0;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_TRY_FINALLY);

        compiler_add_fwref (cs, PCL_OPCODE_SETUP_FINALLY, &finally_anchor);
        compiler_block_push (cs, PCL_OPCODE_SETUP_FINALLY);
        compiler_node (cs, child);
        child = g_node_next_sibling (child);
        compiler_add_byte (cs, PCL_OPCODE_POP_BLOCK);
        compiler_block_pop (cs, PCL_OPCODE_SETUP_FINALLY);
        compiler_block_push (cs, PCL_OPCODE_END_FINALLY);
        compiler_none (cs);
        /* While the generated code pushes only one item, the try-finally
         * handling can enter here with up to three items:
         * 3 for an exception, 2 for RETURN, 1 for BREAK */
        compiler_stack_push (cs, 2);
        compiler_backpatch (cs, finally_anchor);
        compiler_node (cs, child);
        compiler_add_byte (cs, PCL_OPCODE_END_FINALLY);
        compiler_block_pop (cs, PCL_OPCODE_END_FINALLY);
        compiler_stack_pop (cs, 3);
}

static void
compiler_tuple (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);
        guint n_children = g_node_n_children (node);

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_TUPLE);

        while (child != NULL)
        {
                compiler_node (cs, child);
                child = g_node_next_sibling (child);
        }

        compiler_add_oparg (cs, PCL_OPCODE_BUILD_TUPLE, n_children);
        if (n_children > 0)
                compiler_stack_pop (cs, n_children - 1);
        else
                compiler_stack_push (cs, 1);
}

static void
compiler_while (PclCompilerState *cs, GNode *node)
{
        guint break_anchor = 0;
        guint anchor = 0;
        guint save_begin = cs->loop_begin;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_WHILE);

        /* KW_WHILE '(' expression ')' suite [else_clause] */

        /* setup looping structure */
        compiler_add_fwref (cs, PCL_OPCODE_SETUP_LOOP, &break_anchor);
        compiler_block_push (cs, PCL_OPCODE_SETUP_LOOP);
        cs->loop_begin = cs->next_instruction;

        /* evaluate condition */
        compiler_node (cs, g_node_nth_child (node, 0));
        compiler_add_fwref (cs, PCL_OPCODE_JUMP_IF_FALSE, &anchor);

        /* condition is true */
        compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
        compiler_stack_pop (cs, 1);
        cs->loop_count++;
        compiler_node (cs, g_node_nth_child (node, 1));
        cs->loop_count--;
        compiler_add_oparg (cs, PCL_OPCODE_JUMP_ABSOLUTE, cs->loop_begin);

        /* condition is false */
        cs->loop_begin = save_begin;
        compiler_backpatch (cs, anchor);
        compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
        /* stack size is already adjusted from previous POP_TOP */
        compiler_add_byte (cs, PCL_OPCODE_POP_BLOCK);
        compiler_block_pop (cs, PCL_OPCODE_SETUP_LOOP);

        /* handle else clause */
        if (g_node_n_children (node) == 3)
                compiler_node (cs, g_node_nth_child (node, 2));

        compiler_backpatch (cs, break_anchor);
}

static void
compiler_yield (PclCompilerState *cs, GNode *node)
{
        gint ii;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_YIELD);

        if (!cs->in_function)
                compiler_error (cs,
                        pcl_exception_syntax_error (),
                        "'yield' outside function");

        for (ii = 0; ii < cs->block_count; ii++)
        {
                if (cs->block[ii] == PCL_OPCODE_SETUP_FINALLY)
                {
                        compiler_error (cs, pcl_exception_syntax_error (),
                                "'yield' not allowed in a 'try' block with a"
                                "'finally' clause");
                        return;
                }
        }

        compiler_node (cs, g_node_first_child (node));
        compiler_add_byte (cs, PCL_OPCODE_YIELD_VALUE);
        compiler_stack_pop (cs, 1);
}

static void
compiler_node (PclCompilerState *cs, GNode *node)
{
        if (cs->error_count > 0)
                return;

        compiler_set_lineno (cs, node);

        switch (PCL_NODE_TYPE (node))
        {
                case PCL_NODE_TYPE_NAME:
                        compiler_name (cs, node);
                        break;

                case PCL_NODE_TYPE_FLOAT:
                        compiler_float (cs, node);
                        break;

                case PCL_NODE_TYPE_IMAGINARY:
                        compiler_imaginary (cs, node);
                        break;

                case PCL_NODE_TYPE_INTEGER:
                        compiler_integer (cs, node);
                        break;

                case PCL_NODE_TYPE_STRING:
                        compiler_string (cs, node);
                        break;

                case PCL_NODE_TYPE_ASSIGN:
                        compiler_assign (cs, node);
                        break;

                case PCL_NODE_TYPE_AUG_ASSIGN:
                        compiler_aug_assign (cs, node);
                        break;

                case PCL_NODE_TYPE_COMPARISON:
                        compiler_comparison (cs, node);
                        break;

                case PCL_NODE_TYPE_UNARY_OPER:
                        compiler_unary_oper (cs, node);
                        break;

                case PCL_NODE_TYPE_BINARY_OPER:
                        compiler_binary_oper (cs, node);
                        break;

                case PCL_NODE_TYPE_TEST_AND:
                        compiler_test_and (cs, node);
                        break;

                case PCL_NODE_TYPE_TEST_NOT:
                        compiler_test_not (cs, node);
                        break;

                case PCL_NODE_TYPE_TEST_OR:
                        compiler_test_or (cs, node);
                        break;

                case PCL_NODE_TYPE_ARGUMENTS:
                        compiler_arguments (cs, node);
                        break;

                case PCL_NODE_TYPE_ASSERT:
                        compiler_assert (cs, node);
                        break;

                case PCL_NODE_TYPE_ATTRIBUTE:
                        compiler_attribute (cs, node, ACTION_LOAD);
                        break;

                case PCL_NODE_TYPE_BREAK:
                        compiler_break (cs, node);
                        break;

                case PCL_NODE_TYPE_CALL:
                        compiler_call (cs, node);
                        break;

                case PCL_NODE_TYPE_CLASS:
                        compiler_class (cs, node);
                        break;

                case PCL_NODE_TYPE_COMPREHENSION:
                        compiler_comprehension (cs, node);
                        break;

                case PCL_NODE_TYPE_CONDITIONAL:
                        compiler_conditional (cs, node);
                        break;

                case PCL_NODE_TYPE_CONJUNCTION:
                        compiler_conjunction (cs, node);
                        break;

                case PCL_NODE_TYPE_CONTINUE:
                        compiler_continue (cs, node);
                        break;

                case PCL_NODE_TYPE_CUT:
                        compiler_cut (cs, node);
                        break;

                case PCL_NODE_TYPE_DECORATORS:
                        compiler_decorators (cs, node);
                        break;

                case PCL_NODE_TYPE_DEL:
                        compiler_del (cs, node);
                        break;

                case PCL_NODE_TYPE_DICT:
                        compiler_dict (cs, node);
                        break;

                case PCL_NODE_TYPE_DISJUNCTION:
                        compiler_disjunction (cs, node);
                        break;

                case PCL_NODE_TYPE_EXEC:
                        compiler_exec (cs, node);
                        break;

                case PCL_NODE_TYPE_EXPRESSION:
                        compiler_expression (cs, node);
                        break;

                case PCL_NODE_TYPE_FOR:
                        compiler_for (cs, node);
                        break;

                case PCL_NODE_TYPE_FUNCTION:
                        compiler_function (cs, node);
                        break;

                case PCL_NODE_TYPE_GENERATOR:
                        compiler_generator (cs, node);
                        break;

                case PCL_NODE_TYPE_GLOBAL:
                        /* handled by the symbol table */
                        break;

                case PCL_NODE_TYPE_IF:
                        compiler_if (cs, node);
                        break;

                case PCL_NODE_TYPE_IMPORT:
                        compiler_import (cs, node);
                        break;

                case PCL_NODE_TYPE_LIST:
                        compiler_list (cs, node);
                        break;

                case PCL_NODE_TYPE_NONE:
                        compiler_none (cs);
                        break;

                case PCL_NODE_TYPE_PASS:
                        /* no action */
                        break;

                case PCL_NODE_TYPE_PREDICATE:
                        compiler_function (cs, node);
                        break;

                case PCL_NODE_TYPE_PRINT:
                        compiler_print (cs, node);
                        break;

                case PCL_NODE_TYPE_RAISE:
                        compiler_raise (cs, node);
                        break;

                case PCL_NODE_TYPE_RETURN:
                        compiler_return (cs, node);
                        break;

                case PCL_NODE_TYPE_SLICING:
                        compiler_slicing (cs, node, ACTION_LOAD);
                        break;

                case PCL_NODE_TYPE_STATIC:
                        /* Handled by the symbol table */
                        break;

                case PCL_NODE_TYPE_SUBSCRIPT:
                        compiler_subscript (cs, node, ACTION_LOAD);
                        break;

                case PCL_NODE_TYPE_SUITE:
                        compiler_suite (cs, node);
                        break;

                case PCL_NODE_TYPE_TRY_EXCEPT:
                        compiler_try_except (cs, node);
                        break;

                case PCL_NODE_TYPE_TRY_FINALLY:
                        compiler_try_finally (cs, node);
                        break;

                case PCL_NODE_TYPE_TUPLE:
                        compiler_tuple (cs, node);
                        break;

                case PCL_NODE_TYPE_WHILE:
                        compiler_while (cs, node);
                        break;

                case PCL_NODE_TYPE_YIELD:
                        compiler_yield (cs, node);
                        break;

                default:
                        compiler_error (cs,
                                pcl_exception_system_error (),
                                "%s: unexpected node type (%s)", G_STRFUNC,
                                pcl_node_type_name (PCL_NODE_TYPE (node)));
                        break;
        }
}

static void
compiler_top_level_eval_input (PclCompilerState *cs, GNode *node)
{
        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_EVAL_INPUT);

        compiler_node (cs, g_node_first_child (node));
        compiler_add_byte (cs, PCL_OPCODE_RETURN_VALUE);
        compiler_stack_pop (cs, 1);
}

static void
compiler_top_level_file_input (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_FILE_INPUT);

        while (child != NULL)
        {
                compiler_node (cs, child);
                child = g_node_next_sibling (child);
        }
        compiler_strip_lineno_table (cs);
        compiler_none (cs);
        compiler_add_byte (cs, PCL_OPCODE_RETURN_VALUE);
        compiler_stack_pop (cs, 1);
}

static void
compiler_top_level_single_input (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_SINGLE_INPUT);

        cs->interactive++;
        while (child != NULL)
        {
                compiler_node (cs, child);
                child = g_node_next_sibling (child);
        }
        compiler_strip_lineno_table (cs);
        compiler_none (cs);
        compiler_add_byte (cs, PCL_OPCODE_RETURN_VALUE);
        compiler_stack_pop (cs, 1);
        cs->interactive--;
}

static void
compiler_top_level_class (PclCompilerState *cs, GNode *node)
{
        PclObject *name;
        guint index;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_CLASS);

        cs->name = PCL_NODE_STRING (node);

        name = pcl_string_intern_from_string ("__name__");
        index = compiler_add_name (cs, name);
        compiler_add_oparg (cs, PCL_OPCODE_LOAD_GLOBAL, index);
        compiler_stack_push (cs, 1);
        pcl_object_unref (name);

        name = pcl_string_intern_from_string ("__module__");
        index = compiler_add_name (cs, name);
        compiler_add_oparg (cs, PCL_OPCODE_STORE_LOCAL, index);
        compiler_stack_pop (cs, 1);
        pcl_object_unref (name);

        /* XXX Lookup docstring */
        (void) compiler_add_const (cs, PCL_NONE);

        compiler_node (cs, g_node_last_child (node));
        compiler_strip_lineno_table (cs);
        compiler_add_byte (cs, PCL_OPCODE_LOAD_LOCALS);
        compiler_stack_push (cs, 1);
        compiler_add_byte (cs, PCL_OPCODE_RETURN_VALUE);
        compiler_stack_pop (cs, 1);
}

static void
compiler_top_level_conjunction (PclCompilerState *cs, GNode *node)
{
        GNode *child = g_node_first_child (node);
        PclNodeType type = PCL_NODE_TYPE_INVALID;
        guint anchor = 0;
        guint index;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_CONJUNCTION);

        cs->name = "<conjunction>";
        cs->flags |= PCL_CODE_FLAG_CONJUNCTION;

        /* setup accumulator */
        index = compiler_add_const (cs, PCL_TRUE);
        compiler_add_oparg (cs, PCL_OPCODE_LOAD_CONST, index);
        compiler_stack_push (cs, 1);

        while (child != NULL)
        {
                compiler_node (cs, child);
                type = PCL_NODE_TYPE (child);
                if (type == PCL_NODE_TYPE_EXPRESSION ||
                        type == PCL_NODE_TYPE_DISJUNCTION)
                {
                        compiler_add_byte (cs, PCL_OPCODE_CONJUNCT);
                        compiler_stack_pop (cs, 1);
                        compiler_add_fwref (cs,
                                PCL_OPCODE_JUMP_IF_FALSE, &anchor);
                }
                child = g_node_next_sibling (child);
        }

        if (anchor != 0)
                compiler_backpatch (cs, anchor);

        /* return flags: cut, accumulator */
        compiler_add_byte (cs, PCL_OPCODE_LOAD_CUT);
        compiler_stack_push (cs, 1);
        compiler_add_oparg (cs, PCL_OPCODE_BUILD_TUPLE, 2);
        compiler_stack_pop (cs, 1);
        compiler_add_byte (cs, PCL_OPCODE_RETURN_VALUE);
        compiler_stack_pop (cs, 1);
}

static void
compiler_top_level_function (PclCompilerState *cs, GNode *node)
{
        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_FUNCTION);

        cs->name = PCL_NODE_STRING (node);

        cs->in_function = TRUE;
        compiler_node (cs, g_node_last_child (node));
        cs->in_function = FALSE;

        compiler_strip_lineno_table (cs);

        compiler_none (cs);
        compiler_add_byte (cs, PCL_OPCODE_RETURN_VALUE);
        compiler_stack_pop (cs, 1);
}

/* helper for compiler_top_level_generator */
static void
compiler_gen_clause (PclCompilerState *cs, GNode *node, GNode *clause)
{
        /* sanity check */
        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_GENERATOR);
        g_assert (clause == NULL || g_node_is_ancestor (node, clause));

        if (clause == NULL)
        {
                compiler_node (cs, g_node_first_child (node));
                compiler_add_byte (cs, PCL_OPCODE_YIELD_VALUE);
                compiler_stack_pop (cs, 1);
                return;
        }
        else if (PCL_NODE_TYPE (clause) == PCL_NODE_TYPE_FOR)
        {
                guint anchor = 0;
                guint break_anchor = 0;
                guint save_begin = cs->loop_begin;
                GNode *target = g_node_nth_child (clause, 0);
                GNode *source = g_node_nth_child (clause, 1);
                gboolean is_outmost = (clause == g_node_nth_child (node, 1));
                compiler_add_fwref (cs, PCL_OPCODE_SETUP_LOOP, &break_anchor);
                compiler_block_push (cs, PCL_OPCODE_SETUP_LOOP);
                if (is_outmost)
                {
                        const gchar *name = PCL_NODE_STRING (node);
                        compiler_add_variable (cs, ACTION_LOAD, name);
                        compiler_stack_push (cs, 1);
                }
                else
                {
                        compiler_node (cs, source);
                        compiler_add_byte (cs, PCL_OPCODE_GET_ITER);
                }
                cs->loop_begin = cs->next_instruction;
                compiler_add_fwref (cs, PCL_OPCODE_FOR_ITER, &anchor);
                compiler_stack_push (cs, 1);
                compiler_assign_target (cs, target, ACTION_STORE);
                compiler_gen_clause (cs, node, g_node_next_sibling (clause));
                compiler_add_oparg (cs, PCL_OPCODE_JUMP_ABSOLUTE,
                                   cs->loop_begin);
                cs->loop_begin = save_begin;
                compiler_backpatch (cs, anchor);
                compiler_stack_pop (cs, 1);  /* FOR_ITER has popped this */
                compiler_add_byte (cs, PCL_OPCODE_POP_BLOCK);
                compiler_block_pop (cs, PCL_OPCODE_SETUP_LOOP);
                compiler_backpatch (cs, break_anchor);
                return;
        }
        else if (PCL_NODE_TYPE (clause) == PCL_NODE_TYPE_IF)
        {
                guint anchor1 = 0;
                guint anchor2 = 0;
                compiler_node (cs, g_node_first_child (clause));
                compiler_add_fwref (cs, PCL_OPCODE_JUMP_IF_FALSE, &anchor2);
                compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                compiler_stack_pop (cs, 1);
                compiler_gen_clause (cs, node, g_node_next_sibling (clause));
                compiler_add_fwref (cs, PCL_OPCODE_JUMP_FORWARD, &anchor1);
                compiler_backpatch (cs, anchor2);
                compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                compiler_backpatch (cs, anchor1);
                return;
        }

        compiler_error (cs,
                pcl_exception_system_error (),
                "invalid generator expression node type (%s)",
                pcl_node_type_name (PCL_NODE_TYPE (clause)));
}

static void
compiler_top_level_generator (PclCompilerState *cs, GNode *node)
{
        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_GENERATOR);

        cs->name = "<generator expression>";
        compiler_gen_clause (cs, node, g_node_nth_child (node, 1));
        compiler_none (cs);
        compiler_add_byte (cs, PCL_OPCODE_RETURN_VALUE);
        compiler_stack_pop (cs, 1);
}

static void
compiler_top_level_predicate (PclCompilerState *cs, GNode *node)
{
        const gchar *goal_symbol = NULL;
        guint cut_anchor = 0;
        GNode *child;

        g_assert (PCL_NODE_TYPE (node) == PCL_NODE_TYPE_PREDICATE);

        cs->name = PCL_NODE_STRING (node);
        cs->flags |= PCL_CODE_FLAG_PREDICATE;

        /* setup goal value or accumulator */
        child = g_node_nth_child (node, 5);
        if (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_ASSIGN)
        {
                compiler_node (cs, child);
                goal_symbol = PCL_NODE_STRING (g_node_first_child (child));
        }
        else
        {
                guint index = compiler_add_const (cs, PCL_FALSE);
                compiler_add_oparg (cs, PCL_OPCODE_LOAD_CONST, index);
                compiler_stack_push (cs, 1);
        }

        child = g_node_last_child (node);
        g_assert (PCL_NODE_TYPE (child) == PCL_NODE_TYPE_SUITE);
        child = g_node_first_child (child);

        while (child != NULL)
        {
                PclNodeType type = PCL_NODE_TYPE (child);

                compiler_node (cs, child);
                child = g_node_next_sibling (child);

                if (type == PCL_NODE_TYPE_CONJUNCTION)
                {
                        if (goal_symbol == NULL)
                        {
                                /* merge conjunction's accumulator */
                                /* XXX this is ugly, find a better way:
                                 *                      [ACCUM, CUT, SUCCESS]
                                 *     ROTATE_3         [SUCCESS, ACCUM, CUT]
                                 *     ROTATE_3         [CUT, SUCCESS, ACCUM]
                                 *     DISJUNCT         [CUT, ACCUM]
                                 *     ROTATE_2         [ACCUM, CUT]
                                 */
                                compiler_add_byte (cs, PCL_OPCODE_ROTATE_3);
                                compiler_add_byte (cs, PCL_OPCODE_ROTATE_3);
                                compiler_add_byte (cs, PCL_OPCODE_DISJUNCT);
                                compiler_stack_pop (cs, 1);
                                compiler_add_byte (cs, PCL_OPCODE_ROTATE_2);
                        }
                        else
                        {
                                /* discard conjunction's accumulator */
                                compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                                compiler_stack_pop (cs, 1);
                        }

                        /* jump if sibling cut seen */
                        compiler_add_fwref (cs,
                                PCL_OPCODE_JUMP_IF_TRUE, &cut_anchor);
                        compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                        compiler_stack_pop (cs, 1);
                }
        }

        if (cut_anchor != 0)
        {
                guint anchor = 0;
                compiler_add_fwref (cs, PCL_OPCODE_JUMP_FORWARD, &anchor);
                compiler_backpatch (cs, cut_anchor);
                compiler_add_byte (cs, PCL_OPCODE_POP_TOP);
                compiler_backpatch (cs, anchor);
        }

        compiler_strip_lineno_table (cs);

        if (goal_symbol != NULL)
        {
                compiler_add_variable (cs, ACTION_LOAD, goal_symbol);
                compiler_stack_push (cs, 1);
        }
        compiler_add_byte (cs, PCL_OPCODE_RETURN_VALUE);
        compiler_stack_pop (cs, 1);
}

static void
compiler_top_level_node (PclCompilerState *cs, GNode *node)
{
        switch (PCL_NODE_TYPE (node))
        {
                case PCL_NODE_TYPE_EVAL_INPUT:
                        compiler_top_level_eval_input (cs, node);
                        break;

                case PCL_NODE_TYPE_FILE_INPUT:
                        compiler_top_level_file_input (cs, node);
                        break;

                case PCL_NODE_TYPE_SINGLE_INPUT:
                        compiler_top_level_single_input (cs, node);
                        break;

                case PCL_NODE_TYPE_CLASS:
                        compiler_top_level_class (cs, node);
                        break;

                case PCL_NODE_TYPE_CONJUNCTION:
                        compiler_top_level_conjunction (cs, node);
                        break;

                case PCL_NODE_TYPE_FUNCTION:
                        compiler_top_level_function (cs, node);
                        break;

                case PCL_NODE_TYPE_GENERATOR:
                        compiler_top_level_generator (cs, node);
                        break;

                case PCL_NODE_TYPE_PREDICATE:
                        compiler_top_level_predicate (cs, node);
                        break;

                default:
                        compiler_error (cs,
                                pcl_exception_system_error (),
                                "%s: unexpected node type (%s)", G_STRFUNC,
                                pcl_node_type_name (PCL_NODE_TYPE (node)));
                        break;
        }
}

static PclObject *
compiler_dict_keys_inorder (PclObject *dict, guint offset)
{
        PclObject *key;
        PclObject *tuple;
        PclObject *value;
        glong pos = 0;
        glong index;
        glong size;

        /* Pseudocode:
         *     list = [-1] * len (dict)
         *     for key, value in dict.items():
         *         list[value - offset] = key
         *     return tuple (list)
         */

        size = pcl_object_measure (dict);
        tuple = pcl_tuple_new (size);
        if (tuple == NULL)
                return NULL;
        while (pcl_dict_next (dict, &pos, &key, &value))
        {
                index = pcl_int_as_long (value);
                pcl_object_ref (key);
                g_assert ((index - offset) < size);
                PCL_TUPLE_SET_ITEM (tuple, index - offset, key);
        }
        return tuple;
}

static void
compiler_init (PclCompilerState *cs, const gchar *filename)
{
        memset (cs, 0, sizeof (PclCompilerState));
        cs->instructions = pcl_string_from_string ("");
        pcl_string_resize (cs->instructions, 1000);
        cs->constants = pcl_list_new (0);
        cs->const_dict = pcl_dict_new ();
        cs->names = pcl_list_new (0);
        cs->name_dict = pcl_dict_new ();
        cs->locals = pcl_dict_new ();
        cs->varnames = NULL;
        cs->freevars = NULL;
        cs->cellvars = NULL;
        cs->filename = filename;
        cs->name = "?";
        cs->stack_level = 0;
        cs->max_stack_level = 0;
        cs->lineno_table = pcl_string_from_string ("");
        pcl_string_resize (cs->lineno_table, 1000);
}

static void
compiler_free (PclCompilerState *cs)
{
        pcl_object_unref (cs->instructions);
        pcl_object_unref (cs->lineno_table);
        pcl_object_unref (cs->constants);
        pcl_object_unref (cs->const_dict);
        pcl_object_unref (cs->names);
        pcl_object_unref (cs->name_dict);
        pcl_object_unref (cs->globals);
        pcl_object_unref (cs->locals);
        pcl_object_unref (cs->varnames);
        pcl_object_unref (cs->freevars);
        pcl_object_unref (cs->cellvars);
}

static gboolean
compiler_resolve_free (PclCompilerState *cs, PclObject *name, glong flags,
                      struct symbol_info *si)
{
        PclObject *dict;
        PclObject *index;
        PclScope *current_scope;
        gboolean success;

        /* Seperate logic for PCL_SYMBOL_FLAG_DEF_FREE.  If it occurs in a
         * function, it indicates a local that we must allocate storage for
         * (a cell variable).  If it occurs in a class, then the class has a
         * method and a free variable with the same name. */

        current_scope = cs->symbol_table->current_scope->data;
        if (current_scope->type == PCL_SCOPE_TYPE_FUNCTION)
        {
                /* If it isn't declared locally, it can't be a cell. */
                if (!(flags & (PCL_SYMBOL_FLAG_DEF_LOCAL |
                                PCL_SYMBOL_FLAG_DEF_PARAM)))
                        return TRUE;
                index = pcl_int_from_long (si->ncells++);
                if (index == NULL)
                        return FALSE;
                dict = cs->cellvars;
        }
        else
        {
                /* If it is free anyway, then there is no need to do
                 * anything here. */
                if (pcl_symbol_is_free (
                                flags ^ PCL_SYMBOL_FLAG_DEF_FREE_CLASS) ||
                                (flags == PCL_SYMBOL_FLAG_DEF_FREE_CLASS))
                        return TRUE;
                index = pcl_int_from_long (si->nfrees++);
                if (index == NULL)
                        return FALSE;
                dict = cs->freevars;
        }

        if (index == NULL)
                return FALSE;
        success = pcl_dict_set_item (dict, name, index);
        pcl_object_unref (index);
        return success;
}

static gboolean
compiler_cellvar_offsets (PclCompilerState *cs)
{
        PclObject *celllist;
        PclObject *cellvars;
        PclObject *index;
        PclObject *name;
        glong ncellvars;
        glong ii, length;
        glong pos = 0;
        glong argc;

        /* Pseudocode:
         *     celllist = []
         *     argc = cs.argument_count
         *     if cs.flags & PCL_CODE_FLAG_VARARGS:
         *         argc += 1
         *     if cs.flags & PCL_CODE_FLAG_VARKWDS:
         *         argc += 1
         *     for ii in range (argc):
         *         name = cs.varnames[ii]
         *         if name in cs.cellvars:
         *             celllist.append (name)
         *     if len(celllist) == 0:
         *         return
         *     cellvars = {}
         *     for index, name in enumerate (celllist):
         *         cellvars[name] = index
         *         del cs.cellvars[name]
         *     ncellvars = len (celllist)
         *     for name in cs.cellvars.keys ():
         *         cellvars[name] = ncellvars
         *         ncellvars += 1
         *     cs.cellvars = cellvars
         */

        celllist = NULL;
        argc = cs->argument_count;
        if (cs->flags & PCL_CODE_FLAG_VARARGS)
                argc++;
        if (cs->flags & PCL_CODE_FLAG_VARKWDS)
                argc++;
        for (ii = 0; ii < argc; ++ii)
        {
                name = PCL_LIST_GET_ITEM (cs->varnames, ii);
                if (pcl_dict_get_item (cs->cellvars, name) != NULL)
                {
                        if (celllist == NULL)
                        {
                                celllist = pcl_list_new (1);
                                if (celllist == NULL)
                                        return FALSE;
                                pcl_object_ref (name);
                                PCL_LIST_SET_ITEM (celllist, 0, name);
                        }
                        else
                        {
                                if (!pcl_list_insert (celllist, 0, name))
                                {
                                        pcl_object_unref (celllist);
                                        return FALSE;
                                }
                        }
                }
        }

        if (celllist == NULL)
                return TRUE;

        cellvars = pcl_dict_new ();
        if (cellvars == NULL)
                return FALSE;
        length = PCL_LIST_GET_SIZE (celllist);
        for (ii = 0; ii < length; ++ii)
        {
                index = pcl_int_from_long (ii);
                if (index == NULL)
                        goto fail;
                name = PCL_LIST_GET_ITEM (celllist, ii);
                if (!pcl_dict_set_item (cellvars, name, index))
                        goto fail;
                if (!pcl_dict_del_item (cs->cellvars, name))
                        goto fail;
                pcl_object_unref (index);
        }

        pcl_object_unref (celllist);

        ncellvars = length;
        while (pcl_dict_next (cs->cellvars, &pos, &name, NULL))
        {
                index = pcl_int_from_long (ncellvars++);
                if (index == NULL)
                        goto fail;
                if (!pcl_dict_set_item (cellvars, name, index))
                        goto fail;
                pcl_object_unref (index);
        }

        pcl_object_unref (cs->cellvars);
        cs->cellvars = cellvars;
        return TRUE;

fail:
        pcl_object_unref (cellvars);
        if (index != NULL)
                pcl_object_unref (index);
        return FALSE;
}

static gboolean
compiler_freevar_offsets (PclObject *freevars, guint offset)
{
        PclObject *index;
        PclObject *name;
        glong pos = 0;

        /* Pseudocode:
         *     for name, index in freevars:
         *         freevars[name] = index + offset
         */

        while (pcl_dict_next (freevars, &pos, &name, &index))
        {
                PclObject *new_index;
                glong v_long;

                v_long = PCL_INT_AS_LONG (index) + offset;
                new_index = pcl_int_from_long (v_long);
                if (new_index == NULL)
                        return FALSE;
                if (!pcl_dict_set_item (freevars, name, new_index))
                {
                        pcl_object_unref (new_index);
                        return FALSE;
                }
                pcl_object_unref (new_index);
        }
        return TRUE;
}

static void
compiler_update_flags (PclCompilerState *cs, PclScope *scope,
                      struct symbol_info *si)
{
        if (scope->flags & PCL_SCOPE_FLAG_GENERATOR)
                cs->flags |= PCL_CODE_FLAG_GENERATOR;
        if (scope->type != PCL_SCOPE_TYPE_MODULE)
                cs->flags |= PCL_CODE_FLAG_NEWLOCALS;
        if (scope->type == PCL_SCOPE_TYPE_FUNCTION)
                cs->variable_count = si->nlocals;
}

static gboolean
compiler_load_symbols (PclCompilerState *cs)
{
        PclSymbolTable *st;
        PclScope *scope;
        PclObject *index = NULL;
        PclObject *name = NULL;
        struct symbol_info *si;
        GSList *names, *link;
        gint flags;

        st = cs->symbol_table;
        scope = st->current_scope->data;
        names = pcl_scope_get_names (scope);
        si = g_new0 (struct symbol_info, 1);

        cs->varnames = pcl_list_new (0);
        cs->globals = pcl_dict_new ();
        cs->freevars = pcl_dict_new ();
        cs->cellvars = pcl_dict_new ();

        /* Add parameters to the varnames list and locals dictionary. */
        for (link = scope->params; link != NULL; link = link->next)
        {
                name = pcl_string_intern_from_string (link->data);
                if (name == NULL)
                        goto fail;
                index = pcl_int_from_long ((glong) si->nlocals++);
                if (index == NULL)
                        goto fail;
                if (!pcl_list_append (cs->varnames, name))
                        goto fail;
                if (!pcl_dict_set_item (cs->locals, name, index))
                        goto fail;
                pcl_object_unref (index);
                pcl_object_unref (name);
        }

        cs->argument_count = si->nlocals;

        /* Add the rest of the symbols to the varnames list as well as
         * other dictionaries where appropriate. */
        for (link = names; link != NULL; link = link->next)
        {
                name = pcl_string_intern_from_string (link->data);
                flags = pcl_scope_get_flags (scope, link->data);

                if (flags & PCL_SYMBOL_FLAG_DEF_FREE_GLOBAL)
                        flags &= ~(PCL_SYMBOL_FLAG_DEF_FREE |
                                PCL_SYMBOL_FLAG_DEF_FREE_CLASS);

                /* Deal with names that need two actions:
                 *   1. Cell variables that are also locals.
                 *   2. Free variables in methods that are also class
                 *      variables or declared global. */
                if (flags & (PCL_SYMBOL_FLAG_DEF_FREE |
                                PCL_SYMBOL_FLAG_DEF_FREE_CLASS))
                        if (!compiler_resolve_free (cs, name, flags, si))
                                goto fail;

                if (flags & PCL_SYMBOL_FLAG_DEF_STAR)
                {
                        cs->argument_count--;
                        cs->flags |= PCL_CODE_FLAG_VARARGS;
                }
                else if (flags & PCL_SYMBOL_FLAG_DEF_DOUBLE_STAR)
                {
                        cs->argument_count--;
                        cs->flags |= PCL_CODE_FLAG_VARKWDS;
                }
                else if (flags & PCL_SYMBOL_FLAG_DEF_GLOBAL)
                {
                        g_assert (!(flags & PCL_SYMBOL_FLAG_DEF_PARAM));
                        if (!pcl_dict_set_item (cs->globals, name, PCL_NONE))
                                goto fail;
                }
                else if (flags & PCL_SYMBOL_FLAG_DEF_FREE_GLOBAL)
                {
                        si->nimplicit++;
                        if (!pcl_dict_set_item (cs->globals, name, PCL_TRUE))
                                goto fail;
                }
                else if ((flags & PCL_SYMBOL_FLAG_DEF_LOCAL) &&
                        !(flags & PCL_SYMBOL_FLAG_DEF_PARAM))
                {
                        index = pcl_int_from_long (si->nlocals++);
                        if (index == NULL)
                                goto fail;
                        if (!pcl_dict_set_item (cs->locals, name, index))
                                goto fail;
                        pcl_object_unref (index);
                        if (scope->type != PCL_SCOPE_TYPE_CLASS)
                                if (!pcl_list_append (cs->varnames, name))
                                        goto fail;
                }
                else if (pcl_symbol_is_free (flags))
                {
                        if (scope->flags & PCL_SCOPE_FLAG_NESTED)
                        {
                                index = pcl_int_from_long (si->nfrees++);
                                if (index == NULL)
                                        goto fail;
                                if (!pcl_dict_set_item (cs->freevars,
                                                        name, index))
                                        goto fail;
                                pcl_object_unref (index);
                        }
                        else
                        {
                                si->nimplicit++;
                                if (!pcl_dict_set_item (cs->globals,
                                                        name, PCL_TRUE))
                                        goto fail;
                                if (st->current_scope != st->global_scope)
                                        pcl_scope_set_flags (
                                                st->global_scope->data,
                                                link->data, flags);
                        }
                }

                pcl_object_unref (name);
        }
        g_slist_free (names);
        g_assert (pcl_object_measure (cs->freevars) == si->nfrees);

        if (si->ncells > 1)
        {
                if (!compiler_cellvar_offsets (cs))
                        return FALSE;
        }
        if (!compiler_freevar_offsets (cs->freevars, si->ncells))
                return FALSE;
        compiler_update_flags (cs, scope, si);
        g_free (si);
        return TRUE;

fail:

        if (index != NULL)
                pcl_object_unref (index);
        if (name != NULL)
                pcl_object_unref (name);
        g_slist_free (names);
        return FALSE;
}

PclCode *
pcl_compile (GNode *parse_tree, const gchar *filename, PclCompilerState *base)
{
        PclCompilerState cs;
        PclCode *code = NULL;

        compiler_init (&cs, filename);
        if (base == NULL)
                cs.symbol_table = pcl_symbol_table_new (parse_tree);
        else
        {
                PclScope *scope;
                cs.symbol_table = base->symbol_table;
                scope = cs.symbol_table->current_scope->data;
                cs.nested = base->nested ||
                        (scope->type == PCL_SCOPE_TYPE_FUNCTION);
                cs.flags |= base->flags;
        }

        if (compiler_load_symbols (&cs))
        {
                compiler_top_level_node (&cs, parse_tree);
                compiler_done (&cs);
        }
        else
                cs.error_count++;

        if (cs.stack_level != 0)
        {
                gchar *contents = pcl_node_contents (PCL_NODE (parse_tree));
                g_warning ("%s: Stack level is (%d) after compiling (%s)",
                        G_STRFUNC, cs.stack_level, contents);
                g_free (contents);
        }

        if (cs.error_count == 0)
        {
                /* Build a code object from the compile-state struct. */
                PclObject *constants, *names, *varnames, *freevars;
                PclObject *cellvars, *filename, *name, *instructions;
                constants = pcl_list_as_tuple (cs.constants);
                names = pcl_list_as_tuple (cs.names);
                varnames = pcl_list_as_tuple (cs.varnames);
                freevars = compiler_dict_keys_inorder (cs.freevars, 0);
                cellvars = compiler_dict_keys_inorder (cs.cellvars,
                                pcl_object_measure (freevars));
                filename = pcl_string_intern_from_string (cs.filename);
                name = pcl_string_intern_from_string (cs.name);
                instructions = pcl_optimize (
                        cs.instructions, cs.constants,
                        names, cs.lineno_table);
                
                if (!pcl_error_occurred ())
                        code = pcl_code_new (cs.argument_count,
                                             cs.variable_count,
                                             cs.max_stack_level,
                                             instructions,
                                             cs.lineno_table,
                                             constants,
                                             names,
                                             varnames,
                                             freevars,
                                             cellvars,
                                             filename,
                                             name,
                                             cs.first_lineno,
                                             cs.flags);
                if (constants != NULL)
                        pcl_object_unref (constants);
                if (names != NULL)
                        pcl_object_unref (names);
                if (varnames != NULL)
                        pcl_object_unref (varnames);
                if (freevars != NULL)
                        pcl_object_unref (freevars);
                if (cellvars != NULL)
                        pcl_object_unref (cellvars);
                if (filename != NULL)
                        pcl_object_unref (filename);
                if (name != NULL)
                        pcl_object_unref (name);
                if (instructions != NULL)
                        pcl_object_unref (instructions);
        }
        else if (!pcl_error_occurred ())
        {
                pcl_error_set_string (
                        pcl_exception_system_error (),
                        "lost syntax error");
        }

        if (base == NULL)
        {
                pcl_symbol_table_destroy (cs.symbol_table);
                cs.symbol_table = NULL;
        }
        compiler_free (&cs);
        return code;
}
