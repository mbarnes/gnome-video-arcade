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

#define ERROR_LT        " is less than minimum"
#define ERROR_GT        " is greater than maximum"

#define TYPE_b          "unsigned byte integer"
#define TYPE_h          "signed short integer"
#define TYPE_i          "signed integer"

/* forward declarations */
static gchar *arg_convert_item (PclObject *arg, const gchar **p_format,
                                va_list *p_va, glong *levels);

static gchar *
arg_convert_error (const gchar *expected, PclObject *object)
{
        return g_strdup_printf ("must be %s, not %s", expected,
               PCL_IS_NONE (object) ? "None" : PCL_GET_TYPE_NAME (object));
}

static gchar *
arg_convert_simple (PclObject *arg, const gchar **p_format, va_list *p_va)
{
        const gchar *format = *p_format;
        gchar c = *format++;

        switch (c)
        {
                case 'b':  /* unsigned byte */
                {
                        guchar *p = va_arg (*p_va, guchar *);
                        glong v_long = pcl_int_as_long (arg);
                        if (v_long == -1 && pcl_error_occurred ())
                                return arg_convert_error ("integer<b>", arg);
                        else if (v_long < 0)
                        {
                                pcl_error_set_string (
                                        pcl_exception_overflow_error (),
                                        TYPE_b ERROR_LT);
                                return arg_convert_error ("integer<b>", arg);
                        }
                        else if (v_long > 255)  /* XXX hard-coded! */
                        {
                                pcl_error_set_string (
                                        pcl_exception_overflow_error (),
                                        TYPE_b ERROR_GT);
                                return arg_convert_error ("integer<b>", arg);
                        }
                        else
                                *p = (guchar) v_long;
                        break;
                }

                case 'c':  /* char */
                {
                        gchar *p = va_arg (*p_va, gchar *);
                        if (PCL_IS_STRING (arg) &&
                                        pcl_object_measure (arg) == 1)
                                *p = pcl_string_as_string (arg)[0];
                        else
                                return arg_convert_error ("char", arg);
                        break;
                }

                case 'd':  /* double */
                {
                        gdouble *p = va_arg (*p_va, gdouble *);
                        gdouble v_double = pcl_float_as_double (arg);
                        if (pcl_error_occurred ())
                                return arg_convert_error ("float<d>", arg);
                        else
                                *p = v_double;
                        break;
                }

                case 'D':  /* complex double */
                {
                        PclComplexValue *p = va_arg (*p_va, PclComplexValue *);
                        PclComplexValue v_complex;
                        v_complex = pcl_complex_as_complex (arg);
                        if (pcl_error_occurred ())
                                return arg_convert_error ("complex<D>", arg);
                        else
                                *p = v_complex;
                        break;
                }

                case 'f':  /* float */
                {
                        gfloat *p = va_arg (*p_va, gfloat *);
                        gdouble v_double = pcl_float_as_double (arg);
                        if (pcl_error_occurred ())
                                return arg_convert_error ("float<f>", arg);
                        else
                                *p = (gfloat) v_double;
                        break;
                }

                case 'h':  /* signed short int */
                {
                        gshort *p = va_arg (*p_va, gshort *);
                        glong v_long = pcl_int_as_long (arg);
                        if (v_long == -1 && pcl_error_occurred ())
                                return arg_convert_error ("integer<h>", arg);
                        else if (v_long < G_MINSHORT)
                        {
                                pcl_error_set_string (
                                        pcl_exception_overflow_error (),
                                        TYPE_h ERROR_LT);
                                return arg_convert_error ("integer<h>", arg);
                        }
                        else if (v_long > G_MAXSHORT)
                        {
                                pcl_error_set_string (
                                        pcl_exception_overflow_error (),
                                        TYPE_h ERROR_GT);
                                return arg_convert_error ("integer<h>", arg);
                        }
                        else
                                *p = (gshort) v_long;
                        break;
                }

                case 'i':  /* signed int */
                {
                        gint *p = va_arg (*p_va, gint *);
                        glong v_long = pcl_int_as_long (arg);
                        if (v_long == -1 && pcl_error_occurred ())
                                return arg_convert_error ("integer<i>", arg);
                        else if (v_long < G_MININT)
                        {
                                pcl_error_set_string (
                                        pcl_exception_overflow_error (),
                                        TYPE_i ERROR_LT);
                                return arg_convert_error ("integer<i>", arg);
                        }
                        else if (v_long > G_MAXINT)
                        {
                                pcl_error_set_string (
                                        pcl_exception_overflow_error (),
                                        TYPE_i ERROR_GT);
                                return arg_convert_error ("integer<i>", arg);
                        }
                        else
                                *p = (gint) v_long;
                        break;
                }

                case 'l':  /* long int */
                {
                        glong *p = va_arg (*p_va, glong *);
                        glong v_long = pcl_int_as_long (arg);
                        if (v_long == -1 && pcl_error_occurred ())
                                return arg_convert_error ("integer<i>", arg);
                        else
                                *p = v_long;
                        break;
                }

                case 'O':  /* object */
                {
                        PclObject **p;
                        if (*format == '!')
                        {
                                PclObject *arg_type;
                                PclObject *exp_type;

                                arg_type = PCL_GET_TYPE_OBJECT (arg);
                                exp_type = va_arg (*p_va, PclObject *);
                                p = va_arg (*p_va, PclObject **);
                                format++;
                                if (pcl_type_is_subtype (arg_type, exp_type))
                                        *p = arg;
                                else
                                {
                                        const gchar *name;
                                        name = PCL_TYPE (exp_type)->name;
                                        return arg_convert_error (name, arg);
                                }
                        }
                        else
                        {
                                p = va_arg (*p_va, PclObject **);
                                *p = arg;
                        }
                        break;
                }

                case 's':  /* string */
                {
                        if (*format == '#')
                        {
                                gchar **p = va_arg (*p_va, gchar **);
                                glong *q = va_arg (*p_va, glong *);
                                if (!PCL_IS_STRING (arg))
                                        return arg_convert_error (
                                                "string", arg);
                                *p = PCL_STRING_AS_STRING (arg);
                                *q = PCL_STRING_GET_SIZE (arg);
                                format++;
                        }
                        else
                        {
                                gchar **p = va_arg (*p_va, gchar **);
                                if (!PCL_IS_STRING (arg))
                                        return arg_convert_error (
                                                "string", arg);
                                *p = pcl_string_as_string (arg);
                        }
                        break;
                }

                case 'S':  /* string object */
                {
                        PclObject **p = va_arg (*p_va, PclObject **);
                        if (PCL_IS_STRING (arg))
                                *p = arg;
                        else
                                return arg_convert_error ("string", arg);
                        break;
                }

                default:
                        g_assert_not_reached ();
        }

        *p_format = format;
        return NULL;
}

static gchar *
arg_convert_tuple (PclObject *arg, const gchar **p_format, va_list *p_va,
                   glong *levels, gboolean top_level)
{
        const gchar *format = *p_format;
        glong level = 0;
        glong ii, nn = 0;

        while (TRUE)
        {
                gchar c = *format++;
                if (c == '(')
                {
                        if (level == 0)
                                nn++;
                        level++;
                }
                else if (c == ')')
                {
                        if (level == 0)
                                break;
                        level--;
                }
                else if (c == ':' || c == ';' || c == '\0')
                        break;
                else if (level == 0 && g_ascii_isalpha (c))
                        nn++;
        }

        if (!PCL_IS_SEQUENCE (arg) || PCL_IS_STRING (arg))
        {
                levels[0] = 0;
                return g_strdup_printf (
                       top_level ? "expected %ld arguments, not %s"
                                 : "must be %ld-item sequence, not %s",
                       nn, G_OBJECT_TYPE_NAME (arg));
        }

        ii = (glong) pcl_object_measure (arg);
        if (ii != nn)
        {
                levels[0] = 0;
                return g_strdup_printf (
                       top_level ? "expected %ld arguments, not %ld"
                                 : "must be sequence of length %ld, not %ld",
                       nn, ii);
        }

        format = *p_format;
        for (ii = 0; ii < nn; ++ii)
        {
                gchar *error_message;
                PclObject *item = pcl_sequence_get_item (arg, ii);
                error_message = arg_convert_item (
                                item, &format, p_va, levels + 1);
                pcl_object_unref (item);
                if (error_message != NULL)
                {
                        levels[0] = ii + 1;
                        return error_message;
                }
        }

        *p_format = format;
        return NULL;
}

static gchar *
arg_convert_item (PclObject *arg, const gchar **p_format,
                  va_list *p_va, glong *levels)
{
        const gchar *format = *p_format;
        gchar *error_message;

        if (*format == ')')
        {
                format++;
                error_message = arg_convert_tuple (
                                arg, &format, p_va, levels, TRUE);
                if (error_message == NULL)
                        format++;
        }
        else
        {
                error_message = arg_convert_simple (arg, &format, p_va);
                if (error_message != NULL)
                        levels[0] = 0;
        }

        if (error_message == NULL)
                *p_format = format;
        return error_message;
}

static void
arg_set_error (glong position, gchar *description, glong *levels,
               const gchar *function_name, const gchar *error_message)
{
        if (pcl_error_occurred ())
                return;

        if (error_message == NULL)
        {
                GString *buffer = g_string_sized_new (512);
                if (function_name != NULL)
                        g_string_printf (buffer, "%s() ", function_name);
                g_string_append (buffer, "argument");
                if (position > 0)
                {
                        glong ii;
                        g_string_append_printf (buffer, " %ld", position);
                        for (ii = 0; levels[ii] > 0; ii++)
                                g_string_append_printf (buffer,
                                        ", item %ld", levels[ii] - 1);
                }
                g_string_append_printf (buffer, " %s", description);
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        buffer->str);
                g_string_free (buffer, TRUE);
        }
        else
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        error_message);
}

static gchar *
arg_skip_item (const gchar **p_format, va_list *p_va)
{
        const gchar *format = *p_format;
        gchar c = *format++;

        switch (c)
        {
                /* simple codes */

                case 'b':  /* byte */
                case 'B':  /* byte as bitfield */
                case 'h':  /* short int */
                case 'H':  /* short int as bitfield */
                case 'i':  /* int */
                case 'I':  /* int sized bitfield */
                case 'l':  /* long int */
                case 'k':  /* long int sized bitfield */
                case 'f':  /* float */
                case 'd':  /* double */
                case 'D':  /* complex double */
                case 'c':  /* char */
                        (void) va_arg (*p_va, gpointer);
                        break;

                /* string codes */

                case 's':  /* string */
                case 'z':  /* string or None */
                        (void) va_arg (*p_va, gchar **);
                        if (*format == '#')
                        {
                                (void) va_arg (*p_va, glong *);
                                format++;
                        }
                        break;

                /* object codes */

                case 'S':  /* string object */
                        (void) va_arg (*p_va, PclObject **);
                        break;

                case 'O':  /* object */
                        if (*format == '!')
                        {
                                (void) va_arg (*p_va, PclType *);
                                (void) va_arg (*p_va, PclObject **);
                                format++;
                        }
                        else if (*format == '&')
                        {
                                typedef gint (*convert) (PclObject *, gpointer);
                                (void) va_arg (*p_va, convert);
                                (void) va_arg (*p_va, gpointer);
                                format++;
                        }
                        else
                                (void) va_arg (*p_va, PclObject **);
                        break;

                default:
                        return "impossible<bad format char>";
        }

        /* The "(...)" format code for tuples is not handled here because
         * it is not allowed with keyword args. */

        *p_format = format;
        return NULL;
}

static gboolean
arg_parse (PclObject *args, const gchar *format, va_list *p_va)
{
        const gchar *saved_format = format;
        const gchar *function_name = NULL;
        const gchar *error_message = NULL;
        gboolean done = FALSE;
        gchar *description;
        glong min = -1;
        glong max = 0;
        glong level = 0;
        glong levels[32];
        glong ii, size;

        g_assert (PCL_IS_TUPLE (args));
        g_assert (format != NULL);
        g_assert (p_va != NULL);

        while (!done)
        {
                gchar c = *format++;
                switch (c)
                {
                        case '(':
                                if (level == 0)
                                        max++;
                                level++;
                                break;
                        case ')':
                                if (level == 0)
                                {
                                        g_error (
                                        "%s: Excess ')' in format",
                                        G_STRFUNC);
                                }
                                level--;
                                break;
                        case '\0':
                                done = TRUE;
                                break;
                        case ':':
                                function_name = format;
                                done = TRUE;
                                break;
                        case ';':
                                error_message = format;
                                done = TRUE;
                                break;
                        default:
                                if (level == 0)
                                {
                                        if (c == 'O')
                                                max++;
                                        else if (g_ascii_isalpha (c))
                                        {
                                                /* skip encoded */
                                                if (c != 'e')
                                                        max++;
                                        }
                                        else if (c == '|')
                                                min = max;
                                }
                                break;
                }
        }

        if (level != 0)
                g_error ("%s: Missing ')' in format", G_STRFUNC);

        if (min < 0)
                min = max;

        format = saved_format;

        g_assert (PCL_IS_TUPLE (args));
        size = PCL_TUPLE_GET_SIZE (args);

        if (size < min || size > max)
        {
                if (error_message != NULL)
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                error_message);
                else
                        pcl_error_set_format (
                                pcl_exception_type_error (),
                                "%s%s takes %s %ld argument%s (%ld given)",
                                function_name ? function_name : "function",
                                function_name ? "()" : "",
                                min == max ? "exactly" :
                                size < min ? "at least" : "at most",
                                size < min ? min : max,
                                (size < min ? min : max) == 1 ? "" : "s",
                                size);
                return FALSE;
        }

        for (ii = 0; ii < size; ++ii)
        {
                if (*format == '|')
                        format++;
                description = arg_convert_item (
                        PCL_TUPLE_GET_ITEM (args, ii),
                        &format, p_va, levels);
                if (description != NULL)
                {
                        arg_set_error (
                                ii + 1, description, levels,
                                function_name, error_message);
                        g_free (description);
                        return FALSE;
                }
        }

        if (*format != '\0' && !g_ascii_isalpha (*format) &&
                        *format != '(' && *format != '|' &&
                        *format != ':' && *format != ';')
        {
                pcl_error_set_format (
                        pcl_exception_system_error (),
                        "bad format string: %s", saved_format);
                return FALSE;
        }

        return TRUE;
}

static gboolean
arg_parse_kw (PclObject *args, PclObject *kwds, const gchar *format,
              const gchar **kwlist, va_list *p_va)
{
        const gchar *saved_format = format;
        const gchar *function_name = NULL;
        const gchar *error_message = NULL;
        const gchar **p_kw = kwlist;
        gboolean done = FALSE;
        gchar *description;
        glong min = -1;
        glong max = 0;
        glong levels[32];
        glong ii, jj, size, nargs, nkwds;

        g_assert (PCL_IS_TUPLE (args));
        g_assert (kwds == NULL || PCL_IS_DICT (kwds));
        g_assert (format != NULL);
        g_assert (kwlist != NULL);
        g_assert (p_va != NULL);

        while (!done)
        {
                gchar c = *format++;
                switch (c)
                {
                        case '(':
                                pcl_error_set_string (
                                        pcl_exception_runtime_error (),
                                        "tuple found in format when using "
                                        "keyword arguments");
                                return FALSE;
                        case '\0':
                                done = TRUE;
                                break;
                        case '|':
                                min = max;
                                break;
                        case ':':
                                function_name = format;
                                done = TRUE;
                                break;
                        case ';':
                                error_message = format;
                                done = TRUE;
                                break;
                        default:
                                if (g_ascii_isalpha (c) && c != 'e')
                                {
                                        max++;
                                        if (*p_kw++ == NULL)
                                        {
                                                pcl_error_set_string (
                                                pcl_exception_runtime_error (),
                                                "more argument specifiers "
                                                "than keyword list entries");
                                                return FALSE;
                                        }
                                }
                                break;
                }
        }

        if (min < 0)
                min = max;

        format = saved_format;

        if (*p_kw != NULL)
        {
                pcl_error_set_string (
                        pcl_exception_runtime_error (),
                        "more keyword list entries than argument specifiers");
                return FALSE;
        }

        g_assert (PCL_IS_TUPLE (args));
        nargs = PCL_TUPLE_GET_SIZE (args);
        nkwds = (kwds != NULL) ? pcl_object_measure (kwds) : 0;

        /* Make sure there are no duplicate values for an argument. */
        if (nkwds > 0)
        {
                for (ii = 0; ii < nargs; ii++)
                {
                        const gchar *kw = kwlist[ii];
                        if (kw == NULL)
                                break;
                        if (pcl_dict_get_item_string (kwds, kw) != NULL)
                        {
                                pcl_error_set_format (
                                        pcl_exception_type_error (),
                                        "keyword parameter '%s' was given "
                                        "by position and by name", kw);
                                return FALSE;
                        }
                        else if (pcl_error_occurred ())
                                return FALSE;
                }
        }

        /* Required arguments missing from 'args' can be supplied by keyword
         * arguments.  Set 'size' to the number of positional arguments, and,
         * if that's less than the minimum required, add in the number of
         * required arguments that are supplied by keywords. */
        size = nargs;
        if (nkwds > 0 && nargs < min)
        {
                for (ii = nargs; ii < min; ii++)
                {
                        const gchar *kw = kwlist[ii];
                        if (pcl_dict_get_item_string (kwds, kw) != NULL)
                                size++;
                        else if (pcl_error_occurred ())
                                return FALSE;
                }
        }

        /* Make sure we got an acceptable number of arguments. */
        if (size < min || size > max)
        {
                if (error_message != NULL)
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                error_message);
                else
                        pcl_error_set_format (
                                pcl_exception_type_error (),
                                "%s%s takes %s %ld argument%s (%ld given)",
                                function_name ? function_name : "function",
                                function_name ? "()" : "",
                                min == max ? "exactly" :
                                size < min ? "at least" : "at most",
                                size < min ? min : max,
                                (size < min ? min : max) == 1 ? "" : "s",
                                size);
                return FALSE;
        }

        /* Convert the positional arguments. */
        for (ii = 0; ii < nargs; ii++)
        {
                if (*format == '|')
                        format++;
                description = arg_convert_item (
                        PCL_TUPLE_GET_ITEM (args, ii),
                        &format, p_va, levels);
                if (description != NULL)
                {
                        arg_set_error (
                                ii + 1, description, levels,
                                function_name, error_message);
                        g_free (description);
                        return FALSE;
                }
        }

        /* Handle no keyword arguments in call. */
        if (nkwds == 0)
                return TRUE;

        /* Convert the keyword arguments.  This uses the format string where
         * it was left after processing the positional arguments. */
        for (ii = nargs; ii < max; ii++)
        {
                PclObject *item;
                if (*format == '|')
                        format++;
                item = pcl_dict_get_item_string (kwds, kwlist[ii]);
                if (item != NULL)
                {
                        pcl_object_ref (item);
                        description = arg_convert_item (
                                item, &format, p_va, levels);
                        pcl_object_unref (item);
                        if (description != NULL)
                        {
                                arg_set_error (
                                        ii + 1, description, levels,
                                        function_name, error_message);
                                return FALSE;
                        }
                        if (--nkwds == 0)
                                break;
                }
                else if (pcl_error_occurred ())
                        return FALSE;
                else
                {
                        description = arg_skip_item (&format, p_va);
                        if (description != NULL)
                        {
                                arg_set_error (
                                        ii + 1, description, levels,
                                        function_name, error_message);
                                return FALSE;
                        }
                }
        }

        /* Make sure there are no extraneous keyword arguments. */
        if (nkwds > 0)
        {
                PclObject *key;
                PclObject *value;
                glong pos = 0;

                while (pcl_dict_next (kwds, &pos, &key, &value))
                {
                        gboolean match = FALSE;
                        gchar *keyword;

                        if (!PCL_IS_STRING (key))
                        {
                                pcl_error_set_string (
                                        pcl_exception_type_error (),
                                        "keywords must be strings");
                                return FALSE;
                        }
                        keyword = pcl_string_as_string (key);
                        for (jj = 0; jj < max; jj++)
                        {
                                if (strcmp (keyword, kwlist[jj]) == 0)
                                {
                                        match = TRUE;
                                        break;
                                }
                        }
                        if (!match)
                        {
                                pcl_error_set_format (
                                        pcl_exception_type_error (),
                                        "'%s' is an invalid keyword "
                                        "argument for this function",
                                        keyword);
                                return FALSE;
                        }
                }
        }

        return TRUE;
}

gboolean
pcl_arg_parse_tuple (PclObject *args, const gchar *format, ...)
{
        va_list va;
        gboolean success;

        va_start (va, format);
        success = arg_parse (args, format, &va);
        va_end (va);
        return success;
}

gboolean
pcl_arg_va_parse_tuple (PclObject *args, const gchar *format, va_list va)
{
        va_list va_copy;
        G_VA_COPY (va_copy, va);
        return arg_parse (args, format, &va_copy);
}

gboolean
pcl_arg_parse_tuple_and_keywords (PclObject *args, PclObject *kwds,
                                  const gchar *format, const gchar **kwlist,
                                  ...)
{
        va_list va;
        gboolean success;

        if (!PCL_IS_TUPLE (args) ||
                (kwds != NULL && !PCL_IS_DICT (kwds)) ||
                format == NULL || kwlist == NULL)
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        va_start (va, kwlist);
        success = arg_parse_kw (args, kwds, format, kwlist, &va);
        va_end (va);
        return success;
}

gboolean
pcl_arg_va_parse_tuple_and_keywords (PclObject *args, PclObject *kwds,
                                     const gchar *format, const gchar **kwlist,
                                     va_list va)
{
        va_list va_copy;

        if (!PCL_IS_TUPLE (args) ||
                (kwds != NULL && !PCL_IS_DICT (kwds)) ||
                format == NULL || kwlist == NULL)
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        G_VA_COPY (va_copy, va);
        return arg_parse_kw (args, kwds, format, kwlist, &va_copy);
}

gboolean
pcl_arg_unpack_tuple (PclObject *args, const gchar *name,
                      glong min, glong max, ...)
{
        glong ii, size;
        PclObject **p_object;
        va_list va;

        if (!PCL_IS_TUPLE (args))
        {
                pcl_error_set_format (
                        pcl_exception_system_error (),
                        "%s() argument list is not a tuple", G_STRFUNC);
                return FALSE;
        }

        va_start (va, max);
        g_assert (min <= max);
        size = PCL_TUPLE_GET_SIZE (args);
        if (size < min)
        {
                if (name != NULL)
                        pcl_error_set_format (
                                pcl_exception_type_error (),
                                "%s excepted %s%ld arguments, got %ld",
                                name, (min == max ? "" : "at least "),
                                min, size);
                else
                        pcl_error_set_format (
                                pcl_exception_type_error (),
                                "unpackaged tuple should have %s%ld elements, "
                                "but has %ld", (min == max ? "" : "at least "),
                                min, size);
                va_end (va);
                return FALSE;
        }
        if (size > max)
        {
                if (name != NULL)
                        pcl_error_set_format (
                                pcl_exception_type_error (),
                                "%s expected %s%ld arguments, got %ld",
                                name, (min == max ? "" : "at most "),
                                max, size);
                else
                        pcl_error_set_format (
                                pcl_exception_type_error (),
                                "unpacked tuple should have %s%ld elements, "
                                "but has %ld", (min == max ? "" : "at most "),
                                max, size);
                va_end (va);
                return FALSE;
        }
        for (ii = 0; ii < size; ++ii)
        {
                p_object = va_arg (va, PclObject **);
                *p_object = PCL_TUPLE_GET_ITEM (args, ii);
        }
        va_end (va);
        return TRUE;
}

gboolean
pcl_arg_no_keywords (const gchar *name, PclObject *kwds)
{
        if (kwds == NULL)
                return TRUE;
        if (!PCL_IS_DICT (kwds))
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }
        if (pcl_object_measure (kwds) == 0)
                return TRUE;

        pcl_error_set_format (
                pcl_exception_type_error (),
                "%s does not take keyword arguments", name);
        return FALSE;
}
