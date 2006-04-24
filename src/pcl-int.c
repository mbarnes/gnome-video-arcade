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

#define CONVERT_TO_LONG(obj, lng) \
        if (PCL_IS_INT (obj)) \
                lng = PCL_INT_AS_LONG (obj); \
        else \
                return pcl_object_ref (PCL_NOT_IMPLEMENTED);

#ifndef NSMALLPOSINTS
#define NSMALLPOSINTS   100
#endif
#ifndef NSMALLNEGINTS
#define NSMALLNEGINTS   5
#endif

#if NSMALLNEGINTS + NSMALLPOSINTS > 0
/* References to integers in range(-NSMALLNEGINTS, NSMALLPOSINTS)
 * are saved in this array so that they can be shared. */
static PclInt *small_ints[NSMALLNEGINTS + NSMALLPOSINTS];
#endif

static gchar int_doc[] =
"int(x[, base]) -> integer\n\
\n\
Convert a string or number to an integer, if possible.  A floating point\n\
argument will be truncated towards zero (this does not include a string\n\
representation of a floating point number!)  When converting a string, use\n\
the optional base.  It is an error to supply a base when converting a\n\
non-string.  If the argument is outside the integer range a long object\n\
will be returned instead.";

static PclFactory int_factory;

static gpointer int_parent_class = NULL;

/* Forward Declarations */
static PclObject * int_number_negative (PclObject *object);
static PclObject * int_number_positive (PclObject *object);

enum divmod_result {
        DIVMOD_OK,              /* Correct result */
        DIVMOD_OVERFLOW,        /* Overflow, try again using longs */
        DIVMOD_ERROR            /* Exception raised */
};

static enum divmod_result
int_divmod (register glong x, register glong y, glong *p_xdivy, glong *p_xmody)
{
        glong xdivy, xmody;

        if (y == 0)
        {
                pcl_error_set_string (
                        pcl_exception_zero_division_error (),
                        "integer division or modulo by zero");
                return DIVMOD_ERROR;
        }

        /* (-sys.maxint - 1) / -1 is the only overflow case */
        if (y == -1 && x < 0 && x == -x)
                return DIVMOD_OVERFLOW;

        xdivy = x / y;
        xmody = x - xdivy * y;

        /* If the signs of x and y differ, and the remainder is non-zero,
         * C89 doesn't define whether xdivy is now the floor or the ceiling
         * of the infinitely precise quotient.  We want the floor, and we
         * have it iff the remainder's sign matches y's. */
        if (xmody != 0 && ((y ^ xmody) < 0))  /* i.e. and signs differ */
        {
                xmody += y;
                --xdivy;
                g_assert (xmody != 0 && ((y ^ xmody) >= 0));
        }

        *p_xdivy = xdivy;
        *p_xmody = xmody;
        return DIVMOD_OK;
}

static PclObject *
int_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        const gchar *kwlist[] = { "x", "base", NULL };
        PclObject *x = NULL;
        gint base = -909;

        if (!pcl_arg_parse_tuple_and_keywords (
                        args, kwds, "|Oi:int", kwlist, &x, &base))
                return NULL;
        if (x == NULL)
                return pcl_int_from_long (0L);
        if (base == -909)
                return pcl_number_as_int (x);
        if (PCL_IS_STRING (x))
                return pcl_int_from_string (
                       pcl_string_as_string (x), NULL, base);
        pcl_error_set_string (
                pcl_exception_type_error (),
                "int() can't convert non-string with explicit base");
        return NULL;
}

static gboolean
int_print (PclObject *object, FILE *stream, PclPrintFlags flags)
{
        fprintf (stream, "%ld", PCL_INT_AS_LONG (object));
        return TRUE;
}

static guint
int_hash (PclObject *object)
{
        guint hash = (guint) PCL_INT_AS_LONG (object);
        return PCL_HASH_VALIDATE (hash);
}

static PclObject *
int_repr (PclObject *object)
{
        return pcl_string_from_format ("%ld", PCL_INT_AS_LONG (object));
}

static gint
int_compare (PclObject *object1, PclObject *object2)
{
        register glong a, b;

        if (PCL_IS_INT (object1))
                a = PCL_INT_AS_LONG (object1);
        else
                return 2;  /* undefined */
        if (PCL_IS_INT (object2))
                b = PCL_INT_AS_LONG (object2);
        else
                return 2;  /* undefined */
        return (a < b) ? -1 : (a > b) ? 1 : 0;
}

static PclObject *
int_number_as_int (PclObject *object)
{
        return pcl_object_ref (object);
}

static PclObject *
int_number_as_float (PclObject *object)
{
        return pcl_float_from_double ((gdouble) PCL_INT_AS_LONG (object));
}

static PclObject *
int_number_as_complex (PclObject *object)
{
        glong v_long = PCL_INT_AS_LONG (object);
        return pcl_complex_from_doubles ((gdouble) v_long, 0.0);
}

static PclObject *
int_number_oct (PclObject *object)
{
        glong v_long = PCL_INT_AS_LONG (object);
        if (v_long < 0)
                return pcl_string_from_format ("-0%lo", -v_long);
        else if (v_long > 0)
                return pcl_string_from_format ("0%lo", v_long);
        else
                return pcl_string_from_string ("0");
}

static PclObject *
int_number_hex (PclObject *object)
{
        glong v_long = PCL_INT_AS_LONG (object);
        if (v_long < 0)
                return pcl_string_from_format ("-0x%lx", -v_long);
        else
                return pcl_string_from_format ("0x%lx", v_long);
}

static PclObject *
int_number_add (PclObject *object1, PclObject *object2)
{
        register glong a, b, x;

        CONVERT_TO_LONG (object1, a);
        CONVERT_TO_LONG (object2, b);
        x = a + b;
        if ((x ^ a) >= 0 || (x ^ b) >= 0)
                return pcl_int_from_long (x);

        /* XXX Create a PclLong here. */
        pcl_error_set_string (
                pcl_exception_overflow_error (),
                "integer overflow");
        return NULL;
}

static PclObject *
int_number_subtract (PclObject *object1, PclObject *object2)
{
        register glong a, b, x;

        CONVERT_TO_LONG (object1, a);
        CONVERT_TO_LONG (object2, b);
        x = a - b;
        if ((x ^ a) >= 0 || (x ^ ~b) >= 0)
                return pcl_int_from_long (x);

        /* XXX Create a PclLong here. */
        pcl_error_set_string (
                pcl_exception_overflow_error (),
                "integer underflow");
        return NULL;
}

static PclObject *
int_number_multiply (PclObject *object1, PclObject *object2)
{
        /* The native long product x*y is either exactly right or *way* off,
         * being just the last n bits of the true product, where n is the
         * number of bits in a long (the delivered product is the true product
         * plus i*2**n for some integer i).
         *
         * The native double product (double)x * (double)y is subject to three
         * rounding errors: on a sizeof(long)==8 box, each cast to double can
         * lose info, and even on a sizeof(long)==4 box, the multiplication can
         * lose info.  But, unlike the native long product, it's not in *range*
         * trouble: even if sizeof(long)==32 (256-longs), the product easily
         * fits in the dynamic range of a double.  So the leading 50 (or so)
         * bits of the double product are correct.
         *
         * We check these two ways against each other, and declare victory if
         * they're approximately the same.  Else, because the native long
         * product is the only one that can lose catastrophic amounts of
         * information, it's the native long product that must have overflowed.
         */

        glong a, b;
        glong longprod;
        gdouble doubleprod;
        gdouble doubled_longprod;

        CONVERT_TO_LONG (object1, a);
        CONVERT_TO_LONG (object2, b);
        longprod = a * b;
        doubleprod = (gdouble) a * (gdouble) b;
        doubled_longprod = (gdouble) longprod;

        /* Fast path for normal case: small multiplicands, and no info is lost
         * in either method. */
        if (doubled_longprod == doubleprod)
                return pcl_int_from_long (longprod);

        /* Somebody somewhere lost info.  Close enough or way off?  Note
         * that a != 0 and b != 0 (else doubled_longprod == doubleprod == 0).
         * The difference either is or isn't significant compared to the true
         * value (of which doubleprod is a good approximation). */
        else
        {
                const gdouble diff = doubled_longprod - doubleprod;
                const gdouble absdiff = ABS (diff);
                const gdouble absprod = ABS (doubleprod);

                /* absdiff / absprod <= 1/32 iff 32 * absdiff <= absprod
                 * 5 good bits is "close enough" */
                if (32.0 * absdiff <= absprod)
                        return pcl_int_from_long (longprod);
        }

        /* XXX Create a PclLong here. */
        pcl_error_set_string (
                pcl_exception_overflow_error (),
                "integer overflow");
        return NULL;
}

static PclObject *
int_number_divide (PclObject *object1, PclObject *object2)
{
        /* If they aren't both ints, give someone else a chance.  In
         * particular, this lets int/long get handled by longs, which
         * underflows to 0 gracefully if the long is too big to convert
         * to float. */
        if (PCL_IS_INT (object1) && PCL_IS_INT (object2))
        {
                PclFloatClass *class;
                PclNumberIface *iface;

                class = g_type_class_peek_static (PCL_TYPE_FLOAT);
                iface = g_type_interface_peek (class, PCL_TYPE_NUMBER);
                return iface->divide (object1, object2);
        }
        return pcl_object_ref (PCL_NOT_IMPLEMENTED);
}

static PclObject *
int_number_divmod (PclObject *object1, PclObject *object2)
{
        glong a, b;
        glong div, mod;

        CONVERT_TO_LONG (object1, a);
        CONVERT_TO_LONG (object2, b);
        switch (int_divmod (a, b, &div, &mod))
        {
                case DIVMOD_OK:
                        return pcl_build_value ("(ll)", div, mod);
                case DIVMOD_OVERFLOW:
                        /* XXX Create a PclLong here */
                        pcl_error_set_string (
                                pcl_exception_overflow_error (),
                                "integer overflow");
                        /* fallthrough */
                default:
                        return NULL;
        }
}

static PclObject *
int_number_modulo (PclObject *object1, PclObject *object2)
{
        glong a, b;
        glong div, mod;

        CONVERT_TO_LONG (object1, a);
        CONVERT_TO_LONG (object2, b);
        switch (int_divmod (a, b, &div, &mod))
        {
                case DIVMOD_OK:
                        return pcl_int_from_long (mod);
                case DIVMOD_OVERFLOW:
                        /* XXX Create a PclLong here */
                        pcl_error_set_string (
                                pcl_exception_overflow_error (),
                                "integer overflow");
                        /* fallthrough */
                default:
                        return NULL;
        }
}

static PclObject *
int_number_power (PclObject *object1, PclObject *object2, PclObject *object3)
{
        register long a, b, c = 0, x;
        register long temp, prev;

        CONVERT_TO_LONG (object1, a);
        CONVERT_TO_LONG (object2, b);
        if (b < 0)
        {
                PclFloatClass *class;
                PclNumberIface *iface;

                if (object3 != PCL_NONE)
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "pow(): 2nd argument cannot be negative when "
                                "3rd argument is specified");
                        return NULL;
                }

                class = g_type_class_peek_static (PCL_TYPE_FLOAT);
                iface = g_type_interface_peek (class, PCL_TYPE_NUMBER);
                return iface->power (object1, object2, object3);
        }
        if (object3 != PCL_NONE)
        {
                CONVERT_TO_LONG (object3, c);
                if (c == 0)
                {
                        pcl_error_set_string (
                                pcl_exception_value_error (),
                                "pow(): 3rd argument cannot be 0");
                        return NULL;
                }
        }

        temp = a;
        x = 1;
        while (b > 0)
        {
                prev = x;
                if (b & 1)
                {
                        x *= temp;
                        if (temp == 0)
                                break;
                        if (x / temp != prev)
                        {
                                /* XXX Create a PclLong here */
                                pcl_error_set_string (
                                        pcl_exception_overflow_error (),
                                        "integer overflow");
                                return NULL;
                        }
                }
                b >>= 1;
                if (b == 0)
                        break;
                prev = temp;
                temp *= temp;
                if (prev != 0 && temp / prev != prev)
                {
                        /* XXX Create a PclLong here */
                        pcl_error_set_string (
                                pcl_exception_overflow_error (),
                                "integer overflow");
                        return NULL;
                }
                if (c != 0)
                {
                        x %= c;
                        temp %= c;
                }
        }
        if (c != 0)
        {
                glong div, mod;
                switch (int_divmod (x, c, &div, &mod))
                {
                        case DIVMOD_OK:
                                x = mod;
                                break;
                        case DIVMOD_OVERFLOW:
                                /* XXX Create a PclLong here */
                                pcl_error_set_string (
                                        pcl_exception_overflow_error (),
                                        "integer overflow");
                                /* fallthrough */
                        default:
                                return NULL;
                }
        }

        return pcl_int_from_long (x);
}

static PclObject *
int_number_floordiv (PclObject *object1, PclObject *object2)
{
        glong a, b;
        glong div, mod;

        CONVERT_TO_LONG (object1, a);
        CONVERT_TO_LONG (object2, b);
        switch (int_divmod (a, b, &div, &mod))
        {
                case DIVMOD_OK:
                        return pcl_int_from_long (div);
                case DIVMOD_OVERFLOW:
                        /* XXX Create a PclLong here */
                        pcl_error_set_string (
                                pcl_exception_overflow_error (),
                                "integer overflow");
                        /* fallthrough */
                default:
                        return NULL;
        }
}

static PclObject *
int_number_negative (PclObject *object)
{
        register glong a, x;
        a = PCL_INT_AS_LONG (object);
        x = -a;
        if (a < 0 && x < 0)
        {
                /* XXX Create a long here */
                pcl_error_set_string (
                        pcl_exception_overflow_error (),
                        "integer overflow");
                return NULL;
        }
        return pcl_int_from_long (x);
}

static PclObject *
int_number_positive (PclObject *object)
{
        if (PCL_IS_INT (object))
                return pcl_object_ref (object);
        return pcl_int_from_long (PCL_INT_AS_LONG (object));
}

static PclObject *
int_number_absolute (PclObject *object)
{
        if (PCL_INT_AS_LONG (object) >= 0)
                return int_number_positive (object);
        else
                return int_number_negative (object);
}

static PclObject *
int_number_bitwise_or (PclObject *object1, PclObject *object2)
{
        register glong a, b;

        CONVERT_TO_LONG (object1, a);
        CONVERT_TO_LONG (object2, b);
        return pcl_int_from_long (a | b);
}

static PclObject *
int_number_bitwise_xor (PclObject *object1, PclObject *object2)
{
        register glong a, b;

        CONVERT_TO_LONG (object1, a);
        CONVERT_TO_LONG (object2, b);
        return pcl_int_from_long (a ^ b);
}

static PclObject *
int_number_bitwise_and (PclObject *object1, PclObject *object2)
{
        register glong a, b;

        CONVERT_TO_LONG (object1, a);
        CONVERT_TO_LONG (object2, b);
        return pcl_int_from_long (a & b);
}

static PclObject *
int_number_bitwise_lshift (PclObject *object1, PclObject *object2)
{
        /* XXX Incomplete, Python is much more complicated */
        register glong a, b;

        CONVERT_TO_LONG (object1, a);
        CONVERT_TO_LONG (object2, b);
        if (b < 0)
        {
                pcl_error_set_string (
                        pcl_exception_value_error (),
                        "negative shift count");
                return NULL;
        }
        if (a == 0 || b == 0)
                return int_number_positive (object1);
        return pcl_int_from_long (a << b);
}

static PclObject *
int_number_bitwise_rshift (PclObject *object1, PclObject *object2)
{
        /* XXX Incomplete, Python is much more complicated */
        register glong a, b;

        CONVERT_TO_LONG (object1, a);
        CONVERT_TO_LONG (object2, b);
        if (b < 0)
        {
                pcl_error_set_string (
                        pcl_exception_value_error (),
                        "negative shift count");
                return NULL;
        }
        if (a == 0 || b == 0)
                return int_number_positive (object1);
        return pcl_int_from_long (a >> b);
}

static PclObject *
int_number_bitwise_invert (PclObject *object)
{
        return pcl_int_from_long (~PCL_INT_AS_LONG (object));
}

static gint
int_number_nonzero (PclObject *object)
{
        return (PCL_INT_AS_LONG (object) != 0);
}

static void
int_number_init (PclNumberIface *iface)
{
        iface->as_int = int_number_as_int;
        iface->as_float = int_number_as_float;
        iface->as_complex = int_number_as_complex;
        iface->oct = int_number_oct;
        iface->hex = int_number_hex;
        iface->add = int_number_add;
        iface->subtract = int_number_subtract;
        iface->multiply = int_number_multiply;
        iface->divide = int_number_divide;
        iface->divmod = int_number_divmod;
        iface->modulo = int_number_modulo;
        iface->power = int_number_power;
        iface->floordiv = int_number_floordiv;
        iface->negative = int_number_negative;
        iface->positive = int_number_positive;
        iface->absolute = int_number_absolute;
        iface->bitwise_or = int_number_bitwise_or;
        iface->bitwise_xor = int_number_bitwise_xor;
        iface->bitwise_and = int_number_bitwise_and;
        iface->bitwise_lshift = int_number_bitwise_lshift;
        iface->bitwise_rshift = int_number_bitwise_rshift;
        iface->bitwise_invert = int_number_bitwise_invert;
        iface->nonzero = int_number_nonzero;
}

static PclObject *
int_method_getnewargs (PclInt *self)
{
        return pcl_build_value ("(l)", PCL_INT_AS_LONG (self));
}

static void
int_class_init (PclIntClass *class)
{
        PclObjectClass *object_class;

        int_parent_class = g_type_class_peek_parent (class);

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_int_get_type_object;
        object_class->new_instance = int_new_instance;
        object_class->print = int_print;
        object_class->hash = int_hash;
        object_class->repr = int_repr;
        object_class->compare = int_compare;
        object_class->doc = int_doc;
}

static PclMethodDef int_methods[] = {
        { "__getnewargs__",     (PclCFunction) int_method_getnewargs,
                                PCL_METHOD_FLAG_NOARGS },
        { NULL }
};

GType
pcl_int_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclIntClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) int_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclInt),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                static const GInterfaceInfo number_info = {
                        (GInterfaceInitFunc) int_number_init,
                        (GInterfaceFinalizeFunc) NULL,
                        NULL   /* interface_data */
                };

                type = g_type_register_static (
                        PCL_TYPE_OBJECT, "PclInt", &type_info, 0);
                g_type_add_interface_static (
                        type, PCL_TYPE_NUMBER, &number_info);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, int_methods);
        }
        return type;
}

/**
 * pcl_int_get_type_object:
 *
 * Returns the type object for #PclInt.  During runtime this is the built-in
 * object %int.
 *
 * Returns: a borrowed reference to the type object for #PclInt
 */
PclObject *
pcl_int_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_INT, "int");
                pcl_register_singleton ("<type 'int'>", &object);
        }
        return object;
}

glong
pcl_int_as_long (PclObject *object)
{
        PclObject *int_object;
        glong v_long;

        if (PCL_IS_INT (object))
                return PCL_INT_AS_LONG (object);

        if (!PCL_IS_NUMBER (object))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "an integer is required");
                return -1;
        }

        int_object = pcl_number_as_int (object);
        if (int_object == NULL)
                return -1;
        if (!PCL_IS_INT (int_object))
        {
                pcl_object_unref (int_object);
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "as_int should return int object");
                return -1;
        }

        v_long = PCL_INT_AS_LONG (int_object);
        pcl_object_unref (int_object);
        return v_long;
}

PclObject *
pcl_int_from_long (glong v_long)
{
        PclInt *intobj;
#if NSMALLNEGINTS + NSMALLPOSINTS > 0
        if (-NSMALLNEGINTS <= v_long && v_long < NSMALLPOSINTS)
                return pcl_object_ref (small_ints[v_long + NSMALLNEGINTS]);
#endif
        intobj = pcl_factory_order (&int_factory);
        intobj->v_long = v_long;
        return PCL_OBJECT (intobj);
}

PclObject *
pcl_int_from_string (const gchar *v_string, gchar **endptr, guint base)
{
        guint64 v_uint64;
        gchar *end;

        errno = 0;
        v_uint64 = g_ascii_strtoull (v_string, &end, base);
        if (v_uint64 > G_MAXLONG)
                errno = ERANGE;
        if (endptr != NULL)
                *endptr = end;

        if (errno == EINVAL)
        {
                pcl_error_set_string (
                        pcl_exception_value_error (),
                        "int() base must be >= 2 and <= 36");
                return NULL;
        }
        else if (errno != 0)
        {
                g_assert (errno == ERANGE);
                /* XXX Python would convert it to a PyLong. */
                pcl_error_set_from_errno (pcl_exception_overflow_error ());
                return NULL;
        }
        else if (v_uint64 == 0 && end == v_string)
        {
                pcl_error_set_format (
                        pcl_exception_value_error (),
                        "invalid literal for int(): %s", v_string);
                return NULL;
        }

        return pcl_int_from_long ((glong) v_uint64);
}

void
_pcl_int_init (void)
{
#if NSMALLNEGINTS + NSMALLPOSINTS > 0
        gint ii;

        for (ii = 0; ii < G_N_ELEMENTS (small_ints); ii++)
        {
                small_ints[ii] = pcl_object_new (PCL_TYPE_INT, NULL);
                small_ints[ii]->v_long = ii - NSMALLNEGINTS;
        }
#endif

        int_factory.type = PCL_TYPE_INT;
}

void
_pcl_int_fini (void)
{
#if NSMALLNEGINTS + NSMALLPOSINTS > 0
        gint ii;

        for (ii = 0; ii < G_N_ELEMENTS (small_ints); ii++)
        {
                if (small_ints[ii] != NULL)
                {
                        pcl_object_unref (small_ints[ii]);
                        small_ints[ii] = NULL;
                }
        }
#endif

        pcl_factory_close (&int_factory);
}
