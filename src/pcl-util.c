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

GQuark
pcl_data_getsets_quark (void)
{
        static GQuark quark = 0;
        if (G_UNLIKELY (quark == 0))
                quark = g_quark_from_static_string ("getsets");
        return quark;
}

GQuark
pcl_data_members_quark (void)
{
        static GQuark quark = 0;
        if (G_UNLIKELY (quark == 0))
                quark = g_quark_from_static_string ("members");
        return quark;
}

GQuark
pcl_data_methods_quark (void)
{
        static GQuark quark = 0;
        if (G_UNLIKELY (quark == 0))
                quark = g_quark_from_static_string ("methods");
        return quark;
}

gint
pcl_repr_enter (PclObject *object)
{
        PclObject *dict;
        PclObject *list;
        glong ii;

        dict = pcl_thread_state_get_dict ();
        if (dict == NULL)
                return 0;
        list = pcl_dict_get_item_string (dict, "PclRepr");
        if (list == NULL)
        {
                list = pcl_list_new (0);
                if (list == NULL)
                        return -1;
                if (!pcl_dict_set_item_string (dict, "PclRepr", list))
                {
                        pcl_object_unref (list);
                        return -1;
                }
                pcl_object_unref (list);
        }
        ii = PCL_LIST_GET_SIZE (list);
        while (--ii >= 0)
        {
                if (PCL_LIST_GET_ITEM (list, ii) == object)
                        return 1;
        }
        if (!pcl_list_append (list, object))
                return -1;
        return 0;
}

void
pcl_repr_leave (PclObject *object)
{
        PclObject *dict;
        PclObject *list;
        glong ii;

        dict = pcl_thread_state_get_dict ();
        if (dict == NULL)
                return;
        list = pcl_dict_get_item_string (dict, "PclRepr");
        if (!PCL_IS_LIST (list))
                return;
        ii = PCL_LIST_GET_SIZE (list);
        while (--ii >= 0)
        {
                if (PCL_LIST_GET_ITEM (list, ii) == object)
                {
                        pcl_list_del_item (list, ii);
                        break;
                }
        }
}

gboolean
pcl_flush_line (void)
{
        gboolean success = TRUE;
        PclObject *file = pcl_sys_get_object ("stdout");
        if (file != NULL && pcl_file_soft_space (file, FALSE))
                success = pcl_file_write_string (file, "\n");
        return success;
}

guint
pcl_hash_double (gdouble v_double)
{
        GDoubleIEEE754 ieee754;
        gdouble fract_part;
        gdouble int_part;
        guint hash;

        /* This is designed so that PCL numbers of different types that compare
         * equal hash to the same value; otherwise comparisons of mapping keys
         * will turn out weird. */

        fract_part = modf (v_double, &int_part);
        if (fract_part == 0.0)
        {
                /* This must return the same hash as an equal int or long. */
                if (ABS (int_part) > G_MAXLONG)
                        return PCL_HASH_DEFAULT;  /* XXX no longs yet */
                /* Fits in a C long == a PCL int, so is its own hash. */
                hash = (guint) int_part;
                return PCL_HASH_VALIDATE (hash);
        }

        ieee754.v_double = v_double;
        hash = ieee754.mpn.mantissa_high + ieee754.mpn.mantissa_low +
                (ieee754.mpn.biased_exponent << 15);
        return PCL_HASH_VALIDATE (hash);
}

FILE *
pcl_open_source (const gchar *filename)
{
        PclObject *iterator;
        PclObject *next;
        PclObject *sys_path;
        gchar *basename;
        FILE *stream;

        /* Try opening the file as given. */
        stream = fopen (filename, "r");
        if (stream != NULL)
                return stream;

        /* Search sys.path for the file. */
        sys_path = pcl_sys_get_object ("path");
        iterator = pcl_object_iterate (sys_path);
        if (iterator == NULL)
        {
                pcl_error_clear ();
                return NULL;
        }
        basename = g_path_get_basename (filename);
        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                if (PCL_IS_STRING (next))
                {
                        gchar *abspath;
                        gchar *dirname;

                        dirname = PCL_STRING_AS_STRING (next);
                        abspath = g_build_filename (dirname, basename, NULL);

                        stream = fopen (abspath, "r");
                        g_free (abspath);
                }

                pcl_object_unref (next);

                if (stream != NULL)
                {
                        pcl_object_unref (iterator);
                        return stream;
                }
        }
        pcl_object_unref (iterator);
        if (pcl_error_occurred ())
                pcl_error_clear ();
        return NULL;
}

/* Helper for make_value() to scan the length of a format. */
static gint
count_format (const gchar *format, gchar end)
{
        gint count = 0;
        gint level = 0;
        while (level > 0 || *format != end)
        {
                switch (*format)
                {
                        case '\0':
                                /* premature end */
                                pcl_error_set_string (
                                        pcl_exception_system_error (),
                                        "unmatched paren in format");
                                g_assert_not_reached ();
                        case '(':
                        case '[':
                        case '{':
                                if (level == 0)
                                        count++;
                                level++;
                                break;
                        case ')':
                        case ']':
                        case '}':
                                if (level > 0)
                                        level--;
                                break;
                        case '#':
                        case '&':
                        case ',':
                        case ':':
                        case ' ':
                        case '\t':
                                break;
                        default:
                                if (level == 0)
                                        count++;
                }
                format++;
        }
        return count;
}

/* Forward Declarations */
static PclObject *make_dict (const gchar **, va_list *, gchar, gint);
static PclObject *make_list (const gchar **, va_list *, gchar, gint);
static PclObject *make_tuple (const gchar **, va_list *, gchar, gint);
static PclObject *make_value (const gchar **, va_list *);

static PclObject *
make_dict (const gchar **p_format, va_list *p_va, gchar end, gint length)
{
        gboolean failure = FALSE;
        PclObject *object;
        gint ii;

        if (length < 0)
                return NULL;
        object = pcl_dict_new ();
        for (ii = 0; ii < length; ii += 2)
        {
                PclObject *key, *value;
                key = make_value (p_format, p_va);
                if (key == NULL)
                {
                        failure = TRUE;
                        key = pcl_object_ref (PCL_NONE);
                }
                value = make_value (p_format, p_va);
                if (value == NULL)
                {
                        failure = TRUE;
                        value = pcl_object_ref (PCL_NONE);
                }
                pcl_dict_set_item (object, key, value);
                pcl_object_unref (key);
                pcl_object_unref (value);
                if (failure)
                {
                        pcl_object_unref (object);
                        return NULL;
                }
        }
        if (object != NULL && **p_format != end)
        {
                pcl_object_unref (object);
                object = NULL;
                pcl_error_set_string (
                        pcl_exception_system_error (),
                        "unmatched paren in format");
        }
        else if (end != '\0')
                ++*p_format;
        return object;
}

static PclObject *
make_list (const gchar **p_format, va_list *p_va, gchar end, gint length)
{
        gboolean failure = FALSE;
        PclObject *object;
        glong ii;

        if (length < 0)
                return NULL;
        object = pcl_list_new (length);
        for (ii = 0; ii < length; ii++)
        {
                PclObject *value = make_value (p_format, p_va);
                if (value == NULL)
                {
                        failure = TRUE;
                        value = pcl_object_ref (PCL_NONE);
                }
                pcl_list_set_item (object, ii, value);
        }
        if (object != NULL && **p_format != end)
        {
                pcl_object_unref (object);
                object = NULL;
                pcl_error_set_string (
                        pcl_exception_system_error (),
                        "unmatched paren in format");
        }
        else if (end != '\0')
                ++*p_format;
        if (failure)
        {
                pcl_object_unref (object);
                object = NULL;
        }
        return object;
}

static PclObject *
make_tuple (const gchar **p_format, va_list *p_va, gchar end, gint length)
{
        gboolean failure = FALSE;
        PclObject *object;
        glong ii;

        if (length < 0)
                return NULL;
        object = pcl_tuple_new (length);
        for (ii = 0; ii < length; ii++)
        {
                PclObject *value = make_value (p_format, p_va);
                if (value == NULL)
                {
                        failure = TRUE;
                        value = pcl_object_ref (PCL_NONE);
                }
                pcl_tuple_set_item (object, ii, value);
        }
        if (object != NULL && **p_format != end)
        {
                pcl_object_unref (object);
                object = NULL;
                pcl_error_set_string (
                        pcl_exception_system_error (),
                        "unmatched paren in format");
        }
        else if (end != '\0')
                ++*p_format;
        if (failure)
        {
                pcl_object_unref (object);
                object = NULL;
        }
        return object;
}

static PclObject *
make_value (const gchar **p_format, va_list *p_va)
{
        while (TRUE)
        {
                switch (*(*p_format)++)
                {
                        case '(':
                                return make_tuple (p_format, p_va, ')',
                                       count_format (*p_format, ')'));
                        case '[':
                                return make_list (p_format, p_va, ']',
                                       count_format (*p_format, ']'));
                        case '{':
                                return make_dict (p_format, p_va, '}',
                                       count_format (*p_format, '}'));

                        case 'b':
                        case 'B':
                        case 'h':
                        case 'i':
                                return pcl_int_from_long (
                                       (glong) va_arg (*p_va, gint));

                        case 'H':
                                return pcl_int_from_long (
                                       (glong) va_arg (*p_va, guint));

                        case 'l':
                                return pcl_int_from_long (
                                       (glong) va_arg (*p_va, glong));

                        case 'k':
                                return pcl_int_from_long (
                                       (glong) va_arg (*p_va, gulong));

                        case 'f':
                        case 'd':
                                return pcl_float_from_double (
                                       (gdouble) va_arg (*p_va, gdouble));

                        case 'D':
                                return pcl_complex_from_complex (
                                        *((PclComplexValue *) va_arg (
                                        *p_va, PclComplexValue *)));

                        case 'c':
                        {
                                gchar p[1];
                                p[0] = va_arg (*p_va, gint);
                                return pcl_string_from_string_and_size (p, 1);
                        }

                        case 's':
                        case 'z':
                        {
                                PclObject *object;
                                gchar *string = va_arg (*p_va, gchar *);
                                gssize size;
                                if (string == NULL)
                                {
                                        ++*p_format;
                                        object = PCL_NONE;
                                        pcl_object_ref (object);
                                }
                                else if (**p_format == '#')
                                {
                                        ++*p_format;
                                        size = va_arg (*p_va, gssize);
                                        object =
                                            pcl_string_from_string_and_size (
                                                string, size);
                                }
                                else
                                        object = 
                                            pcl_string_from_string (string);
                                return object;
                        }

                        case 'N':
                        case 'S':
                        case 'O':
                                if (**p_format == '&')
                                {
                                        typedef PclObject *(*converter)(gpointer);
                                        converter func = va_arg (*p_va, converter);
                                        gpointer arg = va_arg (*p_va, gpointer);
                                        ++*p_format;
                                        return (*func) (arg);
                                }
                                else
                                {
                                        PclObject *object;
                                        object = va_arg (*p_va, PclObject *);
                                        if (object != NULL)
                                        {
                                                if (*(*p_format - 1) != 'N')
                                                        pcl_object_ref (object);
                                        }
                                        return object;
                                }

                        case ':':
                        case ',':
                        case ' ':
                        case '\t':
                                break;

                        default:
                                pcl_error_set_format (
                                        pcl_exception_system_error (),
                                        "bad format char passed to %s",
                                        G_STRFUNC);
                                return NULL;
                }
        }
}

PclObject *
pcl_build_value (const gchar *format, ...)
{
        va_list va;
        PclObject *result;
        va_start (va, format);
        result = pcl_va_build_value (format, va);
        va_end (va);
        return result;
}

PclObject *
pcl_va_build_value (const gchar *format, va_list va)
{
        gint length = count_format (format, '\0');
        va_list lva;

        G_VA_COPY (lva, va);

        if (length < 0)
                return NULL;
        if (length == 0)
                return pcl_object_ref (PCL_NONE);
        if (length == 1)
                return make_value (&format, &lva);
        return make_tuple (&format, &lva, '\0', length);
}
