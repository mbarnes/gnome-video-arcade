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

static gpointer ellipsis_parent_class = NULL;

static guint
ellipsis_hash (PclObject *object)
{
        return (guint) G_OBJECT_TYPE (object);
}

static PclObject *
ellipsis_repr (PclObject *object)
{
        return pcl_string_intern_from_string ("Ellipsis");
}

static void
ellipsis_class_init (PclEllipsisClass *class)
{
        PclObjectClass *object_class;

        ellipsis_parent_class = g_type_class_peek_parent (class);

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_ellipsis_get_type_object;
        object_class->hash = ellipsis_hash;
        object_class->repr = ellipsis_repr;
}

GType
pcl_ellipsis_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclEllipsisClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) ellipsis_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclEllipsis),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_OBJECT, "PclEllipsis", &type_info, 0);
        }
        return type;
}

/**
 * pcl_ellipsis_get_type_object:
 *
 * Returns the type object for #PclEllipsis.
 *
 * Returns: a borrowed reference to the type object for #PclEllipsis
 */
PclObject *
pcl_ellipsis_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_ELLIPSIS, "EllipsisType");
                pcl_register_singleton ("<type 'EllipsisType'>", &object);
        }
        return object;
}

/**
 * pcl_ellipsis_get_instance:
 *
 * Returns the singleton instance of #PclEllipsis.  During runtime this is the
 * built-in object %Ellipsis.
 *
 * Do not call this function directly.  Instead, use #PCL_ELLIPSIS.
 *
 * Returns: a borrowed reference to the singleton instance of #PclEllipsis
 */
PclObject *
pcl_ellipsis_get_instance (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_object_new (PCL_TYPE_ELLIPSIS, NULL);
                pcl_register_singleton ("Ellipsis", &object);
        }
        return object;
}
