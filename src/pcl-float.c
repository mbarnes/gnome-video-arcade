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

/* Macro and helper that convert PclObject obj to a C double and store the
 * value in dbl.  If conversion to double raises an exception, obj is set
 * to NULL, and the function invoking this macro returns NULL.  If obj is
 * not of float, int or long type, PCL_NOT_IMPLEMENTED is incref'ed, stored
 * in obj, and returned from the function invoking this macro. */

#define CONVERT_TO_DOUBLE(obj, dbl) \
        if (PCL_IS_FLOAT (obj)) \
                dbl = PCL_FLOAT_AS_DOUBLE (obj); \
        else if (!convert_to_double (&(obj), &(dbl))) \
                return obj;

static gboolean
convert_to_double (PclObject **p_object, gdouble *p_double)
{
        register PclObject *object = *p_object;

        if (PCL_IS_INT (object))
                *p_double = (gdouble) PCL_INT_AS_LONG (object);
        /* XXX Add check for PclLong here. */
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

static gchar float_doc[] =
"float(x) -> floating point number\n\
\n\
Convert a string or number to a floating point number, if possible.";

static PclFactory float_factory;

static gpointer float_parent_class = NULL;

static void
float_format (gchar *buffer, gint buf_len, PclObject *value, gint precision)
{
        register gchar *cp;
        gchar format[32];

        /* We want float numbers to be recognizable as such (i.e., they should
         * contain a decimal point or an exponent).  However, %g may print the
         * number as an integer.  In such cases, we append ".0" to the string.
         */
        g_assert (PCL_IS_FLOAT (value));
        g_snprintf (format, sizeof (format), "%%.%ig", precision);
        g_ascii_formatd (buffer, buf_len, format, PCL_FLOAT_AS_DOUBLE (value));
        cp = buffer;
        if (*cp == '-')
                cp++;
        while (*cp != '\0')
        {
                /* Any non-digit means it's not an integer.
                 * This takes care of NAN and INF as well. */
                if (!g_ascii_isdigit (*cp++))
                        break;
        }
        if (*cp == '\0')
        {
                *cp++ = '.';
                *cp++ = '0';
                *cp++ = '\0';
        }
}

static PclObject *
float_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        const gchar *kwlist[] = { "x", NULL };
        PclObject *x = NULL;

        if (!pcl_arg_parse_tuple_and_keywords (
                        args, kwds, "|O:float", kwlist, &x))
                return NULL;
        if (x == NULL)
                return pcl_float_from_double (0.0);
        if (PCL_IS_STRING (x))
                return pcl_float_from_string (
                       pcl_string_as_string (x), NULL);
        return pcl_number_as_float (x);
}

static gboolean
float_print (PclObject *object, FILE *stream, PclPrintFlags flags)
{
        gchar buffer[100];
        float_format (buffer, sizeof (buffer), object,
                (flags & PCL_PRINT_FLAG_RAW) ? PRECISION_STR : PRECISION_REPR);
        fputs (buffer, stream);
        return TRUE;
}

static guint
float_hash (PclObject *object)
{
        return pcl_hash_double (PCL_FLOAT_AS_DOUBLE (object));
}

static PclObject *
float_repr (PclObject *object)
{
        gchar buffer[100];
        float_format (buffer, sizeof (buffer), object, PRECISION_REPR);
        return pcl_string_from_string (buffer);
}

static PclObject *
float_str (PclObject *object)
{
        gchar buffer[100];
        float_format (buffer, sizeof (buffer), object, PRECISION_STR);
        return pcl_string_from_string (buffer);
}

static PclObject *
float_rich_compare (PclObject *object1, PclObject *object2,
                    PclRichCompareOps op)
{
        /* XXX Python's way of doing this is way, WAY more complex.
         *     I'll copy it someday... today I'm lazy. */
        gdouble a, b;
        gboolean is_true = 0;

        CONVERT_TO_DOUBLE (object1, a);
        CONVERT_TO_DOUBLE (object2, b);

        switch (op)
        {
                case PCL_EQ:
                        is_true = (a == b);
                        break;
                case PCL_NE:
                        is_true = (a != b);
                        break;
                case PCL_LE:
                        is_true = (a <= b);
                        break;
                case PCL_GE:
                        is_true = (a >= b);
                        break;
                case PCL_LT:
                        is_true = (a < b);
                        break;
                case PCL_GT:
                        is_true = (a > b);
                        break;
        }

        return pcl_bool_from_boolean (is_true);
}

static PclObject *
float_number_as_int (PclObject *object)
{
        return pcl_int_from_long ((glong) PCL_FLOAT_AS_DOUBLE (object));
}

static PclObject *
float_number_as_float (PclObject *object)
{
        return pcl_object_ref (object);
}

static PclObject *
float_number_as_complex (PclObject *object)
{
        return pcl_complex_from_doubles (PCL_FLOAT_AS_DOUBLE (object), 0.0);
}

static PclObject *
float_number_add (PclObject *object1, PclObject *object2)
{
        gdouble a, b;

        CONVERT_TO_DOUBLE (object1, a);
        CONVERT_TO_DOUBLE (object2, b);
        return pcl_float_from_double (a + b);
}

static PclObject *
float_number_subtract (PclObject *object1, PclObject *object2)
{
        gdouble a, b;

        CONVERT_TO_DOUBLE (object1, a);
        CONVERT_TO_DOUBLE (object2, b);
        return pcl_float_from_double (a - b);
}

static PclObject *
float_number_multiply (PclObject *object1, PclObject *object2)
{
        gdouble a, b;

        CONVERT_TO_DOUBLE (object1, a);
        CONVERT_TO_DOUBLE (object2, b);
        return pcl_float_from_double (a * b);
}

static PclObject *
float_number_divide (PclObject *object1, PclObject *object2)
{
        gdouble a, b;

        CONVERT_TO_DOUBLE (object1, a);
        CONVERT_TO_DOUBLE (object2, b);
        if (b == 0.0)
        {
                pcl_error_set_string (
                        pcl_exception_zero_division_error (),
                        "float division");
                return NULL;
        }
        return pcl_float_from_double (a / b);
}

static PclObject *
float_number_divmod (PclObject *object1, PclObject *object2)
{
        gdouble a, b;
        gdouble div, mod, floordiv;

        CONVERT_TO_DOUBLE (object1, a);
        CONVERT_TO_DOUBLE (object2, b);
        if (b == 0.0)
        {
                pcl_error_set_string (
                        pcl_exception_zero_division_error (),
                        "float divmod()");
                return NULL;
        }
        /* fmod is typically exact, so (a - mod) is *mathematically* an exact
         * multiple of b.  But this is fp arithmetic, and fp (a - mod) is an
         * approximation.  The result is that div may not be an exact integral
         * value after the division, although it will always be very close to
         * one. */
        mod = fmod (a, b);
        div = (a - mod) / b;
        if (mod != 0.0)
        {
                /* Ensure the modulo has the same sign as the denominator. */
                if ((b < 0.0) != (mod < 0.0))
                {
                        mod += b;
                        div -= 1.0;
                }
        }
        else
        {
                /* The modulo is zero, and in the presence of signed zeroes
                 * fmod returns different results across platforms.  Ensure it
                 * has the same sign as the denominator.  We'd like to do
                 * "mod = a * 0.0", but that may get optimized away. */
                mod *= mod;  /* hide "mod = +0" from optimizer */
                if (b < 0.0)
                        mod = -mod;
        }
        /* Snap quotient to nearest integral value. */
        if (div != 0.0)
        {
                floordiv = floor (div);
                if (div - floordiv > 0.5)
                        floordiv += 1.0;
        }
        else
        {
                /* div is zero - get the same sign as the true quotient */
                div *= div;  /* hide "div = +0" from optimizer */
                floordiv = div * a / b;  /* zero w/ sign of a/b */
        }
        return pcl_build_value ("(dd)", floordiv, mod);
}

static PclObject *
float_number_modulo (PclObject *object1, PclObject *object2)
{
        gdouble a, b;
        gdouble mod;

        CONVERT_TO_DOUBLE (object1, a);
        CONVERT_TO_DOUBLE (object2, b);
        if (b == 0.0)
        {
                pcl_error_set_string (
                        pcl_exception_zero_division_error (),
                        "float modulo");
                return NULL;
        }
        mod = fmod (a, b);
        if (mod != 0.0 && ((b < 0.0) != (mod < 0.0)))
                mod += b;
        return pcl_float_from_double (mod);
}

static PclObject *
float_number_power (PclObject *object1, PclObject *object2, PclObject *object3)
{
        /* XXX Python's implementation is much more complicated. */
        gdouble a, b, x;

        if (object3 != PCL_NONE)
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "pow(): 3rd argument not allowed unless all "
                        "arguments are integers");
                return NULL;
        }

        CONVERT_TO_DOUBLE (object1, a);
        CONVERT_TO_DOUBLE (object2, b);
        errno = 0;
        x = pow (a, b);
        if (errno != 0)
        {
                pcl_error_set_from_errno (
                        errno == ERANGE ? pcl_exception_overflow_error ()
                                        : pcl_exception_value_error ());
                return NULL;
        }
        return pcl_float_from_double (x);
}

static PclObject *
float_number_floordiv (PclObject *object1, PclObject *object2)
{
        PclObject *tuple;
        PclObject *result;

        tuple = float_number_divmod (object1, object2);
        if (tuple == NULL || tuple == PCL_NOT_IMPLEMENTED)
                return tuple;
        g_assert (PCL_IS_TUPLE (tuple));
        result = pcl_object_ref (PCL_TUPLE_GET_ITEM (tuple, 0));
        pcl_object_unref (tuple);
        return result;
}

static PclObject *
float_number_negative (PclObject *object)
{
        return pcl_float_from_double (-PCL_FLOAT_AS_DOUBLE (object));
}

static PclObject *
float_number_positive (PclObject *object)
{
        if (PCL_IS_FLOAT (object))
                return pcl_object_ref (object);
        return pcl_float_from_double (PCL_FLOAT_AS_DOUBLE (object));
}

static PclObject *
float_number_absolute (PclObject *object)
{
        return pcl_float_from_double (fabs (PCL_FLOAT_AS_DOUBLE (object)));
}

static gint
float_number_nonzero (PclObject *object)
{
        return (PCL_FLOAT_AS_DOUBLE (object) != 0.0);
}

static void
float_number_init (PclNumberIface *iface)
{
        iface->as_int = float_number_as_int;
        iface->as_float = float_number_as_float;
        iface->as_complex = float_number_as_complex;
        iface->add = float_number_add;
        iface->subtract = float_number_subtract;
        iface->multiply = float_number_multiply;
        iface->divide = float_number_divide;
        iface->divmod = float_number_divmod;
        iface->modulo = float_number_modulo;
        iface->power = float_number_power;
        iface->floordiv = float_number_floordiv;
        iface->negative = float_number_negative;
        iface->positive = float_number_positive;
        iface->absolute = float_number_absolute;
        iface->nonzero = float_number_nonzero;
}

static PclObject *
float_method_getnewargs (PclFloat *self)
{
        return pcl_build_value ("(d)", PCL_FLOAT_AS_DOUBLE (self));
}

static void
float_class_init (PclFloatClass *class)
{
        PclObjectClass *object_class;

        float_parent_class = g_type_class_peek_parent (class);

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_float_get_type_object;
        object_class->new_instance = float_new_instance;
        object_class->print = float_print;
        object_class->hash = float_hash;
        object_class->repr = float_repr;
        object_class->str = float_str;
        object_class->rich_compare = float_rich_compare;
        object_class->doc = float_doc;
}

static PclMethodDef float_methods[] = {
        { "__getnewargs__",     (PclCFunction) float_method_getnewargs,
                                PCL_METHOD_FLAG_NOARGS },
        { NULL }
};

GType
pcl_float_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclFloatClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) float_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclFloat),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                static const GInterfaceInfo number_info = {
                        (GInterfaceInitFunc) float_number_init,
                        (GInterfaceFinalizeFunc) NULL,
                        NULL   /* interface_data */
                };

                type = g_type_register_static (
                        PCL_TYPE_OBJECT, "PclFloat", &type_info, 0);
                g_type_add_interface_static (
                        type, PCL_TYPE_NUMBER, &number_info);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, float_methods);
        }
        return type;
}

/**
 * pcl_float_get_type_object:
 *
 * Returns the type object for #PclFloat.  During runtime this is the built-in
 * object %float.
 *
 * Returns: a borrowed reference to the type object for #PclFloat
 */
PclObject *
pcl_float_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_FLOAT, "float");
                pcl_register_singleton ("<type 'float'>", &object);
        }
        return object;
}

gdouble
pcl_float_as_double (PclObject *object)
{
        PclObject *float_object;
        gdouble v_double;

        if (PCL_IS_FLOAT (object))
                return PCL_FLOAT_AS_DOUBLE (object);

        if (!PCL_IS_NUMBER (object))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "a float is required");
                return -1;
        }

        float_object = pcl_number_as_float (object);
        if (float_object == NULL)
                return -1;
        if (!PCL_IS_FLOAT (float_object))
        {
                pcl_object_unref (float_object);
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "as_float should return float object");
                return -1;
        }

        v_double = PCL_FLOAT_AS_DOUBLE (float_object);
        pcl_object_unref (float_object);
        return v_double;
}

PclObject *
pcl_float_from_double (gdouble v_double)
{
        PclFloat *floatobj;
        floatobj = pcl_factory_order (&float_factory);
        floatobj->v_double = v_double;
        return PCL_OBJECT (floatobj);
}

PclObject *
pcl_float_from_string (const gchar *v_string, gchar **endptr)
{
        gdouble v_double;
        gchar *end;

        errno = 0;
        v_double = g_ascii_strtod (v_string, &end);
        if (endptr != NULL)
                *endptr = end;

        if (errno == ERANGE)
        {
                pcl_error_set_from_errno (pcl_exception_overflow_error ());
                return NULL;
        }
        else if (v_double == 0.0 && end == v_string)
        {
                /* EINVAL */
                pcl_error_set_format (
                        pcl_exception_value_error (),
                        "invalid literal for float(): %s", v_string);
                return NULL;
        }

        return pcl_float_from_double (v_double);
}

void
_pcl_float_init (void)
{
        float_factory.type = PCL_TYPE_FLOAT;
}

void
_pcl_float_fini (void)
{
        pcl_factory_close (&float_factory);
}
