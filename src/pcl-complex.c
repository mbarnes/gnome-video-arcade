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

/* Macro and helper that convert PclObject obj to a complex and store the
 * value in cplx.  If conversion to complex raises an exception, obj is set
 * to NULL, and the function invoking this macro returns NULL.  If obj is
 * not of complex, float, int, or long type, PCL_NOT_IMPLEMENTED is incref'ed,
 * stored in obj, and returned from the function invoking this macro. */

#define CONVERT_TO_COMPLEX(obj, cplx) \
        if (PCL_IS_COMPLEX (obj)) \
                cplx = PCL_COMPLEX_AS_COMPLEX (obj); \
        else if (!convert_to_complex (&(obj), &(cplx))) \
                return obj;

static gboolean
convert_to_complex (PclObject **p_object, PclComplexValue *p_complex)
{
        register PclObject *object = *p_object;

        if (PCL_IS_FLOAT (object))
        {
                p_complex->real = PCL_FLOAT_AS_DOUBLE (object);
                p_complex->imag = 0.0;
        }
        else if (PCL_IS_INT (object))
        {
                p_complex->real = (gdouble) PCL_INT_AS_LONG (object);
                p_complex->imag = 0.0;
        }
        else
        {
                *p_object = pcl_object_ref (PCL_NOT_IMPLEMENTED);
                return FALSE;
        }
        return TRUE;
}

/* Precisions used by repr() and str(), respectively.
 *
 * The repr() precision (17 significant decimal digits) is the minimal number
 * that is guaranteed to have enough precision so that if the number is read
 * back in, the exact same binary value is recreated.  This is true for IEEE
 * floating point by design, and also happens to work for all other modern
 * hardware.
 *
 * The str() precision is chosen so that in most cases, the rounding noise
 * created by various operations is suppressed, while giving plenty of
 * precision for practical use.
 */

#define PRECISION_REPR  17
#define PRECISION_STR   12

static gchar complex_doc[] =
"complex(real[, imag]) -> complex number\n\
\n\
Create a complex number from a real part and an optional imaginary part.\n\
This is equivalent to (real + imag*1j) where imag defaults to 0.";

static gpointer complex_parent_class = NULL;

static PclComplexValue complex_0 = { 0.0, 0.0 };
static PclComplexValue complex_1 = { 1.0, 0.0 };

static PclComplexValue
complex_value_powu (PclComplexValue a, glong n)
{
        PclComplexValue x = complex_1;
        PclComplexValue t = a;
        glong mask = 1;
        while (mask > 0 && n >= mask)
        {
                if (n & mask)
                        x = pcl_complex_value_mul (x, t);
                mask <<= 1;
                t = pcl_complex_value_mul (t, t);
        }
        return x;
}

static PclComplexValue
complex_value_powi (PclComplexValue a, glong n)
{
        PclComplexValue tmp;
        if (n > 100 || n < -100)
        {
                tmp.real = (gdouble) n;
                tmp.imag = 0.0;
                return pcl_complex_value_pow (a, tmp);
        }
        else if (n > 0)
                return complex_value_powu (a, n);
        else
        {
                tmp = complex_value_powu (a, -n);
                return pcl_complex_value_div (complex_1, tmp);
        }
}

static void
complex_format (gchar *buffer, gint buf_len, PclObject *value, gint precision)
{
        gdouble real = PCL_COMPLEX_REAL_AS_DOUBLE (value);
        gdouble imag = PCL_COMPLEX_IMAG_AS_DOUBLE (value);
        gchar format[32];

        if (real == 0.0)
        {
                g_snprintf (format, sizeof (format), "%%.%ig", precision);
                g_ascii_formatd (buffer, buf_len, format, imag);
                g_strlcat (buffer, "j", buf_len);
        }
        else
        {
                gchar rs[64], is[64];
                /* Format imaginary part with sign, real part without. */
                g_snprintf (format, sizeof (format), "%%.%ig", precision);
                g_ascii_formatd (rs, sizeof (rs), format, real);
                g_snprintf (format, sizeof (format), "%%+.%ig", precision);
                g_ascii_formatd (is, sizeof (is), format, imag);
                g_snprintf (buffer, buf_len, "(%s%sj)", rs, is);
        }
}

static PclObject *
complex_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        const gchar *kwlist[] = { "real", "imag", NULL };
        PclObject *x = PCL_FALSE;
        PclObject *y = NULL;
        PclObject *func;
        PclComplexValue cx, cy;
        gboolean own_x = FALSE;

        if (!pcl_arg_parse_tuple_and_keywords (
                        args, kwds, "|OO:complex", kwlist, &x, &y))
                return NULL;

        /* Special case for single argument that is already complex. */
        if (PCL_IS_COMPLEX (x) && y == NULL)
                return pcl_object_ref (x);

        if (PCL_IS_STRING (x))
        {
                const gchar *v_string;

                if (y != NULL)
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "complex() can't take second argument "
                                "if first is a string");
                        return NULL;
                }

                v_string = PCL_STRING_AS_STRING (x);
                return pcl_complex_from_string (v_string, NULL);
        }

        if (y != NULL && PCL_IS_STRING (y))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "complex() second argument can't be a string");
                return NULL;
        }

        func = pcl_object_get_attr_string (x, "__complex__");
        if (func == NULL)
                pcl_error_clear ();
        else
        {
                PclObject *args = pcl_tuple_new (0);
                if (args == NULL)
                        return NULL;
                x = pcl_eval_call_object (func, args);
                pcl_object_unref (args);
                pcl_object_unref (func);
                if (x == NULL)
                        return NULL;
                own_x = TRUE;
        }

        if (!PCL_IS_NUMBER (x) || (y != NULL && !PCL_IS_NUMBER (y)))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "complex() argument must be a string or a number");
                if (own_x)
                        pcl_object_unref (x);
                return NULL;
        }

        if (PCL_IS_COMPLEX (x))
        {
                cx = PCL_COMPLEX_AS_COMPLEX (x);
                if (own_x)
                        pcl_object_unref (x);
        }
        else
        {
                PclObject *tmp;
                tmp = pcl_number_as_float (x);
                if (own_x)
                        pcl_object_unref (x);
                if (tmp == NULL)
                        return NULL;
                if (!PCL_IS_FLOAT (tmp))
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "float(x) didn't return a float");
                        pcl_object_unref (tmp);
                        return NULL;
                }
                cx.real = pcl_float_as_double (tmp);
                cx.imag = 0.0;
                pcl_object_unref (tmp);
        }

        if (y == NULL)
        {
                cy.real = 0.0;
                cy.imag = 0.0;
        }
        else if (PCL_IS_COMPLEX (y))
                cy = PCL_COMPLEX_AS_COMPLEX (y);
        else
        {
                PclObject *tmp;
                tmp = pcl_number_as_float (y);
                if (tmp == NULL)
                        return NULL;
                if (!PCL_IS_FLOAT (tmp))
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "float(y) didn't return a float");
                        pcl_object_unref (tmp);
                        return NULL;
                }
                cy.real = pcl_float_as_double (tmp);
                cy.imag = 0.0;
                pcl_object_unref (tmp);
        }

        cx.real -= cy.imag;
        cx.imag += cy.real;
        return pcl_complex_from_complex (cx);
}

static gboolean
complex_print (PclObject *object, FILE *stream, PclPrintFlags flags)
{
        gchar buffer[100];
        complex_format (buffer, sizeof (buffer), object,
                (flags & PCL_PRINT_FLAG_RAW) ? PRECISION_STR : PRECISION_REPR);
        fputs (buffer, stream);
        return TRUE;
}

static guint
complex_hash (PclObject *object)
{
        PclComplex *self = PCL_COMPLEX (object);
        gdouble real = PCL_COMPLEX_REAL_AS_DOUBLE (self);
        gdouble imag = PCL_COMPLEX_IMAG_AS_DOUBLE (self);
        guint hash_real, hash_imag, hash;

        hash_real = pcl_hash_double (real);
        if (hash_real == PCL_HASH_INVALID)
                return PCL_HASH_INVALID;
        hash_imag = pcl_hash_double (imag);
        if (hash_imag == PCL_HASH_INVALID)
                return PCL_HASH_INVALID;
        /* NOTE: If the imaginary part is 0, hash_imag is 0 now, so the
         *       following returns hash_real unchanged.  This is important
         *       because numbers of different types that compare equal
         *       must have the same hash value, so that hash(x + 0j)
         *       must equal hash(x).  */
        hash = hash_real + 1000003 * hash_imag;
        return PCL_HASH_VALIDATE (hash);
}

static PclObject *
complex_repr (PclObject *object)
{
        gchar buffer[100];
        complex_format (buffer, sizeof (buffer), object, PRECISION_REPR);
        return pcl_string_from_string (buffer);
}

static PclObject *
complex_str (PclObject *object)
{
        gchar buffer[100];
        complex_format (buffer, sizeof (buffer), object, PRECISION_STR);
        return pcl_string_from_string (buffer);
}

static PclObject *
complex_rich_compare (PclObject *object1, PclObject *object2,
                      PclRichCompareOps op)
{
        PclComplexValue a, b;

        CONVERT_TO_COMPLEX (object1, a);
        CONVERT_TO_COMPLEX (object2, b);

        if (op != PCL_EQ && op != PCL_NE)
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "no ordering relation is defined for complex numbers");
                return NULL;
        }

        return pcl_bool_from_boolean (
                (a.real == b.real && a.imag == b.imag) == (op == PCL_EQ));
}

static PclObject *
complex_number_as_int (PclObject *object)
{
        pcl_error_set_string (
                pcl_exception_type_error (),
                "can't convert complex to int; use int(abs(z))");
        return NULL;
}

static PclObject *
complex_number_as_float (PclObject *object)
{
        pcl_error_set_string (
                pcl_exception_type_error (),
                "can't convert complex to float; use abs(z)");
        return NULL;
}

static PclObject *
complex_number_as_complex (PclObject *object)
{
        return pcl_object_ref (object);
}

static PclObject *
complex_number_add (PclObject *object1, PclObject *object2)
{
        PclComplexValue a, b, x;

        CONVERT_TO_COMPLEX (object1, a);
        CONVERT_TO_COMPLEX (object2, b);
        x = pcl_complex_value_add (a, b);
        return pcl_complex_from_complex (x);
}

static PclObject *
complex_number_subtract (PclObject *object1, PclObject *object2)
{
        PclComplexValue a, b, x;

        CONVERT_TO_COMPLEX (object1, a);
        CONVERT_TO_COMPLEX (object2, b);
        x = pcl_complex_value_sub (a, b);
        return pcl_complex_from_complex (x);
}

static PclObject *
complex_number_multiply (PclObject *object1, PclObject *object2)
{
        PclComplexValue a, b, x;

        CONVERT_TO_COMPLEX (object1, a);
        CONVERT_TO_COMPLEX (object2, b);
        x = pcl_complex_value_mul (a, b);
        return pcl_complex_from_complex (x);
}

static PclObject *
complex_number_divide (PclObject *object1, PclObject *object2)
{
        PclComplexValue a, b, x;

        CONVERT_TO_COMPLEX (object1, a);
        CONVERT_TO_COMPLEX (object2, b);
        errno = 0;
        x = pcl_complex_value_div (a, b);
        if (errno == EDOM)
        {
                pcl_error_set_string (
                        pcl_exception_zero_division_error (),
                        "complex division");
                return NULL;
        }
        return pcl_complex_from_complex (x);
}

static PclObject *
complex_number_divmod (PclObject *object1, PclObject *object2)
{
        pcl_error_set_string (
                pcl_exception_value_error (),
                "complex divmod() not supported");
        return NULL;
}

static PclObject *
complex_number_modulo (PclObject *object1, PclObject *object2)
{
        pcl_error_set_string (
                pcl_exception_value_error (),
                "complex modulo not supported");
        return NULL;
}

static PclObject *
complex_number_power (PclObject *object1, PclObject *object2,
                      PclObject *object3)
{
        PclComplexValue a, b, x;
        glong int_b_real;

        if (object3 != PCL_NONE)
        {
                pcl_error_set_string (
                        pcl_exception_value_error (),
                        "complex modulo not supported");
                return NULL;
        }

        CONVERT_TO_COMPLEX (object1, a);
        CONVERT_TO_COMPLEX (object2, b);
        int_b_real = (glong) b.real;
        errno = 0;
        if (b.imag == 0.0 && b.real == int_b_real)
                x = complex_value_powi (a, int_b_real);
        else
                x = pcl_complex_value_pow (a, b);
        if (errno == EDOM)
        {
                pcl_error_set_string (
                        pcl_exception_zero_division_error (),
                        "0.0 to a negative or complex power");
                return NULL;
        }
        if (errno == ERANGE)
        {
                pcl_error_set_string (
                        pcl_exception_overflow_error (),
                        "complex exponentiation");
                return NULL;
        }
        return pcl_complex_from_complex (x);
}

static PclObject *
complex_number_floordiv (PclObject *object1, PclObject *object2)
{
        PclComplexValue a, b, x;

        CONVERT_TO_COMPLEX (object1, a);
        CONVERT_TO_COMPLEX (object2, b);
        errno = 0;
        x = pcl_complex_value_div (a, b);
        if (errno == EDOM)
        {
                pcl_error_set_string (
                        pcl_exception_zero_division_error (),
                        "complex floor division");
                return NULL;
        }
        return pcl_complex_from_doubles (floor (x.real), 0.0);
}

static PclObject *
complex_number_negative (PclObject *object)
{
        gdouble real = PCL_COMPLEX_REAL_AS_DOUBLE (object);
        gdouble imag = PCL_COMPLEX_IMAG_AS_DOUBLE (object);
        return pcl_complex_from_doubles (-real, -imag);
}

static PclObject *
complex_number_positive (PclObject *object)
{
        return pcl_object_ref (object);
}

static PclObject *
complex_number_absolute (PclObject *object)
{
        gdouble x;
        x = pcl_complex_value_abs (PCL_COMPLEX_AS_COMPLEX (object));
        return pcl_float_from_double (x);
}

static gboolean
complex_number_nonzero (PclObject *object)
{
        gdouble real = PCL_COMPLEX_REAL_AS_DOUBLE (object);
        gdouble imag = PCL_COMPLEX_IMAG_AS_DOUBLE (object);
        return (real != 0.0) && (imag != 0.0);
}

static void
complex_number_init (PclNumberIface *iface)
{
        iface->as_int = complex_number_as_int;
        iface->as_float = complex_number_as_float;
        iface->as_complex = complex_number_as_complex;
        iface->add = complex_number_add;
        iface->subtract = complex_number_subtract;
        iface->multiply = complex_number_multiply;
        iface->divide = complex_number_divide;
        iface->divmod = complex_number_divmod;
        iface->modulo = complex_number_modulo;
        iface->power = complex_number_power;
        iface->floordiv = complex_number_floordiv;
        iface->negative = complex_number_negative;
        iface->positive = complex_number_positive;
        iface->absolute = complex_number_absolute;
        iface->nonzero = complex_number_nonzero;
}

static PclObject *
complex_method_getnewargs (PclComplex *self)
{
        return pcl_build_value ("(D)", &PCL_COMPLEX_AS_COMPLEX (self));
}

static PclObject *
complex_method_conjugate (PclComplex *self)
{
        PclComplexValue v_complex = PCL_COMPLEX_AS_COMPLEX (self);
        v_complex.imag = -v_complex.imag;
        return pcl_complex_from_complex (v_complex);
}

static PclMemberDef complex_members[] = {
        { "real",               G_TYPE_DOUBLE,
                                G_STRUCT_OFFSET (PclComplex, v_complex.real),
                                PCL_MEMBER_FLAG_READONLY,
                                "the real part of a complex number" },
        { "imag",               G_TYPE_DOUBLE,
                                G_STRUCT_OFFSET (PclComplex, v_complex.imag),
                                PCL_MEMBER_FLAG_READONLY,
                                "the imaginary part of a complex number" },
        { NULL }
};

static PclMethodDef complex_methods[] = {
        { "__getnewargs__",     (PclCFunction)  complex_method_getnewargs,
                                                PCL_METHOD_FLAG_NOARGS },
        { "conjugate",          (PclCFunction)  complex_method_conjugate,
                                                PCL_METHOD_FLAG_NOARGS },
        { NULL }
};

static void
complex_class_init (PclComplexClass *class)
{
        PclObjectClass *object_class;

        complex_parent_class = g_type_class_peek_parent (class);

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_complex_get_type_object;
        object_class->new_instance = complex_new_instance;
        object_class->print = complex_print;
        object_class->hash = complex_hash;
        object_class->repr = complex_repr;
        object_class->str = complex_str;
        object_class->rich_compare = complex_rich_compare;
        object_class->doc = complex_doc;
}

GType
pcl_complex_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclComplexClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) complex_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclComplex),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                static const GInterfaceInfo number_info = {
                        (GInterfaceInitFunc) complex_number_init,
                        (GInterfaceFinalizeFunc) NULL,
                        NULL   /* interface_data */
                };

                type = g_type_register_static (
                        PCL_TYPE_OBJECT, "PclComplex", &type_info, 0);
                g_type_add_interface_static (
                        type, PCL_TYPE_NUMBER, &number_info);
                g_type_set_qdata (
                        type, PCL_DATA_MEMBERS, complex_members);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, complex_methods);
        }
        return type;
}

/**
 * pcl_complex_get_type_object:
 *
 * Returns the type object for #PclComplex.  During runtime this is the
 * built-in object %complex.
 *
 * Returns: a borrowed reference to the type object for #PclComplex
 */
PclObject *
pcl_complex_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_COMPLEX, "complex");
                pcl_register_singleton ("<type 'complex'>", &object);
        }
        return object;
}

PclComplexValue
pcl_complex_as_complex (PclObject *object)
{
        if (PCL_IS_COMPLEX (object))
                return PCL_COMPLEX_AS_COMPLEX (object);
        else
        {
                PclComplexValue v_complex;
                v_complex.real = pcl_float_as_double (object);
                v_complex.imag = 0.0;
                return v_complex;
        }
}

gdouble
pcl_complex_real_as_double (PclObject *object)
{
        if (PCL_IS_COMPLEX (object))
                return PCL_COMPLEX_REAL_AS_DOUBLE (object);
        else
                return pcl_float_as_double (object);
}

gdouble
pcl_complex_imag_as_double (PclObject *object)
{
        if (PCL_IS_COMPLEX (object))
                return PCL_COMPLEX_IMAG_AS_DOUBLE (object);
        else
                return 0.0;
}

PclObject *
pcl_complex_from_complex (PclComplexValue v_complex)
{
        PclComplex *complexobj = pcl_object_new (PCL_TYPE_COMPLEX, NULL);
        complexobj->v_complex = v_complex;
        return PCL_OBJECT (complexobj);
}

PclObject *
pcl_complex_from_doubles (gdouble real, gdouble imag)
{
        PclComplex *complexobj = pcl_object_new (PCL_TYPE_COMPLEX, NULL);
        complexobj->v_complex.real = real;
        complexobj->v_complex.imag = imag;
        return PCL_OBJECT (complexobj);
}

PclObject *
pcl_complex_from_string (const gchar *v_string, gchar **endptr)
{
        const gchar *cp, *start;
        gdouble x = 0.0, y = 0.0, z;
        gboolean got_real = FALSE;
        gboolean got_imag = FALSE;
        gboolean sw_error = FALSE;
        gboolean done = FALSE;
        gint sign;
        gsize len;

#define SKIP_SPACE(cp) \
        while (*(cp) != '\0' && g_ascii_isspace (*(cp))) (cp)++

        cp = start = v_string;
        SKIP_SPACE (cp);
        if (*cp == '\0')
        {
                pcl_error_set_string (
                        pcl_exception_value_error (),
                        "complex() argument is an empty string");
                return NULL;
        }

        len = strlen (v_string);

        z = -1.0;
        sign = 1;
        do
        {
                switch (*cp)
                {
                        case '\0':
                                if (cp - start != len)
                                {
                                        pcl_error_set_string (
                                                pcl_exception_value_error (),
                                                "complex() argument contains "
                                                "a null byte");
                                        return NULL;
                                }
                                if (!done)
                                        sw_error = TRUE;
                                break;

                        case '-':
                                sign = -1;
                                /* fallthrough */

                        case '+':
                                if (done)
                                        sw_error = TRUE;
                                cp++;
                                if (*cp == '\0' || *cp == '+' || *cp == '-' ||
                                                g_ascii_isspace (*cp))
                                        sw_error = TRUE;
                                break;

                        case 'J':
                        case 'j':
                                if (got_imag || done)
                                {
                                        sw_error = TRUE;
                                        break;
                                }
                                if (z < 0.0)
                                        y = sign;
                                else
                                        y = sign * z;
                                got_imag = TRUE;
                                cp++;
                                if (*cp != '+' && *cp != '-')
                                        done = TRUE;
                                break;

                        default:
                        {
                                gboolean digit_or_dot;
                                gchar *end;

                                if (g_ascii_isspace (*cp))
                                {
                                        SKIP_SPACE (cp);
                                        if (*cp != '\0')
                                                sw_error = TRUE;
                                        else
                                                done = TRUE;
                                        break;
                                }
                                digit_or_dot =
                                        (*cp == '.' || g_ascii_isdigit (*cp));
                                if (done || !digit_or_dot)
                                {
                                        sw_error = TRUE;
                                        break;
                                }
                                errno = 0;
                                z = g_ascii_strtod (cp, &end);
                                if (errno != 0)
                                {
                                        pcl_error_set_format (
                                                pcl_exception_value_error (),
                                                "float() out of range: %s",
                                                cp);
                                        return NULL;
                                }
                                cp = end;
                                if (*cp == 'J' || *cp == 'j')
                                        break;
                                if (got_real)
                                {
                                        sw_error = TRUE;
                                        break;
                                }
                                x = sign * z;
                                got_real = TRUE;
                                if (got_imag)
                                        done = TRUE;
                                z = -1.0;
                                sign = 1;
                                break;
                        }
                }
        }
        while (cp - start < len && !sw_error);

#undef SKIP_SPACE

        if (sw_error)
        {
                pcl_error_set_string (
                        pcl_exception_value_error (),
                        "complex() argument is a malformed string");
                return NULL;
        }

        if (endptr != NULL)
                *endptr = (gchar *) cp;  /* discard const */
        return pcl_complex_from_doubles (x, y);
}

PclComplexValue
pcl_complex_value_add (PclComplexValue a, PclComplexValue b)
{
        PclComplexValue x;
        x.real = a.real + b.real;
        x.imag = a.imag + b.imag;
        return x;
}

PclComplexValue
pcl_complex_value_sub (PclComplexValue a, PclComplexValue b)
{
        PclComplexValue x;
        x.real = a.real - b.real;
        x.imag = a.imag - b.imag;
        return x;
}

PclComplexValue
pcl_complex_value_mul (PclComplexValue a, PclComplexValue b)
{
        PclComplexValue x;
        x.real = a.real * b.real - a.imag * b.imag;
        x.imag = a.real * b.imag + a.imag * b.real;
        return x;
}

PclComplexValue
pcl_complex_value_div (PclComplexValue a, PclComplexValue b)
{
        PclComplexValue x;
        const gdouble abs_b_real = ABS (b.real);
        const gdouble abs_b_imag = ABS (b.imag);
        if (abs_b_real >= abs_b_imag)
        {
                if (abs_b_real == 0.0)
                {
                        errno = EDOM;
                        x = complex_0;
                }
                else
                {
                        const gdouble ratio = b.imag / b.real;
                        const gdouble denom = b.real + b.imag * ratio;
                        x.real = (a.real + a.imag * ratio) / denom;
                        x.imag = (a.imag - a.real * ratio) / denom;
                }
        }
        else
        {
                const gdouble ratio = b.real / b.imag;
                const gdouble denom = b.real * ratio + b.imag;
                x.real = (a.real * ratio + a.imag) / denom;
                x.imag = (a.imag * ratio - a.real) / denom;
        }
        return x;
}

PclComplexValue
pcl_complex_value_pow (PclComplexValue a, PclComplexValue b)
{
        PclComplexValue x;
        if (b.real == 0.0 && b.imag == 0.0)
                x = complex_1;
        else if (a.real == 0.0 && a.imag == 0.0)
        {
                if (b.imag != 0.0 || b.real < 0.0)
                        errno = EDOM;
                x = complex_0;
        }
        else
        {
                const gdouble abs_a = pcl_complex_value_abs (a);
                const gdouble atan2_a = atan2 (a.imag, a.real);
                gdouble length = pow (abs_a, b.real);
                gdouble phase = atan2_a * b.real;
                if (b.imag != 0.0)
                {
                        length /= exp (atan2_a * b.imag);
                        phase += b.imag * log (abs_a);
                }
                x.real = length * cos (phase);
                x.imag = length * sin (phase);
        }
        return x;
}

gdouble
pcl_complex_value_abs (PclComplexValue a)
{
        return hypot (a.real, a.imag);
}
