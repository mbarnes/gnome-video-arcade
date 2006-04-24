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

PclObject *
pcl_member_get (guint8 *address, PclMemberDef *memdef)
{
        PclObject *object;

        address += memdef->offset;
        switch (memdef->type)
        {
                case G_TYPE_CHAR:
                        object = pcl_int_from_long (
                                (glong) *(gchar *) address);
                        break;
                case G_TYPE_UCHAR:
                        object = pcl_int_from_long (
                                (glong) *(guchar *) address);
                        break;
                case G_TYPE_BOOLEAN:
                        object = pcl_bool_from_boolean (
                                *(gboolean *) address);
                        break;
                case G_TYPE_INT:
                        object = pcl_int_from_long (
                                (glong) *(gint *) address);
                        break;
                case G_TYPE_UINT:
                        object = pcl_int_from_long (
                                (glong) *(guint *) address);
                        break;
                case G_TYPE_LONG:
                        object = pcl_int_from_long (
                                *(glong *) address);
                        break;
                case G_TYPE_ULONG:
                        /* XXX Python does PyLong_FromDouble here. */
                        object = pcl_int_from_long (
                                (glong) *(gulong *) address);
                        break;
                case G_TYPE_FLOAT:
                        object = pcl_float_from_double (
                                (gdouble) *(gfloat *) address);
                        break;
                case G_TYPE_DOUBLE:
                        object = pcl_float_from_double (
                                *(gdouble *) address);
                        break;
                case G_TYPE_STRING:
                        if (*(gchar **) address == NULL)
                                object = pcl_object_ref (PCL_NONE);
                        else
                                object = pcl_string_from_string (
                                        *(gchar **) address);
                        break;
                case G_TYPE_OBJECT:
                        object = *(PclObject **) address;
                        if (object == NULL)
                                object = PCL_NONE;
                        pcl_object_ref (object);
                        break;
                default:
                        pcl_error_set_string (
                                pcl_exception_system_error (),
                                "bad PclMemberDef type");
                        object = NULL;
        }
        return object;
}

gboolean
pcl_member_set (guint8 *address, PclMemberDef *memdef, PclObject *value)
{
        PclObject *old_value;

        if ((memdef->flags & PCL_MEMBER_FLAG_READONLY) ||
                (memdef->type == G_TYPE_STRING))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "readonly attribute");
                return FALSE;
        }

        if (value == NULL && memdef->type != G_TYPE_OBJECT)
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "can't delete numeric/char attribute");
                return FALSE;
        }

        address += memdef->offset;
        switch (memdef->type)
        {
                case G_TYPE_CHAR:
                case G_TYPE_UCHAR:
                        if (!PCL_IS_INT (value))
                        {
                                pcl_error_bad_argument ();
                                return FALSE;
                        }
                        *(gchar *) address = (gchar) pcl_int_as_long (value);
                        break;
                case G_TYPE_BOOLEAN:
                        *(gboolean *) address = pcl_object_is_true (value);
                        break;
                case G_TYPE_INT:
                case G_TYPE_UINT:
                        if (!PCL_IS_INT (value))
                        {
                                pcl_error_bad_argument ();
                                return FALSE;
                        }
                        *(gint *) address = (gint) pcl_int_as_long (value);
                        break;
                case G_TYPE_LONG:
                case G_TYPE_ULONG:
                        /* XXX A G_TYPE_ULONG may require a PclLong object
                         *     (when I get around to implementing them). */
                        if (!PCL_IS_INT (value))
                        {
                                pcl_error_bad_argument ();
                                return FALSE;
                        }
                        *(glong *) address = pcl_int_as_long (value);
                        break;
                case G_TYPE_FLOAT:
                        if (PCL_IS_INT (value))
                                *(gfloat *) address =
                                        (gfloat) pcl_int_as_long (value);
                        else if (PCL_IS_FLOAT (value))
                                *(gfloat *) address =
                                        (gfloat) pcl_float_as_double (value);
                        else
                        {
                                pcl_error_bad_argument ();
                                return FALSE;
                        }
                        break;
                case G_TYPE_DOUBLE:
                        if (PCL_IS_INT (value))
                                *(gdouble *) address =
                                        (gdouble) pcl_int_as_long (value);
                        else if (PCL_IS_FLOAT (value))
                                *(gdouble *) address =
                                        pcl_float_as_double (value);
                        else
                        {
                                pcl_error_bad_argument ();
                                return FALSE;
                        }
                        break;
                case G_TYPE_OBJECT:
                        if (value != NULL)
                                pcl_object_ref (value);
                        old_value = *(PclObject **) address;
                        *(PclObject **) address = value;
                        if (old_value != NULL)
                                pcl_object_unref (old_value);
                        break;
                default:
                        pcl_error_set_format (
                                pcl_exception_system_error (),
                                "bad PclMemberDef type for '%s'",
                                memdef->name);
                        return FALSE;
        }
        return TRUE;
}
