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

static gchar exceptions_doc[] =
"PCL's standard exception class hierarchy.\n";

static PclMethodDef exceptions_methods[] = {
        { NULL }
};

PclObject *
_pcl_exceptions_init (void)
{
        PclObject *exc_mod = pcl_module_init ("exceptions",
                                              exceptions_methods,
                                              exceptions_doc);
        PclObject *exc_dict = pcl_module_get_dict (exc_mod);
        PclObject *bin_mod = pcl_import_module ("__builtin__");
        PclObject *bin_dict = pcl_module_get_dict (bin_mod);
        PclObject *exc;

        pcl_object_unref (bin_mod);

        /* XXX Might be easier to populate exceptions.__dict__ and then do
         *     __builtin__.__dict__.update(exceptions.__dict__)
         *     That would require a dict.update() method. */

        /* exceptions */
        exc = pcl_exception_assertion_error ();
        pcl_dict_set_item_string (exc_dict, "AssertionError", exc);
        pcl_dict_set_item_string (bin_dict, "AssertionError", exc);

        exc = pcl_exception_attribute_error ();
        pcl_dict_set_item_string (exc_dict, "AttributeError", exc);
        pcl_dict_set_item_string (bin_dict, "AttributeError", exc);

        exc = pcl_exception_eof_error ();
        pcl_dict_set_item_string (exc_dict, "EOFError", exc);
        pcl_dict_set_item_string (bin_dict, "EOFError", exc);

        exc = pcl_exception_floating_point_error ();
        pcl_dict_set_item_string (exc_dict, "FloatingPointError", exc);
        pcl_dict_set_item_string (bin_dict, "FloatingPointError", exc);

        exc = pcl_exception_io_error ();
        pcl_dict_set_item_string (exc_dict, "IOError", exc);
        pcl_dict_set_item_string (bin_dict, "IOError", exc);

        exc = pcl_exception_import_error ();
        pcl_dict_set_item_string (exc_dict, "ImportError", exc);
        pcl_dict_set_item_string (bin_dict, "ImportError", exc);

        exc = pcl_exception_index_error ();
        pcl_dict_set_item_string (exc_dict, "IndexError", exc);
        pcl_dict_set_item_string (bin_dict, "IndexError", exc);

        exc = pcl_exception_key_error ();
        pcl_dict_set_item_string (exc_dict, "KeyError", exc);
        pcl_dict_set_item_string (bin_dict, "KeyError", exc);

        exc = pcl_exception_keyboard_interrupt ();
        pcl_dict_set_item_string (exc_dict, "KeyboardInterrupt", exc);
        pcl_dict_set_item_string (bin_dict, "KeyboardInterrupt", exc);

        exc = pcl_exception_memory_error ();
        pcl_dict_set_item_string (exc_dict, "MemoryError", exc);
        pcl_dict_set_item_string (bin_dict, "MemoryError", exc);

        exc = pcl_exception_name_error ();
        pcl_dict_set_item_string (exc_dict, "NameError", exc);
        pcl_dict_set_item_string (bin_dict, "NameError", exc);

        exc = pcl_exception_not_implemented_error ();
        pcl_dict_set_item_string (exc_dict, "NotImplementedError", exc);
        pcl_dict_set_item_string (bin_dict, "NotImplementedError", exc);

        exc = pcl_exception_os_error ();
        pcl_dict_set_item_string (exc_dict, "OSError", exc);
        pcl_dict_set_item_string (bin_dict, "OSError", exc);

        exc = pcl_exception_overflow_error ();
        pcl_dict_set_item_string (exc_dict, "OverflowError", exc);
        pcl_dict_set_item_string (bin_dict, "OverflowError", exc);

        exc = pcl_exception_reference_error ();
        pcl_dict_set_item_string (exc_dict, "ReferenceError", exc);
        pcl_dict_set_item_string (bin_dict, "ReferenceError", exc);

        exc = pcl_exception_runtime_error ();
        pcl_dict_set_item_string (exc_dict, "RuntimeError", exc);
        pcl_dict_set_item_string (bin_dict, "RuntimeError", exc);

        exc = pcl_exception_stop_iteration ();
        pcl_dict_set_item_string (exc_dict, "StopIteration", exc);
        pcl_dict_set_item_string (bin_dict, "StopIteration", exc);

        exc = pcl_exception_syntax_error ();
        pcl_dict_set_item_string (exc_dict, "SyntaxError", exc);
        pcl_dict_set_item_string (bin_dict, "SyntaxError", exc);

        exc = pcl_exception_system_error ();
        pcl_dict_set_item_string (exc_dict, "SystemError", exc);
        pcl_dict_set_item_string (bin_dict, "SystemError", exc);

        exc = pcl_exception_system_exit ();
        pcl_dict_set_item_string (exc_dict, "SystemExit", exc);
        pcl_dict_set_item_string (bin_dict, "SystemExit", exc);

        exc = pcl_exception_type_error ();
        pcl_dict_set_item_string (exc_dict, "TypeError", exc);
        pcl_dict_set_item_string (bin_dict, "TypeError", exc);

        exc = pcl_exception_unbound_local_error ();
        pcl_dict_set_item_string (exc_dict, "UnboundLocalError", exc);
        pcl_dict_set_item_string (bin_dict, "UnboundLocalError", exc);

        exc = pcl_exception_value_error ();
        pcl_dict_set_item_string (exc_dict, "ValueError", exc);
        pcl_dict_set_item_string (bin_dict, "ValueError", exc);

        exc = pcl_exception_zero_division_error ();
        pcl_dict_set_item_string (exc_dict, "ZeroDivisionError", exc);
        pcl_dict_set_item_string (bin_dict, "ZeroDivisionError", exc);

        return exc_mod;
}
