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

static gchar string_doc[] =
"str(object) -> string\n\
\n\
Return a nice string representation of the object.\n\
If the argument is a string, the return value is the same object.";

GData *internal_strings = NULL;
static gpointer string_parent_class = NULL;

#define STR_LEFT_STRIP 1
#define STR_RIGHT_STRIP 2
#define STR_BOTH_STRIP 3

/* Begin support code for pcl_string_format() */

/* Format Flags */
#define FMT_FLAG_LJUST  (1 << 0)        /* '-' */
#define FMT_FLAG_SIGN   (1 << 1)        /* '+' */
#define FMT_FLAG_BLANK  (1 << 2)        /* ' ' */
#define FMT_FLAG_ALT    (1 << 3)        /* '#' */
#define FMT_FLAG_ZERO   (1 << 4)        /* '0' */

static PclObject *
string_format_next_arg (PclObject *args, gint arg_len, gint *p_arg_idx)
{
        gint arg_idx = *p_arg_idx;

        if (arg_idx < arg_len)
        {
                (*p_arg_idx)++;
                if (arg_len < 0)
                        return args;
                return pcl_tuple_get_item (args, arg_idx);
        }

        pcl_error_set_string (
                pcl_exception_type_error (),
                "not enough arguments for format string");
        return NULL;
}

static gboolean
string_format_float (GString *buffer, gint flags, gint prec, gchar ch,
                     PclObject *value)
{
        static gchar format[64];
        gdouble v_double;

        v_double = pcl_float_as_double (value);
        if (v_double == -1.0 && pcl_error_occurred ())
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "float argument required");
                return FALSE;
        }
        if (prec < 0)
                prec = 6;
        if (ch == 'f' && fabs (v_double) / 1e25 >= 1e25)
                ch = 'g';
        g_snprintf (format, sizeof (format), "%%%s.%d%c",
                (flags & FMT_FLAG_ALT) ? "#" : "", prec, ch);
        g_string_append_printf (buffer, format, v_double);
        return TRUE;
}

static gboolean
string_format_int (GString *buffer, gint flags, gint prec, gchar ch,
                   PclObject *value)
{
        static gchar format[64];
        glong v_long;
        gchar *sign;

        v_long = pcl_int_as_long (value);
        if (v_long == -1 && pcl_error_occurred ())
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "int argument required");
                return FALSE;
        }
        if (v_long < 0 && ch == 'u')
                ch = 'd';
        if (v_long < 0 && (ch == 'x' || ch == 'X' || ch == 'o'))
                sign = "-";
        else
                sign = "";
        if (prec < 0)
                prec = 1;
        if ((flags & FMT_FLAG_ALT) && (ch == 'x' || ch == 'X'))
                g_snprintf (format, sizeof (format), "%s0%c%%.%dl%c",
                        sign, ch, prec, ch);
        else
                g_snprintf (format, sizeof (format), "%s%%%s.%dl%c",
                        sign, (flags & FMT_FLAG_ALT) ? "#" : "", prec, ch);
        g_string_append_printf (buffer, format, sign[0] ? -v_long : v_long);
        return TRUE;
}

static gboolean
string_format_char (GString *buffer, PclObject *value)
{
        gchar ch;

        if (PCL_IS_STRING (value))
        {
                if (PCL_STRING_GET_SIZE (value) != 1)
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "%c requires int or char");
                        return FALSE;
                }
                ch = PCL_STRING_AS_STRING (value)[0];
        }
        else
        {
                glong v_long;

                v_long = pcl_int_as_long (value);
                if (v_long == -1 && pcl_error_occurred ())
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "%c requires int or char");
                        return FALSE;
                }
                else if (v_long < 0)
                {
                        pcl_error_set_string (
                                pcl_exception_overflow_error (),
                                "unsigned byte integer "
                                "is less than minimum");
                        return FALSE;
                }
                else if (v_long > 255)  /* XXX hardcoded! */
                {
                        pcl_error_set_string (
                                pcl_exception_overflow_error (),
                                "unsigned byte integer "
                                "is greater than maximum");
                        return FALSE;
                }
                ch = (gchar) v_long;
        }
        g_string_append_c (buffer, ch);
        return TRUE;
}

/* End of support code for pcl_string_format() */

static PclObject *
string_from_g_string (GString *g_string)
{
        PclString *strobj = pcl_object_new (PCL_TYPE_STRING, NULL);
        strobj->string = g_string;
        return PCL_OBJECT (strobj);
}

static PclObject *
string_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        const gchar *kwlist[] = { "object", NULL };
        PclObject *object = NULL;

        if (!pcl_arg_parse_tuple_and_keywords (
                        args, kwds, "|O:str", kwlist, &object))
                return NULL;
        if (object == NULL)
                return pcl_string_from_string ("");
        return pcl_object_str (object);
}

static gboolean
string_print (PclObject *object, FILE *stream, PclPrintFlags flags)
{
        gchar *string = PCL_STRING_AS_STRING (object);
        glong ii, length = PCL_STRING_GET_SIZE (object);
        gint quote;
        gchar c;

        if (flags & PCL_PRINT_FLAG_RAW)
        {
                fwrite (string, length, 1, stream);
                return TRUE;
        }

        /* figure out which quote to use; single is preferred */
        quote = '\'';
        if (memchr (string, '\'', length) &&
                !memchr (string, '"', length))
                quote = '"';

        fputc (quote, stream);
        for (ii = 0; ii < length; ii++)
        {
                c = string[ii];
                if (c == quote || c == '\\')
                        fprintf (stream, "\\%c", c);
                else if (c == '\t')
                        fprintf (stream, "\\t");
                else if (c == '\n')
                        fprintf (stream, "\\n");
                else if (c == '\r')
                        fprintf (stream, "\\r");
                else if (c < 0x20 || c >= 0x7F)
                        fprintf (stream, "\\x%02x", c & 0xFF);
                else
                        fputc (c, stream);
        }
        fputc (quote, stream);
        return TRUE;
}

static guint
string_hash (PclObject *object)
{
        PclString *self = PCL_STRING (object);
        register glong length;
        register guchar *cp;
        register guint hash;

        if (self->hash != PCL_HASH_INVALID)
                return self->hash;
        length = PCL_STRING_GET_SIZE (self);
        cp = (guchar *) PCL_STRING_AS_STRING (self);
        hash = *cp << 7;
        while (--length >= 0)
                hash = (1000003 * hash) ^ *cp++;
        hash ^= PCL_STRING_GET_SIZE (self);
        self->hash = PCL_HASH_VALIDATE (hash);
        return self->hash;
}

static PclObject *
string_repr (PclObject *object)
{
        return pcl_string_from_format ("'%s'", PCL_STRING_AS_STRING (object));
}

static PclObject *
string_str (PclObject *object)
{
        return pcl_object_ref (object);
}

static PclObject *
string_rich_compare (PclObject *object1, PclObject *object2,
                     PclRichCompareOps op)
{
        gchar *string1, *string2;
        glong length1, length2, min_length;
        gboolean is_true = 0;
        gint cmp;

        if (!PCL_IS_STRING (object1) || !PCL_IS_STRING (object2))
                return pcl_object_ref (PCL_NOT_IMPLEMENTED);

        if (object1 == object2)
        {
                switch (op)
                {
                        case PCL_EQ:
                        case PCL_LE:
                        case PCL_GE:
                                return pcl_object_ref (PCL_TRUE);
                        case PCL_NE:
                        case PCL_LT:
                        case PCL_GT:
                                return pcl_object_ref (PCL_FALSE);
                }
        }

        string1 = PCL_STRING_AS_STRING (object1);
        length1 = PCL_STRING_GET_SIZE (object1);

        string2 = PCL_STRING_AS_STRING (object2);
        length2 = PCL_STRING_GET_SIZE (object2);

        if (op == PCL_EQ)
        {
                /* Supporting PCL_NE here as well does not save much time,
                 * since PCL_NE is rarely used. */
                if (length1 == length2 && (string1[0] == string2[0] &&
                                memcmp (string1, string2, length1) == 0))
                        return pcl_object_ref (PCL_TRUE);
                else
                        return pcl_object_ref (PCL_FALSE);
        }

        min_length = MIN (length1, length2);
        if (min_length > 0)
        {
                cmp = *string1 - *string2;
                if (cmp == 0)
                        cmp = memcmp (string1, string2, min_length);
        }
        else
                cmp = 0;
        if (cmp == 0)
                cmp = (length1 < length2) ? -1 : (length1 > length2) ? 1 : 0;
        switch (op)
        {
                case PCL_LT:
                        is_true = (cmp < 0);
                        break;
                case PCL_LE:
                        is_true = (cmp <= 0);
                        break;
                case PCL_EQ:
                        g_assert_not_reached ();
                        break;
                case PCL_NE:
                        is_true = (cmp != 0);
                        break;
                case PCL_GT:
                        is_true = (cmp > 0);
                        break;
                case PCL_GE:
                        is_true = (cmp >= 0);
                        break;
                default:
                        return pcl_object_ref (PCL_NOT_IMPLEMENTED);
        }

        return pcl_bool_from_boolean (is_true);
}

static gint
string_contains (PclObject *object, PclObject *value)
{
        gchar *haystack = PCL_STRING_AS_STRING (object);
        gchar *needle = PCL_STRING_AS_STRING (value);
        return (g_strrstr (haystack, needle) != NULL);
}

static PclObject *
string_iterate (PclObject *object)
{
        return pcl_sequence_iterator_new (object);
}

static glong
string_measure (PclObject *object)
{
        return PCL_STRING_GET_SIZE (object);
}

static PclObject *
string_get_item (PclObject *object, PclObject *subscript)
{
        if (PCL_IS_INT (subscript))
        {
                gchar *string;
                glong length;
                glong index;

                string = PCL_STRING_AS_STRING (object);
                length = PCL_STRING_GET_SIZE (object);
                index = pcl_int_as_long (subscript);
                if (index < 0)
                        index += length;
                if (index >= 0 && index < length)
                {
                        gchar result[] = { '\0', '\0' };
                        result[0] = string[index];
                        return pcl_string_from_string (result);
                }

                pcl_error_set_string (
                        pcl_exception_index_error (),
                        "string index out of range");
                return NULL;
        }
        else if (PCL_IS_SLICE (subscript))
        {
                gchar *string;
                glong start, stop, step, length, ii;
                GString *g_string;

                if (!pcl_slice_get_indices (PCL_SLICE (subscript),
                                (guint) PCL_STRING_GET_SIZE (object),
                                &start, &stop, &step, &length))
                        return NULL;

                string = PCL_STRING_AS_STRING (object);
                g_string = g_string_sized_new (length);
                if (length > 0)
                        for (ii = start; ii < stop; ii += step)
                                g_string_append_c (g_string, string[ii]);
                return string_from_g_string (g_string);
        }

        pcl_error_set_string (
                pcl_exception_type_error (),
                "string indices must be integers");
        return NULL;
}

static PclObject *
string_sequence_concat (PclObject *object1, PclObject *object2)
{
        if (!PCL_IS_STRING (object2))
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "cannot concatenate '%s' and '%s' objects",
                        PCL_GET_TYPE_NAME (object1),
                        PCL_GET_TYPE_NAME (object2));
                return NULL;
        }

        return pcl_string_from_format ("%s%s",
                PCL_STRING_AS_STRING (object1),
                PCL_STRING_AS_STRING (object2));
}

static PclObject *
string_sequence_repeat (PclObject *object, glong times)
{
        GString *g_string;
        glong ii, length;

        times = MAX (0, times);
        length = PCL_STRING_GET_SIZE (object);

        g_string = g_string_sized_new (length * times);
        for (ii = 0; ii < times; ii++)
                g_string_append (g_string, PCL_STRING_AS_STRING (object));
        return string_from_g_string (g_string);
}

static PclObject *
string_sequence_as_list (PclObject *object)
{
        gchar *string = PCL_STRING_AS_STRING (object);
        glong ii, length = PCL_STRING_GET_SIZE (object);
        PclObject *result = pcl_list_new (length);
        PclObject *item;

        for (ii = 0; ii < length; ii++)
        {
                /* pcl_list_set_item() steals reference */
                item = pcl_string_from_string_and_size (&string[ii], 1);
                pcl_list_set_item (result, ii, item);
        }

        return result;
}

static PclObject *
string_sequence_as_tuple (PclObject *object)
{
        gchar *string = PCL_STRING_AS_STRING (object);
        glong ii, length = PCL_STRING_GET_SIZE (object);
        PclObject *result = pcl_tuple_new (length);
        PclObject *item;

        for (ii = 0; ii < length; ii++)
        {
                /* pcl_tuple_set_item() steals reference */
                item = pcl_string_from_string_and_size (&string[ii], 1);
                pcl_tuple_set_item (result, ii, item);
        }

        return result;
}

static void
string_sequence_init (PclSequenceIface *iface)
{
        iface->concat = string_sequence_concat;
        iface->repeat = string_sequence_repeat;
        iface->as_list = string_sequence_as_list;
        iface->as_tuple = string_sequence_as_tuple;
}

/* Returns a new string object, padded as desired */
static PclObject *
string_pad (gchar *s, gint size, gint margin, gint left, gchar fill)
{
        gint i, right = size + left, total_size = size + margin;
        PclObject *new_obj = pcl_string_from_string_and_size (NULL, total_size);
        gchar *new_s = PCL_STRING_AS_STRING (new_obj);
        for (i = 0; i < total_size; i++)
        {
                if (i < left || i >= right)
                        *new_s++ = fill;
                else
                        *new_s++ = *s++;
        }
        return new_obj;
}

static void
string_adjust_slice (gint *start, gint *stop, gint *str_len)
{
        if (*stop > *str_len)
                *stop = *str_len;
        else if (*stop < 0)
                *stop += *str_len;
        if (*stop < 0)
                *stop = 0;
        if (*start < 0)
                *start += *str_len;
        else if (*start > *str_len)
                *start = *str_len;
        if (*start < 0)
                *start = 0;
}

static PclObject *
string_strip (PclObject *self, gint side)
{
        gchar *str = PCL_STRING_AS_STRING (self);
        gint first = 0, last, len = PCL_STRING_GET_SIZE (self);
        last = len;

        if (side != STR_RIGHT_STRIP)
        {
                while(first < last && g_ascii_isspace (str[first]))
                        first++;
        }
        if (side != STR_LEFT_STRIP)
        {
                last--;
                while (first <= last && g_ascii_isspace (str[last]))
                        last--;
                last++;
        }
        
        if (last - first == len)
                return pcl_object_ref (self);
        
        return pcl_string_from_string_and_size (str + first, last - first);
}

static PclObject *
string_arg_strip (PclObject *self, gint side, PclObject *args)
{
        gchar *str = PCL_STRING_AS_STRING (self);
        PclObject *charset = NULL;
        gchar *setstr = NULL;
        gint setstrlen;
        gint first = 0, last, len = PCL_STRING_GET_SIZE(self);
        last = len;
        
        if (!pcl_arg_parse_tuple (args, "O:arg_strip", &charset))
                return NULL;
                
        if (charset == NULL || charset == PCL_NONE)
                return string_strip (self, side);
                
        if (!PCL_IS_STRING (charset))
        {
                pcl_error_set_string (pcl_exception_type_error (),
                                      "argument 0 is not a string");
                return NULL;
        }
        
        setstr = PCL_STRING_AS_STRING (charset);
        setstrlen = PCL_STRING_GET_SIZE (charset);

        if (side != STR_RIGHT_STRIP)
        {
                while (first < last && memchr (setstr, str[first], setstrlen))
                        first++;
        }
        if (side != STR_LEFT_STRIP)
        {
                last--;
                while (first <= last && memchr (setstr, str[last], setstrlen))
                        last--;
                last++;
        }
        
        if (last - first == len)
                return pcl_object_ref (self);
        
        return pcl_string_from_string_and_size (str + first, last - first);
}

static PclObject *
string_method_getnewargs (PclString *self)
{
        gchar *string = PCL_STRING_AS_STRING (self);
        glong length = PCL_STRING_GET_SIZE (self);
        return pcl_build_value ("(s#)", string, length);
}

static PclObject *
string_method_mod (PclString *self, PclObject *other)
{
        return pcl_string_format (PCL_OBJECT (self), other);
}

static PclObject *
string_method_imod (PclString *self, PclObject *other)
{
        return pcl_string_format (other, PCL_OBJECT (self));
}

static PclObject *
string_method_capitalize (PclString *self)
{
        gchar *s = PCL_STRING_AS_STRING (PCL_OBJECT (self));
        glong i, n = PCL_STRING_GET_SIZE (PCL_OBJECT (self));
        PclObject *new_obj = pcl_string_from_string_and_size (NULL, n);
        gchar *new_s = PCL_STRING_AS_STRING (new_obj);
        if (n > 0)
        {
                if (g_ascii_islower(*s))
                        *new_s++ = g_ascii_toupper(*s++);
                else
                        *new_s++ = *s++;
        }
        for (i = 1; i < n; i++)
        {
                if (g_ascii_isupper(*s))
                        *new_s++ = g_ascii_tolower(*s++);
                else
                        *new_s++ = *s++;
        }
        
        return new_obj;
}

static PclObject *
string_method_center (PclString *self, PclObject *args)
{
        gint width, margin, left, size;
        gchar fillchar = ' ';
        gchar *s = PCL_STRING_AS_STRING (PCL_OBJECT (self));
        
        if (!pcl_arg_parse_tuple (args, "i|c:center", &width, &fillchar))
                return NULL;
        
        size = PCL_STRING_GET_SIZE (PCL_OBJECT (self));        
        if (size >= width)
                return pcl_object_ref(self);
                
        margin = width - size;
        left = margin / 2 + (margin & width & 1);
                
        return string_pad (s, size, margin, left, fillchar);
}

static PclObject *
string_method_count (PclString *self, PclObject *args)
{
        gint sub_len, i, j, start = 0, count = 0;
        gchar *sub = NULL;
        gchar *str = PCL_STRING_AS_STRING (self);
        gint stop, str_len = PCL_STRING_GET_SIZE (self);
        stop = str_len;
        
        if (!pcl_arg_parse_tuple (args, "s|ii:count", &sub, &start, &stop))
                return NULL;
        
        string_adjust_slice (&start, &stop, &str_len);
        sub_len = strlen(sub);
        
        /* Quick exception check */
        if (sub_len == 0 || sub_len > stop - start)
                return pcl_int_from_long (0);
        
        for (i = start; i <= stop - sub_len; i++)
        {
                j = 0;
                while (j < sub_len)
                {
                        if (sub[j] != str[i + j])
                                break;
                        j++;
                }
                if (j == sub_len)
                {
                        /* Match found */
                        count++;
                        i += (j - 1);
                }
        }
        
        return pcl_int_from_long ((glong) count);
}

static PclObject *
string_method_decode (PclString *self, PclObject *args)
{
        /* TODO: Implement This */
        return pcl_object_ref (PCL_NOT_IMPLEMENTED);
}

static PclObject *
string_method_encode (PclString *self, PclObject *args)
{
        /* TODO: Implement This */
        return pcl_object_ref (PCL_NOT_IMPLEMENTED);
}

static PclObject *
string_method_endswith (PclString *self, PclObject *args)
{
        gchar *str = PCL_STRING_AS_STRING (self);
        gchar *sub, *search_str;
        gint start = 0, str_len = PCL_STRING_GET_SIZE (self);
        gint stop = str_len;
        
        if (!pcl_arg_parse_tuple (args, "s|ii:endswith", &sub, &start, &stop))
                return NULL;
                
        string_adjust_slice (&start, &stop, &str_len);
        if (strlen (sub) <= 0 || start >= stop)
                return pcl_object_ref (PCL_FALSE);
                
        search_str = g_strndup (str + start, stop - start);
        if (g_str_has_suffix (search_str, sub))
        {
                g_free (search_str);
                return pcl_object_ref (PCL_TRUE);
        }
        g_free (search_str);
        return pcl_object_ref (PCL_FALSE);
}

static PclObject *
string_method_expandtabs (PclString *self, PclObject *args)
{
        const gchar *str = PCL_STRING_AS_STRING (self);
        const gchar *end = str + PCL_STRING_GET_SIZE (self);
        gchar *exp_str;
        gint i, j;
        PclObject *expanded;
        gint tabsize = 8;

        if (!pcl_arg_parse_tuple(args, "|i:expandtabs", &tabsize))
                return NULL;

        /* First pass: determine size of output string */
        i = j = 0;
        for (; str < end; str++)
        {
                if (*str == '\t')
                {
	                if (tabsize > 0)
		                j += tabsize - (j % tabsize);
	        }
                else 
                {
                        j++;
                        if (*str == '\n' || *str == '\r')
                        {
                                i += j;
                                j = 0;
                        }
                }
        }
        
        /* Second pass: create output string and fill it */
        expanded = pcl_string_from_string_and_size (NULL, i + j);
        if (!expanded)
                return NULL;
        j = 0;
        exp_str = PCL_STRING_AS_STRING (expanded);
        for (str = PCL_STRING_AS_STRING (self); str < end; str++)
        {
                if (*str == '\t')
                {
	                if (tabsize > 0)
                        {
		                i = tabsize - (j % tabsize);
		                j += i;
		                while (i--)
		                        *exp_str++ = ' ';
	                }
	        }
	        else
                {
                        j++;
	                *exp_str++ = *str;
                        if (*str == '\n' || *str == '\r')
                                j = 0;
                }
        }
        return expanded;
}

static PclObject *
string_method_find (PclString *self, PclObject *args)
{
        gint sub_len, i, j, start = 0;
        gchar *sub = NULL;
        gchar *str = PCL_STRING_AS_STRING (self);
        gint stop, str_len = PCL_STRING_GET_SIZE (self);
        stop = str_len;
        
        if (!pcl_arg_parse_tuple (args, "s|ii:count", &sub, &start, &stop))
                return NULL;
        
        string_adjust_slice (&start, &stop, &str_len);
        sub_len = strlen(sub);
        
        /* Quick exception check */
        if (sub_len == 0 || sub_len > stop - start)
                return pcl_int_from_long (-1);
        
        for (i = start; i <= stop - sub_len; i++)
        {
                j = 0;
                while (j < sub_len)
                {
                        if (sub[j] != str[i + j])
                                break;
                        j++;
                }
                if (j == sub_len)
                        return pcl_int_from_long ((glong) i);
        }
        return pcl_int_from_long (-1);
}

static PclObject *
string_method_index (PclString *self, PclObject *args)
{
        PclObject *retval = string_method_find (self, args);
        glong index;
        if (retval)
        {
                index = pcl_int_as_long (retval);
                if (index >= 0)
                        return retval;
                        
                pcl_error_set_string (pcl_exception_value_error (),
                                      "substring not found");
                pcl_object_unref (retval);
        }
        return NULL;
}

static PclObject *
string_method_isalnum (PclString *self)
{
        gchar *str = PCL_STRING_AS_STRING (self);
        gint i, len = PCL_STRING_GET_SIZE (self);
        
        /* Special case for empty string */
        if (len == 0)
                return pcl_object_ref (PCL_FALSE);
                
        for (i = 0; i < len; i++)
        {
                if (!g_ascii_isalnum (str[i]))
                        return pcl_object_ref (PCL_FALSE);
        }
        return pcl_object_ref (PCL_TRUE);
}

static PclObject *
string_method_isalpha (PclString *self)
{
        gchar *str = PCL_STRING_AS_STRING (self);
        gint i, len = PCL_STRING_GET_SIZE (self);
        
        /* Special case for empty string */
        if (len == 0)
                return pcl_object_ref (PCL_FALSE);
                
        for (i = 0; i < len; i++)
        {
                if (!g_ascii_isalpha (str[i]))
                        return pcl_object_ref (PCL_FALSE);
        }
        return pcl_object_ref (PCL_TRUE);
}

static PclObject *
string_method_isdigit (PclString *self)
{
        gchar *str = PCL_STRING_AS_STRING (self);
        gint i, len = PCL_STRING_GET_SIZE (self);
        
        /* Special case for empty string */
        if (len == 0)
                return pcl_object_ref (PCL_FALSE);
                
        for (i = 0; i < len; i++)
        {
                if (!g_ascii_isdigit (str[i]))
                        return pcl_object_ref (PCL_FALSE);
        }
        return pcl_object_ref (PCL_TRUE);
}

static PclObject *
string_method_islower (PclString *self)
{
        gchar *str = PCL_STRING_AS_STRING (self);
        gint i, len = PCL_STRING_GET_SIZE (self);
        gboolean cased = FALSE;
        
        /* Special case for empty string */
        if (len == 0)
                return pcl_object_ref (PCL_FALSE);
                
        for (i = 0; i < len; i++)
        {
                if (g_ascii_isupper (str[i]))
                        return pcl_object_ref (PCL_FALSE);
                if (!cased && g_ascii_islower (str[i]))
                        cased = TRUE;
        }
        return pcl_bool_from_boolean (cased);
}

static PclObject *
string_method_isspace (PclString *self)
{
        gchar *str = PCL_STRING_AS_STRING (self);
        gint i, len = PCL_STRING_GET_SIZE (self);
        
        /* Special case for empty string */
        if (len == 0)
                return pcl_object_ref (PCL_FALSE);
                
        for (i = 0; i < len; i++)
        {
                if (!g_ascii_isspace (str[i]))
                        return pcl_object_ref (PCL_FALSE);
        }
        return pcl_object_ref (PCL_TRUE);
}

static PclObject *
string_method_istitle (PclString *self)
{
        gchar *str = PCL_STRING_AS_STRING (self);
        gint i, len = PCL_STRING_GET_SIZE (self);
        gboolean cased = FALSE, prev_cased = FALSE;
        
        /* Special case for empty string */
        if (len == 0)
                return pcl_object_ref (PCL_FALSE);
                
        for (i = 0; i < len; i++)
        {
                if (g_ascii_isupper (str[i]))
                {
                        if (prev_cased)
                                return pcl_object_ref (PCL_FALSE);
                        prev_cased = TRUE;
                        cased = TRUE;
                }
                else if (g_ascii_islower (str[i]))
                {
                        if (!prev_cased)
                                return pcl_object_ref (PCL_FALSE);
                        prev_cased = TRUE;
                        cased = TRUE;
                }
                else
                        prev_cased = FALSE;
        }
        return pcl_bool_from_boolean (cased);
}

static PclObject *
string_method_isupper (PclString *self)
{
        gchar *str = PCL_STRING_AS_STRING (self);
        gint i, len = PCL_STRING_GET_SIZE (self);
        gboolean cased = FALSE;
        
        /* Special case for empty string */
        if (len == 0)
                return pcl_object_ref (PCL_FALSE);
                
        for (i = 0; i < len; i++)
        {
                if (g_ascii_islower (str[i]))
                        return pcl_object_ref (PCL_FALSE);
                if (!cased && g_ascii_isupper (str[i]))
                        cased = TRUE;
        }
        return pcl_bool_from_boolean (cased);
}

static PclObject *
string_method_join (PclString *self, PclObject *object)
{
        gint index = 0;
        gchar *new_str, *sep = PCL_STRING_AS_STRING (self);
        glong length;
        gchar **ary;
        PclObject *retstr, *next, *iterator;

        if (!PCL_IS_SEQUENCE (object) || !PCL_IS_ITERABLE (object))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "argument must be an iterable sequence");
                return NULL;
        }
        length = pcl_object_measure (object);
        ary = g_new0(gchar *, length + 1);
        iterator = pcl_object_iterate (object);
        
        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                if (index < length)
                {
                        if (PCL_IS_STRING (next))
                                ary[index++] = PCL_STRING_AS_STRING (next);
                        else
                        {
                                pcl_error_set_format (
                                        pcl_exception_type_error (),
                                        "sequence item %d is not a string",
                                        index);
                                pcl_object_unref (iterator);
                                pcl_object_unref (next);
                                g_free (ary);
                                return NULL;
                        }
                        pcl_object_unref (next);
                }
                else
                {
                        pcl_object_unref (next);
                        break;
                }
        }
        pcl_object_unref (iterator);
        g_assert (ary[length] == NULL);
        new_str = g_strjoinv (sep, ary);
        g_free (ary);
        if (new_str)
        {
                retstr = pcl_string_from_string (new_str);
                g_free (new_str);
                return retstr;
        }
        pcl_error_bad_internal_call ();
        return NULL;
}

static PclObject *
string_method_ljust (PclString *self, PclObject *args)
{
        gint width, margin, size;
        gchar fillchar = ' ';
        gchar *s = PCL_STRING_AS_STRING (self);
        
        if (!pcl_arg_parse_tuple (args, "i|c:ljust", &width, &fillchar))
                return NULL;
        
        size = PCL_STRING_GET_SIZE (self);        
        if (size >= width)
                return pcl_object_ref(self);
                
        margin = width - size;
                
        return string_pad (s, size, margin, 0, fillchar);
}

static PclObject *
string_method_lower (PclString *self)
{
        gchar *str = PCL_STRING_AS_STRING (self);
        gint i, len = PCL_STRING_GET_SIZE (self);
        PclObject *new_obj = pcl_string_from_string_and_size (str, len);
        gchar *new_str = PCL_STRING_AS_STRING (new_obj);
        for (i = 0; i < len; i++)
                if (g_ascii_isupper (str[i]))
                        new_str[i] = g_ascii_tolower (str[i]);
                        
        return new_obj;
}

static PclObject *
string_method_lstrip (PclString *self, PclObject *args)
{
        if (pcl_object_measure (args) == 0)
                return string_strip (PCL_OBJECT (self), STR_LEFT_STRIP);
        return string_arg_strip (PCL_OBJECT (self), STR_LEFT_STRIP, args);
}

static PclObject *
string_method_replace (PclString *self, PclObject *args)
{
        gint i, j, k, max_count = -1, count = 0;
        gchar *str = PCL_STRING_AS_STRING (self);
        gint str_len = PCL_STRING_GET_SIZE (self);
        PclObject *new_obj, *old_obj, *ret_obj;
        gchar *new_str, *old_str, *ret_str;
        gint new_len, old_len, ret_len;
        
        if (!pcl_arg_parse_tuple (args, "OO|i:replace", 
                                  &old_obj, &new_obj, &max_count))
                return NULL;
                
        if (!PCL_IS_STRING (old_obj) || !PCL_IS_STRING (new_obj))
        {
                pcl_error_set_string (pcl_exception_type_error (),
                                      "first two arguments must be strings");
                return NULL;
        }
                
        old_str = PCL_STRING_AS_STRING (old_obj);
        old_len = PCL_STRING_GET_SIZE (old_obj);
        if (old_len == 0)
        {
                pcl_error_set_string (pcl_exception_value_error (),
                                      "empty pattern string");
                return NULL;
        }
        
        /* Count instances of old string, so we can allocate return string */
        for (i = 0; i <= str_len - old_len; i++)
        {
                j = 0;
                while (j < old_len)
                {
                        if (old_str[j] != str[i + j])
                                break;
                        j++;
                }
                if (j == old_len)
                {
                        count++;
                        i += (j - 1);
                }
        }
        if (count == 0)
                return pcl_object_ref (self);
        if (max_count >= 0 && count > max_count)
                count = max_count;
                
        new_str = PCL_STRING_AS_STRING (new_obj);
        new_len = PCL_STRING_GET_SIZE (new_obj);
                
        /* Allocate return string */
        ret_len = str_len + (count * (new_len - old_len));
        ret_obj = pcl_string_from_string_and_size (NULL, ret_len);
        if (ret_obj == NULL)
                return NULL;
        ret_str = PCL_STRING_AS_STRING (ret_obj);
        
        /* Copy characters to return string */
        i = j = 0;
        while (i < ret_len)
        {
                if (count > 0)
                {
                        k = 0;
                        while (k < old_len && (j + k) < str_len)
                        {
                                if (str[j + k] != old_str[k])
                                        break;
                                k++;
                        }
                        if (k == old_len)       /* Match */
                        {
                                k = 0;
                                count--;
                                while (k < new_len && i < ret_len)
                                {
                                        ret_str[i++] = new_str[k++];
                                }
                                j += old_len;
                        }
                        else
                                ret_str[i++] = str[j++];
                }
                else
                        ret_str[i++] = str[j++];
        }
        return ret_obj;
}

static PclObject *
string_method_rfind (PclString *self, PclObject *args)
{
        gint sub_len, i, j, start = 0;
        gchar *sub = NULL;
        gchar *str = PCL_STRING_AS_STRING (self);
        gint stop, str_len = PCL_STRING_GET_SIZE (self);
        stop = str_len;
        
        if (!pcl_arg_parse_tuple (args, "s|ii:count", &sub, &start, &stop))
                return NULL;
        
        string_adjust_slice (&start, &stop, &str_len);
        sub_len = strlen(sub);
        
        /* Quick exception check */
        if (sub_len == 0 || sub_len > stop - start)
                return pcl_int_from_long (-1);
        
        for (i = stop - sub_len; i >= start; i--)
        {
                j = 0;
                while (j < sub_len)
                {
                        if (sub[j] != str[i + j])
                                break;
                        j++;
                }
                if (j == sub_len)
                        return pcl_int_from_long ((glong) i);
        }
        return pcl_int_from_long (-1);
}

static PclObject *
string_method_rindex (PclString *self, PclObject *args)
{
        PclObject *retval = string_method_rfind (self, args);
        glong index;
        if (retval)
        {
                index = pcl_int_as_long (retval);
                if (index >= 0)
                        return retval;
                        
                pcl_error_set_string (pcl_exception_value_error (),
                                      "substring not found");
                pcl_object_unref (retval);
        }
        return NULL;
}

static PclObject *
string_method_rjust (PclString *self, PclObject *args)
{
        gint width, margin, size;
        gchar fillchar = ' ';
        gchar *s = PCL_STRING_AS_STRING (self);
        
        if (!pcl_arg_parse_tuple (args, "i|c:rjust", &width, &fillchar))
                return NULL;
        
        size = PCL_STRING_GET_SIZE (self);        
        if (size >= width)
                return pcl_object_ref(self);
                
        margin = width - size;
                
        return string_pad (s, size, margin, margin, fillchar);
}

static PclObject *
string_method_rsplit (PclString *self, PclObject *args)
{
        gchar *token = NULL, *delim = NULL;
        gint delim_len = 0;
        gchar *str = PCL_STRING_AS_STRING (self);
        gint str_len = PCL_STRING_GET_SIZE (self);
        gint i, j, count = 0, max_count = -1;
        PclObject *token_obj, *ret_list;
        gboolean prev_white;
        
        if (!pcl_arg_parse_tuple (args, "|si:rsplit", &delim, &max_count))
                return NULL;
                
        ret_list = pcl_list_new (0);
        if (delim != NULL)
        {
                delim_len = strlen (delim);
                /* Special case */
                if (str_len == 0)
                {
                        pcl_list_append (ret_list, PCL_OBJECT (self));
                        return ret_list;
                }
                /* Tokenize and append if possible */
                while (((max_count < 0) || (count < max_count)) &&
                       ((token = g_strrstr_len(str, str_len, delim)) != NULL))
                {
                        count++;
                        token_obj = pcl_string_from_string_and_size (
                                token + delim_len,
                                (str + str_len) - (token + delim_len));
                        pcl_list_append (ret_list, token_obj);
                        pcl_object_unref (token_obj);
                        str_len = token - str;
                }
                /* Clean up remainder of the string */
                token_obj = pcl_string_from_string_and_size (str, str_len);
                pcl_list_append (ret_list, token_obj);
                pcl_object_unref (token_obj);
                pcl_list_reverse(ret_list);
                return ret_list;
        }
        
        i = j = 0;
        prev_white = TRUE;
        while (i < str_len)
        {
                if (g_ascii_isspace(str[i]))
                {
                        if (!prev_white)
                        {
                                prev_white = TRUE;
                                token_obj = pcl_string_from_string_and_size (
                                        str + j, i - j);
                                pcl_list_append (ret_list, token_obj);
                                pcl_object_unref (token_obj);
                        }
                }
                else if (prev_white)
                {
                        prev_white = FALSE;
                        j = i;
                }
                i++;
        }
        /* Clean up remainder of the string */
        if (!prev_white)
        {
                token_obj = pcl_string_from_string_and_size (str + j, i - j);
                pcl_list_append (ret_list, token_obj);
                pcl_object_unref (token_obj);
        }
        return ret_list;
}

static PclObject *
string_method_rstrip (PclString *self, PclObject *args)
{
        if (pcl_object_measure (args) == 0)
                return string_strip (PCL_OBJECT (self), STR_RIGHT_STRIP);
        return string_arg_strip (PCL_OBJECT (self), STR_RIGHT_STRIP, args);
}

static PclObject *
string_method_split (PclString *self, PclObject *args)
{
        gchar *token = NULL, *delim = NULL;
        gint delim_len = 0;
        gchar *str = PCL_STRING_AS_STRING (self);
        gint str_len = PCL_STRING_GET_SIZE (self);
        gint i, j, count = 0, max_count = -1;
        PclObject *token_obj, *ret_list;
        gboolean prev_white;
        
        if (!pcl_arg_parse_tuple (args, "|si:split", &delim, &max_count))
                return NULL;
                
        ret_list = pcl_list_new (0);
        if (delim != NULL)
        {
                delim_len = strlen (delim);
                /* Special case */
                if (str_len == 0)
                {
                        pcl_list_append (ret_list, PCL_OBJECT (self));
                        return ret_list;
                }
                /* Tokenize and append if possible */
                while (((max_count < 0) || (count < max_count)) &&
                       ((token = g_strstr_len(str, str_len, delim)) != NULL))
                {
                        count++;
                        token_obj = pcl_string_from_string_and_size (
                                str,
                                token - str);
                        pcl_list_append (ret_list, token_obj);
                        pcl_object_unref (token_obj);
                        str_len -= ((token - str) + delim_len);
                        str = token + delim_len;
                }
                /* Clean up remainder of the string */
                token_obj = pcl_string_from_string_and_size (str, str_len);
                pcl_list_append (ret_list, token_obj);
                pcl_object_unref (token_obj);
                return ret_list;
        }
        
        i = j = 0;
        prev_white = TRUE;
        while (i < str_len)
        {
                if (g_ascii_isspace(str[i]))
                {
                        if (!prev_white)
                        {
                                prev_white = TRUE;
                                token_obj = pcl_string_from_string_and_size (
                                        str + j, i - j);
                                pcl_list_append (ret_list, token_obj);
                                pcl_object_unref (token_obj);
                        }
                }
                else if (prev_white)
                {
                        prev_white = FALSE;
                        j = i;
                }
                i++;
        }
        /* Clean up remainder of the string */
        if (!prev_white)
        {
                token_obj = pcl_string_from_string_and_size (str + j, i - j);
                pcl_list_append (ret_list, token_obj);
                pcl_object_unref (token_obj);
        }
        return ret_list;
}

static PclObject *
string_method_splitlines (PclString *self, PclObject *args)
{
        gint i, j, eol, len, keepends = 0;
        PclObject *ret_list;
        PclObject *line;
        gchar *str;

        if (!pcl_arg_parse_tuple(args, "|i:splitlines", &keepends))
                return NULL;

        str = PCL_STRING_AS_STRING (self);
        len = PCL_STRING_GET_SIZE (self);
        ret_list = pcl_list_new (0);
        if (!ret_list)
                return NULL;

        i = j = 0;
        while (i < len)
        {
                /* Keep finding newlines */
                while (i < len && str[i] != '\n' && str[i] != '\r')
	                i++;

	        /* Skip the line break reading CRLF as one line break */
	        eol = i;
	        if (i < len)
                {
	                if (str[i] == '\r' && i + 1 < len && str[i + 1] == '\n')
		                i += 2;
	                else
		                i++;
	                if (keepends)
		                eol = i;
                }
                        
	        line = pcl_string_from_string_and_size (str + j, eol - j);
                pcl_list_append (ret_list, line);
                pcl_object_unref (line);
	        j = i;
        }
        return ret_list;
}

static PclObject *
string_method_startswith (PclString *self, PclObject *args)
{
        gchar *str = PCL_STRING_AS_STRING (self);
        gchar *sub, *search_str;
        gint start = 0, str_len = PCL_STRING_GET_SIZE (self);
        gint stop = str_len;
        
        if (!pcl_arg_parse_tuple (args, "s|ii:endswith", &sub, &start, &stop))
                return NULL;
                
        string_adjust_slice (&start, &stop, &str_len);

        if (strlen (sub) <= 0 || start >= stop)
                return pcl_object_ref (PCL_FALSE);

        search_str = g_strndup (str + start, stop - start);        
        if (g_str_has_prefix (search_str, sub))
        {
                g_free (search_str);
                return pcl_object_ref (PCL_TRUE);
        }
        g_free (search_str);
        return pcl_object_ref (PCL_FALSE);
}

static PclObject *
string_method_strip (PclString *self, PclObject *args)
{
        if (pcl_object_measure (args) == 0)
                return string_strip (PCL_OBJECT (self), STR_BOTH_STRIP);
        return string_arg_strip (PCL_OBJECT (self), STR_BOTH_STRIP, args);
}

static PclObject *
string_method_swapcase (PclString *self)
{
        gint i, len;
        gchar c;
        gchar *str = PCL_STRING_AS_STRING (self);
        PclObject *new_obj = pcl_string_from_string (str);
        str = PCL_STRING_AS_STRING (new_obj);
        len = PCL_STRING_GET_SIZE (new_obj);
        
        for (i = 0; i < len; i++)
        {
                if (g_ascii_isupper (str[i]))
                {
                        c = str[i];
                        str[i] = g_ascii_tolower (c);
                }
                else if (g_ascii_islower (str[i]))
                {
                        c = str[i];
                        str[i] = g_ascii_toupper (c);
                }
        }
        return new_obj;
}

static PclObject *
string_method_translate (PclString *self, PclObject *args)
{
        PclObject *ret_obj = NULL, *table = NULL, *delchars = NULL;
        gchar *str, *del_str, *table_str, *ret_str;
        gint len, del_len, i, j;
        if (!pcl_arg_parse_tuple (args, "S|S:translate", &table, &delchars))
                return NULL;
        
        str = PCL_STRING_AS_STRING (self);
        len = PCL_STRING_GET_SIZE (self);
        
        if (len == 0)
                return pcl_object_ref (self);
        if (PCL_STRING_GET_SIZE (table) != 256)
        {
                pcl_error_set_string (pcl_exception_value_error (),
                                      "table must be 256 characters long");
                return NULL;
        }
                
        if (delchars)
        {
                ret_str = g_new0 (gchar, len + 1);
                j = 0;
                del_str = PCL_STRING_AS_STRING (delchars);
                del_len = PCL_STRING_GET_SIZE (delchars);
                for (i = 0; i < len; i++)
                        if (memchr(del_str, str[i], del_len) == NULL)
                                ret_str[j++] = str[i];
                ret_obj = pcl_string_from_string (ret_str);
                g_free (ret_str);
        }
        else
                ret_obj = pcl_string_from_string (str);
        
        table_str = PCL_STRING_AS_STRING (table);
        ret_str = PCL_STRING_AS_STRING (ret_obj);
        len = PCL_STRING_GET_SIZE (ret_obj);
        
        for (i = 0; i < len; i++)
        {
                j = (gint)ret_str[i];
                if (j >= 0 && j < 256)
                        ret_str[i] = table_str[j];
                else
                {
                        pcl_error_set_format (pcl_exception_index_error (),
                                              "translate(): index %d attempted",
                                              j);
                        pcl_object_unref (ret_obj);
                        return NULL;
                }
        }
        return ret_obj;
}

static PclObject *
string_method_title (PclString *self)
{
        gint i, len;
        gchar c;
        gboolean prev_cased = FALSE;
        gchar *str = PCL_STRING_AS_STRING (self);
        PclObject *new_obj = pcl_string_from_string (str);
        str = PCL_STRING_AS_STRING (new_obj);
        len = PCL_STRING_GET_SIZE (new_obj);
        
        for (i = 0; i < len; i++)
        {
                if (g_ascii_isupper (str[i]))
                {
                        if (prev_cased)
                        {
                                c = str[i];
                                str[i] = g_ascii_tolower (c);
                        }
                        else
                                prev_cased = TRUE;
                }
                else if (g_ascii_islower (str[i]))
                {
                        if (!prev_cased)
                        {
                                c = str[i];
                                str[i] = g_ascii_toupper (c);
                                prev_cased = TRUE;
                        }
                }
                else
                        prev_cased = FALSE;
        }
        return new_obj;
}

static PclObject *
string_method_upper (PclString *self)
{
        gchar *str = PCL_STRING_AS_STRING (self);
        gint i, len = PCL_STRING_GET_SIZE (self);
        PclObject *new_obj = pcl_string_from_string_and_size (str, len);
        gchar *new_str = PCL_STRING_AS_STRING (new_obj);
        for (i = 0; i < len; i++)
                if (g_ascii_islower (str[i]))
                        new_str[i] = g_ascii_toupper (str[i]);
                        
        return new_obj;
}

static PclObject *
string_method_zfill (PclString *self, PclObject *args)
{
        gchar *str = PCL_STRING_AS_STRING (self);
        gint margin, width, size = PCL_STRING_GET_SIZE (self);
        PclObject *retval;
        
        if (!pcl_arg_parse_tuple (args, "i:zfill", &width))
                return NULL;
                
        if (width <= size)
                return pcl_object_ref (self);
                
        margin = width - size;
        retval = string_pad (str, size, margin, margin, '0');
        if (retval)
        {
                /* Carry the leading sign character to the front */
                str = PCL_STRING_AS_STRING (retval);
                if (str[margin] == '+' || str[margin] == '-')
                {
                        str[0] = str[margin];
                        str[margin] = '0';
                }
                return retval;
        }
        return NULL;
}

static void
string_class_init (PclStringClass *class)
{
        PclObjectClass *object_class;

        string_parent_class = g_type_class_peek_parent (class);

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_string_get_type_object;
        object_class->new_instance = string_new_instance;
        object_class->print = string_print;
        object_class->hash = string_hash;
        object_class->repr = string_repr;
        object_class->str = string_str;
        object_class->rich_compare = string_rich_compare;
        object_class->contains = string_contains;
        object_class->iterate = string_iterate;
        object_class->measure = string_measure;
        object_class->get_item = string_get_item;
        object_class->doc = string_doc;
}

static void
string_init (PclString *pcl_string)
{
        pcl_string->hash = PCL_HASH_INVALID;
}

static PclMethodDef string_methods[] = {
        { "__getnewargs__",     (PclCFunction) string_method_getnewargs,
                                PCL_METHOD_FLAG_NOARGS },
        { "__imod__",           (PclCFunction) string_method_imod,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__imod__(y) <==> y % x" },
        { "__mod__",            (PclCFunction) string_method_mod,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__mod__(y) <==> x % y" },
        { "capitalize",         (PclCFunction) string_method_capitalize,
                                PCL_METHOD_FLAG_NOARGS },
        { "center",             (PclCFunction) string_method_center,
                                PCL_METHOD_FLAG_VARARGS },
        { "count",              (PclCFunction) string_method_count,
                                PCL_METHOD_FLAG_VARARGS },
        { "decode",             (PclCFunction) string_method_decode,
                                PCL_METHOD_FLAG_VARARGS },
        { "encode",             (PclCFunction) string_method_encode,
                                PCL_METHOD_FLAG_VARARGS },
        { "endswith",           (PclCFunction) string_method_endswith,
                                PCL_METHOD_FLAG_VARARGS },
        { "expandtabs",         (PclCFunction) string_method_expandtabs,
                                PCL_METHOD_FLAG_VARARGS },
        { "find",               (PclCFunction) string_method_find,
                                PCL_METHOD_FLAG_VARARGS },
        { "index",              (PclCFunction) string_method_index,
                                PCL_METHOD_FLAG_VARARGS },
        { "isalnum",            (PclCFunction) string_method_isalnum,
                                PCL_METHOD_FLAG_NOARGS },
        { "isalpha",            (PclCFunction) string_method_isalpha,
                                PCL_METHOD_FLAG_NOARGS },
        { "isdigit",            (PclCFunction) string_method_isdigit,
                                PCL_METHOD_FLAG_NOARGS },
        { "islower",            (PclCFunction) string_method_islower,
                                PCL_METHOD_FLAG_NOARGS },
        { "isspace",            (PclCFunction) string_method_isspace,
                                PCL_METHOD_FLAG_NOARGS },
        { "istitle",            (PclCFunction) string_method_istitle,
                                PCL_METHOD_FLAG_NOARGS },
        { "isupper",            (PclCFunction) string_method_isupper,
                                PCL_METHOD_FLAG_NOARGS },
        { "join",               (PclCFunction) string_method_join,
                                PCL_METHOD_FLAG_ONEARG },
        { "ljust",              (PclCFunction) string_method_ljust,
                                PCL_METHOD_FLAG_VARARGS },
        { "lower",              (PclCFunction) string_method_lower,
                                PCL_METHOD_FLAG_NOARGS },
        { "lstrip",             (PclCFunction) string_method_lstrip,
                                PCL_METHOD_FLAG_VARARGS },
        { "replace",            (PclCFunction) string_method_replace,
                                PCL_METHOD_FLAG_VARARGS },
        { "rfind",              (PclCFunction) string_method_rfind,
                                PCL_METHOD_FLAG_VARARGS },
        { "rindex",             (PclCFunction) string_method_rindex,
                                PCL_METHOD_FLAG_VARARGS },
        { "rjust",              (PclCFunction) string_method_rjust,
                                PCL_METHOD_FLAG_VARARGS },
        { "rsplit",             (PclCFunction) string_method_rsplit,
                                PCL_METHOD_FLAG_VARARGS },
        { "rstrip",             (PclCFunction) string_method_rstrip,
                                PCL_METHOD_FLAG_VARARGS },
        { "split",              (PclCFunction) string_method_split,
                                PCL_METHOD_FLAG_VARARGS },
        { "splitlines",         (PclCFunction) string_method_splitlines,
                                PCL_METHOD_FLAG_VARARGS },
        { "startswith",         (PclCFunction) string_method_startswith,
                                PCL_METHOD_FLAG_VARARGS },
        { "strip",              (PclCFunction) string_method_strip,
                                PCL_METHOD_FLAG_VARARGS },
        { "swapcase",           (PclCFunction) string_method_swapcase,
                                PCL_METHOD_FLAG_NOARGS },
        { "translate",          (PclCFunction) string_method_translate,
                                PCL_METHOD_FLAG_VARARGS },
        { "title",              (PclCFunction) string_method_title,
                                PCL_METHOD_FLAG_NOARGS },
        { "upper",              (PclCFunction) string_method_upper,
                                PCL_METHOD_FLAG_NOARGS },
        { "zfill",              (PclCFunction) string_method_zfill,
                                PCL_METHOD_FLAG_VARARGS },
        { NULL }
};

GType
pcl_string_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclStringClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) string_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclString),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) string_init,
                        NULL   /* value_table */
                };

                static const GInterfaceInfo sequence_info = {
                        (GInterfaceInitFunc) string_sequence_init,
                        (GInterfaceFinalizeFunc) NULL,
                        NULL   /* interface_data */
                };

                type = g_type_register_static (
                        PCL_TYPE_OBJECT, "PclString", &type_info, 0);
                g_type_add_interface_static (
                        type, PCL_TYPE_SEQUENCE, &sequence_info);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, string_methods);
        }
        return type;
}

/**
 * pcl_string_get_type_object:
 *
 * Returns the type object for #PclString.  During runtime this is the built-in
 * object %str.
 *
 * Returns: a borrowed reference to the type object for #PclString
 */
PclObject *
pcl_string_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_STRING, "string");
                pcl_register_singleton ("<type 'string'>", &object);
        }
        return object;
}

gboolean
pcl_string_resize (PclObject *self, glong length)
{
        if (!PCL_IS_STRING (self))
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        g_string_set_size (PCL_STRING (self)->string, length);
        PCL_STRING (self)->hash = PCL_HASH_INVALID;
        return TRUE;
}

gchar *
pcl_string_as_string (PclObject *object)
{
        if (!PCL_IS_STRING (object))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        return PCL_STRING_AS_STRING (object);
}

PclObject *
pcl_string_from_string (const gchar *v_string)
{
        g_assert (v_string != NULL);
        return string_from_g_string (g_string_new (v_string));
}

PclObject *
pcl_string_from_string_and_size (const gchar *v_string, gssize size)
{
        GString *str;
        if (v_string == NULL)
        {
                str = g_string_sized_new (size);
                g_string_set_size(str, size);
        }
        else
        {
                str = g_string_new_len (v_string, size);
        }       
        return string_from_g_string (str);
}

PclObject *
pcl_string_from_format (const gchar *format, ...)
{
        va_list va;
        PclObject *object;

        va_start (va, format);
        object = pcl_string_from_format_va (format, va);
        va_end (va);
        return object;
}

PclObject *
pcl_string_from_format_va (const gchar *format, va_list va)
{
        gchar *v_string;
        PclObject *object;

        v_string = g_strdup_vprintf (format, va);
        object = pcl_string_from_string (v_string);
        g_free (v_string);
        return object;
}

PclObject *
pcl_string_intern_from_string (const gchar *v_string)
{
        PclObject *object;

        object = pcl_string_from_string (v_string);
        if (object != NULL)
                pcl_string_intern_in_place (&object);
        return object;
}

void
pcl_string_intern_in_place (PclObject **p_object)
{
        PclObject *interned;
        const gchar *v_string;

        if (!PCL_IS_STRING (*p_object))
                g_error ("%s: strings only please!", G_STRFUNC);

        if (G_UNLIKELY (internal_strings == NULL))
                g_datalist_init (&internal_strings);

        v_string = PCL_STRING_AS_STRING (*p_object);
        interned = g_datalist_get_data (&internal_strings, v_string);
        if (interned == NULL)
                g_datalist_set_data_full (&internal_strings, v_string,
                                          pcl_object_ref (*p_object),
                                          pcl_object_unref);
        else
        {
                pcl_object_unref (*p_object);
                *p_object = pcl_object_ref (interned);
        }
}

void
pcl_string_concat (PclObject **p_object, PclObject *string)
{
        register PclObject *object;

        if (*p_object == NULL)
                return;
        if (string == NULL || !PCL_IS_STRING (*p_object))
        {
                pcl_object_unref (*p_object);
                *p_object = NULL;
                return;
        }
        object = string_sequence_concat (*p_object, string);
        pcl_object_unref (*p_object);
        *p_object = object;
}

void
pcl_string_concat_and_del (PclObject **p_object, PclObject *string)
{
        pcl_string_concat (p_object, string);
        if (string != NULL)
                pcl_object_unref (string);
}

PclObject *
pcl_string_join (PclObject *separator, PclObject *sequence)
{
        if (!PCL_IS_STRING (separator) || sequence == NULL)
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        return string_method_join (PCL_STRING (separator), sequence);
}

PclObject *
pcl_string_format (PclObject *format, PclObject *args)
{
        PclObject *mapping = NULL;
        PclObject *original_args;
        PclObject *result;
        GString *result_str;
        gboolean args_owned = FALSE;
        gchar *format_str;
        glong format_len;
        gint arg_len, arg_idx;

        if (!PCL_IS_STRING (format) || args == NULL)
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        original_args = args;
        format_str = PCL_STRING_AS_STRING (format);
        format_len = PCL_STRING_GET_SIZE (format);
        result_str = g_string_sized_new (format_len);

        if (PCL_IS_TUPLE (args))
        {
                arg_len = PCL_TUPLE_GET_SIZE (args);
                arg_idx = 0;
        }
        else
        {
                arg_len = -1;
                arg_idx = -2;
        }
        if (PCL_IS_MAPPING (args))
                mapping = args;

        while (format_len > 0)
        {
                if (*format_str != '%')
                        g_string_append_c (result_str, *format_str++);
                else
                {
                        PclObject *temp = NULL;
                        PclObject *value = NULL;
                        GString *buffer;
                        gint flags = 0;
                        gint prec = -1;
                        gint width = -1;
                        gchar ch = '\0';
                        gchar fill = ' ';
                        gchar sign = 0;
                        gchar *str;
                        gint len;

                        /* got a format specifier */
                        format_str++;
                        if (*format_str == '(')
                        {
                                PclObject *key;
                                gchar *key_str;
                                glong key_len;
                                glong paren_cnt = 1;

                                if (mapping == NULL)
                                {
                                        pcl_error_set_string (
                                                pcl_exception_type_error (),
                                                "format requires a mapping");
                                        goto error;
                                }

                                format_str++;
                                format_len--;
                                key_str = format_str;

                                /* skip over balanced parentheses */
                                while (paren_cnt > 0 && format_len > 0)
                                {
                                        if (*format_str == ')')
                                                paren_cnt--;
                                        else if (*format_str == '(')
                                                paren_cnt++;
                                        format_len--;
                                        format_str++;
                                }
                                if (paren_cnt > 0 || format_len == 0)
                                {
                                        pcl_error_set_string (
                                                pcl_exception_value_error (),
                                                "incomplete format key");
                                        goto error;
                                }

                                /* fetch key from args mapping */
                                key_len = format_str - key_str - 1;
                                key = pcl_string_from_string_and_size (
                                                key_str, key_len);
                                if (key == NULL)
                                        goto error;
                                if (args_owned)
                                {
                                        pcl_object_unref (args);
                                        args_owned = FALSE;
                                }
                                args = pcl_object_get_item (mapping, key);
                                pcl_object_unref (key);
                                if (args == NULL)
                                        goto error;
                                args_owned = TRUE;
                                arg_len = -1;
                                arg_idx = -2;
                        }

                        /* parse modifiers */
                        while (format_len > 0)
                        {
                                format_len--;
                                ch = *format_str++;
                                switch (ch)
                                {
                                        case '-':
                                                flags |= FMT_FLAG_LJUST;
                                                continue;
                                        case '+':
                                                flags |= FMT_FLAG_SIGN;
                                                continue;
                                        case ' ':
                                                flags |= FMT_FLAG_BLANK;
                                                continue;
                                        case '#':
                                                flags |= FMT_FLAG_ALT;
                                                continue;
                                        case '0':
                                                flags |= FMT_FLAG_ZERO;
                                                continue;
                                }
                                break;
                        }
                        if (ch == '*')
                        {
                                value = string_format_next_arg (
                                        args, arg_len, &arg_idx);
                                if (value == NULL)
                                        goto error;
                                if (!PCL_IS_INT (value))
                                {
                                        pcl_error_set_string (
                                                pcl_exception_type_error (),
                                                "* wants int");
                                        goto error;
                                }
                                width = pcl_int_as_long (value);
                                if (width < 0)
                                {
                                        flags |= FMT_FLAG_LJUST;
                                        width = -width;
                                }
                                if (format_len > 0)
                                {
                                        format_len--;
                                        ch = *format_str++;
                                }
                        }
                        else if (g_ascii_isdigit (ch))
                        {
                                width = g_ascii_digit_value (ch);
                                while (format_len > 0)
                                {
                                        format_len--;
                                        ch = *format_str++;
                                        if (!g_ascii_isdigit (ch))
                                                break;
                                        if ((width * 10) / 10 != width)
                                        {
                                                pcl_error_set_string (
                                                pcl_exception_value_error (),
                                                "width too big");
                                                goto error;
                                        }
                                        width = width * 10 +
                                                g_ascii_digit_value (ch);
                                }
                        }
                        if (ch == '.')
                        {
                                prec = 0;
                                if (format_len > 0)
                                {
                                        format_len--;
                                        ch = *format_str++;
                                }
                                if (ch == '*')
                                {
                                        value = string_format_next_arg (
                                                args, arg_len, &arg_idx);
                                        if (value == NULL)
                                                goto error;
                                        if (!PCL_IS_INT (value))
                                        {
                                                pcl_error_set_string (
                                                pcl_exception_type_error (),
                                                "* wants int");
                                                goto error;
                                        }
                                        prec = pcl_int_as_long (value);
                                        if (prec < 0)
                                                prec = 0;
                                        if (format_len > 0)
                                        {
                                                format_len--;
                                                ch = *format_str++;
                                        }
                                }
                                else if (g_ascii_isdigit (ch))
                                {
                                        prec = g_ascii_digit_value (ch);
                                        while (format_len > 0)
                                        {
                                                format_len--;
                                                ch = *format_str++;
                                                if (!g_ascii_isdigit (ch))
                                                        break;
                                                if ((prec * 10) / 10 != prec)
                                                {
                                                pcl_error_set_string (
                                                pcl_exception_value_error (),
                                                "prec too big");
                                                goto error;
                                                }
                                                prec = prec * 10 +
                                                g_ascii_digit_value (ch);
                                        }
                                }
                        }
                        if (format_len > 0)
                        {
                                if (ch == 'h' || ch == 'l' || ch == 'L')
                                {
                                        format_len--;
                                        ch = *format_str++;
                                }
                        }
                        if (format_len == 0)
                        {
                                pcl_error_set_string (
                                        pcl_exception_value_error (),
                                        "incomplete format");
                                goto error;
                        }
                        if (ch != '%')
                        {
                                value = string_format_next_arg (
                                        args, arg_len, &arg_idx);
                                if (value == NULL)
                                        goto error;
                        }

                        /* parse conversion type */
                        buffer = g_string_sized_new (128);
                        switch (ch)
                        {
                                case '%':
                                        str = "%";
                                        len = 1;
                                        break;
                                case 's':
                                case 'r':
                                        if (ch == 's')
                                                temp = pcl_object_str (value);
                                        else
                                                temp = pcl_object_repr (value);
                                        if (temp == NULL)
                                                goto error;
                                        g_assert (PCL_IS_STRING (temp));
                                        str = PCL_STRING_AS_STRING (temp);
                                        len = PCL_STRING_GET_SIZE (temp);
                                        if (prec >= 0 && len > prec)
                                                len = prec;
                                        break;
                                case 'i':
                                case 'd':
                                case 'u':
                                case 'o':
                                case 'x':
                                case 'X':
                                        if (ch == 'i')
                                                ch = 'd';
                                        if (!string_format_int (
                                                        buffer, flags,
                                                        prec, ch, value))
                                                goto error;
                                        str = buffer->str;
                                        len = buffer->len;
                                        sign = 1;
                                        if (flags & FMT_FLAG_ZERO)
                                                fill = '0';
                                        break;
                                case 'e':
                                case 'E':
                                case 'f':
                                case 'F':
                                case 'g':
                                case 'G':
                                        if (ch == 'F')
                                                ch = 'f';
                                        if (!string_format_float (
                                                        buffer, flags,
                                                        prec, ch, value))
                                                goto error;
                                        str = buffer->str;
                                        len = buffer->len;
                                        sign = 1;
                                        if (flags & FMT_FLAG_ZERO)
                                                fill = '0';
                                        break;
                                case 'c':
                                        if (!string_format_char (
                                                        buffer, value))
                                                goto error;
                                        str = buffer->str;
                                        len = buffer->len;
                                        break;
                                default:
                                        pcl_error_set_format (
                                                pcl_exception_value_error (),
                                                "unsupported format "
                                                "character '%c' (0x%x) at "
                                                "index %i", ch, ch,
                                                (gint) (format_str - 1 -
                                                pcl_string_as_string (format)));
                                        goto error;
                        }
                        if (sign)
                        {
                                if (*str == '-' || *str == '+')
                                {
                                        sign = *str++;
                                        len--;
                                }
                                else if (flags & FMT_FLAG_SIGN)
                                        sign = '+';
                                else if (flags & FMT_FLAG_BLANK)
                                        sign = ' ';
                                else
                                        sign = 0;
                        }
                        if (sign)
                        {
                                if (fill != ' ')
                                        g_string_append_c (result_str, sign);
                                if (width > len)
                                        width--;
                        }
                        if ((flags & FMT_FLAG_ALT) && (ch == 'x' || ch == 'X'))
                        {
                                g_assert (str[0] == '0');
                                g_assert (str[1] == ch);
                                if (fill != ' ')
                                {
                                        g_string_append_c (result_str, *str++);
                                        g_string_append_c (result_str, *str++);
                                }
                                width -= 2;
                                if (width < 0)
                                        width = 0;
                                len -= 2;
                        }
                        if (width > len && !(flags & FMT_FLAG_LJUST))
                        {
                                while (width > len)
                                {
                                        width--;
                                        g_string_append_c (result_str, fill);
                                }
                        }
                        if (fill == ' ')
                        {
                                if (sign)
                                        g_string_append_c (result_str, sign);
                                if ((flags & FMT_FLAG_ALT) &&
                                        (ch == 'x' || ch == 'X'))
                                {
                                        g_assert (str[0] == '0');
                                        g_assert (str[1] == ch);
                                        g_string_append_c (result_str, *str++);
                                        g_string_append_c (result_str, *str++);
                                }
                        }
                        g_string_append_len (result_str, str, len);
                        while (width > len)
                        {
                                width--;
                                g_string_append_c (result_str, ' ');
                        }

                        if (temp != NULL)
                                pcl_object_unref (temp);
                        g_string_free (buffer, TRUE);

                        if (mapping != NULL && (arg_idx < arg_len) && ch != '%')
                        {
                                pcl_error_set_string (
                                        pcl_exception_type_error (),
                                        "not all arguments converted during "
                                        "string formatting");
                                goto error;
                        }
                }

                format_len--;
        }

        if (arg_idx < arg_len && mapping == NULL)
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "not all arguments converted during "
                        "string formatting");
                goto error;
        }

        if (args_owned)
                pcl_object_unref (args);

        result = pcl_string_from_string (result_str->str);
        g_string_free (result_str, TRUE);
        return result;

error:

        if (args_owned)
                pcl_object_unref (args);

        return NULL;
}
