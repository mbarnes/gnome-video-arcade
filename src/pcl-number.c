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

typedef PclObject * (*BinaryFunc) (PclObject *, PclObject *);
typedef PclObject * (*TernaryFunc) (PclObject *, PclObject *, PclObject *);

static PclObject *
number_binary_op (PclObject *n1, PclObject *n2, glong offset)
{
        PclNumberIface *iface1;
        PclNumberIface *iface2;
        BinaryFunc f1 = NULL;
        BinaryFunc f2 = NULL;
        PclObject *result;

        iface1 = PCL_NUMBER_GET_IFACE (n1);
        iface2 = PCL_NUMBER_GET_IFACE (n2);

        if (iface1 != NULL)
                f1 = G_STRUCT_MEMBER (BinaryFunc, iface1, offset);
        if (iface2 != NULL)
                f2 = G_STRUCT_MEMBER (BinaryFunc, iface2, offset);

        if (f1 != NULL)
        {
                if (f2 != NULL)
                {
                        PclObject *type1 = PCL_GET_TYPE_OBJECT (n1);
                        PclObject *type2 = PCL_GET_TYPE_OBJECT (n2);

                        if (pcl_type_is_subtype (type2, type1))
                        {
                                result = f2 (n1, n2);
                                if (result != PCL_NOT_IMPLEMENTED)
                                        return result;
                                pcl_object_unref (result);
                                f2 = NULL;
                        }
                }

                result = f1 (n1, n2);
                if (result != PCL_NOT_IMPLEMENTED)
                        return result;
                pcl_object_unref (result);
        }
        if (f2 != NULL && f2 != f1)
        {
                result = f2 (n1, n2);
                if (result != PCL_NOT_IMPLEMENTED)
                        return result;
                pcl_object_unref (result);
        }

        return pcl_object_ref (PCL_NOT_IMPLEMENTED);
}

static PclObject *
number_ternary_op (PclObject *n1, PclObject *n2, PclObject *n3, glong offset)
{
        PclNumberIface *iface1;
        PclNumberIface *iface2;
        PclNumberIface *iface3;
        TernaryFunc f1 = NULL;
        TernaryFunc f2 = NULL;
        TernaryFunc f3 = NULL;
        PclObject *result;

        iface1 = PCL_NUMBER_GET_IFACE (n1);
        iface2 = PCL_NUMBER_GET_IFACE (n2);
        iface3 = PCL_NUMBER_GET_IFACE (n3);

        if (iface1 != NULL)
                f1 = G_STRUCT_MEMBER (TernaryFunc, iface1, offset);
        if (iface2 != NULL)
                f2 = G_STRUCT_MEMBER (TernaryFunc, iface2, offset);
        if (iface3 != NULL)
                f3 = G_STRUCT_MEMBER (TernaryFunc, iface3, offset);

        if (f1 != NULL)
        {
                if (f2 != NULL)
                {
                        PclObject *type1 = PCL_GET_TYPE_OBJECT (n1);
                        PclObject *type2 = PCL_GET_TYPE_OBJECT (n2);

                        if (pcl_type_is_subtype (type2, type1))
                        {
                                result = f2 (n1, n2, n3);
                                if (result != PCL_NOT_IMPLEMENTED)
                                        return result;
                                pcl_object_unref (result);
                                f2 = NULL;
                        }
                }

                result = f1 (n1, n2, n3);
                if (result != PCL_NOT_IMPLEMENTED)
                        return result;
                pcl_object_unref (result);
        }
        if (f2 != NULL && f2 != f1)
        {
                result = f2 (n1, n2, n3);
                if (result != PCL_NOT_IMPLEMENTED)
                        return result;
                pcl_object_unref (result);
        }
        if (f3 != NULL && f3 != f1 && f3 != f2)
        {
                result = f3 (n1, n2, n3);
                if (result != PCL_NOT_IMPLEMENTED)
                        return result;
                pcl_object_unref (result);
        }

        return pcl_object_ref (PCL_NOT_IMPLEMENTED);
}

static PclObject *
number_binary_in_place_op (PclObject *n1, PclObject *n2,
                           glong ip_offset, glong offset)
{
        PclNumberIface *iface;
        BinaryFunc func = NULL;
        PclObject *result;

        iface = PCL_NUMBER_GET_IFACE (n1);
        if (iface != NULL)
                func = G_STRUCT_MEMBER (BinaryFunc, iface, ip_offset);
        if (func != NULL)
        {
                result = func (n1, n2);
                if (result != PCL_NOT_IMPLEMENTED)
                        return result;
                pcl_object_unref (result);
        }

        return number_binary_op (n1, n2, offset);
}

static PclObject *
number_method_abs (PclObject *self)
{
        return pcl_number_absolute (self);
}

static PclObject *
number_method_add (PclObject *self, PclObject *other)
{
        return pcl_number_add (self, other);
}

static PclObject *
number_method_and (PclObject *self, PclObject *other)
{
        return pcl_number_and (self, other);
}

static PclObject *
number_method_complex (PclObject *self)
{
        return pcl_number_as_complex (self);
}

static PclObject *
number_method_div (PclObject *self, PclObject *other)
{
        return pcl_number_divide (self, other);
}

static PclObject *
number_method_divmod (PclObject *self, PclObject *other)
{
        return pcl_number_divmod (self, other);
}

static PclObject *
number_method_float (PclObject *self)
{
        return pcl_number_as_float (self);
}

static PclObject *
number_method_floordiv (PclObject *self, PclObject *other)
{
        return pcl_number_floordiv (self, other);
}

static PclObject *
number_method_hex (PclObject *self)
{
        return pcl_number_hex (self);
}

static PclObject *
number_method_iadd (PclObject *self, PclObject *other)
{
        return pcl_number_in_place_add (self, other);
}

static PclObject *
number_method_iand (PclObject *self, PclObject *other)
{
        return pcl_number_in_place_and (self, other);
}

static PclObject *
number_method_idiv (PclObject *self, PclObject *other)
{
        return pcl_number_in_place_divide (self, other);
}

static PclObject *
number_method_ifloordiv (PclObject *self, PclObject *other)
{
        return pcl_number_in_place_floordiv (self, other);
}

static PclObject *
number_method_ilshift (PclObject *self, PclObject *other)
{
        return pcl_number_in_place_lshift (self, other);
}

static PclObject *
number_method_imod (PclObject *self, PclObject *other)
{
        return pcl_number_in_place_modulo (self, other);
}

static PclObject *
number_method_imul (PclObject *self, PclObject *other)
{
        return pcl_number_in_place_multiply (self, other);
}

static PclObject *
number_method_int (PclObject *self)
{
        return pcl_number_as_int (self);
}

static PclObject *
number_method_invert (PclObject *self)
{
        return pcl_number_invert (self);
}

static PclObject *
number_method_ior (PclObject *self, PclObject *other)
{
        return pcl_number_in_place_or (self, other);
}

static PclObject *
number_method_ipow (PclObject *self, PclObject *other)
{
        return pcl_number_in_place_power (self, other, PCL_NONE);
}

static PclObject *
number_method_irshift (PclObject *self, PclObject *other)
{
        return pcl_number_in_place_rshift (self, other);
}

static PclObject *
number_method_isub (PclObject *self, PclObject *other)
{
        return pcl_number_in_place_subtract (self, other);
}

static PclObject *
number_method_ixor (PclObject *self, PclObject *other)
{
        return pcl_number_in_place_xor (self, other);
}

static PclObject *
number_method_lshift (PclObject *self, PclObject *other)
{
        return pcl_number_lshift (self, other);
}

static PclObject *
number_method_mod (PclObject *self, PclObject *other)
{
        return pcl_number_modulo (self, other);
}

static PclObject *
number_method_mul (PclObject *self, PclObject *other)
{
        return pcl_number_multiply (self, other);
}

static PclObject *
number_method_neg (PclObject *self)
{
        return pcl_number_negative (self);
}

static PclObject *
number_method_nonzero (PclObject *self)
{
        gint nonzero;

        nonzero = pcl_number_nonzero (self);
        if (nonzero < 0)
                return NULL;
        return pcl_bool_from_boolean ((gboolean) nonzero);
}

static PclObject *
number_method_oct (PclObject *self)
{
        return pcl_number_oct (self);
}

static PclObject *
number_method_or (PclObject *self, PclObject *other)
{
        return pcl_number_or (self, other);
}

static PclObject *
number_method_pos (PclObject *self)
{
        return pcl_number_positive (self);
}

static PclObject *
number_method_pow (PclObject *self, PclObject *args)
{
        PclObject *other;
        PclObject *third = PCL_NONE;

        if (!pcl_arg_unpack_tuple (args, "", 1, 2, &other, &third))
                return NULL;
        return pcl_number_power (self, other, third);
}

static PclObject *
number_method_radd (PclObject *self, PclObject *other)
{
        return pcl_number_add (other, self);
}

static PclObject *
number_method_rand (PclObject *self, PclObject *other)
{
        return pcl_number_and (other, self);
}

static PclObject *
number_method_rdiv (PclObject *self, PclObject *other)
{
        return pcl_number_divide (other, self);
}

static PclObject *
number_method_rdivmod (PclObject *self, PclObject *other)
{
        return pcl_number_divmod (other, self);
}

static PclObject *
number_method_rfloordiv (PclObject *self, PclObject *other)
{
        return pcl_number_floordiv (other, self);
}

static PclObject *
number_method_rlshift (PclObject *self, PclObject *other)
{
        return pcl_number_lshift (other, self);
}

static PclObject *
number_method_rmod (PclObject *self, PclObject *other)
{
        return pcl_number_modulo (other, self);
}

static PclObject *
number_method_rmul (PclObject *self, PclObject *other)
{
        return pcl_number_multiply (other, self);
}

static PclObject *
number_method_ror (PclObject *self, PclObject *other)
{
        return pcl_number_or (other, self);
}

static PclObject *
number_method_rpow (PclObject *self, PclObject *args)
{
        PclObject *other;
        PclObject *third = PCL_NONE;

        if (!pcl_arg_unpack_tuple (args, "", 1, 2, &other, &third))
                return NULL;
        return pcl_number_power (other, self, third);
}

static PclObject *
number_method_rshift (PclObject *self, PclObject *other)
{
        return pcl_number_rshift (self, other);
}

static PclObject *
number_method_rrshift (PclObject *self, PclObject *other)
{
        return pcl_number_rshift (other, self);
}

static PclObject *
number_method_rsub (PclObject *self, PclObject *other)
{
        return pcl_number_subtract (other, self);
}

static PclObject *
number_method_rxor (PclObject *self, PclObject *other)
{
        return pcl_number_xor (other, self);
}

static PclObject *
number_method_sub (PclObject *self, PclObject *other)
{
        return pcl_number_subtract (self, other);
}

static PclObject *
number_method_xor (PclObject *self, PclObject *other)
{
        return pcl_number_xor (self, other);
}

#define WRAPS(func)     G_STRUCT_OFFSET (PclNumberIface, func)

static PclMethodDef number_methods[] = {
        { "__abs__",            (PclCFunction) number_method_abs,
                                PCL_METHOD_FLAG_NOARGS,
                                "x.__abs__() <==> abs(x)",
                                WRAPS (absolute) },
        { "__add__",            (PclCFunction) number_method_add,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__add__(y) <==> x + y",
                                WRAPS (add) },
        { "__and__",            (PclCFunction) number_method_and,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__and__(y) <==> x & y",
                                WRAPS (bitwise_and) },
        { "__complex__",        (PclCFunction) number_method_complex,
                                PCL_METHOD_FLAG_NOARGS,
                                "x.__complex__() <==> complex(x)",
                                WRAPS (as_complex) },
        { "__div__",            (PclCFunction) number_method_div,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__div__(y) <==> x / y",
                                WRAPS (divide) },
        { "__divmod__",         (PclCFunction) number_method_divmod,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__divmod__(y) <==> divmod(x, y)",
                                WRAPS (divmod) },
        { "__float__",          (PclCFunction) number_method_float,
                                PCL_METHOD_FLAG_NOARGS,
                                "x.__float__() <==> float(x)",
                                WRAPS (as_float) },
        { "__floordiv__",       (PclCFunction) number_method_floordiv,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__floordiv__(y) <==> x // y",
                                WRAPS (floordiv) },
        { "__hex__",            (PclCFunction) number_method_hex,
                                PCL_METHOD_FLAG_NOARGS,
                                "x.__hex__() <==> hex(x)",
                                WRAPS (hex) },
        { "__iadd__",           (PclCFunction) number_method_iadd,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__iadd__(y) <==> x += y",
                                WRAPS (in_place_add) },
        { "__iand__",           (PclCFunction) number_method_iand,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__iand__(y) <==> x &= y",
                                WRAPS (in_place_and) },
        { "__idiv__",           (PclCFunction) number_method_idiv,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__idiv__(y) <==> x /= y",
                                WRAPS (in_place_divide) },
        { "__ifloordiv__",      (PclCFunction) number_method_ifloordiv,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__ifloordiv__(y) <==> x //= y",
                                WRAPS (in_place_floordiv) },
        { "__int__",            (PclCFunction) number_method_int,
                                PCL_METHOD_FLAG_NOARGS,
                                "x.__int__() <==> int(x)",
                                WRAPS (as_int) },
        { "__ilshift__",        (PclCFunction) number_method_ilshift,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__ilshift__(y) <==> x <<= y",
                                WRAPS (in_place_lshift) },
        { "__imod__",           (PclCFunction) number_method_imod,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__imod__(y) <==> x %= y",
                                WRAPS (in_place_modulo) },
        { "__imul__",           (PclCFunction) number_method_imul,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__imul__(y) <==> x *= y",
                                WRAPS (in_place_multiply) },
        { "__invert__",         (PclCFunction) number_method_invert,
                                PCL_METHOD_FLAG_NOARGS,
                                "x.__invert__() <==> ~x",
                                WRAPS (bitwise_invert) },
        { "__ior__",            (PclCFunction) number_method_ior,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__ior__(y) <==> x |= y",
                                WRAPS (in_place_or) },
        { "__ipow__",           (PclCFunction) number_method_ipow,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__ipow__(y) <==> x **= y",
                                WRAPS (in_place_power) },
        { "__irshift__",        (PclCFunction) number_method_irshift,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__irshift__(y) <==> x >>= y",
                                WRAPS (in_place_rshift) },
        { "__isub__",           (PclCFunction) number_method_isub,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__isub__(y) <==> x -= y",
                                WRAPS (in_place_subtract) },
        { "__ixor__",           (PclCFunction) number_method_ixor,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__ixor__(y) <==> x ^= y",
                                WRAPS (in_place_xor) },
        { "__lshift__",         (PclCFunction) number_method_lshift,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__lshift__(y) <==> x << y",
                                WRAPS (bitwise_lshift) },
        { "__mod__",            (PclCFunction) number_method_mod,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__mod__(y) <==> x % y",
                                WRAPS (modulo) },
        { "__mul__",            (PclCFunction) number_method_mul,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__mul__(y) <==> x * y",
                                WRAPS (multiply) },
        { "__neg__",            (PclCFunction) number_method_neg,
                                PCL_METHOD_FLAG_NOARGS,
                                "x.__neg__() <==> -x",
                                WRAPS (negative) },
        { "__nonzero__",        (PclCFunction) number_method_nonzero,
                                PCL_METHOD_FLAG_NOARGS,
                                "x.__nonzero__() <==> x != 0",
                                WRAPS (nonzero) },
        { "__oct__",            (PclCFunction) number_method_oct,
                                PCL_METHOD_FLAG_NOARGS,
                                "x.__oct__() <==> oct(x)",
                                WRAPS (oct) },
        { "__or__",             (PclCFunction) number_method_or,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__or__(y) <==> x | y",
                                WRAPS (bitwise_or) },
        { "__pos__",            (PclCFunction) number_method_pos,
                                PCL_METHOD_FLAG_NOARGS,
                                "x.__pos__() <==> +x",
                                WRAPS (positive) },
        { "__pow__",            (PclCFunction) number_method_pow,
                                PCL_METHOD_FLAG_VARARGS,
                                "x.__pow__(y[, z]) <==> pow(x, y[, z])",
                                WRAPS (power) },
        { "__radd__",           (PclCFunction) number_method_radd,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__radd__(y) <==> y + x",
                                WRAPS (add) },
        { "__rand__",           (PclCFunction) number_method_rand,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__rand__(y) <==> y & x",
                                WRAPS (bitwise_and) },
        { "__rdiv__",           (PclCFunction) number_method_rdiv,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__rdiv__(y) <==> y / x",
                                WRAPS (divide) },
        { "__rdivmod__",        (PclCFunction) number_method_rdivmod,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__rdivmod__(y) <==> divmod(y, x)",
                                WRAPS (divmod) },
        { "__rfloordiv__",      (PclCFunction) number_method_rfloordiv,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__rfloordiv__(y) <==> y // x",
                                WRAPS (floordiv) },
        { "__rlshift__",        (PclCFunction) number_method_rlshift,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__rlshift__(y) <==> y << x",
                                WRAPS (bitwise_lshift) },
        { "__rmod__",           (PclCFunction) number_method_rmod,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__rmod__(y) <==> y % x",
                                WRAPS (modulo) },
        { "__rmul__",           (PclCFunction) number_method_rmul,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__rmul__(y) <==> y * x",
                                WRAPS (multiply) },
        { "__ror__",            (PclCFunction) number_method_ror,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__ror__(y) <==> y | x",
                                WRAPS (bitwise_or) },
        { "__rpow__",           (PclCFunction) number_method_rpow,
                                PCL_METHOD_FLAG_VARARGS,
                                "y.__rpow__(x[, z]) <==> pow(x, y[, z])",
                                WRAPS (power) },
        { "__rrshift__",        (PclCFunction) number_method_rrshift,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__rrshift__(y) <==> y >> x",
                                WRAPS (bitwise_rshift) },
        { "__rshift__",         (PclCFunction) number_method_rshift,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__rshift__(y) <==> x >> y",
                                WRAPS (bitwise_rshift) },
        { "__rsub__",           (PclCFunction) number_method_rsub,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__rsub__(y) <==> y - x",
                                WRAPS (subtract) },
        { "__rxor__",           (PclCFunction) number_method_rxor,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__rxor__(y) <==> y ^ x",
                                WRAPS (bitwise_xor) },
        { "__sub__",            (PclCFunction) number_method_sub,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__sub__(y) <==> x - y",
                                WRAPS (subtract) },
        { "__xor__",            (PclCFunction) number_method_xor,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__xor__(y) <==> x ^ y",
                                WRAPS (bitwise_xor) },
        { NULL }
};

#undef WRAPS

GType
pcl_number_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclNumberIface),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) NULL,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        0,     /* instance_size */
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        G_TYPE_INTERFACE, "PclNumber", &type_info, 0);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, number_methods);
        }
        return type;
}

PclObject *
pcl_number_as_int (PclObject *object)
{
        PclNumberIface *iface;

        iface = PCL_NUMBER_GET_IFACE (object);
        if (iface != NULL && iface->as_int != NULL)
                return iface->as_int (object);

        if (PCL_IS_STRING (object))
        {
                /* XXX Need to check for null byte in string */
                gchar *string = pcl_string_as_string (object);
                return pcl_int_from_string (string, NULL, 10);
        }

        pcl_error_set_string (
                pcl_exception_type_error (),
                "int() argument must be a string or a number");
        return NULL;
}

PclObject *
pcl_number_as_float (PclObject *object)
{
        PclNumberIface *iface;

        iface = PCL_NUMBER_GET_IFACE (object);
        if (iface != NULL && iface->as_float != NULL)
                return iface->as_float (object);

        if (PCL_IS_STRING (object))
        {
                /* XXX Need to check for null byte in string */
                gchar *string = pcl_string_as_string (object);
                return pcl_float_from_string (string, NULL);
        }

        pcl_error_set_string (
                pcl_exception_type_error (),
                "float() argument must be a string or a number");
        return NULL;
}

PclObject *
pcl_number_as_complex (PclObject *object)
{
        PclNumberIface *iface;

        iface = PCL_NUMBER_GET_IFACE (object);
        if (iface != NULL && iface->as_complex != NULL)
                return iface->as_complex (object);

        if (PCL_IS_STRING (object))
        {
                /* XXX Need to check for null byte in string */
                gchar *string = pcl_string_as_string (object);
                return pcl_complex_from_string (string, NULL);
        }

        pcl_error_set_string (
                pcl_exception_type_error (),
                "complex() argument must be a string or a number");
        return NULL;
}

PclObject *
pcl_number_oct (PclObject *object)
{
        PclNumberIface *iface;

        iface = PCL_NUMBER_GET_IFACE (object);
        if (iface != NULL && iface->oct != NULL)
        {
                PclObject *result;

                result = iface->oct (object);
                if (result != NULL && !PCL_IS_STRING (result))
                {
                        pcl_error_set_format (
                                pcl_exception_type_error (),
                                "__oct__ returned non-string (type %s)",
                                PCL_GET_TYPE_NAME (result));
                        pcl_object_unref (result);
                        return NULL;
                }
                return result;
        }

        pcl_error_set_string (
                pcl_exception_type_error (),
                "__oct__ argument can't be converted to oct");
        return NULL;
}

PclObject *
pcl_number_hex (PclObject *object)
{
        PclNumberIface *iface;

        iface = PCL_NUMBER_GET_IFACE (object);
        if (iface != NULL && iface->hex != NULL)
        {
                PclObject *result;

                result = iface->hex (object);
                if (result != NULL && !PCL_IS_STRING (result))
                {
                        pcl_error_set_format (
                                pcl_exception_type_error (),
                                "__hex__ returned non-string (type %s)",
                                PCL_GET_TYPE_NAME (result));
                        pcl_object_unref (result);
                        return NULL;
                }
                return result;
        }

        pcl_error_set_string (
                pcl_exception_type_error (),
                "__hex__ argument can't be converted to hex");
        return NULL;
}

PclObject *
pcl_number_add (PclObject *object1, PclObject *object2)
{
        PclSequenceIface *iface;
        PclObject *result;

        result = number_binary_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, add));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        iface = PCL_SEQUENCE_GET_IFACE (object1);
        if (iface != NULL && iface->concat != NULL)
                return iface->concat (object1, object2);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for +: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_subtract (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, subtract));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for -: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_multiply (PclObject *object1, PclObject *object2)
{
        PclSequenceIface *iface;
        PclObject *result;

        result = number_binary_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, multiply));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        iface = PCL_SEQUENCE_GET_IFACE (object1);
        if (iface != NULL && iface->repeat != NULL)
        {
                glong count;
                if (PCL_IS_INT (object2))
                        count = pcl_int_as_long (object2);
                else
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "can't multiply sequence by non-int");
                        return NULL;
                }
                return iface->repeat (object1, count);
        }

        iface = PCL_SEQUENCE_GET_IFACE (object2);
        if (iface != NULL && iface->repeat != NULL)
        {
                glong count;
                if (PCL_IS_INT (object1))
                        count = pcl_int_as_long (object1);
                else
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "can't multiply sequence by non-int");
                        return NULL;
                }
                return iface->repeat (object2, count);
        }

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for *: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_divide (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, divide));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for /: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_divmod (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, divmod));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for divmod(): '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_modulo (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, modulo));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        /* special case for strings */
        if (PCL_IS_STRING (object1))
                return pcl_string_format (object1, object2);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for %: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_power (PclObject *object1, PclObject *object2, PclObject *object3)
{
        PclObject *result;

        result = number_ternary_op (object1, object2, object3,
                G_STRUCT_OFFSET (PclNumberIface, power));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        if (object3 == PCL_NONE)
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "unsupported operand type(s) for ** or pow(): "
                        "'%s' and '%s'",
                        PCL_GET_TYPE_NAME (object1),
                        PCL_GET_TYPE_NAME (object2));
        else
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "unsupported operand type(s) for ** or pow(): "
                        "'%s', '%s', '%s'",
                        PCL_GET_TYPE_NAME (object1),
                        PCL_GET_TYPE_NAME (object2),
                        PCL_GET_TYPE_NAME (object3));
        return NULL;
}

PclObject *
pcl_number_floordiv (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, floordiv));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for //: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_negative (PclObject *object)
{
        PclNumberIface *iface;

        iface = PCL_NUMBER_GET_IFACE (object);
        if (iface != NULL && iface->negative != NULL)
                return iface->negative (object);

        pcl_error_set_string (
                pcl_exception_type_error (),
                "bad operand type for unary -");
        return NULL;
}

PclObject *
pcl_number_positive (PclObject *object)
{
        PclNumberIface *iface;

        iface = PCL_NUMBER_GET_IFACE (object);
        if (iface != NULL && iface->positive != NULL)
                return iface->positive (object);

        pcl_error_set_string (
                pcl_exception_type_error (),
                "bad operand type for unary +");
        return NULL;
}

PclObject *
pcl_number_absolute (PclObject *object)
{
        PclNumberIface *iface;

        iface = PCL_NUMBER_GET_IFACE (object);
        if (iface != NULL && iface->absolute != NULL)
                return iface->absolute (object);

        pcl_error_set_string (
                pcl_exception_type_error (),
                "bad operand type for abs()");
        return NULL;
}

PclObject *
pcl_number_or (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, bitwise_or));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for |: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_xor (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, bitwise_xor));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for ^: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_and (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, bitwise_and));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for &: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_lshift (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, bitwise_lshift));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for <<: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_rshift (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, bitwise_rshift));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for >>: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_invert (PclObject *object)
{
        PclNumberIface *iface;

        iface = PCL_NUMBER_GET_IFACE (object);
        if (iface != NULL && iface->bitwise_invert != NULL)
                return iface->bitwise_invert (object);

        pcl_error_set_string (
                pcl_exception_type_error (),
                "bad operand type for unary ~");
        return NULL;
}

gint
pcl_number_nonzero (PclObject *object)
{
        PclNumberIface *iface;

        iface = PCL_NUMBER_GET_IFACE (object);
        if (iface != NULL && iface->nonzero != NULL)
                return iface->nonzero (object);

        pcl_error_bad_internal_call ();
        return -1;
}

PclObject *
pcl_number_in_place_add (PclObject *object1, PclObject *object2)
{
        PclSequenceIface *iface;
        PclObject *result;

        result = number_binary_in_place_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, in_place_add),
                G_STRUCT_OFFSET (PclNumberIface, add));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        iface = PCL_SEQUENCE_GET_IFACE (object1);
        if (iface != NULL && iface->in_place_concat != NULL)
                return iface->in_place_concat (object1, object2);
        if (iface != NULL && iface->concat != NULL)
                return iface->concat (object1, object2);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for +=: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_in_place_subtract (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_in_place_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, in_place_subtract),
                G_STRUCT_OFFSET (PclNumberIface, subtract));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for -=: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_in_place_multiply (PclObject *object1, PclObject *object2)
{
        PclSequenceIface *iface;
        PclObject *result;

        result = number_binary_in_place_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, in_place_multiply),
                G_STRUCT_OFFSET (PclNumberIface, multiply));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        iface = PCL_SEQUENCE_GET_IFACE (object1);
        if (iface != NULL && iface->in_place_repeat != NULL)
        {
                glong count;
                if (PCL_IS_INT (object2))
                        count = pcl_int_as_long (object2);
                else
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "can't multiply sequence by non-int");
                        return NULL;
                }
                return iface->in_place_repeat (object1, count);
        }
        if (iface != NULL && iface->repeat != NULL)
        {
                glong count;
                if (PCL_IS_INT (object2))
                        count = pcl_int_as_long (object2);
                else
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "can't multiply sequence by non-int");
                        return NULL;
                }
                return iface->repeat (object1, count);
        }

        /* Do not mutate the right hand operand. */
        iface = PCL_SEQUENCE_GET_IFACE (object2);
        if (iface != NULL && iface->repeat != NULL)
        {
                glong count;
                if (PCL_IS_INT (object2))
                        count = pcl_int_as_long (object2);
                else
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "can't multiply sequence by non-int");
                        return NULL;
                }
                return iface->repeat (object2, count);
        }

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for *=: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_in_place_divide (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_in_place_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, in_place_divide),
                G_STRUCT_OFFSET (PclNumberIface, divide));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for /=: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_in_place_modulo (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_in_place_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, in_place_modulo),
                G_STRUCT_OFFSET (PclNumberIface, modulo));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for %=: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_in_place_power (PclObject *object1, PclObject *object2,
                           PclObject *object3)
{
        PclNumberIface *iface;
        PclObject *result = PCL_NOT_IMPLEMENTED;

        iface = PCL_NUMBER_GET_IFACE (object1);
        if (iface != NULL && iface->in_place_power != NULL)
                result = number_ternary_op (object1, object2, object3,
                        G_STRUCT_OFFSET (PclNumberIface, in_place_power));
        else if (iface != NULL && iface->power != NULL)
                result = number_ternary_op (object1, object2, object3,
                        G_STRUCT_OFFSET (PclNumberIface, power));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        if (object3 == PCL_NONE)
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "unsupported operand type(s) for **=: "
                        "'%s' and '%s'",
                        PCL_GET_TYPE_NAME (object1),
                        PCL_GET_TYPE_NAME (object2));
        else
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "unsupported operand type(s) for **=: "
                        "'%s', '%s', '%s'",
                        PCL_GET_TYPE_NAME (object1),
                        PCL_GET_TYPE_NAME (object2),
                        PCL_GET_TYPE_NAME (object3));
        return NULL;
}

PclObject *
pcl_number_in_place_floordiv (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_in_place_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, in_place_floordiv),
                G_STRUCT_OFFSET (PclNumberIface, floordiv));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for //=: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_in_place_or (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_in_place_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, in_place_or),
                G_STRUCT_OFFSET (PclNumberIface, bitwise_or));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for |=: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_in_place_xor (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_in_place_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, in_place_xor),
                G_STRUCT_OFFSET (PclNumberIface, bitwise_xor));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for ^=: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_in_place_and (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_in_place_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, in_place_and),
                G_STRUCT_OFFSET (PclNumberIface, bitwise_and));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for &=: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_in_place_lshift (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_in_place_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, in_place_lshift),
                G_STRUCT_OFFSET (PclNumberIface, bitwise_lshift));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for <<=: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}

PclObject *
pcl_number_in_place_rshift (PclObject *object1, PclObject *object2)
{
        PclObject *result;

        result = number_binary_in_place_op (object1, object2,
                G_STRUCT_OFFSET (PclNumberIface, in_place_rshift),
                G_STRUCT_OFFSET (PclNumberIface, bitwise_rshift));
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "unsupported operand type(s) for >>=: '%s' and '%s'",
                PCL_GET_TYPE_NAME (object1),
                PCL_GET_TYPE_NAME (object2));
        return NULL;
}
