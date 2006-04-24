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

static gpointer exception_parent_class = NULL;

static void
exception_finalize (GObject *g_object)
{
        PclException *self = PCL_EXCEPTION (g_object);

        g_free (self->name);

        /* Chain up to parent's finalize method. */
        G_OBJECT_CLASS (exception_parent_class)->finalize (g_object);
}

static void
exception_class_init (PclExceptionClass *class)
{
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        exception_parent_class = g_type_class_peek_parent (class);

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_exception_get_type_object;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->finalize = exception_finalize;
}

GType
pcl_exception_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclExceptionClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) exception_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclException),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_OBJECT, "PclException", &type_info, 0);
        }
        return type;
}

/**
 * pcl_exception_get_type_object:
 *
 * Returns the type object for #PclException.
 *
 * Returns: a borrowed reference to the type object for #PclException
 */
PclObject *
pcl_exception_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_EXCEPTION, "exception");
                pcl_register_singleton ("<type 'exception'>", &object);
        }
        return object;
}

const gchar *
pcl_exception_get_name (PclObject *self)
{
        if (!PCL_IS_EXCEPTION (self))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        return PCL_EXCEPTION (self)->name;
}

#define DEFINE_EXCEPTION(lowercase, titlecase) \
PclObject *pcl_exception_##lowercase (void) \
{ \
        static gpointer object = NULL; \
        if (G_UNLIKELY (object == NULL)) { \
                object = pcl_object_new (PCL_TYPE_EXCEPTION, NULL); \
                PCL_EXCEPTION (object)->name = g_strdup (#titlecase); \
                pcl_register_singleton (#titlecase, &object); } \
        return object; \
}

DEFINE_EXCEPTION (assertion_error,             AssertionError)
DEFINE_EXCEPTION (attribute_error,             AttributeError)
DEFINE_EXCEPTION (eof_error,                   EOFError)
DEFINE_EXCEPTION (floating_point_error,        FloatingPointError)
DEFINE_EXCEPTION (io_error,                    IOError)
DEFINE_EXCEPTION (import_error,                ImportError)
DEFINE_EXCEPTION (index_error,                 IndexError)
DEFINE_EXCEPTION (key_error,                   KeyError)
DEFINE_EXCEPTION (keyboard_interrupt,          KeyboardInterrupt)
DEFINE_EXCEPTION (memory_error,                MemoryError)
DEFINE_EXCEPTION (name_error,                  NameError)
DEFINE_EXCEPTION (not_implemented_error,       NotImplementedError)
DEFINE_EXCEPTION (os_error,                    OSError)
DEFINE_EXCEPTION (overflow_error,              OverflowError)
DEFINE_EXCEPTION (reference_error,             ReferenceError)
DEFINE_EXCEPTION (runtime_error,               RuntimeError)
DEFINE_EXCEPTION (stop_iteration,              StopIteration)
DEFINE_EXCEPTION (syntax_error,                SyntaxError)
DEFINE_EXCEPTION (system_error,                SystemError)
DEFINE_EXCEPTION (system_exit,                 SystemExit)
DEFINE_EXCEPTION (type_error,                  TypeError)
DEFINE_EXCEPTION (unbound_local_error,         UnboundLocalError)
DEFINE_EXCEPTION (value_error,                 ValueError)
DEFINE_EXCEPTION (zero_division_error,         ZeroDivisionError)
