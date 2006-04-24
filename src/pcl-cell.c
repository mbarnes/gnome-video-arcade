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

static gpointer cell_parent_class = NULL;

static PclObject *
cell_clone (PclObject *object)
{
        PclCell *self = PCL_CELL (object);
        PclObject *clone = pcl_cell_new (NULL);
        PclThreadState *ts = pcl_thread_state_get ();

        g_hash_table_insert (ts->cache,
                             pcl_object_ref (object),
                             pcl_object_ref (clone));

        pcl_cell_set (clone, pcl_object_clone (self->reference));

        return clone;
}

static PclObject *
cell_repr (PclObject *object)
{
        PclCell *self = PCL_CELL (object);

        if (self->reference == NULL)
                return pcl_string_from_format (
                                "<cell at %p: empty>",
                                (gpointer) object);
        else
                return pcl_string_from_format (
                                "<cell at %p: %s object at %p>",
                                (gpointer) object,
                                PCL_GET_TYPE_NAME (self->reference),
                                (gpointer) self->reference);
}

static gboolean
cell_traverse (PclContainer *container, PclTraverseFunc func,
               gpointer user_data)
{
        PclCell *self = PCL_CELL (container);

        if (self->reference != NULL)
                if (!func (self->reference, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (cell_parent_class)->
                traverse (container, func, user_data);
}

static void
cell_class_init (PclCellClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;

        cell_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = cell_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_cell_get_type_object;
        object_class->clone = cell_clone;
        object_class->repr = cell_repr;
}

GType
pcl_cell_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclCellClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) cell_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclCell),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclCell", &type_info, 0);
        }
        return type;
}

PclObject *
pcl_cell_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_CELL, "cell");
                pcl_register_singleton ("<type 'cell'>", &object);
        }
        return object;
}

PclObject *
pcl_cell_new (PclObject *reference)
{
        PclCell *cell = pcl_object_new (PCL_TYPE_CELL, NULL);
        if (reference != NULL)
                cell->reference = pcl_object_ref (reference);
        return PCL_OBJECT (cell);
}

PclObject *
pcl_cell_get (PclObject *self)
{
        PclObject *reference;

        if (!PCL_IS_CELL (self))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        reference = PCL_CELL (self)->reference;
        if (reference != NULL)
                pcl_object_ref (reference);
        return reference;
}

gboolean
pcl_cell_set (PclObject *self, PclObject *reference)
{
        PclObject *tmp;

        if (!PCL_IS_CELL (self))
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        tmp = PCL_CELL (self)->reference;
        if (tmp != NULL)
                pcl_object_unref (tmp);
        if (reference != NULL)
                pcl_object_ref (reference);
        PCL_CELL (self)->reference = reference;
        return TRUE;
}
