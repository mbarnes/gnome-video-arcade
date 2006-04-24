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
#include "glib-ext.h"

static gchar builtin_doc[] =
"Built-in functions, exceptions, and other objects.\n\
\n\
Noteworthy: None is the `nil' object; Ellipsis represents `...' in slices.\n";

static PclObject *
builtin___import__(PclObject *self, PclObject *args)
{
        gchar *name;
        PclObject *globals = NULL;
        PclObject *locals = NULL;
        PclObject *fromlist = NULL;

        if (!pcl_arg_parse_tuple (args, "s|OOO:__import__",
                                  &name, &globals, &locals, &fromlist))
                return NULL;
        return pcl_import_module_ex (name, globals, locals, fromlist);
}

static gchar builtin___import___doc[] =
"__import__(name, globals, locals, fromlist) -> module\n\
\n\
Import a module.  The globals are only used to determine the context;\n\
they are not modified.  The locals are currently unused.  The fromlist\n\
should be a list of names to emulate ``from name import ...'', or an\n\
empty list to emulate ``import name''.\n\
When importing a module from a package, note that __import__('A.B', ...)\n\
returns package A when fromlist is empty, but its submodule B when\n\
fromlist is not empty.";

static PclObject *
builtin_abs (PclObject *self, PclObject *object)
{
        return pcl_number_absolute (object);
}

static gchar builtin_abs_doc[] =
"abs(number) -> number\n\
\n\
Return the absolute value of the argument.";

static PclObject *
builtin_all (PclObject *self, PclObject *iterable)
{
        PclObject *iterator;
        PclObject *next;

        iterator = pcl_object_iterate (iterable);
        if (iterator == NULL)
                return NULL;

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                gboolean is_true = pcl_object_is_true (next);
                pcl_object_unref (next);
                /* XXX May need to check for exception here. */
                if (!is_true)
                {
                        pcl_object_unref (iterator);
                        return pcl_object_ref (PCL_FALSE);
                }
        }

        pcl_object_unref (iterator);
        if (pcl_error_occurred ())
                return NULL;
        return pcl_object_ref (PCL_TRUE);
}

static gchar builtin_all_doc[] =
"all(iterable) -> bool\n\
\n\
Return True if bool(x) is True for all values x in the iterable.";

static PclObject *
builtin_any (PclObject *self, PclObject *iterable)
{
        PclObject *iterator;
        PclObject *next;

        iterator = pcl_object_iterate (iterable);
        if (iterator == NULL)
                return NULL;

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                gboolean is_true = pcl_object_is_true (next);
                pcl_object_unref (next);
                /* XXX May need to check for exception here. */
                if (is_true)
                {
                        pcl_object_unref (iterator);
                        return pcl_object_ref (PCL_TRUE);
                }
        }

        pcl_object_unref (iterator);
        if (pcl_error_occurred ())
                return NULL;
        return pcl_object_ref (PCL_FALSE);
}

static gchar builtin_any_doc[] =
"any(iterable) -> bool\n\
\n\
Return True if bool(x) is True for any x in the iterable.";

static PclObject *
builtin_chr (PclObject *self, PclObject *args)
{
        glong x;
        gchar s[1];

        if (!pcl_arg_parse_tuple (args, "l:chr", &x))
                return NULL;

        if (x < 0 || x > 255)
        {
                pcl_error_set_string (
                        pcl_exception_value_error (),
                        "chr() arg not in range(256)");
                return NULL;
        }

        s[0] = (gchar) x;
        return pcl_string_from_string_and_size (s, 1);
}

static gchar builtin_chr_doc[] =
"chr(i) -> character\n\
\n\
Return a string of one character with ordinal i; 0 <= i < 256.";

static PclObject *
builtin_clone (PclObject *self, PclObject *object)
{
        PclThreadState *ts;
        ts = pcl_thread_state_get ();
        /* XXX Not really in GLib 2.6 */
        g_hash_table_remove_all (ts->cache);
        return pcl_object_clone (object);
}

static gchar builtin_clone_doc[] =
"clone(object) -> clone\n\
\n\
If object is mutable, return a new identical copy of object.\n\
If object is immutable, return the object itself.";

static PclObject *
builtin_cmp (PclObject *self, PclObject *args)
{
        PclObject *x, *y;
        gint cmp;

        if (!pcl_arg_unpack_tuple (args, "cmp", 2, 2, &x, &y))
                return NULL;

        cmp = pcl_object_compare (x, y);
        return pcl_int_from_long ((glong) cmp);
}

static gchar builtin_cmp_doc[] =
"cmp(x, y) -> integer\n\
\n\
Return negative if x<y, zero if x==y, positive if x>y.";

static PclObject *
builtin_delattr (PclObject *self, PclObject *args)
{
        PclObject *name;
        PclObject *object;

        if (!pcl_arg_unpack_tuple (args, "delattr", 2, 2,
                                   &object, &name))
                return NULL;

        if (!pcl_object_set_attr (object, name, NULL))
                return NULL;

        return pcl_object_ref (PCL_NONE);
}

static gchar builtin_delattr_doc[] =
"delattr(object, name)\n\
\n\
Delete a named attribute on an object; delattr(x, 'y') is equivalent to\n\
``del x.y''.";

static PclObject *
builtin_dir (PclObject *self, PclObject *args)
{
        PclObject *object = NULL;

        if (!pcl_arg_unpack_tuple (args, "dir", 0, 1, &object))
                return NULL;

        return pcl_object_dir (object);
}

static gchar builtin_dir_doc[] =
"dir([object]) -> list of strings\n\
\n\
Return an alphabetized list of names comprising (some of) the attributes\n\
of the given object, and of attributes reachable from it:\n\
\n\
No argument:  the names in the current scope.\n\
Module object:  the module attributes.\n\
Type of class object:  its attributes, and recursively the attributes of\n\
    its bases.\n\
Otherwise:  its attributes, its class's attributes, and recursively the\n\
    attributes of its class's base classes.";

static PclObject *
builtin_divmod (PclObject *self, PclObject *args)
{
        PclObject *x, *y;

        if (!pcl_arg_unpack_tuple (args, "divmod", 2, 2, &x, &y))
                return NULL;
        return pcl_number_divmod (x, y);
}

static gchar builtin_divmod_doc[] =
"divmod(x, y) -> (div, mod)\n\
\n\
Return the tuple ((x-x%y)/y, x%y).  Invariant: div*y + mod == x.";

static PclObject *
builtin_eval (PclObject *self, PclObject *args)
{
        /* XXX This is almost identical to eval_exec_statement() */

        PclObject *globals = PCL_NONE;
        PclObject *locals = PCL_NONE;
        PclObject *source;
        gchar *string;

        /* check arguments */
        if (!pcl_arg_unpack_tuple (args, "eval", 1, 3,
                        &source, &globals, &locals))
                return NULL;
        if (locals != PCL_NONE && !PCL_IS_MAPPING (locals))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "locals must be a mapping");
                return NULL;
        }
        if (globals != PCL_NONE && !PCL_IS_DICT (globals))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        PCL_IS_MAPPING (globals) ?
                        "globals must be a real dictionary; "
                        "try eval(expr, {}, mapping)" :
                        "globals must be a dictionary");
                return NULL;
        }

        /* handle defaults */
        if (globals == PCL_NONE)
        {
                globals = pcl_eval_get_globals ();
                if (locals == PCL_NONE)
                        locals = pcl_eval_get_locals ();
        }
        else if (locals == PCL_NONE)
                locals = globals;

        if (globals == NULL || locals == NULL)
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "eval must be given globals and locals when called "
                        "without a frame");
                return NULL;
        }

        /* provide built-ins */
        if (pcl_dict_get_item_string (globals, "__builtins__") == NULL)
        {
                if (!pcl_dict_set_item_string (globals, "__builtins__",
                                pcl_eval_get_builtins ()))
                        return NULL;
        }

        /* source might be a code object */
        if (PCL_IS_CODE (source))
        {
                if (PCL_CODE_GET_NUM_FREE (source) > 0)
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "code object passed to eval() may not "
                                "contain free variables");
                        return NULL;
                }
                return pcl_eval_code (PCL_CODE (source), globals, locals);
        }

        /* ... otherwise, source MUST be a string object */
        if (!PCL_IS_STRING (source))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "eval() arg 1 must be a string or code object");
                return NULL;
        }

        /* handle source as a string object */
        string = pcl_string_as_string (source);
        while (*string == ' ' || *string == '\t')
                string++;
        return pcl_run_string (string, PCL_EVAL_INPUT, globals, locals);
}

static gchar builtin_eval_doc[] =
"eval(source[, globals[, locals]]) -> value\n\
\n\
Evaluate the source in the context of globals and locals.\n\
The source may be a string representing a Python expression\n\
or a code object as returned by compile().\n\
The globals must be a dictionary and locals can be any mapping,\n\
defaulting to the current globals and locals.\n\
If only globals is given, locals defaults to it.\n";

static PclObject *
builtin_getattr (PclObject *self, PclObject *args)
{
        PclObject *default_val = NULL;
        PclObject *name;
        PclObject *object;
        PclObject *result;

        if (!pcl_arg_unpack_tuple (args, "getattr", 2, 3,
                                   &object, &name, &default_val))
                return NULL;

        result = pcl_object_get_attr (object, name);
        if (result == NULL && default_val != NULL)
                result = pcl_object_ref (default_val);
        return result;
}

static gchar builtin_getattr_doc[] =
"getattr(object, name[, default]) -> value\n\
\n\
Get a named attribute from an object; getattr(x, 'y') is equivalent to x.y.\n\
When a default argument is given, it is returned when the attribute doesn't\n\
exist; without it, an exception is raised in that case.";

static PclObject *
builtin_globals (PclObject *self)
{
        /* Python doesn't check for NULL either. */
        return pcl_object_ref (pcl_eval_get_globals ());
}

static gchar builtin_globals_doc[] =
"globals() -> dictionary\n\
\n\
Return the dictionary containing the current scope's global variables.";

static PclObject *
builtin_hasattr (PclObject *self, PclObject *args)
{
        PclObject *name;
        PclObject *object;

        if (!pcl_arg_unpack_tuple (args, "hasattr", 2, 2,
                                   &object, &name))
                return NULL;

        return pcl_bool_from_boolean (
               pcl_object_has_attr (object, name));
}

static gchar builtin_hasattr_doc[] =
"hasattr(object, name) -> bool\n\
\n\
Return whether the object has an attribute with the given name.\n\
(This is done by calling getattr(object, name) and catching exceptions.)";

static PclObject *
builtin_hash (PclObject *self, PclObject *object)
{
        guint hash;

        hash = pcl_object_hash (object);
        if (hash == PCL_HASH_INVALID)
                return NULL;
        return pcl_int_from_long ((glong) hash);
}

static gchar builtin_hash_doc[] =
"hash(object) -> integer\n\
\n\
Return a hash value for the object.  Two objects with the same value have\n\
the same hash value.  The reverse is not necessarily true, but likely.";

static PclObject *
builtin_hex(PclObject *self, PclObject *object)
{
        return pcl_number_hex (object);
}

static gchar builtin_hex_doc[] =
"hex(number) -> string\n\
\n\
Return the hexadecimal representation of an integer or long integer.";

static PclObject *
builtin_isinstance (PclObject *self, PclObject *args)
{
        PclObject *instance;
        PclObject *of;
        gint result;

        if (!pcl_arg_unpack_tuple (args, "isinstance", 2, 2, &instance, &of))
                return NULL;

        result = pcl_object_is_instance (instance, of);
        if (result < 0)
                return NULL;
        return pcl_bool_from_boolean ((gboolean) result);
}

static gchar builtin_isinstance_doc[] =
"isinstance(object, class-or-type-or-tuple) -> bool\n\
\n\
Return whether an object is an instance of a class or of a subclass thereof.\n\
With a type as second argument, return whether that is the object's type.\n\
The form using a tuple, isinstance(x, (A, B, ...)), is a shortcut for\n\
isinstance(x, A) or isinstance(x, B) or ... (etc.).";

static PclObject *
builtin_issubclass (PclObject *self, PclObject *args)
{
        PclObject *subclass;
        PclObject *of;
        gint result;

        if (!pcl_arg_unpack_tuple (args, "issubclass", 2, 2, &subclass, &of))
                return NULL;

        result = pcl_object_is_subclass (subclass, of);
        if (result < 0)
                return NULL;
        return pcl_bool_from_boolean ((gboolean) result);
}

static gchar builtin_issubclass_doc[] =
"issubclass(C, B) -> bool\n\
\n\
Return whether class C is a subclass (i.e., a derived class) of class B.\n\
When using a tuple as the second argument issubclass(X, (A, B, ...)),\n\
is a shortcut for issubclass(X, A) or issubclass(X, B) or ... (etc.).";

static PclObject *
builtin_iter (PclObject *self, PclObject *args)
{
        PclObject *object;
        PclObject *sentinel = NULL;

        if (!pcl_arg_unpack_tuple (args, "iter", 1, 2, &object, &sentinel))
                return NULL;
        if (sentinel == NULL)
                return pcl_object_iterate (object);
        if (!PCL_IS_CALLABLE (object))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "iter(v, w): v must be callable");
                return NULL;
        }
        return pcl_callable_iterator_new (object, sentinel);
}

static gchar builtin_iter_doc[] =
"iter(collection) -> iterator\n\
iter(callable, sentinel) -> iterator\n\
\n\
Get an iterator from an object.  In the first form, the argument must\n\
supply its own iterator, or be a sequence.\n\
In the second form, the callable is called until it returns the sentinel.";

static PclObject *
builtin_len (PclObject *self, PclObject *object)
{
        glong length;

        length = pcl_object_measure (object);
        if (length < 0 && pcl_error_occurred ())
                return NULL;
        return pcl_int_from_long ((glong) length);
}

static gchar builtin_len_doc[] =
"len(object) -> integer\n\
\n\
Return the number of items in a sequence or mapping.";

static PclObject *
builtin_locals (PclObject *self)
{
        /* Python doesn't check for NULL either. */
        return pcl_object_ref (pcl_eval_get_locals ());
}

static gchar builtin_locals_doc[] =
"locals() -> dictionary\n\
\n\
Update and return a dictionary containing the current scope's local\n\
variables.";

static PclObject *
builtin_max (PclObject *self, PclObject *args)
{
        PclObject *iterable;
        PclObject *iterator;
        PclObject *next;
        PclObject *result;

        if (pcl_object_measure (args) > 1)
                iterable = args;
        else if (!pcl_arg_unpack_tuple (args, "max", 1, 1, &iterable))
                return NULL;

        iterator = pcl_object_iterate (iterable);
        if (iterator == NULL)
                return NULL;

        result = pcl_iterator_next (iterator);
        if (result == NULL)
        {
                if (!pcl_error_occurred ())
                        pcl_error_set_string (
                                pcl_exception_value_error (),
                                "max() arg is an empty sequence");
                pcl_object_unref (iterator);
                return NULL;
        }

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                if (pcl_object_compare (next, result) > 0)
                {
                        pcl_object_unref (result);
                        result = next;
                }
                else
                        pcl_object_unref (next);
        }

        pcl_object_unref (iterator);

        if (pcl_error_occurred ())
        {
                pcl_object_unref (result);
                return NULL;
        }

        return result;
}

static gchar builtin_max_doc[] =
"max(iterable) -> value\n\
max(a, b, c, ...) -> value\n\
\n\
With a single iterable argument, return its largest item.\n\
With two or more arguments, return the largest argument.";

static PclObject *
builtin_min (PclObject *self, PclObject *args)
{
        PclObject *iterable;
        PclObject *iterator;
        PclObject *next;
        PclObject *result;

        if (pcl_object_measure (args) > 1)
                iterable = args;
        else if (!pcl_arg_unpack_tuple (args, "min", 1, 1, &iterable))
                return NULL;

        iterator = pcl_object_iterate (iterable);
        if (iterator == NULL)
                return NULL;

        result = pcl_iterator_next (iterator);
        if (result == NULL)
        {
                if (!pcl_error_occurred ())
                        pcl_error_set_string (
                                pcl_exception_value_error (),
                                "min() arg is an empty sequence");
                pcl_object_unref (iterator);
                return NULL;
        }

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                if (pcl_object_compare (next, result) < 0)
                {
                        pcl_object_unref (result);
                        result = next;
                }
                else
                        pcl_object_unref (next);
        }

        pcl_object_unref (iterator);

        if (pcl_error_occurred ())
        {
                pcl_object_unref (result);
                return NULL;
        }

        return result;
}

static gchar builtin_min_doc[] =
"min(iterable) -> value\n\
min(a, b, c, ...) -> value\n\
\n\
With a single iterable argument, return its smallest item.\n\
With two or more arguments, return the smallest argument.";

static PclObject *
builtin_oct (PclObject *self, PclObject *object)
{
        return pcl_number_oct (object);
}

static gchar builtin_oct_doc[] =
"oct(number) -> string\n\
\n\
Return the octal representation of an integer or long integer.";

static PclObject *
builtin_ord (PclObject *self, PclObject *object)
{
        glong length;

        if (PCL_IS_STRING (object))
        {
                length = pcl_object_measure (object);
                if (length == 1)
                {
                        guchar chr = (guchar) *pcl_string_as_string (object);
                        return pcl_int_from_long ((glong) chr);
                }
        }
        else
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "ord() expected string of length 1, but %s found",
                        PCL_GET_TYPE_NAME (object));
                return NULL;
        }

        pcl_error_set_format (
                pcl_exception_type_error (),
                "ord() expected a character, "
                "but string of length %d found",
                length);
        return NULL;
}

static gchar builtin_ord_doc[] =
"ord(c) -> integer\n\
\n\
Return the integer ordinal of a one-character string.";

static PclObject *
builtin_pow (PclObject *self, PclObject *args)
{
        PclObject *x, *y, *z = PCL_NONE;

        if (!pcl_arg_unpack_tuple (args, "pow", 2, 3, &x, &y, &z))
                return NULL;
        return pcl_number_power (x, y, z);
}

static gchar builtin_pow_doc[] =
"pow(x, y,[, z]) -> number\n\
\n\
With two arguments, equivalent to x**y.  With three arguments,\n\
equivalent to (x**y) % z, but may be more efficient (e.g. for longs).";

static PclObject *
builtin_reload (PclObject *self, PclObject *module)
{
        return pcl_import_reload_module (module);
}

static gchar builtin_reload_doc[] =
"reload(module) -> module\n\
\n\
Reload the module.  The module must have been successfully imported before.";

static PclObject *
builtin_repr (PclObject *self, PclObject *object)
{
        return pcl_object_repr (object);
}

static gchar builtin_repr_doc[] =
"repr(object) -> string\n\
\n\
Return the canonical string representation of the object.\n\
For most object types, eval(repr(object)) == object.";

static PclObject *
builtin_round (PclObject *self, PclObject *args)
{
        gdouble x, f;
        gint i, ndigits = 0;

        if (!pcl_arg_parse_tuple (args, "d|i:round", &x, &ndigits))
                return NULL;

        f = 1.0;
        i = ABS (ndigits);
        while (--i >= 0)
                f *= 10.0;
        if (ndigits < 0)
                x /= f;
        else
                x *= f;
        if (x >= 0.0)
                x = floor (x + 0.5);
        else
                x = ceil (x - 0.5);
        if (ndigits < 0)
                x *= f;
        else
                x /= f;
        return pcl_float_from_double (x);
}

static gchar builtin_round_doc[] =
"round(number[, ndigits]) -> floating point number\n\
\n\
Round a number to a given percision in decimal digits (default 0 digits).\n\
This always returns a floating point number.  Precision may be negative.";

static PclObject *
builtin_setattr (PclObject *self, PclObject *args)
{
        PclObject *name;
        PclObject *object;
        PclObject *value;

        if (!pcl_arg_unpack_tuple (args, "setattr", 3, 3,
                                   &object, &name, &value))
                return NULL;

        if (!pcl_object_set_attr (object, name, value))
                return NULL;

        return pcl_object_ref (PCL_NONE);
}

static gchar builtin_setattr_doc[] =
"setattr(object, name, value)\n\
\n\
Set a named attribute on an object; setattr(x, 'y', v) is equivalent to\n\
``x.y = v''.";

static PclObject *
builtin_sum (PclObject *self, PclObject *args)
{
        PclObject *iterator;
        PclObject *next;
        PclObject *result = NULL;
        PclObject *sequence;
        PclObject *temp;

        if (!pcl_arg_unpack_tuple (args, "sum", 1, 2, &sequence, &result))
                return NULL;

        iterator = pcl_object_iterate (sequence);
        if (iterator == NULL)
                return NULL;

        if (result == NULL)
        {
                result = pcl_int_from_long (0);
                if (result == NULL)
                {
                        pcl_object_unref (iterator);
                        return NULL;
                }
        }
        else
        {
                if (PCL_IS_STRING (result))
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "sum() can't sum strings "
                                "(use ''.join(seq) instead)");
                        pcl_object_unref (iterator);
                        return NULL;
                }

                pcl_object_ref (result);
        }

        while (TRUE)
        {
                next = pcl_iterator_next (iterator);
                if (next == NULL)
                {
                        /* error, or end-of-sequence */
                        if (pcl_error_occurred ())
                        {
                                pcl_object_unref (result);
                                return NULL;
                        }
                        break;
                }

                temp = pcl_number_add (result, next);
                pcl_object_unref (result);
                pcl_object_unref (next);
                result = temp;
                if (result == NULL)
                        break;
        }

        pcl_object_unref (iterator);
        return result;
}

static gchar builtin_sum_doc[] =
"sum(sequence, start=0) -> value\n\
\n\
Returns the sum of a sequence of numbers (NOT strings) plus the value\n\
of parameter 'start'.  When the sequence is empty, returns start.";

static PclObject *
builtin_vars (PclObject *self, PclObject *args)
{
        PclObject *object = NULL;
        PclObject *vars;

        if (!pcl_arg_unpack_tuple (args, "vars", 0, 1, &object))
                return NULL;

        if (object == NULL)
        {
                vars = pcl_eval_get_locals ();
                if (vars == NULL)
                {
                        if (!pcl_error_occurred ())
                                pcl_error_set_string (
                                        pcl_exception_system_error (),
                                        "vars(): no locals!?");
                }
                else
                        pcl_object_ref (vars);
        }
        else
        {
                vars = pcl_object_get_attr_string (object, "__dict__");
                if (vars == NULL)
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "vars() argument must have "
                                "__dict__ attribute");
        }

        return vars;
}

static gchar builtin_vars_doc[] =
"vars([object]) -> dictionary\n\
\n\
Without arguments, equivalent to locals().\n\
With an argument, equivalent to object.__dict__.";

static PclMethodDef builtin_methods[] = {
        { "__import__",         (PclCFunction) builtin___import__,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin___import___doc},
        { "abs",                (PclCFunction) builtin_abs,
                                PCL_METHOD_FLAG_ONEARG,
                                builtin_abs_doc },
        { "all",                (PclCFunction) builtin_all,
                                PCL_METHOD_FLAG_ONEARG,
                                builtin_all_doc },
        { "any",                (PclCFunction) builtin_any,
                                PCL_METHOD_FLAG_ONEARG,
                                builtin_any_doc },
        { "chr",                (PclCFunction) builtin_chr,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_chr_doc },
        { "clone",              (PclCFunction) builtin_clone,
                                PCL_METHOD_FLAG_ONEARG,
                                builtin_clone_doc },
        { "cmp",                (PclCFunction) builtin_cmp,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_cmp_doc },
        { "delattr",            (PclCFunction) builtin_delattr,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_delattr_doc },
        { "dir",                (PclCFunction) builtin_dir,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_dir_doc },
        { "divmod",             (PclCFunction) builtin_divmod,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_divmod_doc },
        { "eval",               (PclCFunction) builtin_eval,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_eval_doc }, 
        { "getattr",            (PclCFunction) builtin_getattr,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_getattr_doc },
        { "globals",            (PclCFunction) builtin_globals,
                                PCL_METHOD_FLAG_NOARGS,
                                builtin_globals_doc },
        { "hasattr",            (PclCFunction) builtin_hasattr,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_hasattr_doc },
        { "hash",               (PclCFunction) builtin_hash,
                                PCL_METHOD_FLAG_ONEARG,
                                builtin_hash_doc },
        { "hex",                (PclCFunction) builtin_hex,
                                PCL_METHOD_FLAG_ONEARG,
                                builtin_hex_doc },
        { "isinstance",         (PclCFunction) builtin_isinstance,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_isinstance_doc },
        { "issubclass",         (PclCFunction) builtin_issubclass,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_issubclass_doc },
        { "iter",               (PclCFunction) builtin_iter,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_iter_doc },
        { "len",                (PclCFunction) builtin_len,
                                PCL_METHOD_FLAG_ONEARG,
                                builtin_len_doc },
        { "locals",             (PclCFunction) builtin_locals,
                                PCL_METHOD_FLAG_NOARGS,
                                builtin_locals_doc },
        { "max",                (PclCFunction) builtin_max,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_max_doc },
        { "min",                (PclCFunction) builtin_min,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_min_doc },
        { "oct",                (PclCFunction) builtin_oct,
                                PCL_METHOD_FLAG_ONEARG,
                                builtin_oct_doc },
        { "ord",                (PclCFunction) builtin_ord,
                                PCL_METHOD_FLAG_ONEARG,
                                builtin_ord_doc },
        { "pow",                (PclCFunction) builtin_pow,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_pow_doc },
        { "reload",             (PclCFunction) builtin_reload,
                                PCL_METHOD_FLAG_ONEARG,
                                builtin_reload_doc },
        { "repr",               (PclCFunction) builtin_repr,
                                PCL_METHOD_FLAG_ONEARG,
                                builtin_repr_doc },
        { "round",              (PclCFunction) builtin_round,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_round_doc },
        { "setattr",            (PclCFunction) builtin_setattr,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_setattr_doc },
        { "sum",                (PclCFunction) builtin_sum,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_sum_doc },
        { "vars",               (PclCFunction) builtin_vars,
                                PCL_METHOD_FLAG_VARARGS,
                                builtin_vars_doc },
        { NULL }
};

PclObject *
_pcl_builtin_init (void)
{
        PclObject *debug;
        PclObject *dict;
        PclObject *module;

        module = pcl_module_init ("__builtin__", builtin_methods, builtin_doc);
        if (module == NULL)
                return NULL;
        dict = pcl_module_get_dict (module);

#define SETBUILTIN(name, object) \
        if (!pcl_dict_set_item_string (dict, name, object)) \
                return NULL;

        /* singleton objects */
	SETBUILTIN ("Free", PCL_FREE);
        SETBUILTIN ("None", PCL_NONE);
        SETBUILTIN ("True", PCL_TRUE);
        SETBUILTIN ("False", PCL_FALSE);
        SETBUILTIN ("Ellipsis", PCL_ELLIPSIS);
        SETBUILTIN ("NotImplemented", PCL_NOT_IMPLEMENTED);

        /* callable type objects */
        SETBUILTIN ("bool",         pcl_bool_get_type_object ());
        SETBUILTIN ("classmethod",  pcl_class_method_get_type_object ());
        SETBUILTIN ("complex",      pcl_complex_get_type_object ());
        SETBUILTIN ("dict",         pcl_dict_get_type_object ());
        SETBUILTIN ("enumerate",    pcl_enumerate_get_type_object ());
        SETBUILTIN ("file",         pcl_file_get_type_object ());
        SETBUILTIN ("float",        pcl_float_get_type_object ());
        SETBUILTIN ("frozenset",    pcl_frozen_set_get_type_object ());
        SETBUILTIN ("int",          pcl_int_get_type_object ());
        SETBUILTIN ("list",         pcl_list_get_type_object ());
        SETBUILTIN ("object",       pcl_object_get_type_object ());
        SETBUILTIN ("open",         pcl_file_get_type_object ());
        SETBUILTIN ("property",     pcl_property_get_type_object ());
        SETBUILTIN ("range",        pcl_range_get_type_object ());
        SETBUILTIN ("reversed",     pcl_reversed_get_type_object ());
        SETBUILTIN ("set",          pcl_set_get_type_object ());
        SETBUILTIN ("slice",        pcl_slice_get_type_object ());
        SETBUILTIN ("staticmethod", pcl_static_method_get_type_object ());
        SETBUILTIN ("str",          pcl_string_get_type_object ());
        SETBUILTIN ("super",        pcl_super_get_type_object ());
        SETBUILTIN ("table",        pcl_table_get_type_object ());
        SETBUILTIN ("tuple",        pcl_tuple_get_type_object ());
        SETBUILTIN ("type",         pcl_type_get_type_object ());
        SETBUILTIN ("zip",          pcl_zip_get_type_object ());

        debug = pcl_bool_from_boolean (TRUE);
        if (!pcl_dict_set_item_string (dict, "__debug__", debug))
        {
                pcl_object_unref (debug);
                return NULL;
        }
        pcl_object_unref (debug);

#undef SETBUILTIN

        return module;
}
