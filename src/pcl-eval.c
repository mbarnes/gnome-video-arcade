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
#include "pcl-opcode.h"

/* Debugging Facilities */
#ifdef G_ENABLE_DEBUG
#define CHECK_EXC 1	/* Double-check exception checking */
#endif

/* Dynamic execution profile */
#ifdef DYNAMIC_EXECUTION_PROFILE
#ifdef DXPAIRS
static glong dxpairs[257][256];
#define dxp dxpairs[256]
#else
static glong dxp[256];
#endif
#endif

/* Error Messages */
#define NAME_ERROR_MSG \
        "name '%s' is not defined"
#define GLOBAL_NAME_ERROR_MSG \
        "global name '%s' is not defined"
#define NOARGS_ERROR_MSG \
        "%s() takes no arguments (%d given)"
#define ONEARG_ERROR_MSG \
        "%s() takes exactly one argument (%d given)"
#define UNBOUND_LOCAL_ERROR_MSG \
        "local variable '%s' referenced before assignment"
#define UNBOUND_FREE_ERROR_MSG \
        "free variable '%s' referenced before assignment " \
        "in enclosing scope"

/* For popping the stack outside of the main loop */
#define STACK_EXT_POP(stack_pointer) (*--(stack_pointer))

/* Asynchronous event counters */
gint _pcl_check_interval = 100;
volatile gint _pcl_ticker = 100;

/* Mechanism whereby asynchronously executing callbacks (e.g. UNIX signal
 * handlers or Mac I/O completion routines) can schedule calls to a function
 * to be called synchronously.  The synchronous function is called with one
 * gpointer argument.  It should return TRUE for success and FALSE for failure
 * (failure should be accompanied by an exception).
 *
 * If registration succeeds, the registry function returns TRUE; if it fails
 * (e.g. due to too many pending calls) it returns FALSE (without setting an
 * exception condition).
 *
 * Note that because registry may occur from within signal handlers or other
 * asynchronous events, calling malloc() is unsafe!
 */

#define MAX_PENDING_CALLS 32

static struct {
        gboolean (*func) (gpointer);
        gpointer arg;
} pending_calls[MAX_PENDING_CALLS];

static volatile gint pending_first = 0;
static volatile gint pending_last = 0;
static volatile gboolean things_to_do = FALSE;

gboolean
pcl_add_pending_call (gboolean (*func) (gpointer), gpointer arg)
{
        static volatile gboolean busy = FALSE;
        gint next;

        if (busy)
                return FALSE;
        busy = TRUE;
        next = (pending_last + 1) % MAX_PENDING_CALLS;
        if (next == pending_first)
        {
                busy = FALSE;
                return FALSE;  /* queue full */
        }
        pending_calls[pending_last].func = func;
        pending_calls[pending_last].arg = arg;
        pending_last = next;

        _pcl_ticker = 0;
        things_to_do = TRUE;  /* signal main loop */
        busy = FALSE;
        return TRUE;
}

gboolean
pcl_make_pending_calls (void)
{
        static gboolean busy = FALSE;

        /* XXX Skipping main thread check. */
        if (busy)
                return FALSE;
        busy = TRUE;
        things_to_do = FALSE;
        while (TRUE)
        {
                gint next = pending_first;
                if (next == pending_last)
                        break;  /* queue empty */
                pending_first = (next + 1) % MAX_PENDING_CALLS;
                if (!pending_calls[next].func (pending_calls[next].arg))
                {
                        busy = FALSE;
                        things_to_do = 1;  /* we're not done yet */
                        return FALSE;
                }
        }
        busy = FALSE;
        return TRUE;
}

/* The interpreter's recursion limit */

static gint recursion_limit = 1000;

gint
pcl_get_recursion_limit (void)
{
        return recursion_limit;
}

void
pcl_set_recursion_limit (gint limit)
{
        recursion_limit = limit;
}

gboolean
pcl_enter_recursive_call (void)
{
        PclThreadState *ts = pcl_thread_state_get ();
        if (++ts->recursion_depth > recursion_limit)
        {
                --ts->recursion_depth;
                pcl_error_set_string (
                        pcl_exception_runtime_error (),
                        "maximum recursion depth exceeded");
                return FALSE;
        }
        return TRUE;
}

void
pcl_leave_recursive_call (void)
{
        PclThreadState *ts = pcl_thread_state_get ();
        g_assert (ts->recursion_depth > 0);
        --ts->recursion_depth;
}

/* Status code for main loop (reason for stack unwind) */
enum WhyCode {
        WHY_NOT       = 1 << 0,  /* no error */
        WHY_BREAK     = 1 << 1,  /* 'break' statement */
        WHY_CONTINUE  = 1 << 2,  /* 'continue' statement */
        WHY_EXCEPTION = 1 << 3,  /* exception occurred */
        WHY_RERAISE   = 1 << 4,  /* exception re-raised by 'finally' */
        WHY_RETURN    = 1 << 5,  /* 'return' statement */
        WHY_YIELD     = 1 << 6   /* 'yield' statement */
};

static gboolean
eval_update_keyword_args (PclObject *kw_args, gint nka,
                          PclObject ***pp_stack, PclObject *func)
{
        while (--nka >= 0)
        {
                gboolean success;
                PclObject *value = STACK_EXT_POP (*pp_stack);
                PclObject *key = STACK_EXT_POP (*pp_stack);
                if (pcl_dict_get_item (kw_args, key) != NULL)
                {
                        pcl_error_set_format (
                                pcl_exception_type_error (),
                                "%s%s got multiple values "
                                "for keyword argument '%s'",
                                pcl_eval_get_func_name (func),
                                pcl_eval_get_func_desc (func),
                                pcl_string_as_string (key));
                        pcl_object_unref (key);
                        pcl_object_unref (value);
                        pcl_object_unref (kw_args);
                        return FALSE;
                }
                success = pcl_dict_set_item (kw_args, key, value);
                pcl_object_unref (key);
                pcl_object_unref (value);
                if (!success)
                {
                        pcl_object_unref (kw_args);
                        return FALSE;
                }
        }

        return TRUE;
}

static PclObject *
eval_update_positional_args (gint npa, PclObject *ep_args,
                             PclObject ***pp_stack)
{
        glong ii, ep_length = 0;
        PclObject *tmp, *result;

        if (ep_args != NULL)
                ep_length = pcl_object_measure (ep_args);
        result = pcl_tuple_new (npa + ep_length);
        if (result == NULL)
                return NULL;
        for (ii = 0; ii < ep_length; ii++)
        {
                tmp = pcl_sequence_get_item (ep_args, ii);
                pcl_tuple_set_item (result, npa + ii, tmp);
        }
        while (--npa >= 0)
                pcl_tuple_set_item (result, npa, STACK_EXT_POP (*pp_stack));
        return result;
}

static PclObject *
eval_load_args (PclObject ***pp_stack, gint npa)
{
        PclObject *result = pcl_tuple_new (npa);

        if (result == NULL)
                return NULL;
        while (--npa >= 0)
                pcl_tuple_set_item (result, npa, STACK_EXT_POP (*pp_stack));
        return result;
}

static PclObject *
eval_fast_function (PclObject *function, PclObject ***pp_stack,
                    gint na, gint npa, gint nka)
{
        /* Optimize calls for which no argument tuple is necessary; the
         * objects are passed directly from the stack.  For the simplest
         * case -- a function that takes only positional arguments and is
         * called with only positional arguments -- it inlines the most
         * primitive frame setup code from pcl_eval_code_ex(), which vastly
         * reduces the checks that must be done before evaluating the frame. */

        PclCode *code = PCL_FUNCTION_GET_CODE (function);
        PclObject *globals = PCL_FUNCTION_GET_GLOBALS (function);
        PclObject *defaults = PCL_FUNCTION_GET_DEFAULTS (function);
        PclObject **pda = NULL;
        glong nda = 0;

        if (defaults == NULL && code->argument_count == na && nka == 0 &&
                code->flags == (PCL_CODE_FLAG_NEWLOCALS | PCL_CODE_FLAG_NOFREE))
        {
                PclThreadState *ts = pcl_thread_state_get ();
                PclFrame *frame;
                PclObject **slots;
                PclObject **stack;
                PclObject *retval = NULL;
                gint ii;

                if (globals == NULL)
                {
                        pcl_error_set_format (
                                pcl_exception_system_error (),
                                "%s: NULL globals", G_STRFUNC);
                        return NULL;
                }

                frame = pcl_frame_new (ts, code, globals, NULL);
                if (frame == NULL)
                        return NULL;

                slots = frame->slots;
                stack = (*pp_stack) - na;

                for (ii = 0; ii < na; ii++)
                        slots[ii] = pcl_object_ref (*stack++);
                retval = pcl_eval_frame (frame);
                g_assert (ts != NULL);
                ++ts->recursion_depth;
                pcl_object_unref (frame);
                --ts->recursion_depth;
                return retval;
        }
        if (defaults != NULL)
        {
                pda = (PclObject **) &(PCL_TUPLE_GET_ITEM (defaults, 0));
                nda = PCL_TUPLE_GET_SIZE (defaults);
        }
        return pcl_eval_code_ex (code, globals, NULL, (*pp_stack) - na, npa,
                                 (*pp_stack) - 2 * nka, nka, pda, nda,
                                 PCL_FUNCTION_GET_CLOSURE (function));
}

static PclObject *
eval_call_function (PclObject ***pp_stack, gint oparg)
{
        gint npa = oparg & 0xFF;
        gint nka = (oparg >> 8) & 0xFF;
        gint size = npa + 2 * nka;
        PclObject **pfunc = (*pp_stack) - size - 1;
        PclObject *func = *pfunc;
        PclObject *fn_args = NULL;
        PclObject *kw_args = NULL;
        PclObject *result = NULL;

        /* Always dispatch PclMethod first, because these are presumed to be
         * the most frequent callable object. */
        if (PCL_IS_METHOD (func) && nka == 0)
        {
                gint flags = PCL_METHOD_GET_FLAGS (func);

                if (flags & (PCL_METHOD_FLAG_NOARGS | PCL_METHOD_FLAG_ONEARG))
                {
                        PclCFunction cf = PCL_METHOD_GET_FUNCTION (func);
                        PclObject *self = PCL_METHOD_GET_SELF (func);
                        if (flags & PCL_METHOD_FLAG_NOARGS && npa == 0)
                                result = (*cf) (self, NULL);
                        else if (flags & PCL_METHOD_FLAG_ONEARG && npa == 1)
                        {
                                PclObject *arg = STACK_EXT_POP (*pp_stack);
                                result = (*cf) (self, arg);
                                pcl_object_unref (arg);
                        }
                        else
                                pcl_error_set_format (
                                        pcl_exception_type_error (),
                                        (flags & PCL_METHOD_FLAG_NOARGS) ?
                                        NOARGS_ERROR_MSG : ONEARG_ERROR_MSG,
                                        PCL_METHOD_GET_NAME (func), npa);
                }
                else
                {
                        PclObject *args = eval_load_args (pp_stack, npa);
                        result = pcl_object_call (func, args, NULL);
                        if (args != NULL)
                                pcl_object_unref (args);
                }
        }
        else
        {
                /* XXX Skip bound method object check */
                pcl_object_ref (func);
                if (PCL_IS_FUNCTION (func))
                        result = eval_fast_function (
                                        func, pp_stack, size, npa, nka);
                else
                {
                        if (nka > 0)
                        {
                                kw_args = pcl_dict_new ();
                                if (kw_args == NULL)
                                        goto error;
                                if (!eval_update_keyword_args (
                                                kw_args, nka, pp_stack, func))
                                        goto error;
                        }
                        fn_args = eval_load_args (pp_stack, npa);
                        if (fn_args == NULL)
                                goto error;
                        result = pcl_object_call (func, fn_args, kw_args);
                }
                pcl_object_unref (func);
        }

        /* Pop the function object and any leftovers. */
        while ((*pp_stack) > pfunc)
                pcl_object_unref (STACK_EXT_POP (*pp_stack));

error:

        if (fn_args != NULL)
                pcl_object_unref (fn_args);
        if (kw_args != NULL)
                pcl_object_unref (kw_args);

        return result;
}

static PclObject *
eval_ext_call_function (PclObject *func, PclObject ***pp_stack,
                        gint npa, gint nka, gboolean epa, gboolean eka)
{
        PclObject *fn_args = NULL;
        PclObject *kw_args = NULL;
        PclObject *ep_args = NULL;
        PclObject *result = NULL;

        if (eka)
        {
                kw_args = STACK_EXT_POP (*pp_stack);
                if (!PCL_IS_DICT (kw_args))
                {
                        pcl_error_set_format (
                                pcl_exception_type_error (),
                                "%s%s argument after ** must be a dictionary",
                                pcl_eval_get_func_name (func),
                                pcl_eval_get_func_desc (func));
                        goto error;
                }
        }

        if (epa)
        {
                ep_args = STACK_EXT_POP (*pp_stack);
                if (!PCL_IS_SEQUENCE (ep_args))
                {
                        pcl_error_set_format (
                                pcl_exception_type_error (),
                                "%s%s argument after * must be a sequence",
                                pcl_eval_get_func_name (func),
                                pcl_eval_get_func_desc (func));
                        goto error;
                }
        }

        if (nka > 0)
        {
                if (kw_args == NULL)
                        kw_args = pcl_dict_new ();
                else
                {
                        PclObject *tmp = kw_args;
                        kw_args = pcl_object_copy (kw_args);
                        pcl_object_unref (tmp);
                }
                if (kw_args == NULL)
                        goto error;
                if (!eval_update_keyword_args (kw_args, nka, pp_stack, func))
                        goto error;
        }
        fn_args = eval_update_positional_args (npa, ep_args, pp_stack);
        if (fn_args == NULL)
                goto error;
        result = pcl_object_call (func, fn_args, kw_args);

error:
        if (fn_args != NULL)
                pcl_object_unref (fn_args);
        if (kw_args != NULL)
                pcl_object_unref (kw_args);
        if (ep_args != NULL)
                pcl_object_unref (ep_args);
        return result;
}

static PclObject *
eval_build_class (PclObject *methods, PclObject *bases, PclObject *name)
{
        PclObject *class = NULL;
        PclObject *result;

        /* Make sure bases is a non-empty tuple. */
        g_assert (PCL_IS_TUPLE (bases) && PCL_TUPLE_GET_SIZE (bases) > 0);

        if (PCL_IS_DICT (methods))
                class = pcl_dict_get_item_string (methods, "__metaclass__");
        if (class != NULL)
                pcl_object_ref (class);
        else
        {
                PclObject *base = PCL_TUPLE_GET_ITEM (bases, 0);
                class = pcl_object_get_attr_string (base, "__class__");
                if (class == NULL)
                {
                        pcl_error_clear ();
                        class = pcl_object_type (base);
                }
        }
        result = pcl_object_call_function (
                        class, "OOO", name, bases, methods);
        pcl_object_unref (class);
        /* XXX Python checks for a TypeError here and adds some helpful text
         *     to the error message. */
        return result;
}

static gboolean
eval_exec (PclFrame *frame, PclObject *program,
           PclObject *globals, PclObject *locals)
{
        gboolean plain = FALSE;
        PclObject *result;

        if (globals == PCL_NONE)
        {
                globals = pcl_eval_get_globals ();
                if (locals == PCL_NONE)
                {
                        locals = pcl_eval_get_locals ();
                        plain = TRUE;
                }
        }
        else if (locals == PCL_NONE)
                locals = globals;

        if (!PCL_IS_STRING (program) &&
                !PCL_IS_CODE (program) && !PCL_IS_FILE (program))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "exec: arg 1 must be a string, file, or code object");
                return FALSE;
        }

        if (!PCL_IS_DICT (globals))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "exec: arg 2 must be a dictionary or None");
                return FALSE;
        }

        if (!PCL_IS_MAPPING (locals))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "exec: arg 3 must be a mapping or None");
                return FALSE;
        }

        if (pcl_dict_get_item_string (globals, "__builtins__") == NULL)
                pcl_dict_set_item_string (
                        globals, "__builtins__", frame->builtins);

        if (PCL_IS_CODE (program))
        {
                if (PCL_CODE_GET_NUM_FREE (program) > 0)
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "code object passed to exec may not "
                                "contain free variables");
                        return FALSE;
                }
                result = pcl_eval_code (PCL_CODE (program), globals, locals);
        }
        else if (PCL_IS_FILE (program))
        {
                result = pcl_run_file (
                         pcl_file_as_file (program),
                         pcl_string_as_string (pcl_file_name (program)),
                         PCL_FILE_INPUT, globals, locals);
        }
        else  /* it's a string */
        {
                /* XXX What about embedded nul characters? */
                g_assert (PCL_IS_STRING (program));
                result = pcl_run_string (
                         pcl_string_as_string (program),
                         PCL_FILE_INPUT, globals, locals);
        }

        if (plain)
                pcl_frame_locals_to_fast (frame, FALSE);
        if (result == NULL)
                return FALSE;
        pcl_object_unref (result);
        return TRUE;
}

static void
eval_format_exc_check_arg (PclObject *exception, const gchar *format,
                           PclObject *object)
{
        gchar *string;

        if (object == NULL)
                return;

        if ((string = pcl_string_as_string (object)) == NULL)
                return;

        pcl_error_set_format (exception, format, string);
}

static gboolean
eval_import_all_from (PclObject *locals, PclObject *module)
{
        gboolean skip_leading_underscores = FALSE;
        PclObject *iterator;
        PclObject *next;
        PclObject *object;

        object = pcl_object_get_attr_string (module, "__all__");
        if (object == NULL)
        {
                if (!pcl_error_exception_matches (
                                pcl_exception_attribute_error ()))
                        return FALSE;  /* unexpected error */
                pcl_error_clear ();
                object = pcl_object_get_attr_string (module, "__dict__");
                if (object == NULL)
                {
                        if (!pcl_error_exception_matches (
                                        pcl_exception_attribute_error ()))
                                return FALSE;  /* unexpected error */
                        pcl_error_set_string (
                                pcl_exception_import_error (),
                                "from-import-* object has no __dict__ "
                                "and no __all__");
                        return FALSE;
                }
                skip_leading_underscores = TRUE;
        }

        iterator = pcl_object_iterate (object);
        pcl_object_unref (object);
        if (iterator == NULL)
                return FALSE;

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                if (skip_leading_underscores && PCL_IS_STRING (next))
                {
                        if (PCL_STRING_AS_STRING (next)[0] == '_')
                        {
                                pcl_object_unref (next);
                                continue;
                        }
                }

                object = pcl_object_get_attr (module, next);
                if (object == NULL)
                {
                        pcl_object_unref (next);
                        break;
                }
                if (!pcl_dict_set_item (locals, next, object))
                {
                        pcl_object_unref (object);
                        pcl_object_unref (next);
                        break;
                }
                pcl_object_unref (object);
                pcl_object_unref (next);
        }

        pcl_object_unref (iterator);
        if (pcl_error_occurred ())
                return FALSE;
        return TRUE;
}

static PclObject *
eval_import_from (PclObject *module, PclObject *name)
{
        PclObject *object;

        object = pcl_object_get_attr (module, name);
        if (object == NULL && pcl_error_exception_matches (
                pcl_exception_attribute_error ()))
        {
                pcl_error_set_format (
                        pcl_exception_import_error (),
                        "cannot import name %s",
                        pcl_string_as_string (name));
                return NULL;
        }
        return object;
}

static void
eval_set_exc_info (PclThreadState *ts,
                   PclObject *type, PclObject *value, PclObject *traceback)
{
        PclFrame *frame = ts->frame;
        PclObject *tmp_type, *tmp_value, *tmp_traceback;

        if (frame->exc_type == NULL)
        {
                /* This frame didn't catch an exception before.  Save the
                 * previous exception of this thread in this frame. */
                if (ts->exc_type == NULL)
                        ts->exc_type = pcl_object_ref (PCL_NONE);
                tmp_type = frame->exc_type;
                tmp_value = frame->exc_value;
                tmp_traceback = frame->exc_traceback;
                if (ts->exc_type != NULL)
                        pcl_object_ref (ts->exc_type);
                if (ts->exc_value != NULL)
                        pcl_object_ref (ts->exc_value);
                if (ts->exc_traceback != NULL)
                        pcl_object_ref (ts->exc_traceback);
                frame->exc_type = ts->exc_type;
                frame->exc_value = ts->exc_value;
                frame->exc_traceback = ts->exc_traceback;
                if (tmp_type != NULL)
                        pcl_object_unref (tmp_type);
                if (tmp_value != NULL)
                        pcl_object_unref (tmp_value);
                if (tmp_traceback != NULL)
                        pcl_object_unref (tmp_traceback);
        }

        /* Set new exception for this thread. */
        tmp_type = ts->exc_type;
        tmp_value = ts->exc_value;
        tmp_traceback = ts->exc_traceback;
        if (type != NULL)
                pcl_object_ref (type);
        if (value != NULL)
                pcl_object_ref (value);
        if (value != NULL)
                pcl_object_ref (traceback);
        ts->exc_type = type;
        ts->exc_value = value;
        ts->exc_traceback = traceback;
        if (tmp_type != NULL)
                pcl_object_unref (tmp_type);
        if (tmp_value != NULL)
                pcl_object_unref (tmp_value);
        if (tmp_traceback != NULL)
                pcl_object_unref (tmp_traceback);
}

static void
eval_reset_exc_info (PclThreadState *ts)
{
        PclFrame *frame = ts->frame;
        PclObject *tmp_type, *tmp_value, *tmp_traceback;

        if (frame->exc_type != NULL)
        {
                /* This frame caught an exception. */
                tmp_type = ts->exc_type;
                tmp_value = ts->exc_value;
                tmp_traceback = ts->exc_traceback;
                if (frame->exc_type != NULL)
                        pcl_object_ref (frame->exc_type);
                if (frame->exc_value != NULL)
                        pcl_object_ref (frame->exc_value);
                if (frame->exc_traceback != NULL)
                        pcl_object_ref (frame->exc_traceback);
                ts->exc_type = frame->exc_type;
                ts->exc_value = frame->exc_value;
                ts->exc_traceback = frame->exc_traceback;
                if (tmp_type != NULL)
                        pcl_object_unref (tmp_type);
                if (tmp_value != NULL)
                        pcl_object_unref (tmp_value);
                if (tmp_traceback != NULL)
                        pcl_object_unref (tmp_traceback);
        }

        tmp_type = frame->exc_type;
        tmp_value = frame->exc_value;
        tmp_traceback = frame->exc_traceback;
        frame->exc_type = NULL;
        frame->exc_value = NULL;
        frame->exc_traceback = NULL;
        if (tmp_type != NULL)
                pcl_object_unref (tmp_type);
        if (tmp_value != NULL)
                pcl_object_unref (tmp_value);
        if (tmp_traceback != NULL)
                pcl_object_unref (tmp_traceback);
}

static enum WhyCode
eval_raise (PclObject *type, PclObject *value, PclObject *traceback)
{
        /* NOTE: This steals a reference to each of its arguments. */

        if (type == NULL)
        {
                /* reraise */
                PclThreadState *ts = pcl_thread_state_get ();
                if (ts->exc_type != NULL)
                        type = ts->exc_type;
                else
                        type = PCL_NONE;
                value = ts->exc_value;
                traceback = ts->exc_traceback;
                if (type != NULL)
                        pcl_object_ref (type);
                if (value != NULL)
                        pcl_object_ref (value);
                if (traceback != NULL)
                        pcl_object_ref (traceback);
        } 

        if (traceback == PCL_NONE)
        {
                pcl_object_unref (traceback);
                traceback = NULL;
        }
        else if (!PCL_IS_TRACEBACK (traceback))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "raise: arg 3 must be a traceback or None");
                goto raise_error;
        }

        if (value == NULL)
                value = pcl_object_ref (PCL_NONE);

        while (PCL_IS_TUPLE (type) && PCL_TUPLE_GET_SIZE (type) > 0)
        {
                PclObject *tmp = type;
                type = PCL_TUPLE_GET_ITEM (type, 0);
                pcl_object_ref (type);
                pcl_object_unref (tmp);
        }

        /* XXX Only allowing PclException instances for now. */
        if (!PCL_IS_EXCEPTION (type))
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "for now, must raise exception instances, not %s",
                        PCL_GET_TYPE_NAME (type));
                goto raise_error;
        }

        pcl_error_restore (type, value, traceback);

        return (traceback == NULL) ? WHY_EXCEPTION : WHY_RERAISE;

raise_error:

        if (type != NULL)
                pcl_object_unref (type);
        if (value != NULL)
                pcl_object_unref (value);
        if (traceback != NULL)
                pcl_object_unref (traceback);
        return WHY_EXCEPTION;
}

static gboolean
eval_unpack_iterable (PclObject *iterable, guint nargs, PclObject **sp)
{
        PclObject *iterator;
        PclObject *next;
        guint ii;

        if ((iterator = pcl_object_iterate (iterable)) == NULL)
                return FALSE;

        for (ii = 0; ii < nargs; ++ii)
        {
                next = pcl_iterator_next (iterator);
                if (next == NULL)
                {
                        if (!pcl_error_occurred ())
                                pcl_error_set_format (
                                        pcl_exception_value_error (),
                                        "need more than %d value%s to unpack",
                                        ii, (ii == 1) ? "" : "s");
                        goto error;
                }
                *--sp = next;
        }

        /* We better have exhausted the iterator now. */
        next = pcl_iterator_next (iterator);
        if (next == NULL)
        {
                if (pcl_error_occurred ())
                        goto error;
                pcl_object_unref (iterator);
                return TRUE;
        }
        pcl_object_unref (next);
        pcl_error_set_string (
                pcl_exception_value_error (),
                "too many values to unpack");
        /* fall through */

error:

        while (ii > 0)
        {
                pcl_object_unref (*sp++);
                ii--;
        }
        pcl_object_unref (iterator);
        return FALSE;
}

static gboolean
eval_call_trace (PclTraceFunc func, PclObject *object, PclFrame *frame,
                 PclTraceType type, PclObject *arg)
{
       PclThreadState *ts = frame->ts;
       gboolean status = TRUE;
       if (ts->tracing == 0)
       {
               ts->tracing++;
               ts->use_tracing = 0;
               status = func (object, frame, type, arg);
               ts->use_tracing = (ts->trace_function != NULL) ||
                                 (ts->profile_function != NULL);
               ts->tracing--;
       }
       return status;
}

static void
eval_call_exc_trace (PclTraceFunc func, PclObject *object, PclFrame *frame)
{
        PclObject *type, *value, *traceback, *arg;
        pcl_error_fetch (&type, &value, &traceback);
        if (value == NULL)
                value = pcl_object_ref (PCL_NONE);
        arg = pcl_tuple_pack (3, type, value, traceback);
        if (arg == NULL)
        {
                pcl_error_restore (type, value, traceback);
                return;
        }
        if (eval_call_trace (func, object, frame, PCL_TRACE_EXCEPTION, arg))
                pcl_error_restore (type, value, traceback);
        else
        {
                if (type != NULL)
                        pcl_object_unref (type);
                if (value != NULL)
                        pcl_object_unref (value);
                if (traceback != NULL)
                        pcl_object_unref (traceback);
        }
        pcl_object_unref (arg);
}

PclObject *
pcl_eval_frame (PclFrame *frame)
{
        PclThreadState *ts = pcl_thread_state_get ();

        PclObject **stack_pointer;
        guchar *next_instruction;
        gint opcode;
        gint oparg;
        enum WhyCode why;
        gboolean success;
        PclObject *x;
        PclObject *v;
        PclObject *w;
        PclObject *u;
        PclObject **slots;
        PclObject **free_slots;
        PclObject *retval = NULL;
        PclObject *stream = NULL;
        guchar *first_instruction;
        guint variable_count;
        gulong stack_size;
        PclObject *instructions;
        PclObject *constants, *names;
        PclObject *builtins, *globals, *locals;
        PclObject **stack;
        PclCode *code;

#ifdef DXPAIRS
        gint last_opcode = 0;
#endif

/* Tuple Access Macros */
#ifndef G_ENABLE_DEBUG
#define GETITEM(tuple, index) \
        (PCL_TUPLE_GET_ITEM ((tuple), (index)))
#else
#define GETITEM(tuple, index) \
        (pcl_tuple_get_item ((tuple), (index)))
#endif

/* Code Access Macros */
#define IPADDR()  (next_instruction - first_instruction)
#define NEXTOP()  (*next_instruction++)
#define NEXTARG() (next_instruction += 2, \
                  (next_instruction[-1] << 8) + next_instruction[-2])
#define PEEKARG() ((next_instruction[2] << 8) + next_instruction[1])
#define JUMPTO(x) (next_instruction = first_instruction + (x))
#define JUMPBY(x) (next_instruction += (x))

/* Opcode Prediction Macros
 *
 * Some opcodes tend to come in pairs, thus making it possible to predict
 * the second code when the first is run.  For example, comparison operations
 * are often followed by JUMP_IF_FALSE or JUMP_IF_TRUE.  And, those opcodes
 * are often followed by a POP_TOP.
 *
 * Verifying the prediction costs a single high-speed test of register
 * variable against a constant.  If the pairing was good, then the
 * processor has a high likelihood of making its own successful branch
 * prediction which results in a nearly zero overhead transition to the
 * next opcode.
 *
 * A successful prediction saves a trip through the eval-loop including
 * its two unpredictable branches, the HAS_ARG test and the switch-case.
 *
 * If collecting opcode statistics, turn off prediction so that statistics
 * are accurately maintained (the predictions bypass the opcode frequency
 * counter updates).
 */

#ifdef DYNAMIC_EXECUTION_PROFILE
#define PREDICT(op)             if (FALSE) goto PREDICT_##op
#else
#define PREDICT(op)             if (*next_instruction == PCL_OPCODE_##op) \
                                        goto PREDICT_##op
#endif

#define PREDICTED(op)           PREDICT_##op: next_instruction++
#define PREDICTED_WITH_ARG(op)  PREDICT_##op: oparg = PEEKARG(); \
                                              next_instruction += 3

/* Stack Manipulation Macros */
#define STACK_LEVEL()       (stack_pointer - stack)
#define STACK_EMPTY()       (STACK_LEVEL () == 0)
#define STACK_TOP()         (stack_pointer[-1])
#define STACK_SECOND()      (stack_pointer[-2])
#define STACK_THIRD()       (stack_pointer[-3])
#define STACK_FOURTH()      (stack_pointer[-4])
#define STACK_SET_TOP(v)    (stack_pointer[-1] = (v))
#define STACK_SET_SECOND(v) (stack_pointer[-2] = (v))
#define STACK_SET_THIRD(v)  (stack_pointer[-3] = (v))
#define STACK_SET_FOURTH(v) (stack_pointer[-4] = (v))
#define STACK_ADJUST(n)     (stack_pointer += (n))
#define STACK_PUSH(v)       (*stack_pointer++ = (v))
#define STACK_POP()         (*--stack_pointer)

/* Type Check Macros */
#define IS_INT(obj)     (G_OBJECT_TYPE (obj) == PCL_TYPE_INT)
#define IS_DICT(obj)    (G_OBJECT_TYPE (obj) == PCL_TYPE_DICT)
#define IS_LIST(obj)    (G_OBJECT_TYPE (obj) == PCL_TYPE_LIST)
#define IS_TUPLE(obj)   (G_OBJECT_TYPE (obj) == PCL_TYPE_TUPLE)

        if (frame == NULL)
                return NULL;

        if (!pcl_enter_recursive_call ())
                return NULL;

        ts->frame = frame;

        code = frame->code;
        builtins = frame->builtins;
        globals = frame->globals;
        locals = frame->locals;
        stack = frame->stack;
        slots = frame->slots;

        variable_count = code->variable_count;
        stack_size = code->stack_size;
        instructions = code->instructions;
        constants = code->constants;
        names = code->names;

        first_instruction = (guchar *) pcl_string_as_string (instructions);
        next_instruction = first_instruction + frame->lasti + 1;
        free_slots = &slots[variable_count];
        stack_pointer = frame->tos;
        g_assert (stack_pointer != NULL);
        frame->tos = NULL;  /* remains NULL unless yield suspends frame */

        why = WHY_NOT;
        success = TRUE;
        x = PCL_NONE;  /* need something non-NULL */
        w = NULL;

        while (TRUE)
        {
                if (--_pcl_ticker < 0)
                {
                        if (*next_instruction == PCL_OPCODE_SETUP_FINALLY)
                        {
                                /* Make the last opcode before a try/finally
                                 * block uninterruptable. */
                                goto fast_next_opcode;
                        }
                        _pcl_ticker = _pcl_check_interval;

                        if (things_to_do)
                        {
                                if (!pcl_make_pending_calls ())
                                {
                                        why = WHY_EXCEPTION;
                                        goto on_error;
                                }
                                if (things_to_do)
                                {
                                        /* pcl_make_pending_calls() didn't
                                         * succeed.  Force early re-execution
                                         * of this "periodic" code, possibly
                                         * after a thread switch. */
                                        _pcl_ticker = 0;
                                }
                        }
                }

fast_next_opcode:

                frame->lasti = IPADDR ();

                opcode = NEXTOP ();
                oparg = PCL_OPCODE_HAS_ARG (opcode) ? NEXTARG () : -1;

#ifdef DYNAMIC_EXECUTION_PROFILE
#ifdef DXPAIRS
                dxpairs[last_opcode][opcode]++;
                last_opcode = opcode;
#endif
                dxp[opcode]++;
#endif

#if 0
                {
                        PclObject **p = stack;
                        PclObject *repr;
                        guint ii, slot = 0;
                        g_print ("\n---------------------------------------"
                                 "---------------------------------------\n");
                        g_print ("Context: ");
                        pcl_object_print (code->name, stdout, 0);
                        g_print (" (frame %p)\n", (gpointer) frame);
                        g_print ("Stack |%lu|: [", stack_size);
                        while (p != stack_pointer)
                        {
                                repr = pcl_object_repr (*p);
                                pcl_object_print (repr, stdout, 0);
                                pcl_object_unref (repr);
                                /*g_print ("|%d|", G_OBJECT(*p)->ref_count);*/
                                if (++p != stack_pointer)
                                        g_print (", ");
                        }
                        g_print ("]\n");

                        /* make sure we didn't overflow the stack */
                        g_assert (STACK_LEVEL () <= stack_size);

                        g_print ("Opcode: x%x", opcode);
                        if (oparg >= 0)
                                g_print (" (arg=%d)", oparg);
                        g_print ("\n");
#if 0
                        g_print ("Frame state follows:\n");
                        g_print ("Constants @ %p |%d|: ",
                                (gpointer) constants,
                                G_OBJECT (constants)->ref_count);
                        pcl_object_print (constants, stdout, 0);
                        g_print ("\n");
                        g_print ("Names @ %p |%d|: ",
                                (gpointer) names,
                                G_OBJECT (names)->ref_count);
                        pcl_object_print (names, stdout, 0);
                        g_print ("\n");
                        g_print ("Globals @ %p |%d|: ",
                                (gpointer) globals,
                                G_OBJECT (globals)->ref_count);
                        pcl_object_print (globals, stdout, 0);
                        g_print ("\n");
                        g_print ("Locals @ %p |%d|: ",
                                (gpointer) locals,
                                G_OBJECT (locals)->ref_count);
                        pcl_object_print (locals, stdout, 0);
                        g_print ("\n");
                        g_print ("Slots: |v|=%d, |c|=%d, |f|=%d\n",
                                        variable_count,
                                        frame->cell_count,
                                        frame->free_count);
                        for (ii = 0; ii < variable_count; ++ii, ++slot)
                        {
                                g_print ("\tv[%d]: ", ii);
                                if (slots[slot] != NULL)
                                        pcl_object_print (slots[slot],
                                                          stdout, 0);
                                else
                                        g_print ("NULL");
                                g_print ("\n");
                        }
                        for (ii = 0; ii < frame->cell_count; ++ii, ++slot)
                        {
                                g_print ("\tc[%d]: ", ii);
                                if (slots[slot] != NULL)
                                        pcl_object_print (slots[slot],
                                                          stdout, 0);
                                else
                                        g_print ("NULL");
                                g_print ("\n");
                        }
                        for (ii = 0; ii < frame->free_count; ++ii, ++slot)
                        {
                                g_print ("\tf[%d]: ", ii + frame->cell_count);
                                if (slots[slot] != NULL)
                                        pcl_object_print (slots[slot],
                                                          stdout, 0);
                                else
                                        g_print ("NULL");
                                g_print ("\n");
                        }
#endif
                }
#endif

                switch (opcode)
                {
                        case PCL_OPCODE_NONE:
                                goto fast_next_opcode;

                        PREDICTED (POP_TOP);
                        case PCL_OPCODE_POP_TOP:
                                v = STACK_POP ();
                                pcl_object_unref (v);
                                goto fast_next_opcode;

                        case PCL_OPCODE_DUP_TOP:
                                v = STACK_TOP ();
                                STACK_PUSH (pcl_object_ref (v));
                                goto fast_next_opcode;

                        case PCL_OPCODE_ROTATE_2:
                                v = STACK_TOP ();
                                w = STACK_SECOND ();
                                STACK_SET_TOP (w);
                                STACK_SET_SECOND (v);
                                goto fast_next_opcode;

                        case PCL_OPCODE_ROTATE_3:
                                v = STACK_TOP ();
                                w = STACK_SECOND ();
                                x = STACK_THIRD ();
                                STACK_SET_TOP (w);
                                STACK_SET_SECOND (x);
                                STACK_SET_THIRD (v);
                                goto fast_next_opcode;

                        case PCL_OPCODE_ROTATE_4:
                                u = STACK_TOP ();
                                v = STACK_SECOND ();
                                w = STACK_THIRD ();
                                x = STACK_FOURTH ();
                                STACK_SET_TOP (v);
                                STACK_SET_SECOND (w);
                                STACK_SET_THIRD (x);
                                STACK_SET_FOURTH (u);
                                goto fast_next_opcode;

                        case PCL_OPCODE_CONJUNCT:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                /* replace None with True */
                                if (w == PCL_NONE)
                                {
                                        pcl_object_unref (w);
                                        w = pcl_object_ref (PCL_TRUE);
                                }
                                x = pcl_bool_from_boolean (
                                    pcl_object_is_true (w) &&
                                    pcl_object_is_true (v));
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_DISJUNCT:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_bool_from_boolean (
                                    pcl_object_is_true (w) || 
                                    pcl_object_is_true (v));
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_UNARY_POS:
                                v = STACK_TOP ();
                                x = pcl_number_positive (v);
                                pcl_object_unref (v);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_UNARY_NEG:
                                v = STACK_TOP ();
                                x = pcl_number_negative (v);
                                pcl_object_unref (v);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_UNARY_INV:
                                v = STACK_TOP ();
                                x = pcl_number_invert (v);
                                pcl_object_unref (v);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_UNARY_NOT:
                                v = STACK_TOP ();
                                x = pcl_object_not (v) ?
                                        PCL_TRUE : PCL_FALSE;
                                pcl_object_ref (x);
                                STACK_SET_TOP (x);
                                pcl_object_unref (v);
                                continue;

                        case PCL_OPCODE_BINARY_ADD:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                if (IS_INT (v) && IS_INT (w))
                                {
                                        /* INLINE: int + int */
                                        register glong a, b, i;
                                        a = PCL_INT_AS_LONG (v);
                                        b = PCL_INT_AS_LONG (w);
                                        i = a + b;
                                        if ((i ^ a) < 0 && (i ^ b) < 0)
                                                goto slow_add;
                                        x = pcl_int_from_long (i);
                                }
                                else
                                slow_add:
                                        x = pcl_number_add (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_BINARY_SUB:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                if (IS_INT (v) && IS_INT (w))
                                {
                                        /* INLINE: int - int */
                                        register glong a, b, i;
                                        a = PCL_INT_AS_LONG (v);
                                        b = PCL_INT_AS_LONG (w);
                                        i = a - b;
                                        if ((i ^ a) < 0 && (i ^ ~b) < 0)
                                                goto slow_sub;
                                        x = pcl_int_from_long (i);
                                }
                                else
                                slow_sub:
                                        x = pcl_number_subtract (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_BINARY_MUL:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_multiply (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_BINARY_DIV:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_divide (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_BINARY_MOD:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_modulo (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_BINARY_POW:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_power (v, w, PCL_NONE);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_BINARY_FLD:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_floordiv (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_BINARY_LSH:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_lshift (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_BINARY_RSH:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_rshift (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_BINARY_AND:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_and (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;
                        
                        case PCL_OPCODE_BINARY_XOR:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_xor (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_BINARY_OR:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_or (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_INPLACE_ADD:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                if (IS_INT (v) && IS_INT (w))
                                {
                                        /* INLINE: int + int */
                                        register glong a, b, i;
                                        a = PCL_INT_AS_LONG (v);
                                        b = PCL_INT_AS_LONG (w);
                                        i = a + b;
                                        if ((i ^ a) < 0 && (i ^ b) < 0)
                                                goto slow_iadd;
                                        x = pcl_int_from_long (i);
                                }
                                else
                                slow_iadd:
                                        x = pcl_number_in_place_add (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_INPLACE_SUB:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                if (IS_INT (v) && IS_INT (w))
                                {
                                        /* INLINE: int - int */
                                        register glong a, b, i;
                                        a = PCL_INT_AS_LONG (v);
                                        b = PCL_INT_AS_LONG (w);
                                        i = a - b;
                                        if ((i ^ a) < 0 && (i ^ ~b) < 0)
                                                goto slow_isub;
                                        x = pcl_int_from_long (i);
                                }
                                else
                                slow_isub:
                                        x = pcl_number_in_place_subtract (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_INPLACE_MUL:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_in_place_multiply (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_INPLACE_DIV:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_in_place_divide (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_INPLACE_MOD:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_in_place_modulo (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_INPLACE_POW:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_in_place_power (v, w, PCL_NONE);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_INPLACE_FLD:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_in_place_floordiv (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_INPLACE_LSH:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_in_place_lshift (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_INPLACE_RSH:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_in_place_rshift (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_INPLACE_AND:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_in_place_and (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_INPLACE_XOR:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_in_place_xor (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_INPLACE_OR:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_number_in_place_or (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_COMPARISON_LT:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                if (IS_INT (v) && IS_INT (w))
                                {
                                        /* INLINE: int < int */
                                        register glong a, b;
                                        a = PCL_INT_AS_LONG (v);
                                        b = PCL_INT_AS_LONG (w);
                                        x = a < b ? PCL_TRUE : PCL_FALSE;
                                        pcl_object_ref (x);
                                }
                                else
                                        x = pcl_object_rich_compare (
                                                v, w, PCL_LT);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x == NULL)
                                        break;;
                                PREDICT (JUMP_IF_FALSE);
                                PREDICT (JUMP_IF_TRUE);
                                continue;

                        case PCL_OPCODE_COMPARISON_LE:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                if (IS_INT (v) && IS_INT (w))
                                {
                                        /* INLINE: int <= int */
                                        register glong a, b;
                                        a = PCL_INT_AS_LONG (v);
                                        b = PCL_INT_AS_LONG (w);
                                        x = a <= b ? PCL_TRUE : PCL_FALSE;
                                        pcl_object_ref (x);
                                }
                                else
                                        x = pcl_object_rich_compare (
                                                v, w, PCL_LE);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x == NULL)
                                        break;
                                PREDICT (JUMP_IF_FALSE);
                                PREDICT (JUMP_IF_TRUE);
                                continue;

                        case PCL_OPCODE_COMPARISON_EQ:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                if (IS_INT (v) && IS_INT (w))
                                {
                                        /* INLINE: int == int */
                                        register glong a, b;
                                        a = PCL_INT_AS_LONG (v);
                                        b = PCL_INT_AS_LONG (w);
                                        x = a == b ? PCL_TRUE : PCL_FALSE;
                                        pcl_object_ref (x);
                                }
                                else
                                        x = pcl_object_rich_compare (
                                                v, w, PCL_EQ);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x == NULL)
                                        break;
                                PREDICT (JUMP_IF_FALSE);
                                PREDICT (JUMP_IF_TRUE);
                                continue;

                        case PCL_OPCODE_COMPARISON_NE:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                if (IS_INT (v) && IS_INT (w))
                                {
                                        /* INLINE: int != int */
                                        register glong a, b;
                                        a = PCL_INT_AS_LONG (v);
                                        b = PCL_INT_AS_LONG (w);
                                        x = a != b ? PCL_TRUE : PCL_FALSE;
                                        pcl_object_ref (x);
                                }
                                else
                                        x = pcl_object_rich_compare (
                                                v, w, PCL_NE);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x == NULL)
                                        break;
                                PREDICT (JUMP_IF_FALSE);
                                PREDICT (JUMP_IF_TRUE);
                                continue;

                        case PCL_OPCODE_COMPARISON_GE:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                if (IS_INT (v) && IS_INT (w))
                                {
                                        /* INLINE: int >= int */
                                        register glong a, b;
                                        a = PCL_INT_AS_LONG (v);
                                        b = PCL_INT_AS_LONG (w);
                                        x = a >= b ? PCL_TRUE : PCL_FALSE;
                                        pcl_object_ref (x);
                                }
                                else
                                        x = pcl_object_rich_compare (
                                                v, w, PCL_GE);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x == NULL)
                                        break;
                                PREDICT (JUMP_IF_FALSE);
                                PREDICT (JUMP_IF_TRUE);
                                continue;

                        case PCL_OPCODE_COMPARISON_GT:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                if (IS_INT (v) && IS_INT (w))
                                {
                                        /* INLINE: int > int */
                                        register glong a, b;
                                        a = PCL_INT_AS_LONG (v);
                                        b = PCL_INT_AS_LONG (w);
                                        x = a > b ? PCL_TRUE : PCL_FALSE;
                                        pcl_object_ref (x);
                                }
                                else
                                        x = pcl_object_rich_compare (
                                                v, w, PCL_GT);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x == NULL)
                                        break;
                                PREDICT (JUMP_IF_FALSE);
                                PREDICT (JUMP_IF_TRUE);
                                continue;

                        case PCL_OPCODE_COMPARISON_IN:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                if (PCL_IS_ITERABLE (w))
                                {
                                        gint status;
                                        status = pcl_object_contains (w, v);
                                        if (status < 0)
                                                x = NULL;  /* error */
                                        else
                                                x = pcl_bool_from_boolean (
                                                        (gboolean) status);
                                }
                                else
                                        x = NULL;  /* error */
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x == NULL)
                                        break;
                                PREDICT (JUMP_IF_FALSE);
                                PREDICT (JUMP_IF_TRUE);
                                continue;

                        case PCL_OPCODE_COMPARISON_NOT_IN:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                if (PCL_IS_ITERABLE (w))
                                {
                                        gint status;
                                        status = pcl_object_contains (w, v);
                                        if (status < 0)
                                                x = NULL;  /* error */
                                        else
                                                x = pcl_bool_from_boolean (
                                                        (gboolean) !status);
                                }
                                else
                                        x = NULL;  /* error */
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x == NULL)
                                        break;
                                PREDICT (JUMP_IF_FALSE);
                                PREDICT (JUMP_IF_TRUE);
                                continue;

                        case PCL_OPCODE_COMPARISON_IS:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_bool_from_boolean (v == w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x == NULL)
                                        break;
                                PREDICT (JUMP_IF_FALSE);
                                PREDICT (JUMP_IF_TRUE);
                                continue;

                        case PCL_OPCODE_COMPARISON_IS_NOT:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_bool_from_boolean (v != w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x == NULL)
                                        break;
                                PREDICT (JUMP_IF_FALSE);
                                PREDICT (JUMP_IF_TRUE);
                                continue;

                        case PCL_OPCODE_COMPARISON_EXCEPT:
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                x = pcl_bool_from_boolean (
                                pcl_error_given_exception_matches (v, w));
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x == NULL)
                                        break;
                                PREDICT (JUMP_IF_FALSE);
                                PREDICT (JUMP_IF_TRUE);
                                continue;

                        case PCL_OPCODE_LOAD_SUBSCRIPT:
                                /* v[w] */
                                w = STACK_POP ();
                                v = STACK_TOP ();
                                if (IS_LIST (v) && IS_INT (w))
                                {
                                        /* INLINE: list[int] */
                                        register glong i, s;
                                        i = PCL_INT_AS_LONG (w);
                                        s = PCL_LIST_GET_SIZE (v);
                                        if (i < 0)
                                                i += s;
                                        if (i >= 0 && i < s)
                                        {
                                                x = PCL_LIST_GET_ITEM (v, i);
                                                pcl_object_ref (x);
                                        }
                                        else
                                                goto slow_get;
                                }
                                else
                                slow_get:
                                        x = pcl_object_get_item (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_STORE_SUBSCRIPT:
                                /* v[w] = u */
                                w = STACK_TOP ();
                                v = STACK_SECOND ();
                                u = STACK_THIRD ();
                                STACK_ADJUST (-3);
                                success = pcl_object_set_item (v, w, u);
                                pcl_object_unref (u);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                if (success)
                                        continue;
                                break;

                        case PCL_OPCODE_DELETE_SUBSCRIPT:
                                /* del v[w] */
                                w = STACK_TOP ();
                                v = STACK_SECOND ();
                                STACK_ADJUST (-2);
                                success = pcl_object_del_item (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                if (success)
                                        continue;
                                break;

                        case PCL_OPCODE_LOAD_CUT:
                                x = pcl_bool_from_boolean (frame->cut);
                                STACK_PUSH (x);
                                continue;

                        case PCL_OPCODE_LOAD_RESULT:
                                x = pcl_object_ref (frame->result);
                                STACK_PUSH (x);
                                continue;

                        case PCL_OPCODE_PRINT_EXPR:
                                v = STACK_POP ();
                                w = pcl_sys_get_object ("displayhook");
                                if (w == NULL)
                                {
                                        pcl_error_set_string (
                                                pcl_exception_runtime_error (),
                                                "lost sys.displayhook");
                                        success = FALSE;
                                        x = NULL;
                                }
                                if (success)
                                {
                                        x = pcl_tuple_pack (1, v);
                                        success = (x != NULL);
                                }
                                if (success)
                                {
                                        w = pcl_eval_call_object (w, x);
                                        if (w != NULL)
                                                pcl_object_unref (w);
                                        success = (w != NULL);
                                }
                                pcl_object_unref (v);
                                if (x != NULL)
                                        pcl_object_unref (x);
                                break;

                        case PCL_OPCODE_PRINT_ITEM_TO:
                                w = stream = STACK_POP ();
                                /* fall through to PRINT_ITEM */

                        case PCL_OPCODE_PRINT_ITEM:
                                v = STACK_POP ();
                                if (stream == NULL || stream == PCL_NONE)
                                {
                                        w = pcl_sys_get_object ("stdout");
                                        if (w == NULL)
                                        {
                                                pcl_error_set_string (
                                                pcl_exception_runtime_error (),
                                                "lost sys.stdout");
                                                success = FALSE;
                                        }
                                }
                                /* pcl_file_soft_space() can execute arbitrary
                                 * code if sys.stdout is an instance with a
                                 * __getattr__.  If __getattr__ raises an
                                 * exception, w will be freed, so we need to
                                 * prevent that temporarily. */
                                if (w != NULL)
                                        pcl_object_ref (w);
                                if (w != NULL && pcl_file_soft_space (w, FALSE))
                                        success =
                                        pcl_file_write_string (w, " ");
                                if (success)
                                        success =
                                        pcl_file_write_object (w, v,
                                                PCL_PRINT_FLAG_RAW);
                                if (success)
                                {
                                        if (PCL_IS_STRING (v) &&
                                                PCL_STRING_GET_SIZE (v) > 0)
                                        {
                                                gchar *str =
                                                PCL_STRING_AS_STRING (v);
                                                glong len =
                                                PCL_STRING_GET_SIZE (v);
                                                gchar c = str[len - 1];
                                                if (!g_ascii_isspace (c) ||
                                                        c == ' ')
                                                pcl_file_soft_space (w, TRUE);

                                        }
                                        else
                                                pcl_file_soft_space (w, TRUE);
                                }
                                if (w != NULL)
                                        pcl_object_unref (w);
                                pcl_object_unref (v);
                                if (stream != NULL)
                                        pcl_object_unref (stream);
                                stream = NULL;
                                if (success)
                                        continue;
                                break;

                        case PCL_OPCODE_PRINT_NEWLINE_TO:
                                w = stream = STACK_POP ();
                                /* fall through to PRINT_NEWLINE */

                        case PCL_OPCODE_PRINT_NEWLINE:
                                if (stream == NULL || stream == PCL_NONE)
                                {
                                        w = pcl_sys_get_object ("stdout");
                                        if (w == NULL)
                                        {
                                                pcl_error_set_string (
                                                pcl_exception_runtime_error (),
                                                "lost sys.stdout");
                                                success = FALSE;
                                        }
                                }
                                if (w != NULL)
                                {
                                        success =
                                        pcl_file_write_string (w, "\n");
                                        if (success)
                                                pcl_file_soft_space (w, FALSE);
                                }
                                if (stream != NULL)
                                        pcl_object_unref (stream);
                                stream = NULL;
                                break;

                        case PCL_OPCODE_LIST_APPEND:
                                w = STACK_POP ();
                                v = STACK_POP ();
                                success = pcl_list_append (v, w);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                if (success)
                                {
                                        PREDICT (JUMP_ABSOLUTE);
                                        continue;
                                }
                                break;

                        case PCL_OPCODE_RAISE_EXCEPTION:
                                u = STACK_POP ();
                                v = STACK_POP ();
                                w = STACK_POP ();
                                why = eval_raise (w, v, u);
                                break;

                        case PCL_OPCODE_LOAD_LOCALS:
                                x = locals;
                                if (x != NULL)
                                {
                                        STACK_PUSH (pcl_object_ref (x));
                                        continue;
                                }
                                pcl_error_set_string (
                                        pcl_exception_system_error (),
                                        "no locals");
                                break;

                        case PCL_OPCODE_RETURN_VALUE:
                                retval = STACK_POP ();
                                why = WHY_RETURN;
                                goto fast_block_end;

                        case PCL_OPCODE_YIELD_VALUE:
                                retval = STACK_POP ();
                                frame->tos = stack_pointer;
                                why = WHY_YIELD;
                                goto fast_yield;

                        case PCL_OPCODE_EXEC_STMT:
                                w = STACK_TOP ();
                                v = STACK_SECOND ();
                                u = STACK_THIRD ();
                                STACK_ADJUST (-3);
                                success = eval_exec (frame, u, v, w);
                                pcl_object_unref (u);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                break;

                        case PCL_OPCODE_POP_BLOCK:
                        {
                                PclTryBlock *block;
                                block = pcl_frame_block_pop (frame);
                                while (STACK_LEVEL () > block->level)
                                        pcl_object_unref (STACK_POP ());
                                continue;
                        }

                        case PCL_OPCODE_BREAK_LOOP:
                                why = WHY_BREAK;
                                goto fast_block_end;

                        case PCL_OPCODE_GET_ITER:
                                v = STACK_TOP ();
                                x = pcl_object_iterate (v);
                                pcl_object_unref (v);
                                if (x != NULL)
                                {
                                        STACK_SET_TOP (x);
                                        PREDICT (FOR_ITER);
                                        continue;
                                }
                                STACK_ADJUST (-1);
                                break;

                        case PCL_OPCODE_END_FINALLY:
                                v = STACK_POP ();
                                if (PCL_IS_INT (v))
                                {
                                        why = pcl_int_as_long (v);
                                        g_assert (why != WHY_YIELD);
                                        if (why == WHY_RETURN ||
                                            why == WHY_CONTINUE)
                                                retval = STACK_POP ();
                                }
                                /* XXX Python checks for class or string. */
                                else if (PCL_IS_EXCEPTION (v))
                                {
                                        w = STACK_POP ();
                                        u = STACK_POP ();
                                        pcl_error_restore (v, w, u);
                                        why = WHY_RERAISE;
                                        break;
                                }
                                else if (v != PCL_NONE)
                                {
                                        pcl_error_set_string (
                                        pcl_exception_system_error (),
                                        "'finally' pops bad exception");
                                        why = WHY_EXCEPTION;
                                }
                                pcl_object_unref (v);
                                break;

                        case PCL_OPCODE_BUILD_CLASS:
                                u = STACK_TOP ();
                                v = STACK_SECOND ();
                                w = STACK_THIRD ();
                                STACK_ADJUST (-2);
                                x = eval_build_class (u, v, w);
                                STACK_SET_TOP (x);
                                pcl_object_unref (u);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                break;

                        case PCL_OPCODE_IMPORT_STAR:
                                v = STACK_POP ();
                                pcl_frame_fast_to_locals (frame);
                                x = frame->locals;
                                if (x == NULL)
                                {
                                        pcl_error_set_string (
                                                pcl_exception_system_error (),
                                                "no locals found during "
                                                "'import *'");
                                        break;
                                }
                                success = eval_import_all_from (x, v);
                                pcl_frame_locals_to_fast (frame, FALSE);
                                pcl_object_unref (v);
                                if (success)
                                        continue;
                                break;

                        /* opcodes from here on have arguments */

                        case PCL_OPCODE_JUMP_FORWARD:
                                JUMPBY (oparg);
                                goto fast_next_opcode;

                        PREDICTED_WITH_ARG (JUMP_IF_TRUE);
                        case PCL_OPCODE_JUMP_IF_TRUE:
                                /* XXX Need better error handling */
                                w = STACK_TOP ();
                                if (w == PCL_FALSE)
                                {
                                        PREDICT (POP_TOP);
                                        goto fast_next_opcode;
                                }
                                if (w == PCL_TRUE)
                                {
                                        JUMPBY (oparg);
                                        goto fast_next_opcode;
                                }
                                if (pcl_object_is_true (w))
                                        JUMPBY (oparg);
                                continue;

                        PREDICTED_WITH_ARG (JUMP_IF_FALSE);
                        case PCL_OPCODE_JUMP_IF_FALSE:
                                /* XXX Need better error handling */
                                w = STACK_TOP ();
                                if (w == PCL_TRUE)
                                {
                                        PREDICT (POP_TOP);
                                        goto fast_next_opcode;
                                }
                                if (w == PCL_FALSE)
                                {
                                        JUMPBY (oparg);
                                        goto fast_next_opcode;
                                }
                                if (!pcl_object_is_true (w))
                                        JUMPBY (oparg);
                                continue;

                        PREDICTED_WITH_ARG (JUMP_ABSOLUTE);
                        case PCL_OPCODE_JUMP_ABSOLUTE:
                                JUMPTO (oparg);
                                continue;

                        case PCL_OPCODE_SETUP_LOOP:
                        case PCL_OPCODE_SETUP_EXCEPT:
                        case PCL_OPCODE_SETUP_FINALLY:
                                pcl_frame_block_setup (frame, opcode,
                                                       IPADDR () + oparg,
                                                       STACK_LEVEL ());
                                continue;

                        case PCL_OPCODE_CONTINUE_LOOP:
                                retval = pcl_int_from_long (oparg);
                                why = WHY_CONTINUE;
                                goto fast_block_end;

                        case PCL_OPCODE_LOAD_CONST:
                                x = GETITEM (constants, oparg);
                                STACK_PUSH (pcl_object_ref (x));
                                goto fast_next_opcode;

                        case PCL_OPCODE_LOAD_LOCAL:
                                /* XXX Hard to write this cleanly. */
                                w = GETITEM (names, oparg);
                                if (locals == NULL)
                                {
                                        pcl_error_set_format (
                                                pcl_exception_system_error (),
                                                "no locals when loading %s",
                                                pcl_object_repr (w));
                                        success = FALSE;
                                        break;
                                }
                                if (IS_DICT (locals))
                                {
                                        x = pcl_dict_get_item (locals, w);
                                        if (x != NULL)
                                                pcl_object_ref (x);
                                }
                                else
                                {
                                        x = pcl_object_get_item (locals, w);
                                        if (x == NULL && pcl_error_occurred ())
                                        {
                                                if (!pcl_error_exception_matches (
                                                        pcl_exception_key_error ()))
                                                        break;
                                                pcl_error_clear ();
                                        }
                                }
                                if (x == NULL)
                                {
                                        x = pcl_dict_get_item (globals, w);
                                        if (x == NULL)
                                        {
                                                x = pcl_dict_get_item (builtins, w);
                                                if (x == NULL)
                                                {
                                                        eval_format_exc_check_arg (
                                                                pcl_exception_name_error (),
                                                                NAME_ERROR_MSG, w);
                                                        break;
                                                }
                                        }
                                        pcl_object_ref (x);
                                }
                                STACK_PUSH (x);
                                continue;

                        case PCL_OPCODE_STORE_LOCAL:
                                w = GETITEM (names, oparg);
                                v = STACK_POP ();
                                if (locals == NULL)
                                {
                                        pcl_error_set_format (
                                                pcl_exception_system_error (),
                                                "no locals when storing %s",
                                                pcl_object_repr (w));
                                        success = FALSE;
                                        break;
                                }
                                if (IS_DICT (locals))
                                        success =
                                        pcl_dict_set_item (locals, w, v);
                                else
                                        success =
                                        pcl_object_set_item (locals, w, v);
                                pcl_object_unref (v);
                                if (success)
                                        continue;
                                break;

                        case PCL_OPCODE_DELETE_LOCAL:
                                w = GETITEM (names, oparg);
                                if (locals != NULL)
                                {
                                        pcl_error_set_format (
                                                pcl_exception_system_error (),
                                                "no locals when deleting %s",
                                                pcl_object_repr (w));
                                        success = FALSE;
                                        break;
                                }
                                if (!pcl_dict_del_item (locals, w))
                                {
                                        eval_format_exc_check_arg (
                                                pcl_exception_name_error (),
                                                NAME_ERROR_MSG, w);
                                        success = FALSE;
                                }
                                break;

                        case PCL_OPCODE_LOAD_GLOBAL:
                                w = GETITEM (names, oparg);
                                x = pcl_dict_get_item (globals, w);
                                if (x == NULL)
                                {
                                        x = pcl_dict_get_item (builtins, w);
                                        if (x == NULL)
                                        {
                                                eval_format_exc_check_arg (
                                                        pcl_exception_name_error (),
                                                        GLOBAL_NAME_ERROR_MSG, w);
                                                break;
                                        }
                                }
                                STACK_PUSH (pcl_object_ref (x));
                                continue;

                        case PCL_OPCODE_STORE_GLOBAL:
                                w = GETITEM (names, oparg);
                                v = STACK_POP ();
                                success = pcl_dict_set_item (globals, w, v);
                                pcl_object_unref (v);
                                if (success)
                                        continue;
                                break;

                        case PCL_OPCODE_DELETE_GLOBAL:
                                w = GETITEM (names, oparg);
                                if (!pcl_dict_del_item (globals, w))
                                {
                                        eval_format_exc_check_arg (
                                                pcl_exception_name_error (),
                                                GLOBAL_NAME_ERROR_MSG, w);
                                        success = FALSE;
                                }
                                break;

                        case PCL_OPCODE_LOAD_ATTR:
                                w = GETITEM (names, oparg);
                                v = STACK_TOP ();
                                x = pcl_object_get_attr (v, w);
                                pcl_object_unref (v);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_STORE_ATTR:
                                /* v.w = u */
                                w = GETITEM (names, oparg);
                                v = STACK_TOP ();
                                u = STACK_SECOND ();
                                STACK_ADJUST (-2);
                                success = pcl_object_set_attr (v, w, u);
                                pcl_object_unref (v);
                                pcl_object_unref (u);
                                if (success)
                                        continue;
                                break;

                        case PCL_OPCODE_DELETE_ATTR:
                                /* del v.w */
                                w = GETITEM (names, oparg);
                                v = STACK_POP ();
                                success = pcl_object_set_attr (v, w, NULL);
                                pcl_object_unref (v);
                                if (success)
                                        continue;
                                break;

                        case PCL_OPCODE_LOAD_FAST:
                                x = slots[oparg];
                                if (x != NULL)
                                {
                                        STACK_PUSH (pcl_object_ref (x));
                                        goto fast_next_opcode;
                                }
                                eval_format_exc_check_arg (
                                        pcl_exception_unbound_local_error (),
                                        UNBOUND_LOCAL_ERROR_MSG,
                                        GETITEM (code->varnames, oparg));
                                break;

                        PREDICTED_WITH_ARG (STORE_FAST);
                        case PCL_OPCODE_STORE_FAST:
                                v = STACK_POP ();
                                x = slots[oparg];
                                if (x != NULL)
                                        pcl_object_unref (x);
                                slots[oparg] = v;
                                goto fast_next_opcode;

                        case PCL_OPCODE_DELETE_FAST:
                                x = slots[oparg];
                                if (x != NULL)
                                {
                                        pcl_object_unref (x);
                                        slots[oparg] = NULL;
                                        continue;
                                }
                                eval_format_exc_check_arg (
                                        pcl_exception_unbound_local_error (),
                                        UNBOUND_LOCAL_ERROR_MSG,
                                        GETITEM (code->varnames, oparg));
                                break;

                        case PCL_OPCODE_LOAD_DEREF:
                                x = free_slots[oparg];
                                w = pcl_cell_get (x);
                                if (w != NULL)
                                {
                                        STACK_PUSH (w);
                                        continue;
                                }
                                success = FALSE;
                                /* don't stomp existing exception */
                                if (pcl_error_occurred ())
                                        break;
                                if (oparg < frame->cell_count)
                                {
                                        v = GETITEM (code->cellvars, oparg);
                                        eval_format_exc_check_arg (
                                                pcl_exception_unbound_local_error (),
                                                UNBOUND_LOCAL_ERROR_MSG, v);
                                }
                                else
                                {
                                        v = pcl_tuple_get_item (
                                                code->freevars,
                                                oparg - frame->cell_count);
                                        eval_format_exc_check_arg (
                                                pcl_exception_name_error (),
                                                UNBOUND_FREE_ERROR_MSG, v);
                                }
                                break;

                        case PCL_OPCODE_STORE_DEREF:
                                w = STACK_POP ();
                                x = free_slots[oparg];
                                pcl_cell_set (x, w);
                                pcl_object_unref (w);
                                continue;

                        case PCL_OPCODE_LOAD_CLOSURE:
                                x = free_slots[oparg];
                                STACK_PUSH (pcl_object_ref (x));
                                continue;

                        case PCL_OPCODE_MAKE_CLOSURE:
                        {
                                glong nfree;
                                v = STACK_POP ();  /* code object */
                                x = pcl_function_new (PCL_CODE (v), globals);
                                nfree = PCL_CODE_GET_NUM_FREE (v);
                                pcl_object_unref (v);
                                if (x != NULL && nfree > 0)
                                {
                                        v = pcl_tuple_new (nfree);
                                        if (v == NULL)
                                        {
                                                pcl_object_unref (x);
                                                x = NULL;
                                                break;
                                        }
                                        while (nfree-- > 0)
                                        {
                                                w = STACK_POP ();
                                                pcl_tuple_set_item (
                                                        v, nfree, w);
                                        }
                                        pcl_function_set_closure (x, v);
                                        pcl_object_unref (v);
                                }
                                if (x != NULL && oparg > 0)
                                {
                                        v = pcl_tuple_new (oparg);
                                        if (v == NULL)
                                        {
                                                pcl_object_unref (x);
                                                x = NULL;
                                                break;
                                        }
                                        while (oparg-- > 0)
                                        {
                                                w = STACK_POP ();
                                                pcl_tuple_set_item (
                                                        v, oparg, w);
                                        }
                                        pcl_function_set_defaults (x, v);
                                        pcl_object_unref (v);
                                }
                                STACK_PUSH (x);
                                break;
                        }

                        case PCL_OPCODE_MAKE_FUNCTION:
                                v = STACK_POP ();  /* code object */
                                x = pcl_function_new (PCL_CODE (v), globals);
                                pcl_object_unref (v);
                                if (x != NULL && oparg > 0)
                                {
                                        v = pcl_tuple_new (oparg);
                                        if (v == NULL)
                                        {
                                                pcl_object_unref (x);
                                                x = NULL;
                                                break;
                                        }
                                        while (oparg-- > 0)
                                        {
                                                w = STACK_POP ();
                                                pcl_tuple_set_item (
                                                        v, oparg, w);
                                        }
                                        pcl_function_set_defaults (x, v);
                                        pcl_object_unref (v);
                                }
                                STACK_PUSH (x);
                                break;

                        case PCL_OPCODE_CALL_FUNCTION:
                        {
                                PclObject **sp = stack_pointer;
                                x = eval_call_function (&sp, oparg);
                                stack_pointer = sp;
                                STACK_PUSH (x);
                                if (x != NULL)
                                        continue;
                                break;
                        }

                        case PCL_OPCODE_CALL_FUNCTION_VA:
                        case PCL_OPCODE_CALL_FUNCTION_KW:
                        case PCL_OPCODE_CALL_FUNCTION_VA_KW:
                        {
                                gboolean epa =
                                    opcode == PCL_OPCODE_CALL_FUNCTION_VA ||
                                    opcode == PCL_OPCODE_CALL_FUNCTION_VA_KW;
                                gboolean eka =
                                    opcode == PCL_OPCODE_CALL_FUNCTION_KW ||
                                    opcode == PCL_OPCODE_CALL_FUNCTION_VA_KW;
                                gint npa = oparg & 0xFF;
                                gint nka = (oparg >> 8) & 0xFF;
                                gint size = npa + 2 * nka + epa + eka;
                                PclObject **pfunc, *func, **sp;
                                pfunc = stack_pointer - size - 1;
                                func = *pfunc;

                                /* XXX Skip bound method object check */
                                pcl_object_ref (func);
                                sp = stack_pointer;
                                x = eval_ext_call_function (
                                        func, &sp, npa, nka, epa, eka);
                                stack_pointer = sp;
                                pcl_object_unref (func);

                                /* Pop the function object and any leftovers. */
                                while (stack_pointer > pfunc)
                                        pcl_object_unref (STACK_POP ());
                                STACK_PUSH (x);
                                if (x != NULL)
                                        continue;
                                break;
                        }

                        case PCL_OPCODE_BUILD_DICT:
                                x = pcl_dict_new ();
                                while (oparg-- > 0)
                                {
                                        w = STACK_POP ();
                                        v = STACK_POP ();
                                        pcl_object_set_item (x, v, w);
                                        pcl_object_unref (v);
                                        pcl_object_unref (w);
                                }
                                STACK_PUSH (x);
                                continue;

                        case PCL_OPCODE_BUILD_LIST:
                                x = pcl_list_new (oparg);
                                while (oparg-- > 0)
                                {
                                        w = STACK_POP ();
                                        pcl_list_set_item (x, oparg, w);
                                }
                                STACK_PUSH (x);
                                continue;

                        case PCL_OPCODE_BUILD_SLICE:
                                /* [u:v:w] */
                                /* XXX Not using oparg. */
                                w = STACK_POP ();
                                v = STACK_POP ();
                                u = STACK_TOP ();
                                x = pcl_slice_new (u, v, w);
                                pcl_object_unref (u);
                                pcl_object_unref (v);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_BUILD_TUPLE:
                                x = pcl_tuple_new (oparg);
                                while (oparg-- > 0)
                                {
                                        w = STACK_POP ();
                                        pcl_tuple_set_item (x, oparg, w);
                                }
                                STACK_PUSH (x);
                                continue;

                        PREDICTED_WITH_ARG (UNPACK_SEQUENCE);
                        case PCL_OPCODE_UNPACK_SEQUENCE:
                                v = STACK_POP ();
                                if (IS_TUPLE (v) &&
                                        PCL_TUPLE_GET_SIZE (v) == oparg)
                                {
                                        gpointer *pdata;
                                        pdata = PCL_TUPLE (v)->array->pdata;
                                        while (oparg-- > 0)
                                        {
                                                w = pdata[oparg];
                                                pcl_object_ref (w);
                                                STACK_PUSH (w);
                                        }
                                        pcl_object_unref (v);
                                        continue;
                                }
                                else if (IS_LIST (v) &&
                                        PCL_LIST_GET_SIZE (v) == oparg)
                                {
                                        gpointer *pdata;
                                        pdata = PCL_LIST (v)->array->pdata;
                                        while (oparg-- > 0)
                                        {
                                                w = pdata[oparg];
                                                pcl_object_ref (w);
                                                STACK_PUSH (w);
                                        }
                                }
                                else if (eval_unpack_iterable (
                                        v, oparg, stack_pointer + oparg))
                                {
                                        STACK_ADJUST (oparg);
                                }
                                else
                                {
                                        if (pcl_error_exception_matches (
                                                pcl_exception_type_error ()))
                                        {
                                                pcl_error_set_string (
                                                pcl_exception_type_error (),
                                                "unpack non-sequence");
                                        }
                                        why = WHY_EXCEPTION;
                                }
                                pcl_object_unref (v);
                                break;

                        case PCL_OPCODE_IMPORT_FROM:
                                w = GETITEM (names, oparg);
                                v = STACK_TOP ();
                                x = eval_import_from (v, w);
                                STACK_PUSH (x);
                                if (x != NULL)
                                        continue;
                                break;

                        case PCL_OPCODE_IMPORT_NAME:
                                v = GETITEM (names, oparg);
                                x = pcl_dict_get_item_string (
                                        frame->builtins, "__import__");
                                if (x == NULL)
                                {
                                        pcl_error_set_string (
                                                pcl_exception_import_error (),
                                                "__import__ not found");
                                        break;
                                }
                                u = STACK_TOP ();
                                w = pcl_tuple_pack (
                                        4, v, frame->globals,
                                        (frame->locals == NULL) ?
                                        PCL_NONE : frame->locals, u);
                                pcl_object_unref (u);
                                if (w == NULL)
                                {
                                        u = STACK_POP ();
                                        x = NULL;
                                        break;
                                }
                                x = pcl_object_call (x, w, NULL);
                                pcl_object_unref (w);
                                STACK_SET_TOP (x);
                                if (x != NULL)
                                        continue;
                                break;

                        PREDICTED_WITH_ARG (FOR_ITER);
                        case PCL_OPCODE_FOR_ITER:
                                v = STACK_TOP ();
                                x = pcl_iterator_next (v);
                                if (x != NULL)
                                {
                                        STACK_PUSH (x);
                                        PREDICT (STORE_FAST);
                                        PREDICT (UNPACK_SEQUENCE);
                                        continue;
                                }
                                if (pcl_error_occurred ())
                                {
                                        if (!pcl_error_exception_matches (
                                        pcl_exception_stop_iteration ()))
                                                break;
                                        pcl_error_clear ();
                                }
                                /* iterator ended normally */
                                x = v = STACK_POP ();
                                pcl_object_unref (v);
                                JUMPBY (oparg);
                                continue;

                        case PCL_OPCODE_STORE_CUT:
                        {
                                PclFrame *fp = frame;
                                while (oparg-- > 0)
                                {
                                        fp = fp->previous;
                                        g_assert (fp != NULL);
                                }
                                v = STACK_POP ();
                                fp->cut = pcl_object_is_true (v);
                                pcl_object_unref (v);
                                continue;
                        }

                        case PCL_OPCODE_EXTENDED_ARG:
                                opcode = NEXTOP ();
                                oparg = oparg << 16 | NEXTARG ();
                                continue;

                        default:
                                g_printerr ("XXX lineno: %d, opcode: 0x%x\n",
                                        pcl_code_addr_to_line (frame->code,
                                        frame->lasti), opcode);
                                pcl_error_set_string (
                                        pcl_exception_system_error (),
                                        "Unknown opcode");
                                why = WHY_EXCEPTION;
                                break;
                }

on_error:

                /* Quickly continue if no error occurred. */
                if (why == WHY_NOT)
                {
                        if (success && x != NULL)
                        {
#ifdef CHECK_EXC
                                /* This check is expensive! */
                                if (pcl_error_occurred ())
                                        g_printerr ("XXX undetected error\n");
                                else
#endif
                                        continue;
                        }
                        why = WHY_EXCEPTION;
                        success = TRUE;
                        x = PCL_NONE;
                }

                /* Double-check exception status. */
                if (why == WHY_EXCEPTION || why == WHY_RERAISE)
                {
                        if (!pcl_error_occurred ())
                        {
                                pcl_error_set_string (
                                        pcl_exception_system_error (),
                                        "error return without exception set");
                                why = WHY_EXCEPTION;
                        }
                }
#ifdef CHECK_EXC
                else
                {
                        /* This check is expensive! */
                        if (pcl_error_occurred ())
                                g_error ("Stack unwind with exception "
                                         "set and why=%d", why);
                }
#endif

                /* Log traceback info if this is a real exception. */
                if (why == WHY_EXCEPTION)
                {
                        pcl_traceback_here (frame);
                        if (ts->trace_function != NULL)
                                eval_call_exc_trace (ts->trace_function,
                                                     ts->trace_object, frame);
                }

                /* For the rest, treat WHY_RERAISE as WHY_EXCEPTION. */
                if (why == WHY_RERAISE)
                        why = WHY_EXCEPTION;

fast_block_end:

                /* Unwind stacks if a (pseudo) exception occurred. */
                while (why != WHY_NOT && frame->block_count > 0)
                {
                        PclTryBlock *block = pcl_frame_block_pop (frame);

                        g_assert (why != WHY_YIELD);
                        if (block->type == PCL_OPCODE_SETUP_LOOP &&
                                why == WHY_CONTINUE)
                        {
                                /* For a 'continue' inside a 'try' block,
                                 * don't pop the block for the loop. */
                                pcl_frame_block_setup (
                                        frame, block->type,
                                        block->handler, block->level);
                                why = WHY_NOT;
                                JUMPTO (pcl_int_as_long (retval));
                                pcl_object_unref (retval);
                                break;
                        }
                        while (STACK_LEVEL () > block->level)
                                if ((v = STACK_POP ()) != NULL)
                                        pcl_object_unref (v);
                        if (block->type == PCL_OPCODE_SETUP_LOOP &&
                                why == WHY_BREAK)
                        {
                                why = WHY_NOT;
                                JUMPTO (block->handler);
                                break;
                        }
                        if (block->type == PCL_OPCODE_SETUP_FINALLY ||
                                (block->type == PCL_OPCODE_SETUP_EXCEPT &&
                                why == WHY_EXCEPTION))
                        {
                                if (why == WHY_EXCEPTION)
                                {
                                        PclObject *exc, *val, *tb;
                                        pcl_error_fetch (&exc, &val, &tb);
                                        if (val == NULL)
                                                val = pcl_object_ref (PCL_NONE);
                                        /* Make the raw exception data
                                         * available to the handler, so a
                                         * program can emulate the main loop.
                                         * Don't do this for 'finally'. */
                                        if (block->type ==
                                                PCL_OPCODE_SETUP_EXCEPT)
                                        {
                                                eval_set_exc_info (
                                                        ts, exc, val, tb);
                                        }
                                        if (tb == NULL)
                                                tb = pcl_object_ref (PCL_NONE);
                                        STACK_PUSH (tb);
                                        STACK_PUSH (val);
                                        STACK_PUSH (exc);
                                }
                                else
                                {
                                        if (why & (WHY_RETURN | WHY_CONTINUE))
                                                STACK_PUSH (retval);
                                        v = pcl_int_from_long ((glong) why);
                                        STACK_PUSH (v);
                                }
                                why = WHY_NOT;
                                JUMPTO (block->handler);
                                break;
                        }
                }

                /* End the loop if we still have an error (or return). */
                if (why != WHY_NOT)
                        break;

        } /* main loop */

        /* Pop remaining stack entries. */
        g_assert (why != WHY_YIELD);
        while (!STACK_EMPTY ())
                if ((v = STACK_POP ()) != NULL)
                        pcl_object_unref (v);

        if (why != WHY_RETURN)
                retval = NULL;

fast_yield:

        eval_reset_exc_info (ts);

        pcl_leave_recursive_call ();
        ts->frame = frame->previous;
        return retval;
}

PclObject *
pcl_eval_code (PclCode *code, PclObject *globals, PclObject *locals)
{
        return pcl_eval_code_ex (code, globals, locals,
                                 NULL, 0, NULL, 0, NULL, 0, NULL);
}

PclObject *
pcl_eval_code_ex (PclCode *code, PclObject *globals, PclObject *locals,
                  PclObject **arguments, glong n_arguments_given,
                  PclObject **keywords, glong n_keywords_given,
                  PclObject **defaults, glong n_defaults_given,
                  PclObject *closure)
{
        PclThreadState *ts = pcl_thread_state_get ();

        PclFrame *frame;
        PclObject **slots;
        PclObject **cell_slots;
        PclObject **free_slots;
        PclObject *variable_names;
        PclObject *cell_variable_names;
        PclObject *name;
        PclObject *retval = NULL;
        PclObject *tuple;  /* for arg processing */
        guint argument_count;
        guint variable_count;
        guint cell_variable_count;
        guint free_variable_count;
        gint flags;

        if (globals == NULL)
        {
                pcl_error_set_format (
                        pcl_exception_system_error (),
                        "%s: NULL globals", G_STRFUNC);
                return NULL;
        }

        frame = pcl_frame_new (ts, code, globals, locals);
        if (frame == NULL)
                return NULL;

        name = code->name;
        flags = code->flags;

        variable_names = code->varnames;
        cell_variable_names = code->cellvars;

        argument_count = code->argument_count;
        variable_count = code->variable_count;
        cell_variable_count = frame->cell_count;
        free_variable_count = frame->free_count;

        slots = frame->slots;
        cell_slots = &slots[variable_count];
        free_slots = &cell_slots[cell_variable_count];

        /* Process arguments and copy them into the frame slots. */
        if (argument_count > 0 ||
                flags & (PCL_CODE_FLAG_VARARGS | PCL_CODE_FLAG_VARKWDS))
        {
                guint n_arguments = n_arguments_given;
                glong ii, jj, index;
                PclObject *kwdict = NULL;

                /* Initialize excess keyword argument dictionary. */
                if (flags & PCL_CODE_FLAG_VARKWDS)
                {
                        kwdict = pcl_dict_new ();
                        if (kwdict == NULL)
                                goto fail;
                        index = argument_count;
                        if (flags & PCL_CODE_FLAG_VARARGS)
                                index++;
                        slots[index] = kwdict;
                }

                /* Check arguments given vs. arguments expected. */
                if (n_arguments_given > argument_count)
                {
                        if (!(flags & PCL_CODE_FLAG_VARARGS))
                        {
                                pcl_error_set_format (
                                        pcl_exception_type_error (),
                                        "%s() takes %s %d "
                                        "%sarguments%s (%d given)",
                                        pcl_string_as_string (name),
                                        (n_defaults_given > 0) ? "at most"
                                                              : "exactly",
                                        argument_count,
                                        (n_keywords_given > 0) ? "non-keyword "
                                                              : "",
                                        (argument_count == 1) ? "" : "s",
                                        n_arguments_given);
                                goto fail;
                        }
                        n_arguments = argument_count;
                }

                /* Populate argument slots. */
                for (ii = 0; ii < n_arguments; ii++)
                        slots[ii] = pcl_object_ref (arguments[ii]);

                /* Initialize excess positional argument tuple. */
                if (flags & PCL_CODE_FLAG_VARARGS)
                {
                        tuple = pcl_tuple_new (n_arguments_given - n_arguments);
                        if (tuple == NULL)
                                goto fail;
                        slots[argument_count] = tuple;
                        for (ii = n_arguments; ii < n_arguments_given; ii++)
                                pcl_tuple_set_item (tuple, ii - n_arguments,
                                        pcl_object_ref (arguments[ii]));
                }

                /* Process keyword arguments. */
                if (n_keywords_given > 0)
                {
                        for (ii = 0; ii < n_keywords_given; ii++)
                        {
                                PclObject *key, *val;
                                key = keywords[ii * 2];
                                val = keywords[ii * 2 + 1];

                                if (!PCL_IS_STRING (key))
                                {
                                        pcl_error_set_format (
                                                pcl_exception_type_error (),
                                                "%s() keywords must be strings",
                                                pcl_string_as_string (name));
                                        goto fail;
                                }

                                for (jj = 0; jj < argument_count; jj++)
                                {
                                        PclObject *argument_name =
                                                PCL_TUPLE_GET_ITEM (
                                                variable_names, jj);
                                        gint is_equal =
                                                pcl_object_rich_compare_bool (
                                                key, argument_name, PCL_EQ);
                                        if (is_equal > 0)
                                                break;
                                        if (is_equal < 0)
                                                goto fail;
                                }
                                if (jj >= argument_count)
                                {
                                        if (kwdict == NULL)
                                        {
                                                pcl_error_set_format (
                                                pcl_exception_type_error (),
                                                "%s() got an unexpected "
                                                "keyword argument '%s'",
                                                pcl_string_as_string (name),
                                                pcl_string_as_string (key));
                                                goto fail;
                                        }
                                        pcl_dict_set_item (kwdict, key, val);
                                }
                                else
                                {
                                        if (slots[jj] != NULL)
                                        {
                                                pcl_error_set_format (
                                                pcl_exception_type_error (),
                                                "%s() got multiple values for "
                                                "keyword argument '%s'",
                                                pcl_string_as_string (name),
                                                pcl_string_as_string (key));
                                                goto fail;
                                        }
                                        slots[jj] = pcl_object_ref (val);
                                }
                        }
                }

                /* Supply default arguments as needed. */
                if (n_arguments_given < argument_count)
                {
                        guint n_defaults = argument_count - n_defaults_given;
                        for (ii = n_arguments_given; ii < n_defaults; ii++)
                        {
                                if (slots[ii] != NULL)
                                        continue;

                                pcl_error_set_format (
                                        pcl_exception_type_error (),
                                        "%s() takes %s %d "
                                        "%sargument%s (%d given)",
                                        pcl_string_as_string (name),
                                        ((flags & PCL_CODE_FLAG_VARARGS) ||
                                        n_defaults_given > 0)
                                                ? "at least"
                                                : "exactly",
                                        n_defaults,
                                        (n_keywords_given > 0)
                                                ? "non-keyword " : "",
                                        (n_defaults == 1) ? "" : "s", ii);
                                goto fail;
                        }
                        if (n_arguments > n_defaults)
                                ii = n_arguments - n_defaults;
                        else
                                ii = 0;
                        while (ii < n_defaults_given)
                        {
                                if (slots[n_defaults + ii] == NULL)
                                        slots[n_defaults + ii] =
                                                pcl_object_ref (defaults[ii]);
                                ii++;
                        }
                }
        }
        else if (n_arguments_given > 0 || n_keywords_given > 0)
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "%s() takes no arguments (%d given)",
                        pcl_string_as_string (name),
                        n_arguments_given + n_keywords_given);
                goto fail;
        }

        /* Allocate and initialize storage for cell variables. */
        if (cell_variable_count > 0)
        {
                gchar *arg_name, *cell_name;
                guint ii, jj;

                if (flags & PCL_CODE_FLAG_VARARGS)
                        argument_count++;
                if (flags & PCL_CODE_FLAG_VARKWDS)
                        argument_count++;

                /* Check for cells that shadow arguments. */
                for (ii = 0; ii < cell_variable_count; ii++)
                {
                        cell_name =
                                PCL_STRING_AS_STRING (
                                PCL_TUPLE_GET_ITEM (
                                cell_variable_names, ii));
                        for (jj = 0; jj < argument_count; jj++)
                        {
                                arg_name =
                                        PCL_STRING_AS_STRING (
                                        PCL_TUPLE_GET_ITEM (
                                        variable_names, jj));
                                if (strcmp (cell_name, arg_name) == 0)
                                {
                                        pcl_cell_set (
                                                cell_slots[ii],
                                                slots[jj]);
                                        break;
                                }
                        }
                }
        }

        /* Copy free variables into the frame slots. */
        if (free_variable_count > 0)
        {
                glong ii, length = pcl_object_measure (closure);
                for (ii = 0; ii < length; ++ii)
                        /* Use sequence API so item gets referenced. */
                        free_slots[ii] = pcl_sequence_get_item (closure, ii);
        }

        if (flags & PCL_CODE_FLAG_GENERATOR)
        {
                /* Don't need to keep the reference to frame->previous.
                 * It will be set when the generator is resumed. */
                if (frame->previous != NULL)
                {
                        pcl_object_unref (frame->previous);
                        frame->previous = NULL;
                }

                /* Create a new generator that owns the ready-to-run
                 * frame and return that as the value. */
                retval = pcl_generator_new (frame);
        }
        else
        {
                retval = pcl_eval_frame (frame);
        }

fail:
        g_assert (ts != NULL);
        ++ts->recursion_depth;
        pcl_object_unref (frame);
        --ts->recursion_depth;
        return retval;
}

PclFrame *
pcl_eval_get_frame (void)
{
        PclThreadState *ts = pcl_thread_state_get ();
        return pcl_thread_state_get_frame (ts);
}

PclObject *
pcl_eval_get_builtins (void)
{
        PclFrame *current_frame = pcl_eval_get_frame ();
        return current_frame ? current_frame->builtins :
               pcl_thread_state_get ()->is->builtins;
}

PclObject *
pcl_eval_get_globals (void)
{
        PclFrame *current_frame = pcl_eval_get_frame ();
        return current_frame ? current_frame->globals : NULL;
}

PclObject *
pcl_eval_get_locals (void)
{
        PclFrame *current_frame = pcl_eval_get_frame ();
        pcl_frame_fast_to_locals (current_frame);
        return current_frame ? current_frame->locals : NULL;
}

PclObject *
pcl_eval_call_object_with_keywords (PclObject *object, PclObject *arguments,
                                    PclObject *keywords)
{
        /* XXX Not sure why this is here and not in PclCallable */

        PclObject *result;

        if (arguments == NULL)
                arguments = pcl_tuple_new (0);
        else if (!PCL_IS_TUPLE (arguments))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "argument list must be a tuple");
                return NULL;
        }
        else
                pcl_object_ref (arguments);

        if (keywords != NULL && !PCL_IS_DICT (keywords))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "keyword list must be a dictionary");
                pcl_object_unref (arguments);
                return NULL;
        }

        result = pcl_object_call (object, arguments, keywords);
        pcl_object_unref (arguments);
        return result;
}

PclObject *
pcl_eval_call_function (PclFunction *function, const gchar *format, ...)
{
        va_list va;
        PclObject *arguments;
        PclObject *result;

        va_start (va, format);
        arguments = pcl_va_build_value (format, va);
        va_end (va);

        if (arguments == NULL)
                return NULL;

        result = pcl_eval_call_object (PCL_OBJECT (function), arguments);
        pcl_object_unref (arguments);

        return result;
}

const gchar *
pcl_eval_get_func_name (PclObject *func)
{
        /* XXX Needs more work */
        if (PCL_IS_FUNCTION (func))
                return pcl_string_as_string (PCL_FUNCTION_GET_NAME (func));
        if (PCL_IS_METHOD (func))
                return PCL_METHOD_GET_NAME (func);
        return PCL_GET_TYPE_NAME (func);
}

const gchar *
pcl_eval_get_func_desc (PclObject *func)
{
        /* XXX Needs more work */
        if (PCL_IS_FUNCTION (func))
                return "()";
        if (PCL_IS_METHOD (func))
                return "()";
        return " object";
}

void
pcl_eval_set_profile (PclTraceFunc function, PclObject *object)
{
        PclThreadState *ts = pcl_thread_state_get ();
        PclObject *tmp = ts->profile_object;

        /* Thread-safe */
        if (object != NULL)
                pcl_object_ref (object);
        ts->profile_function = NULL;
        ts->profile_object = NULL;
        ts->use_tracing = (ts->trace_function != NULL);
        if (tmp != NULL)
                pcl_object_unref (tmp);
        ts->profile_function = function;
        ts->profile_object = object;
        ts->use_tracing = (function != NULL) || (ts->trace_function != NULL);
}

void
pcl_eval_set_trace (PclTraceFunc function, PclObject *object)
{
        PclThreadState *ts = pcl_thread_state_get ();
        PclObject *tmp = ts->trace_object;

        /* Thread-safe */
        if (object != NULL)
                pcl_object_ref (object);
        ts->trace_function = NULL;
        ts->trace_object = NULL;
        ts->use_tracing = (ts->profile_function != NULL);
        if (tmp != NULL)
                pcl_object_unref (tmp);
        ts->trace_function = function;
        ts->trace_object = object;
        ts->use_tracing = (function != NULL) || (ts->profile_function != NULL);
}

#ifdef DYNAMIC_EXECUTION_PROFILE

static PclObject *
eval_get_array (glong array[256])
{
        glong ii;
        PclObject *list = pcl_list_new (256);
        if (list == NULL)
                return NULL;
        for (ii = 0; ii < 256; ii++)
        {
                PclObject *item = pcl_int_from_long (array[ii]);
                if (item == NULL)
                {
                        pcl_object_unref (list);
                        return NULL;
                }
                pcl_list_set_item (list, ii, item);
        }
        for (ii = 0; ii < 256; ii++)
                array[ii] = 0;
        return list;
}

static PclObject *
pcl_get_dx_profile (PclObject *self, PclObject *unused)
{
#ifndef DXPAIRS
        return eval_get_array (dxp);
#else
        glong ii;
        PclObject *list = pcl_list_new (257);
        if (list == NULL)
                return NULL;
        for (ii = 0; ii < 257; ii++)
        {
                PclObject *item = eval_get_array (dxpairs[ii]);
                if (item == NULL)
                {
                        pcl_object_unref (list);
                        return NULL;
                }
                pcl_list_set_item (list, ii, item);
        }
        return list;
#endif
}

#endif /* DYNAMIC_EXECUTION_PROFILE */
