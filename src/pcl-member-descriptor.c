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

static gpointer member_descriptor_parent_class = NULL;

static PclObject *
member_descriptor_repr (PclObject *object)
{
        PclDescriptor *descriptor = PCL_DESCRIPTOR (object);

        return pcl_string_from_format (
                "<member '%s' of '%s' objects>",
                PCL_IS_STRING (descriptor->name) ?
                PCL_STRING_AS_STRING (descriptor->name) : "?",
                PCL_TYPE (descriptor->type)->name);
}

static PclObject *
member_descriptor_get (PclDescriptor *descriptor,
                       PclObject *object, PclObject *type)
{
        PclMemberDescriptor *self = PCL_MEMBER_DESCRIPTOR (descriptor);

        if (object == NULL)
                return pcl_object_ref (descriptor);
        if (PCL_GET_TYPE_OBJECT (object) != descriptor->type)
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "descriptor '%s' for '%s' objects "
                        "doesn't apply to '%s' object",
                        PCL_IS_STRING (descriptor->name) ?
                        PCL_STRING_AS_STRING (descriptor->name) : "?",
                        PCL_TYPE (descriptor->type)->name,
                        PCL_GET_TYPE_NAME (object));
                return NULL;
        }

        return pcl_member_get ((guint8 *) object, self->member);
}

static gboolean
member_descriptor_set (PclDescriptor *descriptor,
                       PclObject *object, PclObject *value)
{
        PclMemberDescriptor *self = PCL_MEMBER_DESCRIPTOR (descriptor);

        g_assert (object != NULL);
        if (!pcl_object_is_instance (object, descriptor->type))
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "descriptor '%s' for '%s' objects "
                        "doesn't apply to '%s' object",
                        PCL_IS_STRING (descriptor->name) ?
                        PCL_STRING_AS_STRING (descriptor->name) : "?",
                        PCL_TYPE (descriptor->type)->name,
                        PCL_GET_TYPE_NAME (object));
                return FALSE;
        }

        return pcl_member_set ((guint8 *) object, self->member, value);
}

static PclObject *
member_descriptor_get_doc (PclMemberDescriptor *self, gpointer context)
{
        if (self->member->doc == NULL)
                return pcl_object_ref (PCL_NONE);
        return pcl_string_from_string (self->member->doc);
}

static void
member_descriptor_class_init (PclMemberDescriptorClass *class)
{
        PclDescriptorClass *descriptor_class;
        PclObjectClass *object_class;

        member_descriptor_parent_class = g_type_class_peek_parent (class);

        descriptor_class = PCL_DESCRIPTOR_CLASS (class);
        descriptor_class->get = member_descriptor_get;
        descriptor_class->set = member_descriptor_set;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_member_descriptor_get_type_object;
        object_class->repr = member_descriptor_repr;
}

static PclGetSetDef member_descriptor_getsets[] = {
        { "__doc__",            (PclGetFunc) member_descriptor_get_doc,
                                (PclSetFunc) NULL },
        { NULL }
};

GType
pcl_member_descriptor_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclMemberDescriptorClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) member_descriptor_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclMemberDescriptor),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_DESCRIPTOR, "PclMemberDescriptor",
                        &type_info, 0);
                g_type_set_qdata (
                        type, PCL_DATA_GETSETS, member_descriptor_getsets);
        }
        return type;
}

PclObject *
pcl_member_descriptor_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (
                        PCL_TYPE_MEMBER_DESCRIPTOR, "member_descriptor");
                pcl_register_singleton (
                        "<type 'member_descriptor'>", &object);
        }
        return object;
}
