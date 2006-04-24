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

static gchar bool_doc[] =
"bool(x) -> bool\n\
\n\
Returns True when the argument x is true, False otherwise.\n\
The builtins True and False are the only two instances of the class bool.\n\
The class bool is a subclass of the class int, and cannot be subclassed.";

static gpointer bool_parent_class = NULL;

static PclObject *
bool_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        const gchar *kwlist[] = { "x", NULL };
        PclObject *x = PCL_FALSE;

        if (!pcl_arg_parse_tuple_and_keywords (
                        args, kwds, "|O:bool", kwlist, &x))
                return NULL;
        /* XXX pcl_object_is_true() may fail */
        return pcl_bool_from_boolean (pcl_object_is_true (x));
}

static gboolean
bool_print (PclObject *object, FILE *stream, PclPrintFlags flags)
{
        fputs (PCL_INT_AS_LONG (object) ? "True" : "False", stream);
        return TRUE;
}

static PclObject *
bool_repr (PclObject *object)
{
        gboolean v_bool = (PCL_INT_AS_LONG (object) != 0);
        return pcl_string_intern_from_string (v_bool ? "True" : "False");
}

static PclObject *
bool_number_bitwise_or (PclObject *object1, PclObject *object2)
{
        if (!PCL_IS_BOOL (object1) || !PCL_IS_BOOL (object2))
        {
                PclIntClass *class;
                PclNumberIface *iface;

                class = g_type_class_peek_static (PCL_TYPE_INT);
                iface = g_type_interface_peek (class, PCL_TYPE_NUMBER);
                return iface->bitwise_or (object1, object2);
        }
        else
        {
                glong v = PCL_INT_AS_LONG (object1);
                glong w = PCL_INT_AS_LONG (object2);
                return pcl_bool_from_boolean (v | w);
        }
}

static PclObject *
bool_number_bitwise_xor (PclObject *object1, PclObject *object2)
{
        if (!PCL_IS_BOOL (object1) || !PCL_IS_BOOL (object2))
        {
                PclIntClass *class;
                PclNumberIface *iface;

                class = g_type_class_peek_static (PCL_TYPE_INT);
                iface = g_type_interface_peek (class, PCL_TYPE_NUMBER);
                return iface->bitwise_xor (object1, object2);
        }
        else
        {
                glong v = PCL_INT_AS_LONG (object1);
                glong w = PCL_INT_AS_LONG (object2);
                return pcl_bool_from_boolean (v ^ w);
        }
}

static PclObject *
bool_number_bitwise_and (PclObject *object1, PclObject *object2)
{
        if (!PCL_IS_BOOL (object1) || !PCL_IS_BOOL (object2))
        {
                PclIntClass *class;
                PclNumberIface *iface;

                class = g_type_class_peek_static (PCL_TYPE_INT);
                iface = g_type_interface_peek (class, PCL_TYPE_NUMBER);
                return iface->bitwise_and (object1, object2);
        }
        else
        {
                glong v = PCL_INT_AS_LONG (object1);
                glong w = PCL_INT_AS_LONG (object2);
                return pcl_bool_from_boolean (v & w);
        }
}

static void
bool_number_init (PclNumberIface *iface)
{
        iface->bitwise_or = bool_number_bitwise_or;
        iface->bitwise_xor = bool_number_bitwise_xor;
        iface->bitwise_and = bool_number_bitwise_and;
}

static void
bool_class_init (PclBoolClass *class)
{
        PclObjectClass *object_class;

        bool_parent_class = g_type_class_peek_parent (class);

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_bool_get_type_object;
        object_class->new_instance = bool_new_instance;
        object_class->print = bool_print;
        object_class->repr = bool_repr;
        object_class->doc = bool_doc;
}

GType
pcl_bool_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclBoolClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) bool_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclBool),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                static const GInterfaceInfo number_info = {
                        (GInterfaceInitFunc) bool_number_init,
                        (GInterfaceFinalizeFunc) NULL,
                        NULL   /* interface_data */
                };

                type = g_type_register_static (
                        PCL_TYPE_INT, "PclBool", &type_info, 0);
                g_type_add_interface_static (
                        type, PCL_TYPE_NUMBER, &number_info);
        }
        return type;
}

/**
 * pcl_bool_get_type_object:
 *
 * Returns the type object for #PclBool.  During runtime this is the built-in
 * object %bool.
 *
 * Returns: a borrowed reference to the type object for #PclBool
 */
PclObject *
pcl_bool_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_BOOL, "bool");
                pcl_register_singleton ("<type 'bool'>", &object);
        }
        return object;
}

/**
 * pcl_bool_get_instance_true:
 *
 * Returns the singleton instance of #PclBool representing true.  During
 * runtime this is the built-in object %True.
 *
 * Do not call this function directly.  Instead, use #PCL_TRUE.
 *
 * Returns: a borrowed reference to the singleton instance of #PclBool
 *          representing true
 */
PclObject *
pcl_bool_get_instance_true (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_object_new (PCL_TYPE_BOOL, NULL);
                PCL_INT (object)->v_long = (glong) TRUE;
                pcl_register_singleton ("True", &object);
        }
        return object;
}

/**
 * pcl_bool_get_instance_false:
 *
 * Returns the singleton instance of #PclBool representing false.  During
 * runtime this is the built-in object %False.
 *
 * Do not call this function directly.  Instead, use #PCL_FALSE.
 *
 * Returns: a borrowed reference to the singleton instance of #PclBool
 *          representing false
 */
PclObject *
pcl_bool_get_instance_false (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_object_new (PCL_TYPE_BOOL, NULL);
                PCL_INT (object)->v_long = (glong) FALSE;
                pcl_register_singleton ("False", &object);
        }
        return object;
}

/**
 * pcl_bool_from_boolean:
 * @v_boolean: a boolean value
 *
 * Returns a new reference to either the built-in object %True or %False,
 * depending on the value of @v_boolean.
 *
 * Returns: a new reference to %True or %False
 */

PclObject *
pcl_bool_from_boolean (gboolean v_boolean)
{
        return pcl_object_ref (v_boolean ? PCL_TRUE : PCL_FALSE);
}
