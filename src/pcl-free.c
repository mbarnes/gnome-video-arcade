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

static gpointer free_parent_class = NULL;

static guint
free_hash (PclObject *object)
{
        return (guint) G_OBJECT_TYPE (object);
}

static PclObject *
free_repr (PclObject *object)
{
        return pcl_string_intern_from_string ("Free");
}

static void
free_class_init (PclFreeClass *class)
{
        PclObjectClass *object_class;

        free_parent_class = g_type_class_peek_parent (class);

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_free_get_type_object;
        object_class->hash = free_hash;
        object_class->repr = free_repr;
}

GType
pcl_free_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclFreeClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) free_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclFree),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_OBJECT, "PclFree", &type_info, 0);
        }
        return type;
}

/**
 * pcl_free_get_type_object:
 *
 * Returns the type object for #PclFree.
 *
 * Returns: a borrowed reference to the type object for #PclFree
 */
PclObject *
pcl_free_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_FREE, "FreeType");
                pcl_register_singleton ("<type 'FreeType'>", &object);
        }
        return object;
}

/**
 * pcl_free_get_instance:
 *
 * Returns the singleton instance of #PclFree.  During runtime this is the
 * built-in object %Free.
 *
 * Do not call this function directly.  Instead, use #PCL_FREE.
 *
 * Returns: a borrowed reference to the singleton instance of #PclFree
 */
PclObject *
pcl_free_get_instance (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_object_new (PCL_TYPE_FREE, NULL);
                pcl_register_singleton ("Free", &object);
        }
        return object;
}
