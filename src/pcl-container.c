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

static gpointer container_parent_class = NULL;

static void
container_finalize (GObject *g_object)
{
        PclContainer *self = PCL_CONTAINER (g_object);

        pcl_gc_list_remove (&self->gc);

        /* Chain up to parent's finalize method. */
        G_OBJECT_CLASS (container_parent_class)->finalize (g_object);
}

static gboolean
container_traverse (PclContainer *container, PclTraverseFunc func,
                    gpointer user_data)
{
        return TRUE;
}

static void
container_class_init (PclContainerClass *class)
{
        GObjectClass *g_object_class;

        container_parent_class = g_type_class_peek_parent (class);

        class->traverse = container_traverse;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->finalize = container_finalize;
}

static void
container_init (PclContainer *container)
{
        container->gc.object = container;
        pcl_gc_list_insert (&container->gc);
}

GType
pcl_container_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclContainerClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) container_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclContainer),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) container_init,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_OBJECT, "PclContainer", &type_info,
                        G_TYPE_FLAG_ABSTRACT);
        }
        return type;
}

gboolean
pcl_object_traverse (PclObject *object, PclTraverseFunc func,
                     gpointer user_data)
{
        PclContainerClass *class;

        if (object == NULL || func == NULL)
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        class = PCL_CONTAINER_GET_CLASS (object);
        if (class != NULL && class->traverse != NULL)
        {
                PclContainer *container = PCL_CONTAINER (object);
                return class->traverse (container, func, user_data);
        }

        return TRUE;
}
