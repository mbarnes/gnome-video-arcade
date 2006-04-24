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
#include "pcl-opcode.h"

#define GET_ARG(bytes, offset) \
        ((guint) ((bytes[offset + 2] << 8) + bytes[offset + 1]))
#define SET_ARG(bytes, offset, value) \
        bytes[offset + 2] = value >> 8; \
        bytes[offset + 1] = value & 255;
#define ABSOLUTE_JUMP(opcode) \
        (opcode == PCL_OPCODE_JUMP_ABSOLUTE || \
         opcode == PCL_OPCODE_CONTINUE_LOOP)
#define UNCONDITIONAL_JUMP(opcode) \
        (opcode == PCL_OPCODE_JUMP_ABSOLUTE || \
         opcode == PCL_OPCODE_JUMP_FORWARD)
#define GET_JUMP_TARGET(bytes, offset) \
        (GET_ARG (bytes, offset) + \
        (ABSOLUTE_JUMP (bytes[offset]) ? 0 : offset + 3))
#define IS_BASIC_BLOCK(blocks, start, offset) \
        (blocks[start] == blocks[start + offset - 1])
#define OPCODE_SIZE(opcode) \
        (PCL_OPCODE_HAS_ARG (opcode) ? 3 : 1)

typedef struct _OptimizerState OptimizerState;

struct _OptimizerState
{
        PclObject *constants;
        PclObject *names;
        guchar *bytes;
        guint *blocks;
        glong n_bytes;
        gint cum_load_const;
        gint last_load_const;
};

static gboolean
optimizer_tuple_of_constants (guchar *bytes, glong size, PclObject *constants)
{
        /* Original                     Modified
         * ---------------------------  ---------------------------
         * LOAD_CONST c1                LOAD_CONST (c1, c2, ... cn)
         * LOAD_CONST c2
         * ...
         * LOAD_CONST cn
         * BUILD_TUPLE n
         *
         * The constants object must still be in list form so that the
         * new constant (c1, c2, ... cn) can be appended.
         *
         * Called with bytes pointing to the first LOAD_CONST.  Bails out
         * with no change if one or more of the LOAD_CONSTs is missing.
         */

        PclObject *object;
        PclObject *tuple;
        guint arg, index;
        glong ii;

        /* pre-conditions */
        g_assert (PCL_IS_LIST (constants));
        g_assert (bytes[size * 3] == PCL_OPCODE_BUILD_TUPLE);
        g_assert (GET_ARG (bytes, (size * 3)) == size);
        for (ii = 0; ii < size; ii++)
                g_assert (bytes[ii * 3] == PCL_OPCODE_LOAD_CONST);

        /* Build a new tuple of constants. */
        tuple = pcl_tuple_new (size);
        if (tuple == NULL)
                return FALSE;
        index = PCL_LIST_GET_SIZE (constants);
        for (ii = 0; ii < size; ii++)
        {
                arg = GET_ARG (bytes, (ii * 3));
                g_assert (arg < index);
                object = PCL_LIST_GET_ITEM (constants, arg);
                PCL_TUPLE_SET_ITEM (tuple, ii, pcl_object_ref (object));
        }

        /* Append folded constant to constants list. */
        if (!pcl_list_append (constants, tuple))
        {
                pcl_object_unref (tuple);
                return FALSE;
        }
        pcl_object_unref (tuple);

        /* Write NOPs over old LOAD_CONSTs and add a new LOAD_CONST on top
         * of the BUILD_TUPLE. */
        memset (bytes, PCL_OPCODE_NONE, size * 3);
        bytes[size * 3] = PCL_OPCODE_LOAD_CONST;
        SET_ARG (bytes, (size * 3), index);
        return TRUE;
}

static void
optimizer_mark_blocks (OptimizerState *os)
{
        guchar *bytes = os->bytes;
        glong ii, n_bytes = os->n_bytes;
        guint opcode, block_number = 0;

        os->blocks = g_new0 (guint, n_bytes);

        /* Mark labels in the first pass. */
        for (ii = 0; ii < n_bytes; ii += OPCODE_SIZE (opcode))
        {
                opcode = bytes[ii];
                switch (opcode)
                {
                        case PCL_OPCODE_JUMP_FORWARD:
                        case PCL_OPCODE_JUMP_IF_TRUE:
                        case PCL_OPCODE_JUMP_IF_FALSE:
                        case PCL_OPCODE_JUMP_ABSOLUTE:
                        case PCL_OPCODE_SETUP_LOOP:
                        case PCL_OPCODE_SETUP_EXCEPT:
                        case PCL_OPCODE_SETUP_FINALLY:
                        case PCL_OPCODE_CONTINUE_LOOP:
                        case PCL_OPCODE_FOR_ITER:
                                os->blocks[GET_JUMP_TARGET (bytes, ii)] = 1;
                                break;
                }
        }

        /* Build block numbers in the second pass. */
        for (ii = 0; ii < n_bytes; ii++)
        {
                block_number += os->blocks[ii];
                os->blocks[ii] = block_number;
        }
}

static void
optimizer_invert_jump_test (OptimizerState *os, glong offset)
{
        /* Original                     Modified
         * ---------------------------  ---------------------------
         * UNARY_NOT                    JUMP_IF_TRUE (arg)
         * JUMP_IF_FALSE (arg)          POP_TOP
         * POP_TOP
         */

        guchar *bytes = os->bytes;
        guint distance, target;

        /* pre-conditions */
        if (bytes[offset + 1] != PCL_OPCODE_JUMP_IF_FALSE)
                return;
        if (bytes[offset + 4] != PCL_OPCODE_POP_TOP)
                return;
        if (!IS_BASIC_BLOCK (os->blocks, offset, 5))
                return; 

        target = GET_JUMP_TARGET (bytes, (offset + 1));
        if (bytes[target] != PCL_OPCODE_POP_TOP)
                return;

        distance = GET_ARG (bytes, offset + 1) + 1;
        bytes[offset] = PCL_OPCODE_JUMP_IF_TRUE;
        SET_ARG (bytes, offset, distance);
        bytes[offset + 3] = PCL_OPCODE_POP_TOP;
        bytes[offset + 4] = PCL_OPCODE_NONE;
}

static void
optimizer_invert_comparison (OptimizerState *os, glong offset)
{
        /* Original                     Modified
         * ---------------------------  ---------------------------
         * not a is b                   a is not b
         * not a in b                   a not in b
         * not a is not b               a is b
         * not a not in b               a in b
         */

        guchar *bytes = os->bytes;

        /* pre-conditions */
        if (bytes[offset + 1] != PCL_OPCODE_UNARY_NOT)
                return;
        if (!IS_BASIC_BLOCK (os->blocks, offset, 4))
                return;

        switch (bytes[offset])
        {
                case PCL_OPCODE_COMPARISON_IN:
                        bytes[offset] = PCL_OPCODE_COMPARISON_NOT_IN;
                        break;
                case PCL_OPCODE_COMPARISON_NOT_IN:
                        bytes[offset] = PCL_OPCODE_COMPARISON_IN;
                        break;
                case PCL_OPCODE_COMPARISON_IS:
                        bytes[offset] = PCL_OPCODE_COMPARISON_IS_NOT;
                        break;
                case PCL_OPCODE_COMPARISON_IS_NOT:
                        bytes[offset] = PCL_OPCODE_COMPARISON_IS;
                default:
                        return;
        }

        bytes[offset + 1] = PCL_OPCODE_NONE;
}

static void
optimizer_load_none (OptimizerState *os, glong offset)
{
        /* Original                     Modified
         * ---------------------------  ---------------------------
         * LOAD_NAME None               LOAD_CONST None
         *
         * or
         *
         * LOAD_GLOBAL None             LOAD_CONST None
         */

        guchar *bytes = os->bytes;
        guint index = GET_ARG (bytes, offset);
        gchar *name;
        glong ii;

        /* pre-conditions */
        name = pcl_string_as_string (PCL_TUPLE_GET_ITEM (os->names, index));
        if (name == NULL || strcmp (name, "None") != 0)
                return;

        for (ii = 0; ii < PCL_LIST_GET_SIZE (os->constants); ii++)
        {
                if (PCL_LIST_GET_ITEM (os->constants, ii) == PCL_NONE)
                {
                        bytes[offset] = PCL_OPCODE_LOAD_CONST;
                        SET_ARG (bytes, offset, ii);
                        os->cum_load_const = os->last_load_const + 1;
                }
        }
}

static void
optimizer_never_branch (OptimizerState *os, glong offset)
{
        /* Original                     Modified
         * ---------------------------  ---------------------------
         * LOAD_CONST true_const        (skip over it)
         * JUMP_IF_FALSE x
         * POP_TOP
         */

        guchar *bytes = os->bytes;
        guint index = GET_ARG (bytes, offset);

        os->cum_load_const = os->last_load_const + 1;

        /* pre-conditions */
        if (bytes[offset + 3] != PCL_OPCODE_JUMP_IF_FALSE)
                return;
        if (bytes[offset + 6] != PCL_OPCODE_POP_TOP)
                return;
        if (!IS_BASIC_BLOCK (os->blocks, offset, 7))
                return;
        if (!pcl_object_is_true (PCL_LIST_GET_ITEM (os->constants, index)))
                return;

        memset (bytes + offset, PCL_OPCODE_NONE, 7);
        os->cum_load_const = 0;
}

static gboolean
optimizer_build_tuple (OptimizerState *os, glong offset)
{
        guchar *bytes = os->bytes;
        glong size = GET_ARG (bytes, offset);
        glong load_const = offset - 3 * size;

        /* pre-conditions */
        if (load_const < 0)
                return FALSE;
        if (size > os->last_load_const)
                return FALSE;
        if (bytes[load_const] != PCL_OPCODE_LOAD_CONST)
                return FALSE;
        if (!IS_BASIC_BLOCK (os->blocks, load_const, 3 * (size + 1)))
                return FALSE;

        if (!optimizer_tuple_of_constants (
                        &bytes[load_const], size, os->constants))
                return FALSE;
        g_assert (bytes[offset] == PCL_OPCODE_LOAD_CONST);
        os->cum_load_const = 1;
        return TRUE;
}

static void
optimizer_build_unpack (OptimizerState *os, glong offset)
{
        /* Original                     Modified
         * ---------------------------  ---------------------------
         * BUILD_LIST 1                 (skip over it)
         * UNPACK_SEQUENCE 1
         *
         * or
         *
         * BUILD_LIST 2                 ROTATE_2
         * UNPACK_SEQUENCE 2
         *
         * or
         *
         * BUILD_LIST 3                 ROTATE_3
         * UNPACK_SEQUENCE 3            ROTATE_2
         *
         * Similarly for BUILD_TUPLE.
         */

        guchar *bytes = os->bytes;
        guint size = GET_ARG (bytes, offset);

        /* pre-conditions */
        if (bytes[offset + 3] != PCL_OPCODE_UNPACK_SEQUENCE)
                return;
        if (!IS_BASIC_BLOCK (os->blocks, offset, 6))
                return;
        if (size != GET_ARG (bytes, offset + 3))
                return;

        if (size == 1)
                memset (bytes + offset, PCL_OPCODE_NONE, 6);
        if (size == 2)
        {
                bytes[offset] = PCL_OPCODE_ROTATE_2;
                memset (bytes + offset + 1, PCL_OPCODE_NONE, 5);
        }
        if (size == 3)
        {
                bytes[offset] = PCL_OPCODE_ROTATE_3;
                bytes[offset + 1] = PCL_OPCODE_ROTATE_2;
                memset (bytes + offset + 2, PCL_OPCODE_NONE, 4);
        }
}

static gboolean
optimizer_conditional_jump (OptimizerState *os, glong offset)
{
        /* Simplify conditional jump to conditional jump where the result
         * of the first test implies the success of a similar test or the
         * failure of the opposite test.
         *
         * Original                     Modified
         * ---------------------------  ---------------------------
         * x: JUMP_IF_FALSE y           x: JUMP_IF_FALSE z
         * y: JUMP_IF_FALSE z
         *
         * or
         *
         * x: JUMP_IF_TRUE y            x: JUMP_IF_TRUE z
         * y: JUMP_IF_TRUE z
         *
         * or
         *
         * x: JUMP_IF_FALSE y           x: JUMP_IF_FALSE y+3
         * y: JUMP_IF_TRUE z
         *
         * or
         *
         * x: JUMP_IF_TRUE y            x: JUMP_IF_TRUE y+3
         * y: JUMP_IF_FALSE z
         *
         * where y+3 is the instruction following the second test.
         */

        guchar *bytes = os->bytes;
        glong target = GET_JUMP_TARGET (bytes, offset);

        /* pre-condition */
        if (bytes[target] != PCL_OPCODE_JUMP_IF_FALSE &&
                        bytes[target] != PCL_OPCODE_JUMP_IF_TRUE)
                return FALSE;

        if (bytes[target] == bytes[offset])
                target = GET_JUMP_TARGET (bytes, target) - offset - 3;
        else
                target -= offset;
        SET_ARG (bytes, offset, target);
        return TRUE;
}

static void
optimizer_double_jump (OptimizerState *os, glong offset)
{
        /* Replace jumps to unconditional jumps. */

        guchar *bytes = os->bytes;
        guchar opcode = bytes[offset];
        glong target = GET_JUMP_TARGET (bytes, offset);

        /* pre-condition */
        if (!UNCONDITIONAL_JUMP (bytes[target]))
                return;

        target = GET_JUMP_TARGET (bytes, target);
        if (opcode == PCL_OPCODE_JUMP_FORWARD)
                opcode = PCL_OPCODE_JUMP_ABSOLUTE;  /* can go backwards */
        if (!ABSOLUTE_JUMP (opcode))
                target -= offset + 3;  /* relative jump address */
        if (target < 0)
                return;  /* no backward relative jumps */
        bytes[offset] = opcode;
        SET_ARG (bytes, offset, target);
}

/* Perform basic peephole optimizations to components of a bytecode object.
 * The constants object should still be in list form to allow new constants
 * to be appended.
 *
 * To keep the optimizer simple, it bails out (does nothing) for code
 * containing extended arguments or that has a length over 32,700.  That
 * allows us to avoid overflow and sign issues.  Likewise, it bails when
 * the lineno table has complex encoding for gaps >= 255.
 *
 * Optimizations are restricted to simple transformations occuring within a
 * single basic block.  All transformations keep the code size the same or
 * smaller.  For those that reduce size, the gaps are initially filled with
 * NOPs.  Later those NOPs are removed and the jump addresses retargeted in
 * a single pass.  Line numbering is adjusted accordingly.
 */

PclObject *
pcl_optimize (PclObject *bytes, PclObject *constants,
              PclObject *names, PclObject *lineno_table)
{
        OptimizerState os;
        guchar *lineno_table_data;
        glong lineno_table_size;
        glong *address_map = NULL;
        glong original_address;
        glong adjusted_address;
        glong previous_address;
        glong ii, jj, nops;
        guint opcode, oparg;

        os.constants = constants;
        os.names = names;
        os.bytes = NULL;
        os.blocks = NULL;
        os.n_bytes = 0;
        os.cum_load_const = 0;
        os.last_load_const = 0;

        /* XXX Disable this module.
         *
         *     I did not have time to get this completely working, but it
         *     basically mimics Python's own "peephole" optimizer in
         *     Python/compile.c.  Don't expect any miracles from this.  It
         *     just looks for very simple patterns in the byte code string
         *     where it can shave off an instruction or two.  But since
         *     software tends to obey the "90/10" rule (10% of the code
         *     accounts for 90% of the execution time), shaving off a few
         *     instructions in the 10% part *can* make a difference.
         *
         *     This code is *very* sensitive to changes in the compiler,
         *     and requires a deep knowledge of how the byte code is
         *     assembled.  Make sure you fully understand how the compiler
         *     works before tinkering with this.
         *
         *     Contact me if you have questions.   -mfb
         */
        goto exit_unchanged;

        /* Bail out if an exception is set. */
        if (pcl_error_occurred ())
                goto exit_unchanged;

        /* Bypass optimizations when the lineno table is too complex. */
        g_assert (PCL_IS_STRING (lineno_table));
        lineno_table_data = (guchar *) PCL_STRING_AS_STRING (lineno_table);
        lineno_table_size = PCL_STRING_GET_SIZE (lineno_table);
        /* XXX GLib ought to have a G_MAXUCHAR constant. */
        if (memchr (lineno_table_data, 255, lineno_table_size) != NULL)
                goto exit_unchanged;

        /* Avoid situations where jump retargeting could overflow. */
        g_assert (PCL_IS_STRING (bytes));
        os.n_bytes = PCL_STRING_GET_SIZE (bytes);
        if (os.n_bytes > 32700)
                goto exit_unchanged;

        /* Make a modifiable copy of the bytecode string. */
        os.bytes = g_new0 (guchar, os.n_bytes);
        memcpy (os.bytes, (guchar *) PCL_STRING_AS_STRING (bytes), os.n_bytes);

        /* Mapping to new jump targets after NOPs are removed. */
        address_map = g_new0 (glong, os.n_bytes);

        optimizer_mark_blocks (&os);
        g_assert (PCL_IS_LIST (constants));

        for (ii = 0; ii < os.n_bytes; ii += OPCODE_SIZE (opcode))
        {
                opcode = os.bytes[ii];

                os.last_load_const = os.cum_load_const;
                os.cum_load_const = 0;

                switch (opcode)
                {
                        case PCL_OPCODE_UNARY_NOT:
                                optimizer_invert_jump_test (&os, ii);
                                break;

                        case PCL_OPCODE_COMPARISON_IN:
                        case PCL_OPCODE_COMPARISON_NOT_IN:
                        case PCL_OPCODE_COMPARISON_IS:
                        case PCL_OPCODE_COMPARISON_IS_NOT:
                                optimizer_invert_comparison (&os, ii);
                                break;

                        case PCL_OPCODE_LOAD_NAME:
                        case PCL_OPCODE_LOAD_GLOBAL:
                                optimizer_load_none (&os, ii);
                                break;

                        case PCL_OPCODE_LOAD_CONST:
                                optimizer_never_branch (&os, ii);
                                break;

                        case PCL_OPCODE_BUILD_TUPLE:
                                if (optimizer_build_tuple (&os, ii))
                                        break;
                                /* fall through */

                        case PCL_OPCODE_BUILD_LIST:
                                optimizer_build_unpack (&os, ii);
                                break;

                        case PCL_OPCODE_JUMP_IF_TRUE:
                        case PCL_OPCODE_JUMP_IF_FALSE:
                                if (optimizer_conditional_jump (&os, ii))
                                        break;
                                /* fall through */

                        case PCL_OPCODE_JUMP_FORWARD:
                        case PCL_OPCODE_JUMP_ABSOLUTE:
                        case PCL_OPCODE_SETUP_LOOP:
                        case PCL_OPCODE_SETUP_EXCEPT:
                        case PCL_OPCODE_SETUP_FINALLY:
                        case PCL_OPCODE_CONTINUE_LOOP:
                        case PCL_OPCODE_FOR_ITER:
                                optimizer_double_jump (&os, ii);
                                break;

                        case PCL_OPCODE_EXTENDED_ARG:
                                goto exit_unchanged;
                }
        }

        /* Fixup lineno table. */
        for (ii = 0, nops = 0; ii < os.n_bytes; ii += OPCODE_SIZE (opcode))
        {
                opcode = os.bytes[ii];
                address_map[ii] = ii - nops;
                if (opcode == PCL_OPCODE_NONE)
                        nops++;
        }
        original_address = 0;
        previous_address = 0;
        for (ii = 0; ii < lineno_table_size; ii += 2)
        {
                original_address += lineno_table_data[ii];
                adjusted_address = address_map[original_address];
                /* XXX GLib ought to have a G_MAXUCHAR constant. */
                g_assert (adjusted_address - previous_address < 255);
                lineno_table_data[ii] =
                        (guchar) (adjusted_address - previous_address);
                previous_address = adjusted_address;
        }

        /* Remove NOPs and fixup jump targets. */
        for (ii = 0, jj = 0; ii < os.n_bytes; )
        {
                gint adjust;

                opcode = os.bytes[ii];
                switch (opcode)
                {
                        case PCL_OPCODE_NONE:
                                ii++;
                                continue;

                        case PCL_OPCODE_JUMP_ABSOLUTE:
                        case PCL_OPCODE_CONTINUE_LOOP:
                                oparg = address_map[GET_ARG (os.bytes, ii)];
                                SET_ARG (os.bytes, ii, oparg);
                                break;

                        case PCL_OPCODE_JUMP_FORWARD:
                        case PCL_OPCODE_JUMP_IF_TRUE:
                        case PCL_OPCODE_JUMP_IF_FALSE:
                        case PCL_OPCODE_SETUP_LOOP:
                        case PCL_OPCODE_SETUP_EXCEPT:
                        case PCL_OPCODE_SETUP_FINALLY:
                        case PCL_OPCODE_FOR_ITER:
                                oparg = address_map[GET_ARG(os.bytes, ii) + 
                                        ii + 3] - address_map[ii] - 3;
                                SET_ARG (os.bytes, ii, oparg);
                                break;
                }

                adjust = OPCODE_SIZE (opcode);
                while (adjust--)
                        os.bytes[jj++] = os.bytes[ii++];
        }
        g_assert (jj + nops == os.n_bytes);
        g_debug ("Optimized out %ld byte code(s).", os.n_bytes - jj);
 
        bytes = pcl_string_from_string_and_size ((gchar *) os.bytes, jj);
        g_free (address_map);
        g_free (os.blocks);
        g_free (os.bytes);
        return bytes;

exit_unchanged:

        g_free (address_map);
        g_free (os.blocks);
        g_free (os.bytes);
        return pcl_object_ref (bytes);
}
