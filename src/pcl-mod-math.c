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

static gchar math_doc[] =
"This module is always available.  It provides access to the\n\
mathematical functions defined by the C standard.\n";

static gboolean
math_is_error (gdouble x)
{
        gboolean is_error = TRUE;
        g_assert (errno != 0);

        if (errno == EDOM)
                pcl_error_set_string (
                        pcl_exception_value_error (),
                        "math domain error");
        else if (errno == ERANGE)
        {
                /* From Python:
                 * ANSI C generally requires libm functions to set ERANGE
                 * on overflow, but also generally *allows* them to set
                 * ERANGE on underflow too.  There's no consistency about
                 * the latter across platforms.
                 * Alas, C99 never requires that errno be set.
                 * Here we suppress the underflow errors (libm functions
                 * should return a zero on underflow, and +- HUGE_VAL on
                 * overflow, so testing the result for zero suffices to
                 * distinguish the cases).
                 */
                if (x != 0.0)
                        pcl_error_set_string (
                                pcl_exception_overflow_error (),
                                "math range error");
                else
                        is_error = FALSE;
        }
        else
                pcl_error_set_from_errno (
                        pcl_exception_value_error ());
        return is_error;
}

static PclObject *
math_1 (PclObject *args, gdouble (*func) (gdouble), const gchar *fmt)
{
        gdouble x;
        if (!pcl_arg_parse_tuple (args, fmt, &x))
                return NULL;
        errno = 0;
        x = (*func) (x);
        if (errno != 0 && math_is_error (x))
                return NULL;
        return pcl_float_from_double (x);
}

static PclObject *
math_2 (PclObject *args, gdouble (*func) (gdouble, gdouble), const gchar *fmt)
{
        gdouble x, y;
        if (!pcl_arg_parse_tuple (args, fmt, &x, &y))
                return NULL;
        errno = 0;
        x = (*func) (x, y);
        if (errno != 0 && math_is_error (x))
                return NULL;
        return pcl_float_from_double (x);
}

#define MATH_FUNCTION_1(func) \
        static PclObject * math_##func (PclObject *self, PclObject *args) \
        { return math_1 (args, func, "d:"#func); }

#define MATH_FUNCTION_2(func) \
        static PclObject * math_##func (PclObject *self, PclObject *args) \
        { return math_2 (args, func, "dd:"#func); }

/* trivial libm wrappers */
MATH_FUNCTION_1 (acos)
MATH_FUNCTION_1 (asin)
MATH_FUNCTION_1 (atan)
MATH_FUNCTION_2 (atan2)
MATH_FUNCTION_1 (ceil)
MATH_FUNCTION_1 (cos)
MATH_FUNCTION_1 (cosh)
MATH_FUNCTION_1 (exp)
MATH_FUNCTION_1 (fabs)
MATH_FUNCTION_1 (floor)
MATH_FUNCTION_2 (fmod)
MATH_FUNCTION_2 (hypot)
MATH_FUNCTION_1 (log)
MATH_FUNCTION_1 (log10)
MATH_FUNCTION_2 (pow)
MATH_FUNCTION_1 (sin)
MATH_FUNCTION_1 (sinh)
MATH_FUNCTION_1 (sqrt)
MATH_FUNCTION_1 (tan)
MATH_FUNCTION_1 (tanh)

static PclObject *
math_frexp (PclObject *self, PclObject *args)
{
        gdouble x;
        gint exp;
        if (!pcl_arg_parse_tuple (args, "d:frexp", &x))
                return NULL;
        errno = 0;
        x = frexp (x, &exp);
        if (errno != 0 && math_is_error (x))
                return NULL;
        return pcl_build_value ("(di)", x, exp);
}

static PclObject *
math_ldexp (PclObject *self, PclObject *args)
{
        gdouble x;
        gint exp;
        if (!pcl_arg_parse_tuple (args, "di:ldexp", &x, &exp))
                return NULL;
        errno = 0;
        x = ldexp (x, exp);
        if (errno != 0 && math_is_error (x))
                return NULL;
        return pcl_float_from_double (x);
}

static PclObject *
math_modf (PclObject *self, PclObject *args)
{
        gdouble x, y;
        if (!pcl_arg_parse_tuple (args, "d:modf", &x))
                return NULL;
        errno = 0;
        x = modf (x, &y);
        if (errno != 0 && math_is_error (x))
                return NULL;
        return pcl_build_value ("(dd)", x, y);
}

#define DEGREES_TO_RADIANS (G_PI / 180.0)

static PclObject *
math_degrees (PclObject *self, PclObject *args)
{
        gdouble x;
        if (!pcl_arg_parse_tuple (args, "d:degrees", &x))
                return NULL;
        return pcl_float_from_double (x / DEGREES_TO_RADIANS);
}

static PclObject *
math_radians (PclObject *self, PclObject *args)
{
        gdouble x;
        if (!pcl_arg_parse_tuple (args, "d:radians", &x))
                return NULL;
        return pcl_float_from_double (x * DEGREES_TO_RADIANS);
}

static PclMethodDef math_methods[] = {
        { "acos",               (PclCFunction) math_acos,
                                PCL_METHOD_FLAG_VARARGS },
        { "asin",               (PclCFunction) math_asin,
                                PCL_METHOD_FLAG_VARARGS },
        { "atan",               (PclCFunction) math_atan,
                                PCL_METHOD_FLAG_VARARGS },
        { "atan2",              (PclCFunction) math_atan2,
                                PCL_METHOD_FLAG_VARARGS },
        { "ceil",               (PclCFunction) math_ceil,
                                PCL_METHOD_FLAG_VARARGS },
        { "cos",                (PclCFunction) math_cos,
                                PCL_METHOD_FLAG_VARARGS },
        { "cosh",               (PclCFunction) math_cosh,
                                PCL_METHOD_FLAG_VARARGS },
        { "degrees",            (PclCFunction) math_degrees,
                                PCL_METHOD_FLAG_VARARGS },
        { "exp",                (PclCFunction) math_exp,
                                PCL_METHOD_FLAG_VARARGS },
        { "fabs",               (PclCFunction) math_fabs,
                                PCL_METHOD_FLAG_VARARGS },
        { "floor",              (PclCFunction) math_floor,
                                PCL_METHOD_FLAG_VARARGS },
        { "fmod",               (PclCFunction) math_fmod,
                                PCL_METHOD_FLAG_VARARGS },
        { "frexp",              (PclCFunction) math_frexp,
                                PCL_METHOD_FLAG_VARARGS },
        { "hypot",              (PclCFunction) math_hypot,
                                PCL_METHOD_FLAG_VARARGS },
        { "ldexp",              (PclCFunction) math_ldexp,
                                PCL_METHOD_FLAG_VARARGS },
        { "log",                (PclCFunction) math_log,
                                PCL_METHOD_FLAG_VARARGS },
        { "log10",              (PclCFunction) math_log10,
                                PCL_METHOD_FLAG_VARARGS },
        { "modf",               (PclCFunction) math_modf,
                                PCL_METHOD_FLAG_VARARGS },
        { "pow",                (PclCFunction) math_pow,
                                PCL_METHOD_FLAG_VARARGS },
        { "radians",            (PclCFunction) math_radians,
                                PCL_METHOD_FLAG_VARARGS },
        { "sin",                (PclCFunction) math_sin,
                                PCL_METHOD_FLAG_VARARGS },
        { "sinh",               (PclCFunction) math_sinh,
                                PCL_METHOD_FLAG_VARARGS },
        { "sqrt",               (PclCFunction) math_sqrt,
                                PCL_METHOD_FLAG_VARARGS },
        { "tan",                (PclCFunction) math_tan,
                                PCL_METHOD_FLAG_VARARGS },
        { "tanh",               (PclCFunction) math_tanh,
                                PCL_METHOD_FLAG_VARARGS },
        { NULL }
};

void
_pcl_math_init (void)
{
        PclObject *module = pcl_module_init ("math", math_methods, math_doc);

        /* useful mathematical constants */
        pcl_module_add_float (module, "e", G_E);
        pcl_module_add_float (module, "pi", G_PI);
}
