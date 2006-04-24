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

#define null_error() \
        if (!pcl_error_occurred ()) { \
        pcl_error_set_string (pcl_exception_system_error (), \
        G_STRLOC ": null argument to internal function"); }

static gchar object_doc[] =
"The most base type";

static gpointer object_parent_class = NULL;

/* String Objects */
static PclObject *__bases__ = NULL;
static PclObject *__class__ = NULL;

/* MRO Utilities */
extern PclObject *mro_lookup (PclType *type, const gchar *name);

static PclObject *
object_abstract_get_bases (PclObject *class)
{
        PclObject *bases;

        if (__bases__ == NULL)
        {
                __bases__ = pcl_string_from_string ("__bases__");
                if (__bases__ == NULL)
                        return NULL;
        }
        bases = pcl_object_get_attr (class, __bases__);
        if (bases == NULL)
        {
                if (pcl_error_exception_matches (
                                pcl_exception_attribute_error ()))
                        pcl_error_clear ();
                return NULL;
        }
        if (!PCL_IS_TUPLE (bases))
        {
                pcl_object_unref (bases);
                return NULL;
        }
        return bases;
}

static gint
object_abstract_is_subclass (PclObject *subclass, PclObject *of)
{
        PclObject *bases;
        glong ii, size;
        gint result = 0;

        if (subclass == of)
                return 1;
        if (PCL_IS_TUPLE (of))
        {
                size = PCL_TUPLE_GET_SIZE (of);
                for (ii = 0; ii < size; ii++)
                        if (subclass == PCL_TUPLE_GET_ITEM (of, ii))
                                return 1;
        }
        bases = object_abstract_get_bases (subclass);
        if (bases == NULL)
        {
                if (pcl_error_occurred ())
                        return -1;
                return 0;
        }
        size = PCL_TUPLE_GET_SIZE (bases);
        for (ii = 0; ii < size; ii++)
        {
                PclObject *item;
                item = PCL_TUPLE_GET_ITEM (bases, ii);
                result = object_abstract_is_subclass (item, of);
                if (result != 0)
                        break;
        }
        pcl_object_unref (bases);
        return result;
}

static gboolean
object_check_class (PclObject *class, const gchar *error)
{
        PclObject *bases = object_abstract_get_bases (class);
        if (bases == NULL)
        {
                if (!pcl_error_occurred ())
                        pcl_error_set_string (
                                pcl_exception_type_error (), error);
                return FALSE;
        }
        pcl_object_unref (bases);
        return TRUE;
}

static gint
object_recursive_is_instance (PclObject *instance, PclObject *of,
                              gint recursion_depth)
{
        PclObject *class;
        PclObject *instance_type;
        glong ii, size;
        gint result = 0;

        if (__class__ == NULL)
        {
                __class__ = pcl_string_from_string ("__class__");
                if (__class__ == NULL)
                        return -1;
        }

        if (PCL_IS_TYPE (of))
                goto type;
        if (PCL_IS_TUPLE (of))
                goto tuple;

        if (!object_check_class (of,
                        "isinstance() arg 2 must be a class, type, "
                        "or tuple of classes and types"))
                return -1;
        class = pcl_object_get_attr_string (instance, "__class__");
        if (class == NULL)
        {
                pcl_error_clear ();
                result = 0;
        }
        else
        {
                result = object_abstract_is_subclass (class, of);
                pcl_object_unref (class);
        }
        return result;

type:
        instance_type = PCL_GET_TYPE_OBJECT (instance);
        result = pcl_type_is_subtype (instance_type, of);
        if (result == 0)
        {
                class = pcl_object_get_attr_string (instance, "__class__");
                if (class == NULL)
                        pcl_error_clear ();
                else
                {
                        if (class != instance_type && PCL_IS_TYPE (class))
                                result = pcl_type_is_subtype (of, class);
                        pcl_object_unref (class);
                }
        }
        return result;

tuple:
        if (recursion_depth == 0)
        {
                pcl_error_set_string (
                        pcl_exception_runtime_error (),
                        "nest level of tuple too deep");
                return -1;
        }
        size = PCL_TUPLE_GET_SIZE (of);
        for (ii = 0; ii < size; ii++)
        {
                PclObject *item = PCL_TUPLE_GET_ITEM (of, ii);
                result = object_recursive_is_instance (
                        instance, item, recursion_depth - 1);
                if (result != 0)
                        break;
        }
        return result;
}

static gint
object_recursive_is_subclass (PclObject *subclass, PclObject *of,
                              gint recursion_depth)
{
        return 0;  /* FIXME */
}

static gboolean
object_internal_print (PclObject *object, FILE *stream,
                       PclPrintFlags flags, gint nesting)
{
        gboolean success = TRUE;
        if (nesting > 10)
        {
                pcl_error_set_string (
                        pcl_exception_runtime_error (),
                        "print recursion");
                return FALSE;
        }
        if (!pcl_error_check_signals ())
                return FALSE;
        clearerr (stream);
        if (object == NULL)
                fprintf (stream, "<nil>");
        else
        {
                PclObjectClass *class = PCL_OBJECT_GET_CLASS (object);
                if (class->print == NULL)
                {
                        PclObject *result;
                        if (flags & PCL_PRINT_FLAG_RAW)
                                result = pcl_object_str (object);
                        else
                                result = pcl_object_repr (object);
                        if (result == NULL)
                                success = FALSE;
                        else
                        {
                                success = object_internal_print (
                                                result, stream,
                                                PCL_PRINT_FLAG_RAW,
                                                nesting + 1);
                                pcl_object_unref (result);
                        }
                }
                else
                        success = class->print (object, stream, flags);
        }
        if (success && ferror (stream))
        {
                pcl_error_set_from_errno (pcl_exception_io_error ());
                clearerr (stream);
                return FALSE;
        }
        return success;
}

static PclObject *
object_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        const gchar *kwlist[] = { NULL };

        if (!pcl_arg_parse_tuple_and_keywords (args, kwds, ":object", kwlist))
                return NULL;
        return pcl_object_new (PCL_TYPE_OBJECT, NULL);
}

static PclObject *
object_copy (PclObject *object)
{
        /* Default behavior for immutable objects. */
        return pcl_object_ref (object);
}

static PclObject *
object_dict (PclObject *object)
{
        return NULL;
}

static PclObject *
object_repr (PclObject *object)
{
        PclObject *module;
        PclObject *name;
        PclObject *result;
        PclObject *type;

        type = PCL_GET_TYPE_OBJECT (object);

        module = pcl_object_get_attr_string (type, "__module__");
        if (module == NULL)
                pcl_error_clear ();
        else if (!PCL_IS_STRING (module))
        {
                pcl_object_unref (module);
                module = NULL;
        }
        else if (strcmp (PCL_STRING_AS_STRING (module), "__builtin__") == 0)
        {
                pcl_object_unref (module);
                module = NULL;
        }

        name = pcl_object_get_attr_string (type, "__name__");
        if (name == NULL)
        {
                if (module != NULL)
                        pcl_object_unref (module);
                return NULL;
        }

        if (module != NULL)
                result = pcl_string_from_format (
                        "<%s.%s object at %p>",
                        PCL_STRING_AS_STRING (module),
                        PCL_STRING_AS_STRING (name),
                        (gpointer) object);
        else
                result = pcl_string_from_format (
                        "<%s object at %p>",
                        PCL_GET_TYPE_NAME (object),
                        (gpointer) object);

        if (module != NULL)
                pcl_object_unref (module);
        pcl_object_unref (name);
        return result;
}

static PclObject *
object_str (PclObject *object)
{
        PclObjectClass *class;

        class = PCL_OBJECT_GET_CLASS (object);
        if (class != NULL && class->repr != NULL)
                return class->repr (object);
        return object_repr (object);
}

static PclObject *
object_get_attr (PclObject *object, const gchar *name)
{
        PclObject *(*get) (PclDescriptor *, PclObject *, PclObject *) = NULL;

        PclObject *dict;
        PclObject *lookup;
        PclObject *result;
        PclObject *type;

        type = PCL_GET_TYPE_OBJECT (object);
        g_assert (PCL_IS_TYPE (type));
        lookup = mro_lookup (PCL_TYPE (type), name);

        if (PCL_IS_DESCRIPTOR (lookup))
        {
                get = PCL_DESCRIPTOR_GET_CLASS (lookup)->get;
                if (get != NULL && PCL_DESCRIPTOR_IS_DATA (lookup))
                        return get (PCL_DESCRIPTOR (lookup), object, type);
        }

        dict = pcl_object_get_dict (object);
        if (dict != NULL)
        {
                result = pcl_dict_get_item_string (dict, name);
                if (result != NULL)
                        return pcl_object_ref (result);
        }
        else if (pcl_error_occurred ())
                return NULL;

        if (get != NULL)
                return get (PCL_DESCRIPTOR (lookup), object, type);

        if (lookup != NULL)
                return pcl_object_ref (lookup);

        pcl_error_set_format (
                pcl_exception_attribute_error (),
                "'%s' object has no attribute '%s'",
                PCL_GET_TYPE_NAME (object), name);

        return NULL;
}

static gboolean
object_set_attr (PclObject *object, const gchar *name, PclObject *value)
{
        gboolean (*set) (PclDescriptor *, PclObject *, PclObject *) = NULL;

        PclObject *dict;
        PclObject *lookup;
        PclObject *type;
        gboolean success;

        type = PCL_GET_TYPE_OBJECT (object);
        g_assert (PCL_IS_TYPE (type));
        lookup = mro_lookup (PCL_TYPE (type), name);

        if (PCL_IS_DESCRIPTOR (lookup))
        {
                set = PCL_DESCRIPTOR_GET_CLASS (lookup)->set;
                if (set != NULL && PCL_DESCRIPTOR_IS_DATA (lookup))
                        return set (PCL_DESCRIPTOR (lookup), object, value);
        }

        dict = pcl_object_get_dict (object);
        if (dict != NULL)
        {
                if (value == NULL)
                        success = pcl_dict_del_item_string (dict, name);
                else
                        success = pcl_dict_set_item_string (dict, name, value);
                if (!success && pcl_error_exception_matches (
                                pcl_exception_key_error ()))
                        pcl_error_set_string (
                                pcl_exception_attribute_error (),
                                name);
                return success;
        }
        else if (pcl_error_occurred ())
                return FALSE;

        if (set != NULL)
                return set (PCL_DESCRIPTOR (lookup), object, value);

        pcl_error_set_format (
                pcl_exception_attribute_error (),
                (lookup == NULL) ?
                "'%s' object has no attribute '%s'" :
                "'%s' object attribute '%s' is read-only",
                PCL_GET_TYPE_NAME (object), name);

        return FALSE;
}

/* object getsets */

static PclObject *
object_get_class (PclObject *self, gpointer context)
{
        return pcl_object_type (self);
}

static gboolean
object_set_class (PclObject *self, PclObject *value, gpointer context)
{
        pcl_error_set_string (
                pcl_exception_system_error (),
                "__class__ assignment is not yet supported");
        return FALSE;
}

/* object methods */

static PclObject *
object_method_call (PclObject *self, PclObject *args, PclObject *kwds)
{
        return pcl_object_call (self, args, kwds);
}

static PclObject *
object_method_cmp (PclObject *self, PclObject *other)
{
        gint cmp;

        cmp = pcl_object_compare (self, other);
        if (pcl_error_occurred ())
                return NULL;
        return pcl_int_from_long ((glong) cmp);
}

static PclObject *
object_method_contains (PclObject *self, PclObject *value)
{
        gint status;

        status = pcl_object_contains (self, value);
        if (status < 0)
                return NULL;
        return pcl_bool_from_boolean (status > 0);
}

static PclObject *
object_method_delitem (PclObject *self, PclObject *subscript)
{
        if (!pcl_object_del_item (self, subscript))
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

static PclObject *
object_method_eq (PclObject *self, PclObject *other)
{
        return pcl_object_rich_compare (self, other, PCL_EQ);
}

static PclObject *
object_method_ge (PclObject *self, PclObject *other)
{
        return pcl_object_rich_compare (self, other, PCL_GE);
}

static PclObject *
object_method_getitem (PclObject *self, PclObject *subscript)
{
        return pcl_object_get_item (self, subscript);
}

static PclObject *
object_method_gt (PclObject *self, PclObject *other)
{
        return pcl_object_rich_compare (self, other, PCL_GT);
}

static PclObject *
object_method_hash (PclObject *self)
{
        guint hash;

        hash = pcl_object_hash (self);
        if (hash == PCL_HASH_INVALID && pcl_error_occurred ())
                return NULL;
        return pcl_int_from_long ((glong) hash);
}

static PclObject *
object_method_iter (PclObject *self)
{
        return pcl_object_iterate (self);
}

static PclObject *
object_method_le (PclObject *self, PclObject *other)
{
        return pcl_object_rich_compare (self, other, PCL_LE);
}

static PclObject *
object_method_len (PclObject *self)
{
        glong size;

        size = pcl_object_measure (self);
        if (size < 0 && pcl_error_occurred ())
                return NULL;
        return pcl_int_from_long (size);
}

static PclObject *
object_method_lt (PclObject *self, PclObject *other)
{
        return pcl_object_rich_compare (self, other, PCL_LT);
}

static PclObject *
object_method_ne (PclObject *self, PclObject *other)
{
        return pcl_object_rich_compare (self, other, PCL_NE);
}

static PclObject *
object_method_repr (PclObject *self)
{
        return pcl_object_repr (self);
}

static PclObject *
object_method_setitem (PclObject *self, PclObject *args)
{
        PclObject *subscript;
        PclObject *value;

        if (!pcl_arg_unpack_tuple (args, "__setitem__", 2, 2,
                                &subscript, &value))
                return NULL;
        if (!pcl_object_set_item (self, subscript, value))
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

static PclObject *
object_method_str (PclObject *self)
{
        return pcl_object_str (self);
}

static void
object_class_init (PclObjectClass *class)
{
        object_parent_class = g_type_class_peek_parent (class);

        class->type = pcl_object_get_type_object;
        class->new_instance = object_new_instance;
        class->copy = object_copy;
        class->dict = object_dict;
        class->repr = object_repr;
        class->str = object_str;
        class->get_attr = object_get_attr;
        class->set_attr = object_set_attr;
        class->doc = object_doc;
}

static PclGetSetDef object_getsets[] = {
        { "__class__",          (PclGetFunc) object_get_class,
                                (PclSetFunc) object_set_class,
                                "the object's class" },
        { NULL }
};

#define WRAPS(func)     G_STRUCT_OFFSET (PclObjectClass, func)

static PclMethodDef object_methods[] = {
        { "__call__",           (PclCFunction) object_method_call,
                                PCL_METHOD_FLAG_VARARGS |
                                PCL_METHOD_FLAG_KEYWORDS,
                                "x.__call__(...) <==> x(...)",
                                WRAPS (call) },
        { "__cmp__",            (PclCFunction) object_method_cmp,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__cmp__(y) <==> cmp(x,y)",
                                WRAPS (compare) },
        { "__contains__",       (PclCFunction) object_method_contains,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__contains__(y) <==> y in x",
                                WRAPS (contains) },
        { "__delitem__",        (PclCFunction) object_method_delitem,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__delitem__(y) <==> del x[y]",
                                WRAPS (set_item) },
        { "__eq__",             (PclCFunction) object_method_eq,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__eq__(y) <==> x == y",
                                WRAPS (rich_compare) },
        { "__ge__",             (PclCFunction) object_method_ge,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__ge__(y) <==> x >= y",
                                WRAPS (rich_compare) },
        { "__getitem__",        (PclCFunction) object_method_getitem,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__getitem__(y) <==> x[y]",
                                WRAPS (get_item) },
        { "__gt__",             (PclCFunction) object_method_gt,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__gt__(y) <==> x > y",
                                WRAPS (rich_compare) },
        { "__hash__",           (PclCFunction) object_method_hash,
                                PCL_METHOD_FLAG_NOARGS,
                                "x.__hash__() <==> hash(x)",
                                WRAPS (hash) },
        { "__iter__",           (PclCFunction) object_method_iter,
                                PCL_METHOD_FLAG_NOARGS,
                                "x.__iter__() <==> iter(x)",
                                WRAPS (iterate) },
        { "__le__",             (PclCFunction) object_method_le,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__le__(y) <==> x <= y",
                                WRAPS (rich_compare) },
        { "__len__",            (PclCFunction) object_method_len,
                                PCL_METHOD_FLAG_NOARGS,
                                "x.__len__() <==> len(x)",
                                WRAPS (measure) },
        { "__lt__",             (PclCFunction) object_method_lt,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__lt__(y) <==> x < y",
                                WRAPS (rich_compare) },
        { "__ne__",             (PclCFunction) object_method_ne,
                                PCL_METHOD_FLAG_ONEARG,
                                "x.__ne__(y) <==> x != y",
                                WRAPS (rich_compare) },
        { "__repr__",           (PclCFunction) object_method_repr,
                                PCL_METHOD_FLAG_NOARGS,
                                "x.__repr__() <==> repr(x)",
                                WRAPS (repr) },
        { "__setitem__",        (PclCFunction) object_method_setitem,
                                PCL_METHOD_FLAG_VARARGS,
                                "x.__setitem__(y, z) <==> x[y]=z",
                                WRAPS (set_item) },
        { "__str__",            (PclCFunction) object_method_str,
                                PCL_METHOD_FLAG_NOARGS,
                                "x.__str__() <==> str(x)",
                                WRAPS (str) },
        { NULL }
};

#undef WRAPS

GType
pcl_object_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclObjectClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) object_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclObject),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        G_TYPE_OBJECT, "PclObject", &type_info, 0);
                g_type_set_qdata (
                        type, PCL_DATA_GETSETS, object_getsets);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, object_methods);
        }
        return type;
}

/**
 * pcl_object_get_type_object:
 *
 * Returns the type object for #PclObject.  During runtime this is the built-in
 * object %object.
 *
 * Returns: a borrowed reference to the type object for #PclObject
 */
PclObject *
pcl_object_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_OBJECT, "object");
                pcl_register_singleton ("<type 'object'>", &object);
        }
        return object;
}

PclObject *
pcl_object_type (PclObject *object)
{
        PclObjectClass *class;

        if (object == NULL)
        {
                null_error ();
                return NULL;
        }
        class = PCL_OBJECT_GET_CLASS (object);
        return pcl_object_ref (class->type ());
}

PclObject *
pcl_object_clone (PclObject *object)
{
        PclObjectClass *class = PCL_OBJECT_GET_CLASS (object);
        if (class->clone != NULL)
        {
                if (PCL_IS_CONTAINER (object))
                {
                        PclThreadState *ts;
                        PclObject *cached;
                        ts = pcl_thread_state_get ();
                        cached = g_hash_table_lookup (ts->cache, object);
                        if (cached != NULL)
                                return pcl_object_ref (cached);
                }
                return class->clone (object);
        }
        return pcl_object_ref (object);
}

gboolean
pcl_object_print (PclObject *object, FILE *stream, PclPrintFlags flags)
{
        return object_internal_print (object, stream, flags, 0);
}

PclObject *
pcl_object_call (PclObject *object, PclObject *args, PclObject *kwds)
{
        PclObjectClass *class;

        class = PCL_OBJECT_GET_CLASS (object);

        if (class->call != NULL)
        {
                PclObject *result;

                result = class->call (object, args, kwds);
                if (result == NULL && !pcl_error_occurred ())
                        pcl_error_set_format (
                                pcl_exception_system_error (),
                                "NULL result without error in %s",
                                G_STRFUNC);
                return result;
        }

        pcl_error_set_format (
                pcl_exception_type_error (),
                "'%s' object is not callable",
                PCL_GET_TYPE_NAME (object));
        return NULL;
}

PclObject *
pcl_object_call_function (PclObject *object, const gchar *format, ...)
{
        PclObject *args = NULL;
        PclObject *result = NULL;
        va_list va;

        if (object == NULL)
        {
                null_error ();
                return NULL;
        }
        
        if (format != NULL && *format != '\0')
        {
                va_start (va, format);
                args = pcl_va_build_value (format, va);
                va_end (va);
        }
        else
                args = pcl_tuple_new (0);

        if (args == NULL)
                return NULL;

        if (!PCL_IS_TUPLE (args))
        {
                PclObject *tuple;

                tuple = pcl_tuple_new (1);
                if (tuple == NULL)
                {
                        pcl_object_unref (args);
                        return NULL;
                }
                if (!pcl_tuple_set_item (tuple, 0, args))
                {
                        pcl_object_unref (args);
                        return NULL;
                }
                args = tuple;
        }

        result = pcl_object_call (object, args, NULL);

        pcl_object_unref (args);

        return result;
}

PclObject *
pcl_object_call_method (PclObject *object, const gchar *name,
                        const gchar *format, ...)
{
        PclObject *args = NULL;
        PclObject *method = NULL;
        PclObject *result = NULL;
        va_list va;

        if (object == NULL || name == NULL)
        {
                null_error ();
                return NULL;
        }

        method = pcl_object_get_attr_string (object, name);

        if (method == NULL)
        {
                pcl_error_set_string (
                        pcl_exception_attribute_error (), name);
                return NULL;
        }

        if (!PCL_IS_CALLABLE (method))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "call of non-callable attribute");
                pcl_object_unref (method);
                return NULL;
        }

        if (format != NULL && *format != '\0')
        {
                va_start (va, format);
                args = pcl_va_build_value (format, va);
                va_end (va);
        }
        else
                args = pcl_tuple_new (0);

        if (args == NULL)
        {
                pcl_object_unref (method);
                return NULL;
        }

        if (!PCL_IS_TUPLE (args))
        {
                PclObject *tuple;

                tuple = pcl_tuple_new (1);
                if (tuple == NULL)
                {
                        pcl_object_unref (args);
                        pcl_object_unref (method);
                        return NULL;
                }
                if (!pcl_tuple_set_item (tuple, 0, args))
                {
                        pcl_object_unref (args);
                        pcl_object_unref (method);
                        return NULL;
                }
                args = tuple;
        }

        result = pcl_object_call (method, args, NULL);

        pcl_object_unref (args);
        pcl_object_unref (method);

        return result;
}

PclObject *
pcl_object_call_object (PclObject *object, PclObject *args)
{
        /* XXX Is this function really necessary? */
        return pcl_eval_call_object_with_keywords (object, args, NULL);
}

PclObject *
pcl_object_copy (PclObject *object)
{
        PclObjectClass *class = PCL_OBJECT_GET_CLASS (object);

        if (class->copy != NULL)
                return class->copy (object);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "don't know how to copy a '%s' object",
                PCL_GET_TYPE_NAME (object));
        return NULL;
}

PclObject *
pcl_object_get_dict (PclObject *object)
{
        PclObjectClass *class;
        if (object == NULL)
                return NULL;
        class = PCL_OBJECT_GET_CLASS (object);
        if (class->dict == NULL)
                return NULL;
        return class->dict (object);
}

guint
pcl_object_hash (PclObject *object)
{
        PclObjectClass *class;
        guint hash = PCL_HASH_INVALID;

        class = PCL_OBJECT_GET_CLASS (object);

        if (class->hash != NULL)
                hash = class->hash (object);

        /* TODO: If object is neither comparable nor rich-comparable,
         *       use the object's address as the hash value.
         *       Requires long integers. */

        if (hash == PCL_HASH_INVALID && !pcl_error_occurred ())
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "%s objects are unhashable",
                        PCL_GET_TYPE_NAME (object));
        return hash;
}

PclObject *
pcl_object_repr (PclObject *object)
{
        PclObject *result;
        PclObjectClass *class;
        if (!pcl_error_check_signals ())
                return NULL;
        if (object == NULL)
                return pcl_string_from_string ("<NULL>");
        class = PCL_OBJECT_GET_CLASS (object);
        if ((result = class->repr (object)) == NULL)
                return NULL;
        if (!PCL_IS_STRING (result))
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "__repr__ returned non-string (type %s)",
                        PCL_GET_TYPE_NAME (result));
                pcl_object_unref (result);
                return NULL;
        }
        return result;
}

PclObject *
pcl_object_str (PclObject *object)
{
        PclObject *result;
        PclObjectClass *class;
        if (object == NULL)
                return pcl_string_from_string ("<NULL>");
        class = PCL_OBJECT_GET_CLASS (object);
        if (class->str == NULL)
                return class->repr (object);
        if ((result = class->str (object)) == NULL)
                return NULL;
        if (!PCL_IS_STRING (result))
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "__str__ returned non-string (type %s)",
                        PCL_GET_TYPE_NAME (result));
                pcl_object_unref (result);
                return NULL;
        }
        return result;
}

guint
pcl_object_hash_pointer (gpointer pointer)
{
        /* XXX This is not at all portable! */
        g_assert (sizeof (guint) >= sizeof (gpointer));
        return GPOINTER_TO_UINT (pointer);
}

/* helper functions for pcl_object_compare() */

static gint
rich_compare_try_bool (PclObject *object1, PclObject *object2,
                       PclRichCompareOps op);

static gint
compare_adjust_result (gint cmp)
{
        if (pcl_error_occurred ())
        {
                /* XXX Runtime warning here... */
                return -2;
        }
        else if (cmp < -1 || cmp > 1)
        {
                /* XXX Runtime warning here... */
                return (cmp < -1) ? -1 : 1;
        }
        else
        {
                g_assert (-1 <= cmp && cmp <= 1);
                return cmp;
        }
}

static gint
compare_try_3way (PclObject *object1, PclObject *object2)
{
        PclObjectClass *class;
        gint cmp;

        /* XXX Check if either object is an instance here... */

        class = PCL_OBJECT_GET_CLASS (object1);
        if (class->compare != NULL)
        {
                cmp = class->compare (object1, object2);
                return compare_adjust_result (cmp);
        }

        class = PCL_OBJECT_GET_CLASS (object2);
        if (class->compare != NULL)
        {
                cmp = class->compare (object2, object1);
                cmp = compare_adjust_result (cmp);
                if (cmp >= -1)
                        cmp = -cmp;
                return cmp;
        }

        return 2;
}

static gint
compare_try_rich_to_3way (PclObject *object1, PclObject *object2)
{
        gint is_true;

        is_true = rich_compare_try_bool (object1, object2, PCL_EQ);
        if (is_true == -1)
                return -2;
        if (is_true == 1)
                return 0;

        is_true = rich_compare_try_bool (object1, object2, PCL_LT);
        if (is_true == -1)
                return -2;
        if (is_true == 1)
                return -1;

        is_true = rich_compare_try_bool (object1, object2, PCL_GT);
        if (is_true == -1)
                return -2;
        if (is_true == 1)
                return 1;

        return 2;
}

static gint
compare_default_3way (PclObject *object1, PclObject *object2)
{
        gint cmp;

        /* Same type - compare addresses */
        if (G_OBJECT_TYPE (object1) == G_OBJECT_TYPE (object2))
                return (object1 < object2) ? -1 : (object1 > object2) ? 1 : 0;

        /* None is smaller than anything. */
        if (object1 == PCL_NONE)
                return -1;
        if (object2 == PCL_NONE)
                return 1;

        /* Different types - compare type names (numbers are smaller) */
        cmp = strcmp (
                !PCL_IS_NUMBER (object1) ? PCL_GET_TYPE_NAME (object1) : "",
                !PCL_IS_NUMBER (object2) ? PCL_GET_TYPE_NAME (object2) : "");
        if (cmp < 0)
                return -1;
        if (cmp > 0)
                return 1;

        /* Same type name, or (more likely) incomparable numeric types */
        return (object1 < object2) ? -1 : 1;
}

static gint
compare_algorithm (PclObject *object1, PclObject *object2)
{
        gint cmp;

        if (G_OBJECT_TYPE (object1) == G_OBJECT_TYPE (object2))
        {
                PclObjectClass *class;

                class = PCL_OBJECT_GET_CLASS (object1);
                if (class->compare != NULL)
                {
                        cmp = class->compare (object1, object2);
                        return compare_adjust_result (cmp);
                }
        }

        /* We only get here if one of the following is true:
         * a) The objects have different types.
         * b) The objects have the same type, which isn't Comparable.
         * c) The objects are instances, and either __cmp__ is not defined or
         *    __cmp__ returns NotImplemented. */
        cmp = compare_try_rich_to_3way (object1, object2);
        if (cmp < 2)
                return cmp;
        cmp = compare_try_3way (object1, object2);
        if (cmp < 2)
                return cmp;
        return compare_default_3way (object1, object2);
}

gint
pcl_object_compare (PclObject *object1, PclObject *object2)
{
        gint result;

        if (object1 == NULL || object2 == NULL)
        {
                pcl_error_bad_internal_call ();
                return -1;
        }

        if (object1 == object2)
                return 0;
        if (!pcl_enter_recursive_call ())
                return -1;
        result = compare_algorithm (object1, object2);
        pcl_leave_recursive_call ();
        return (result < 0) ? -1 : result;
}

gint
pcl_object_contains (PclObject *object, PclObject *value)
{
        PclObjectClass *class;

        if (object == NULL || value == NULL)
        {
                null_error ();
                return -1;
        }

        class = PCL_OBJECT_GET_CLASS (object);

        if (class->contains != NULL)
                return class->contains (object, value);

        pcl_error_set_string (
                pcl_exception_type_error (),
                "iterable argument required");
        return -1;
}

/* helper functions for pcl_object_rich_compare() */

static PclObject *
rich_compare_try (PclObject *object1, PclObject *object2,
                  PclRichCompareOps op)
{
        static gint swap[] =
                { PCL_GT, PCL_GE, PCL_EQ, PCL_NE, PCL_LT, PCL_LE };

        PclObjectClass *class;

        /* XXX Skipped a section here... */

        class = PCL_OBJECT_GET_CLASS (object1);
        if (class->rich_compare != NULL)
        {
                PclObject *result;

                result = class->rich_compare (object1, object2, op);
                if (result != PCL_NOT_IMPLEMENTED)
                        return result;
                pcl_object_unref (result);
        }

        op = swap[op];

        class = PCL_OBJECT_GET_CLASS (object2);
        if (class->rich_compare != NULL)
                return class->rich_compare (object2, object1, op);

        return pcl_object_ref (PCL_NOT_IMPLEMENTED);
}

static gint
rich_compare_try_bool (PclObject *object1, PclObject *object2,
                       PclRichCompareOps op)
{
        PclObject *result;
        gint is_true;

        result = rich_compare_try (object1, object2, op);
        if (result == NULL)
                return -1;
        if (result == PCL_NOT_IMPLEMENTED)
        {
                pcl_object_unref (result);
                return 2;
        }
        is_true = pcl_object_is_true (result);
        pcl_object_unref (result);
        return is_true;
}

static PclObject *
rich_compare_3way_to_object (PclRichCompareOps op, gint cmp)
{
        switch (op)
        {
                case PCL_LT:
                        cmp = cmp < 0;
                        break;
                case PCL_LE:
                        cmp = cmp <= 0;
                        break;
                case PCL_EQ:
                        cmp = cmp == 0;
                        break;
                case PCL_NE:
                        cmp = cmp != 0;
                        break;
                case PCL_GT:
                        cmp = cmp > 0;
                        break;
                case PCL_GE:
                        cmp = cmp >= 0;
                        break;
        }

        return pcl_object_ref ((cmp != 0) ? PCL_TRUE : PCL_FALSE);
}

static PclObject *
rich_compare_try_3way (PclObject *object1, PclObject *object2,
                       PclRichCompareOps op)
{
        gint cmp;

        cmp = compare_try_3way (object1, object2);
        if (cmp >= 2)
                cmp = compare_default_3way (object1, object2);
        if (cmp <= -2)
                return NULL;
        return rich_compare_3way_to_object (op, cmp);
}

static PclObject *
rich_compare_algorithm (PclObject *object1, PclObject *object2,
                        PclRichCompareOps op)
{
        PclObject *result;

        result = rich_compare_try (object1, object2, op);
        if (result != PCL_NOT_IMPLEMENTED)
                return result;
        pcl_object_unref (result);

        return rich_compare_try_3way (object1, object2, op);
}

PclObject *
pcl_object_rich_compare (PclObject *object1, PclObject *object2,
                         PclRichCompareOps op)
{
        PclObject *result;

        if (object1 == NULL || object2 == NULL)
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        g_assert (PCL_LT <= op && op <= PCL_GE);

        if (!pcl_enter_recursive_call ())
                return NULL;

        /* If the types are equal, try to get out cheap. */
        if (G_OBJECT_TYPE (object1) == G_OBJECT_TYPE (object2))
        {
                PclObjectClass *class;

                class = PCL_OBJECT_GET_CLASS (object1);

                if (class->rich_compare != NULL)
                {
                        result = class->rich_compare (object1, object2, op);
                        if (result != PCL_NOT_IMPLEMENTED)
                                goto done;
                        pcl_object_unref (result);
                }

                if (class->compare != NULL)
                {
                        gint cmp;

                        cmp = class->compare (object1, object2);
                        cmp = compare_adjust_result (cmp);
                        if (cmp == -2)
                        {
                                result = NULL;
                                goto done;
                        }
                        result = rich_compare_3way_to_object (op, cmp);
                        goto done;
                }
        }

        /* Fast path not taken, or couldn't deliver a useful result. */
        result = rich_compare_algorithm (object1, object2, op);

done:

        pcl_leave_recursive_call ();
        return result;
}

gint
pcl_object_rich_compare_bool (PclObject *object1, PclObject *object2,
                              PclRichCompareOps op)
{
        PclObject *result;
        gint is_true;

        /* Quick result when objects are the same.
         * Guarantees that identity implies equality. */
        if (object1 == object2)
        {
                if (op == PCL_EQ)
                        return 1;
                if (op == PCL_NE)
                        return 0;
        }

        result = pcl_object_rich_compare (object1, object2, op);
        if (result == NULL)
                return -1;
        if (PCL_IS_BOOL (result))
                is_true = (result == PCL_TRUE);
        else
                is_true = pcl_object_is_true (result);
        pcl_object_unref (result);
        return is_true;
}

PclObject *
pcl_object_iterate (PclObject *object)
{
        PclObjectClass *class;

        if (object == NULL)
        {
                null_error ();
                return NULL;
        }

        class = PCL_OBJECT_GET_CLASS (object);

        if (class->iterate != NULL)
        {
                PclObject *result;

                result = class->iterate (object);
                if (result != NULL && !PCL_IS_ITERATOR (result))
                {
                        pcl_error_set_format (
                                pcl_exception_type_error (),
                                "iter() returned non-iterator of type '%s'",
                                PCL_GET_TYPE_NAME (result));
                        pcl_object_unref (result);
                        return NULL;
                }
                return result;
        }

        pcl_error_set_string (
                pcl_exception_type_error (),
                "iterable argument required");
        return NULL;
}

glong
pcl_object_measure (PclObject *object)
{
        PclObjectClass *class;

        if (object == NULL)
        {
                null_error ();
                return -1;
        }

        class = PCL_OBJECT_GET_CLASS (object);

        if (class->measure != NULL)
                return class->measure (object);

        pcl_error_set_string (
                pcl_exception_type_error (),
                "len() of unsized object");
        return -1;
}

PclObject *
pcl_object_get_attr (PclObject *object, PclObject *name)
{
        gchar *name_string;

        if (!PCL_IS_STRING (name))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "attribute name must be a string");
                return NULL;
        }

        pcl_string_intern_in_place (&name);
        name_string = pcl_string_as_string (name);
        return pcl_object_get_attr_string (object, name_string);
}

gboolean
pcl_object_has_attr (PclObject *object, PclObject *name)
{
        PclObject *result;

        result = pcl_object_get_attr (object, name);
        if (result != NULL)
        {
                pcl_object_unref (result);
                return TRUE;
        }
        pcl_error_clear ();
        return FALSE;
}

gboolean
pcl_object_set_attr (PclObject *object, PclObject *name, PclObject *value)
{
        gchar *name_string;

        if (!PCL_IS_STRING (name))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "attribute name must be a string");
                return FALSE;
        }

        pcl_string_intern_in_place (&name);
        name_string = pcl_string_as_string (name);
        return pcl_object_set_attr_string (object, name_string, value);
}

PclObject *
pcl_object_get_attr_string (PclObject *object, const gchar *name)
{
        PclObjectClass *class;

        class = PCL_OBJECT_GET_CLASS (object);
        if (class->get_attr != NULL)
                return class->get_attr (object, name);
        pcl_error_set_format (
                pcl_exception_attribute_error (),
                "'%s' object has no attribute '%s'",
                PCL_GET_TYPE_NAME (object), name);
        return NULL;
}

gboolean
pcl_object_has_attr_string (PclObject *object, const gchar *name)
{
        PclObject *result;

        result = pcl_object_get_attr_string (object, name);
        if (result != NULL)
        {
                pcl_object_unref (result);
                return TRUE;
        }
        pcl_error_clear ();
        return FALSE;
}

gboolean
pcl_object_set_attr_string (PclObject *object, const gchar *name,
                            PclObject *value)
{
        PclObjectClass *class;

        class = PCL_OBJECT_GET_CLASS (object);
        if (class->set_attr != NULL)
                return class->set_attr (object, name, value);
        if (class->get_attr == NULL)
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "'%s' object has no attributes (%s .%s)",
                        PCL_GET_TYPE_NAME (object),
                        (value == NULL) ? "del" : "assign to", name);
        else
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "'%s' object has only read-only attributes "
                        "(%s .%s)", PCL_GET_TYPE_NAME (object),
                        (value == NULL) ? "del" : "assign to", name);
        return FALSE;
}

PclObject *
pcl_object_get_item (PclObject *object, PclObject *subscript)
{
        PclObjectClass *class;

        if (object == NULL || subscript == NULL)
        {
                null_error ();
                return NULL;
        }

        class = PCL_OBJECT_GET_CLASS (object);

        if (class->get_item != NULL)
                return class->get_item (object, subscript);

        pcl_error_set_string (
                pcl_exception_type_error (),
                "unsubscriptable object");
        return NULL;
}

gboolean
pcl_object_set_item (PclObject *object, PclObject *subscript, PclObject *value)
{
        PclObjectClass *class;

        if (object == NULL || subscript == NULL)
        {
                null_error ();
                return FALSE;
        }

        class = PCL_OBJECT_GET_CLASS (object);

        if (class->set_item != NULL)
                return class->set_item (object, subscript, value);

        pcl_error_set_format (
                pcl_exception_type_error (),
                "object doesn't support %s %s",
                PCL_IS_SLICE (subscript) ? "slice" : "item",
                (value != NULL) ? "assignment" : "deletion");
        return FALSE;
}

gboolean
pcl_object_del_item (PclObject *object, PclObject *subscript)
{
        return pcl_object_set_item (object, subscript, NULL);
}

/* helper for pcl_object_dir() */
static gboolean
object_merge_class_dict (PclObject *dict, PclObject *class)
{
        PclObject *class_dict;
        PclObject *bases;

        g_assert (PCL_IS_DICT (dict));
        g_assert (class != NULL);

        /* merge in the type's dict (if any) */
        class_dict = pcl_object_get_attr_string (class, "__dict__");
        if (class_dict == NULL)
                pcl_error_clear ();
        else
        {
                gboolean success;

                success = pcl_mapping_update (dict, class_dict);
                pcl_object_unref (class_dict);
                if (!success)
                        return FALSE;
        }

        /* recursively merge in the base types' (if any) dicts */
        bases = pcl_object_get_attr_string (class, "__bases__");
        if (bases == NULL)
                pcl_error_clear ();
        else
        {
                PclObject *iterator;
                PclObject *next;

                iterator = pcl_object_iterate (bases);
                while ((next = pcl_iterator_next (iterator)) != NULL)
                {
                        gboolean success;

                        success = object_merge_class_dict (dict, next);
                        pcl_object_unref (next);
                        if (!success)
                        {
                                pcl_object_unref (bases);
                                pcl_object_unref (iterator);
                                return FALSE;
                        }
                }
                pcl_object_unref (bases);
                pcl_object_unref (iterator);
        }
        return TRUE;
}

PclObject *
pcl_object_dir (PclObject *object)
{
        PclObject *iterator = NULL;
        PclObject *master_dict = NULL;
        PclObject *result = NULL;

        if (object == NULL)
        {
                PclObject *locals = pcl_eval_get_locals ();
                if (locals == NULL)
                        goto error;
                iterator = pcl_mapping_keys (locals);
                if (iterator == NULL)
                        goto error;
        }
        else if (PCL_IS_MODULE (object))
        {
                master_dict = pcl_object_get_attr_string (object, "__dict__");
                if (master_dict == NULL)
                        goto error;
                if (!PCL_IS_DICT (master_dict))
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "module.__dict__ is not a dictionary");
                        goto error;
                }
        }
        else if (PCL_IS_TYPE (object))
        {
                master_dict = pcl_dict_new ();
                if (master_dict == NULL)
                        goto error;
                if (!object_merge_class_dict (master_dict, object))
                        goto error;
        }
        else
        {
                PclObject *class;

                /* Create a dict to start with.  CAUTION: Not everything
                 * responding to __dict__ returns a dict! */
                master_dict = pcl_object_get_attr_string (object, "__dict__");
                if (master_dict == NULL)
                {
                        pcl_error_clear ();
                        master_dict = pcl_dict_new ();
                }
                else if (!PCL_IS_DICT (master_dict))
                {
                        pcl_object_unref (master_dict);
                        master_dict = pcl_dict_new ();
                }
                else
                {
                        /* The object may have returned a reference to its
                         * dict, so copy it to avoid mutating it. */
                        PclObject *temp = pcl_object_copy (master_dict);
                        pcl_object_unref (master_dict);
                        master_dict = temp;
                }
                if (master_dict == NULL)
                        goto error;

                /* Merge in attrs reachable from its class. */
                class = pcl_object_get_attr_string (object, "__class__");
                if (class == NULL)
                        pcl_error_clear ();
                else
                {
                        gboolean success;

                        success = object_merge_class_dict (master_dict, class);
                        pcl_object_unref (class);
                        if (!success)
                                goto error;
                }
        }

        g_assert ((iterator == NULL) ^ (master_dict == NULL));

        if (master_dict != NULL)
        {
                iterator = pcl_mapping_keys (master_dict);
                if (iterator == NULL)
                        goto error;
        }

        if (!PCL_IS_ITERATOR (iterator))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "expected keys() to be an iterator");
                goto error;
        }

        result = pcl_list_from_iterator (iterator);
        if (result == NULL)
                goto error;
        pcl_object_unref (iterator);
        pcl_list_sort (result);
        goto success;

error:
        if (iterator != NULL)
                pcl_object_unref (iterator);
        if (result != NULL)
                pcl_object_unref (result);
        result = NULL;

success:
        if (master_dict != NULL)
                pcl_object_unref (master_dict);
        return result;
}

gint
pcl_object_is_instance (PclObject *instance, PclObject *of)
{
        gint limit = pcl_get_recursion_limit ();
        return object_recursive_is_instance (instance, of, limit);
}

gint
pcl_object_is_subclass (PclObject *subclass, PclObject *of)
{
        gint limit = pcl_get_recursion_limit ();
        return object_recursive_is_subclass (subclass, of, limit);
}

gint
pcl_object_is_true (PclObject *object)
{
        if (object == PCL_TRUE)
                return TRUE;
        if (object == PCL_FALSE)
                return FALSE;
        if (object == PCL_NONE)
                return FALSE;
        /* XXX We need to check measurable first because sets implement
         *     the number interface for bitwise operators, but not
         *     the nonzero() method.  More evidence that the operator
         *     architecture for built-in types needs an overhaul. */
        if (PCL_IS_MEASURABLE (object))
                return (pcl_object_measure (object) > 0);
        if (PCL_IS_NUMBER (object))
                return pcl_number_nonzero (object);
        return TRUE;
}

gint
pcl_object_not (PclObject *object)
{
        gint result;

        result = pcl_object_is_true (object);
        if (result < 0)
                return result;
        return (result == 0);
}
