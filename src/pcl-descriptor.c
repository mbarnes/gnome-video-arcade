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

static gpointer descriptor_parent_class = NULL;

static void
descriptor_dispose (GObject *g_object)
{
        PclDescriptor *self = PCL_DESCRIPTOR (g_object);

        PCL_CLEAR (self->type);
        PCL_CLEAR (self->name);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (descriptor_parent_class)->dispose (g_object);
}

static gboolean
descriptor_traverse (PclContainer *container, PclTraverseFunc func,
                     gpointer user_data)
{
        PclDescriptor *self = PCL_DESCRIPTOR (container);

        if (self->type != NULL)
                if (!func (self->type, user_data))
                        return FALSE;
        if (self->name != NULL)
                if (!func (self->name, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (descriptor_parent_class)->
                traverse (container, func, user_data);
}

static void
descriptor_class_init (PclDescriptorClass *class)
{
        PclContainerClass *container_class;
        GObjectClass *g_object_class;

        descriptor_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = descriptor_traverse;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = descriptor_dispose;
}

static PclMemberDef descriptor_members[] = {
        { "__objclass__",       G_TYPE_OBJECT,
                                G_STRUCT_OFFSET (PclDescriptor, type),
                                PCL_MEMBER_FLAG_READONLY },
        { "__name__",           G_TYPE_OBJECT,
                                G_STRUCT_OFFSET (PclDescriptor, name),
                                PCL_MEMBER_FLAG_READONLY },
        { NULL }
};

GType
pcl_descriptor_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclDescriptorClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) descriptor_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclDescriptor),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclDescriptor", &type_info,
                        G_TYPE_FLAG_ABSTRACT);
                g_type_set_qdata (
                        type, PCL_DATA_MEMBERS, descriptor_members);
        }
        return type;
}
