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

static gchar operator_doc[] =
"Operator interface.\n\
\n\
This module exports a set of functions implemented in C corresponding\n\
to the intrinsic operators of PCL.  For example, operator.add(x, y)\n\
is equivalent to the expression x+y.  The function names are those\n\
used for special class methods; variants without leading and trailing\n\
'__' are also provided for convenience.";

static PclObject *
operator_abs (PclObject *self, PclObject *object)
{
        return pcl_number_absolute (object);
}

static PclObject *
operator_add (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "add", 2, 2, &a, &b))
                return NULL;
        return pcl_number_add (a, b);
}

static PclObject *
operator_and (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "and_", 2, 2, &a, &b))
                return NULL;
        return pcl_number_and (a, b);
}

static PclObject *
operator_concat (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "concat", 2, 2, &a, &b))
                return NULL;
        return pcl_sequence_concat (a, b);
}

static PclObject *
operator_contains (PclObject *self, PclObject *args)
{
        PclObject *a, *b;
        gint result;

        if (!pcl_arg_unpack_tuple (args, "contains", 2, 2, &a, &b))
                return NULL;
        result = pcl_object_contains (a, b);
        if (result < 0)
                return NULL;
        return pcl_bool_from_boolean ((gboolean) result);
}

static PclObject *
operator_countof (PclObject *self, PclObject *args)
{
        PclObject *a, *b;
        glong count;

        if (!pcl_arg_unpack_tuple (args, "countOf", 2, 2, &a, &b))
                return NULL;
        count = pcl_sequence_count (a, b);
        if (count < 0)
                return NULL;
        return pcl_int_from_long (count);
}

static PclObject *
operator_delitem (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "delitem", 2, 2, &a, &b))
                return NULL;
        if (!pcl_object_del_item (a, b))
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

static PclObject *
operator_div (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "div", 2, 2, &a, &b))
                return NULL;
        return pcl_number_divide (a, b);
}

static PclObject *
operator_eq (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "eq", 2, 2, &a, &b))
                return NULL;
        return pcl_object_rich_compare (a, b, PCL_EQ);
}

static PclObject *
operator_floordiv (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "floordiv", 2, 2, &a, &b))
                return NULL;
        return pcl_number_floordiv (a, b);
}

static PclObject *
operator_ge (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "ge", 2, 2, &a, &b))
                return NULL;
        return pcl_object_rich_compare (a, b, PCL_GE);
}

static PclObject *
operator_getitem (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "getitem", 2, 2, &a, &b))
                return NULL;
        return pcl_object_get_item (a, b);
}

static PclObject *
operator_gt (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "gt", 2, 2, &a, &b))
                return NULL;
        return pcl_object_rich_compare (a, b, PCL_GT);
}

static PclObject *
operator_indexof (PclObject *self, PclObject *args)
{
        PclObject *a, *b;
        glong index;

        if (!pcl_arg_unpack_tuple (args, "indexOf", 2, 2, &a, &b))
                return NULL;
        index = pcl_sequence_index (a, b);
        if (index < 0)
                return NULL;
        return pcl_int_from_long (index);
}

static PclObject *
operator_invert (PclObject *self, PclObject *object)
{
        return pcl_number_invert (object);
}

static PclObject *
operator_ismappingtype (PclObject *self, PclObject *object)
{
        return pcl_bool_from_boolean (PCL_IS_MAPPING (object));
}

static PclObject *
operator_isnumbertype (PclObject *self, PclObject *object)
{
        return pcl_bool_from_boolean (PCL_IS_NUMBER (object));
}

static PclObject *
operator_issequencetype (PclObject *self, PclObject *object)
{
        return pcl_bool_from_boolean (PCL_IS_SEQUENCE (object));
}

static PclObject *
operator_is (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "is_", 2, 2, &a, &b))
                return NULL;
        return pcl_object_ref ((a == b) ? PCL_TRUE : PCL_FALSE);
}

static PclObject *
operator_is_not (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "is_not", 2, 2, &a, &b))
                return NULL;
        return pcl_object_ref ((a != b) ? PCL_TRUE : PCL_FALSE);
}

static PclObject *
operator_le (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "le", 2, 2, &a, &b))
                return NULL;
        return pcl_object_rich_compare (a, b, PCL_LE);
}

static PclObject *
operator_lshift (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "lshift", 2, 2, &a, &b))
                return NULL;
        return pcl_number_lshift (a, b);
}

static PclObject *
operator_lt (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "lt", 2, 2, &a, &b))
                return NULL;
        return pcl_object_rich_compare (a, b, PCL_LT);
}

static PclObject *
operator_mod (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "mod", 2, 2, &a, &b))
                return NULL;
        return pcl_number_modulo (a, b);
}

static PclObject *
operator_mul (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "mul", 2, 2, &a, &b))
                return NULL;
        return pcl_number_multiply (a, b);
}

static PclObject *
operator_ne (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "ne", 2, 2, &a, &b))
                return NULL;
        return pcl_object_rich_compare (a, b, PCL_NE);
}

static PclObject *
operator_neg (PclObject *self, PclObject *object)
{
        return pcl_number_negative (object);
}

static PclObject *
operator_not (PclObject *self, PclObject *object)
{
        gint result;

        result = pcl_object_not (object);
        if (result < 0)
                return NULL;
        return pcl_bool_from_boolean ((gboolean) result);
}

static PclObject *
operator_or (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "or", 2, 2, &a, &b))
                return NULL;
        return pcl_number_or (a, b);
}

static PclObject *
operator_pos (PclObject *self, PclObject *object)
{
        return pcl_number_positive (object);
}

static PclObject *
operator_pow (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "pow", 2, 2, &a, &b))
                return NULL;
        return pcl_number_power (a, b, PCL_NONE);
}

static PclObject *
operator_repeat (PclObject *self, PclObject *args)
{
        PclObject *a;
        glong b;

        if (!pcl_arg_parse_tuple (args, "Ol:repeat", &a, &b))
                return NULL;
        return pcl_sequence_repeat (a, b);
}

static PclObject *
operator_rshift (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "rshift", 2, 2, &a, &b))
                return NULL;
        return pcl_number_rshift (a, b);
}

static PclObject *
operator_setitem (PclObject *self, PclObject *args)
{
        PclObject *a, *b, *c;

        if (!pcl_arg_unpack_tuple (args, "setitem", 3, 3, &a, &b, &c))
                return NULL;
        if (!pcl_object_set_item (a, b, c))
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

static PclObject *
operator_sub (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "sub", 2, 2, &a, &b))
                return NULL;
        return pcl_number_subtract (a, b);
}

static PclObject *
operator_truth (PclObject *self, PclObject *object)
{
        gint result;

        result = pcl_object_is_true (object);
        if (result < 0)
                return NULL;
        return pcl_bool_from_boolean ((gboolean) result);
}

static PclObject *
operator_xor (PclObject *self, PclObject *args)
{
        PclObject *a, *b;

        if (!pcl_arg_unpack_tuple (args, "xor", 2, 2, &a, &b))
                return NULL;
        return pcl_number_xor (a, b);
}

static PclMethodDef operator_methods[] = {
        { "abs",                (PclCFunction) operator_abs,
                                PCL_METHOD_FLAG_ONEARG,
                                "abs(a) -- Same as abs(a)." },
        { "__abs__",            (PclCFunction) operator_abs,
                                PCL_METHOD_FLAG_ONEARG,
                                "__abs__(a) -- Same as abs(a)." },
        { "add",                (PclCFunction) operator_add,
                                PCL_METHOD_FLAG_VARARGS,
                                "add(a, b) -- Same as a + b." },
        { "__add__",            (PclCFunction) operator_add,
                                PCL_METHOD_FLAG_VARARGS,
                                "__add__(a, b) -- Same as a + b." },
        { "and_",               (PclCFunction) operator_and,
                                PCL_METHOD_FLAG_VARARGS,
                                "and(a, b) -- Same as a & b." },
        { "__and__",            (PclCFunction) operator_and,
                                PCL_METHOD_FLAG_VARARGS,
                                "__and__(a, b) -- Same as a & b." },
        { "concat",             (PclCFunction) operator_concat,
                                PCL_METHOD_FLAG_VARARGS,
                                "concat(a, b) -- Same as a + b, "
                                "where a and b are sequences." },
        { "__concat__",         (PclCFunction) operator_concat,
                                PCL_METHOD_FLAG_VARARGS,
                                "__concat__(a, b) -- Same as a + b, "
                                "where a and b are sequences." },
        { "contains",           (PclCFunction) operator_contains,
                                PCL_METHOD_FLAG_VARARGS,
                                "contains(a, b) -- Same as b in a "
                                "(note reversed operands)." },
        { "__contains__",       (PclCFunction) operator_contains,
                                PCL_METHOD_FLAG_VARARGS,
                                "__contains__(a, b) -- Same as b in a "
                                "(note reversed operands)." },
        { "countOf",            (PclCFunction) operator_countof,
                                PCL_METHOD_FLAG_VARARGS,
                                "countOf(a, b) -- Return the number of "
                                "times b occurs in a." },
        { "delitem",            (PclCFunction) operator_delitem,
                                PCL_METHOD_FLAG_VARARGS,
                                "delitem(a, b) -- Same as del a[b]." },
        { "__delitem__",        (PclCFunction) operator_delitem,
                                PCL_METHOD_FLAG_VARARGS,
                                "__delitem__(a, b) -- Same as del a[b]." },
        { "div",                (PclCFunction) operator_div,
                                PCL_METHOD_FLAG_VARARGS,
                                "div(a, b) -- Same as a / b." },
        { "__div__",            (PclCFunction) operator_div,
                                PCL_METHOD_FLAG_VARARGS,
                                "__div__(a, b) -- Same as a / b." },
        { "eq",                 (PclCFunction) operator_eq,
                                PCL_METHOD_FLAG_VARARGS,
                                "eq(a, b) -- Same as a == b." },
        { "__eq__",             (PclCFunction) operator_eq,
                                PCL_METHOD_FLAG_VARARGS,
                                "__eq__(a, b) -- Same as a == b." },
        { "floordiv",           (PclCFunction) operator_floordiv,
                                PCL_METHOD_FLAG_VARARGS,
                                "floordiv(a, b) -- Same as a // b." },
        { "__floordiv__",       (PclCFunction) operator_floordiv,
                                PCL_METHOD_FLAG_VARARGS,
                                "__floordiv__(a, b) -- Same as a // b." },
        { "ge",                 (PclCFunction) operator_ge,
                                PCL_METHOD_FLAG_VARARGS,
                                "ge(a, b) -- Same as a >= b." },
        { "__ge__",             (PclCFunction) operator_ge,
                                PCL_METHOD_FLAG_VARARGS,
                                "__ge__(a, b) -- Same as a >= b." },
        { "getitem",            (PclCFunction) operator_getitem,
                                PCL_METHOD_FLAG_VARARGS,
                                "getitem(a, b) -- Same as a[b]." },
        { "__getitem__",        (PclCFunction) operator_getitem,
                                PCL_METHOD_FLAG_VARARGS,
                                "__getitem__(a, b) -- Same as a[b]." },
        { "gt",                 (PclCFunction) operator_gt,
                                PCL_METHOD_FLAG_VARARGS,
                                "gt(a, b) -- Same as a > b." },
        { "__gt__",             (PclCFunction) operator_gt,
                                PCL_METHOD_FLAG_VARARGS,
                                "__gt__(a, b) -- Same as a > b." },
        { "indexOf",            (PclCFunction) operator_indexof,
                                PCL_METHOD_FLAG_VARARGS,
                                "indexOf(a, b) -- Return the first index of "
                                "b in a." },
        { "inv",                (PclCFunction) operator_invert,
                                PCL_METHOD_FLAG_ONEARG,
                                "inv(a) -- Same as ~a." },
        { "invert",             (PclCFunction) operator_invert,
                                PCL_METHOD_FLAG_ONEARG,
                                "invert(a) -- Same as ~a." },
        { "__inv__",            (PclCFunction) operator_invert,
                                PCL_METHOD_FLAG_ONEARG,
                                "__inv__(a) -- Same as ~a." },
        { "__invert__",         (PclCFunction) operator_invert,
                                PCL_METHOD_FLAG_ONEARG,
                                "__invert__(a) -- Same as ~a." },
        { "isMappingType",      (PclCFunction) operator_ismappingtype,
                                PCL_METHOD_FLAG_ONEARG,
                                "isMappingType(a) -- Return True if a has a "
                                "mapping type, False otherwise." },
        { "isNumberType",       (PclCFunction) operator_isnumbertype,
                                PCL_METHOD_FLAG_ONEARG,
                                "isNumberType(a) -- Return True if a has a "
                                "numeric type, False otherwise." },
        { "isSequenceType",     (PclCFunction) operator_issequencetype,
                                PCL_METHOD_FLAG_ONEARG,
                                "isSequenceType(a) -- Return True if a has a "
                                "sequence type, False otherwise." },
        { "is_",                (PclCFunction) operator_is,
                                PCL_METHOD_FLAG_VARARGS,
                                "is_(a, b) -- Same as a is b." },
        { "is_not",             (PclCFunction) operator_is_not,
                                PCL_METHOD_FLAG_VARARGS,
                                "is_not(a, b) -- Same as a is not b." },
        { "le",                 (PclCFunction) operator_le,
                                PCL_METHOD_FLAG_VARARGS,
                                "le(a, b) -- Same as a <= b." },
        { "__le__",             (PclCFunction) operator_le,
                                PCL_METHOD_FLAG_VARARGS,
                                "__le__(a, b) -- Same as a <= b." },
        { "lshift",             (PclCFunction) operator_lshift,
                                PCL_METHOD_FLAG_VARARGS,
                                "lshift(a, b) -- Same as a << b." },
        { "__lshift__",         (PclCFunction) operator_lshift,
                                PCL_METHOD_FLAG_VARARGS,
                                "__lshift__(a, b) -- Same as a << b." },
        { "lt",                 (PclCFunction) operator_lt,
                                PCL_METHOD_FLAG_VARARGS,
                                "lt(a, b) -- Same as a < b." },
        { "__lt__",             (PclCFunction) operator_lt,
                                PCL_METHOD_FLAG_VARARGS,
                                "__lt__(a, b) -- Same as a < b." },
        { "mod",                (PclCFunction) operator_mod,
                                PCL_METHOD_FLAG_VARARGS,
                                "mod(a, b) -- Same as a % b." },
        { "__mod__",            (PclCFunction) operator_mod,
                                PCL_METHOD_FLAG_VARARGS,
                                "__mod__(a, b) -- Same as a % b." },
        { "mul",                (PclCFunction) operator_mul,
                                PCL_METHOD_FLAG_VARARGS,
                                "mul(a, b) -- Same as a * b." },
        { "__mul__",            (PclCFunction) operator_mul,
                                PCL_METHOD_FLAG_VARARGS,
                                "__mul__(a, b) -- Same as a * b." },
        { "ne",                 (PclCFunction) operator_ne,
                                PCL_METHOD_FLAG_VARARGS,
                                "ne(a, b) -- Same as a != b." },
        { "__ne__",             (PclCFunction) operator_ne,
                                PCL_METHOD_FLAG_VARARGS,
                                "__ne__(a, b) -- Same as a != b." },
        { "neg",                (PclCFunction) operator_neg,
                                PCL_METHOD_FLAG_ONEARG,
                                "neg(a) -- Same as -a." },
        { "__neg__",            (PclCFunction) operator_neg,
                                PCL_METHOD_FLAG_ONEARG,
                                "__neg__(a) -- Same as -a." }, 
        { "not_",               (PclCFunction) operator_not,
                                PCL_METHOD_FLAG_ONEARG,
                                "not_(a) -- Same as not a." },
        { "__not__",            (PclCFunction) operator_not,
                                PCL_METHOD_FLAG_ONEARG,
                                "__not__(a) -- Same as not a." },
        { "or_",                (PclCFunction) operator_or,
                                PCL_METHOD_FLAG_VARARGS,
                                "or_(a, b) -- Same as a | b." },
        { "__or__",             (PclCFunction) operator_or,
                                PCL_METHOD_FLAG_VARARGS,
                                "__or__(a, b) -- Same as a | b." },
        { "pos",                (PclCFunction) operator_pos,
                                PCL_METHOD_FLAG_ONEARG,
                                "pos(a) -- Same as +a." },
        { "__pos__",            (PclCFunction) operator_pos,
                                PCL_METHOD_FLAG_ONEARG,
                                "__pos__(a) -- Same as +a." },
        { "pow",                (PclCFunction) operator_pow,
                                PCL_METHOD_FLAG_VARARGS,
                                "pow(a, b) -- Same as a ** b." },
        { "__pow__",            (PclCFunction) operator_pow,
                                PCL_METHOD_FLAG_VARARGS,
                                "__pow__(a, b) -- Same as a ** b." },
        { "repeat",             (PclCFunction) operator_repeat,
                                PCL_METHOD_FLAG_VARARGS,
                                "repeat(a, b) -- Return a * b, where a "
                                "is a sequence, and b is an integer." },
        { "__repeat__",         (PclCFunction) operator_repeat,
                                PCL_METHOD_FLAG_VARARGS,
                                "__repeat__(a, b) -- Return a * b, where a "
                                "is a sequence, and b is an integer." },
        { "rshift",             (PclCFunction) operator_rshift,
                                PCL_METHOD_FLAG_VARARGS,
                                "rshift(a, b) -- Same as a >> b." },
        { "__rshift__",         (PclCFunction) operator_rshift,
                                PCL_METHOD_FLAG_VARARGS,
                                "__rshift__(a, b) -- Same as a >> b." },
        { "setitem",            (PclCFunction) operator_setitem,
                                PCL_METHOD_FLAG_VARARGS,
                                "setitem(a, b, c) -- Same as a[b] = c." },
        { "__setitem__",        (PclCFunction) operator_setitem,
                                PCL_METHOD_FLAG_VARARGS,
                                "__setitem__(a, b, c) -- Same as a[b] = c." },
        { "sub",                (PclCFunction) operator_sub,
                                PCL_METHOD_FLAG_VARARGS,
                                "sub(a, b) -- Same as a - b." },
        { "__sub__",            (PclCFunction) operator_sub,
                                PCL_METHOD_FLAG_VARARGS,
                                "__sub__(a, b) -- Same as a - b." },
        { "truth",              (PclCFunction) operator_truth,
                                PCL_METHOD_FLAG_ONEARG,
                                "truth(a) -- Return True if a is true, "
                                "False otherwise." },
        { "xor",                (PclCFunction) operator_xor,
                                PCL_METHOD_FLAG_VARARGS,
                                "xor(a, b) -- Same as a ^ b." },
        { "__xor__",            (PclCFunction) operator_xor,
                                PCL_METHOD_FLAG_VARARGS,
                                "__xor__(a, b) -- Same as a ^ b." },
        { NULL }
};

PclObject *
_pcl_operator_init (void)
{
        PclObject *module;

        module = pcl_module_init ("operator", operator_methods, operator_doc);
        if (module == NULL)
                return NULL;

        pcl_module_add_object (module, "attrgetter",
                pcl_object_ref (pcl_attr_getter_get_type_object ()));
        pcl_module_add_object (module, "itemgetter",
                pcl_object_ref (pcl_item_getter_get_type_object ()));

        return module;
}
