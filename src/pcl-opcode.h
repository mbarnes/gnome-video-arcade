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

#ifndef PCL_OPCODE_H
#define PCL_OPCODE_H

#define PCL_OPCODE_STOP                 0x00
#define PCL_OPCODE_NONE                 0x01
#define PCL_OPCODE_POP_TOP              0x02
#define PCL_OPCODE_DUP_TOP              0x03
#define PCL_OPCODE_ROTATE_2             0x04
#define PCL_OPCODE_ROTATE_3             0x05
#define PCL_OPCODE_ROTATE_4             0x06
#define PCL_OPCODE_CONJUNCT             0x07
#define PCL_OPCODE_DISJUNCT             0x08

#define PCL_OPCODE_UNARY_POS            0x10
#define PCL_OPCODE_UNARY_NEG            0x11
#define PCL_OPCODE_UNARY_INV            0x12
#define PCL_OPCODE_UNARY_NOT            0x13
#define PCL_OPCODE_BINARY_ADD           0x14
#define PCL_OPCODE_BINARY_SUB           0x15
#define PCL_OPCODE_BINARY_MUL           0x16
#define PCL_OPCODE_BINARY_DIV           0x17
#define PCL_OPCODE_BINARY_MOD           0x18
#define PCL_OPCODE_BINARY_POW           0x19
#define PCL_OPCODE_BINARY_FLD           0x1A
#define PCL_OPCODE_BINARY_LSH           0x1B
#define PCL_OPCODE_BINARY_RSH           0x1C
#define PCL_OPCODE_BINARY_AND           0x1D
#define PCL_OPCODE_BINARY_XOR           0x1E
#define PCL_OPCODE_BINARY_OR            0x1F

#define PCL_OPCODE_INPLACE_ADD          0x20
#define PCL_OPCODE_INPLACE_SUB          0x21
#define PCL_OPCODE_INPLACE_MUL          0x22
#define PCL_OPCODE_INPLACE_DIV          0x23
#define PCL_OPCODE_INPLACE_MOD          0x24
#define PCL_OPCODE_INPLACE_POW          0x25
#define PCL_OPCODE_INPLACE_FLD          0x26
#define PCL_OPCODE_INPLACE_LSH          0x27
#define PCL_OPCODE_INPLACE_RSH          0x28
#define PCL_OPCODE_INPLACE_AND          0x29
#define PCL_OPCODE_INPLACE_XOR          0x2A
#define PCL_OPCODE_INPLACE_OR           0x2B

#define PCL_OPCODE_COMPARISON_LT        0x30
#define PCL_OPCODE_COMPARISON_LE        0x31
#define PCL_OPCODE_COMPARISON_EQ        0x32
#define PCL_OPCODE_COMPARISON_NE        0x33
#define PCL_OPCODE_COMPARISON_GE        0x34
#define PCL_OPCODE_COMPARISON_GT        0x35
#define PCL_OPCODE_COMPARISON_IN        0x36
#define PCL_OPCODE_COMPARISON_NOT_IN    0x37
#define PCL_OPCODE_COMPARISON_IS        0x38
#define PCL_OPCODE_COMPARISON_IS_NOT    0x39
#define PCL_OPCODE_COMPARISON_EXCEPT    0x3A

#define PCL_OPCODE_LOAD_SUBSCRIPT       0x40
#define PCL_OPCODE_STORE_SUBSCRIPT      0x41
#define PCL_OPCODE_DELETE_SUBSCRIPT     0x42

#define PCL_OPCODE_LOAD_CUT             0x50
#define PCL_OPCODE_LOAD_RESULT          0x51

#define PCL_OPCODE_PRINT_EXPR           0x60
#define PCL_OPCODE_PRINT_ITEM_TO        0x61
#define PCL_OPCODE_PRINT_ITEM           0x62
#define PCL_OPCODE_PRINT_NEWLINE_TO     0x63
#define PCL_OPCODE_PRINT_NEWLINE        0x64

#define PCL_OPCODE_LIST_APPEND          0x70
#define PCL_OPCODE_RAISE_EXCEPTION      0x71
#define PCL_OPCODE_LOAD_LOCALS          0x72
#define PCL_OPCODE_RETURN_VALUE         0x73
#define PCL_OPCODE_YIELD_VALUE          0x74
#define PCL_OPCODE_EXEC_STMT            0x75
#define PCL_OPCODE_POP_BLOCK            0x76
#define PCL_OPCODE_BREAK_LOOP           0x77
#define PCL_OPCODE_GET_ITER             0x78
#define PCL_OPCODE_END_FINALLY          0x79
#define PCL_OPCODE_BUILD_CLASS          0x7A
#define PCL_OPCODE_IMPORT_STAR          0x7B

#define PCL_OPCODE_HAVE_ARG             0x80  /* Argument Description: */

#define PCL_OPCODE_JUMP_FORWARD         0x80  /* Number of bytes to skip */
#define PCL_OPCODE_JUMP_IF_TRUE         0x81  /* Number of bytes to skip */
#define PCL_OPCODE_JUMP_IF_FALSE        0x82  /* Number of bytes to skip */
#define PCL_OPCODE_JUMP_ABSOLUTE        0x83  /* Absolute target address */
#define PCL_OPCODE_SETUP_LOOP           0x84  /* Absolute target address */
#define PCL_OPCODE_SETUP_EXCEPT         0x85  /* Absolute target address */
#define PCL_OPCODE_SETUP_FINALLY        0x86  /* Absolute target address */
#define PCL_OPCODE_CONTINUE_LOOP        0x87  /* Absolute target address */

#define PCL_OPCODE_LOAD_CONST           0x90  /* Index in const list */
#define PCL_OPCODE_LOAD_LOCAL           0x91  /* Index in names list */
#define PCL_OPCODE_STORE_LOCAL          0x92  /* Index in names list */
#define PCL_OPCODE_DELETE_LOCAL         0x93  /* Index in names list */
#define PCL_OPCODE_LOAD_GLOBAL          0x94  /* Index in names list */
#define PCL_OPCODE_STORE_GLOBAL         0x95  /* Index in names list */
#define PCL_OPCODE_DELETE_GLOBAL        0x96  /* Index in names list */
#define PCL_OPCODE_LOAD_ATTR            0x97  /* Index in names list */
#define PCL_OPCODE_STORE_ATTR           0x98  /* Index in names list */
#define PCL_OPCODE_DELETE_ATTR          0x99  /* Index in names list */
#define PCL_OPCODE_LOAD_FAST            0x9A  /* Index in slots list */
#define PCL_OPCODE_STORE_FAST           0x9B  /* Index in slots list */
#define PCL_OPCODE_DELETE_FAST          0x9C  /* Index in slots list */
#define PCL_OPCODE_LOAD_DEREF           0x9D  /* Index in slots list */
#define PCL_OPCODE_STORE_DEREF          0x9E  /* Index in slots list */

#define PCL_OPCODE_LOAD_CLOSURE         0xA0  /* Index in freevars list */
#define PCL_OPCODE_MAKE_CLOSURE         0xA1  /* Number of default args */
#define PCL_OPCODE_MAKE_FUNCTION        0xA2  /* Number of default args */
#define PCL_OPCODE_CALL_FUNCTION        0xA3  /* #args + (#kwargs << 8) */
#define PCL_OPCODE_CALL_FUNCTION_VA     0xA4  /* #args + (#kwargs << 8) */
#define PCL_OPCODE_CALL_FUNCTION_KW     0xA5  /* #args + (#kwargs << 8) */
#define PCL_OPCODE_CALL_FUNCTION_VA_KW  0xA6  /* #args + (#kwargs << 8) */

#define PCL_OPCODE_BUILD_DICT           0xB0  /* Number of items */
#define PCL_OPCODE_BUILD_LIST           0xB1  /* Number of items */
#define PCL_OPCODE_BUILD_SLICE          0xB2  /* Number of items */
#define PCL_OPCODE_BUILD_TUPLE          0xB3  /* Number of items */
#define PCL_OPCODE_UNPACK_SEQUENCE      0xB4  /* Sequence length */

#define PCL_OPCODE_FOR_ITER             0xC0  /* Number of bytes to skip */
#define PCL_OPCODE_IMPORT_FROM          0xC1  /* Index in names list */
#define PCL_OPCODE_IMPORT_NAME          0xC2  /* Index in names list */
#define PCL_OPCODE_STORE_CUT            0xC3  /* Number of frames back */
#define PCL_OPCODE_EXTENDED_ARG         0xC4  /* Upper two bytes of arg */

#define PCL_OPCODE_HAS_ARG(opcode)      ((opcode) >= PCL_OPCODE_HAVE_ARG) 

#endif /* PCL_OPCODE_H */
