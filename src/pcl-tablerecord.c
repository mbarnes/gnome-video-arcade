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

static gpointer tablerecord_parent_class = NULL;

static void
tablerecord_dispose (GObject *g_object)
{
        PclTableRecord *self = PCL_TABLE_RECORD (g_object);

        PCL_CLEAR (self->data);
        PCL_CLEAR (self->fields);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (tablerecord_parent_class)->dispose (g_object);
}

static PclObject *
tablerecord_clone (PclObject *object)
{
        PclTableRecord *self = PCL_TABLE_RECORD (object);
        PclTableRecord *clone = pcl_object_new (PCL_TYPE_TABLE_RECORD, NULL);
        PclThreadState *ts = pcl_thread_state_get ();
 
        g_hash_table_insert (ts->cache, 
                             pcl_object_ref (self), 
                             pcl_object_ref (clone));
 
        if (self->data != NULL)
                clone->data = pcl_object_clone (self->data);
        if (self->fields != NULL)
                clone->fields = pcl_object_clone (self->fields);
                
        clone->num_keys = self->num_keys;

        return PCL_OBJECT (clone);
}

static gboolean
tablerecord_print (PclObject *object, FILE *stream, PclPrintFlags flags)
{
        PclTableRecord *self = PCL_TABLE_RECORD (object);
        return pcl_object_print (self->data, stream, flags);
}

static PclObject *
tablerecord_copy (PclObject *object)
{
        PclTableRecord *self = PCL_TABLE_RECORD (object);
        return pcl_tablerecord_new (self->data, self->fields, self->num_keys);
}

static PclObject *
tablerecord_repr (PclObject *object)
{
        PclTableRecord *self = PCL_TABLE_RECORD (object);
        return pcl_object_repr (self->data);
}

static PclObject *
tablerecord_str (PclObject *object)
{
        PclTableRecord *self = PCL_TABLE_RECORD (object);
        return pcl_object_str (self->data);
}

static gint
tablerecord_compare (PclObject *object1, PclObject *object2)
{
        if (!PCL_IS_TABLE_RECORD (object1) || !PCL_IS_TABLE_RECORD (object2))
                return 2;  /* undefined */
        return pcl_object_compare (
                PCL_TABLE_RECORD (object1)->data,
                PCL_TABLE_RECORD (object2)->data);
}

static gint
tablerecord_contains (PclObject *object, PclObject *value)
{
        PclTableRecord *self = PCL_TABLE_RECORD (object);
        return pcl_object_contains (self->data, value);
}

static PclObject *
tablerecord_iterate (PclObject *object)
{
        PclTableRecord *self = PCL_TABLE_RECORD (object);
        return pcl_object_iterate (self->data);
}

static glong
tablerecord_measure (PclObject *object)
{
        PclTableRecord *self = PCL_TABLE_RECORD (object);
        return pcl_object_measure (self->data);
}

static PclObject *
tablerecord_get_item (PclObject *object, PclObject *subscript)
{
        PclTableRecord *self = PCL_TABLE_RECORD (object);
        glong index = pcl_sequence_index (self->fields, subscript);

        if (index < 0)
        {
                pcl_error_clear ();
                pcl_error_set_object (
                        pcl_exception_key_error (),
                        subscript);
        }

        return pcl_sequence_get_item (self->data, index);
}

static gboolean
tablerecord_traverse (PclContainer *container, PclTraverseFunc func,
                      gpointer user_data)
{
        PclTableRecord *self = PCL_TABLE_RECORD (container);

        if (self->data != NULL)
                if (!func (self->data, user_data))
                        return FALSE;
        if (self->fields != NULL)
                if (!func (self->fields, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (tablerecord_parent_class)->
                traverse (container, func, user_data);
}

static void
tablerecord_class_init (PclTableRecordClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        tablerecord_parent_class = g_type_class_peek_parent (class);
 
        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = tablerecord_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_tablerecord_get_type_object;
        object_class->clone = tablerecord_clone;
        object_class->print = tablerecord_print;
        object_class->copy = tablerecord_copy;
        object_class->repr = tablerecord_repr;
        object_class->str = tablerecord_str;
        object_class->compare = tablerecord_compare;
        object_class->contains = tablerecord_contains;
        object_class->iterate = tablerecord_iterate;
        object_class->measure = tablerecord_measure;
        object_class->get_item = tablerecord_get_item;
        
        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = tablerecord_dispose;
}

GType
pcl_tablerecord_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclTableRecordClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) tablerecord_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclTableRecord),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclTableRecord", &type_info, 0);
        }
        return type;
}

/**
 * pcl_tablerecord_get_type_object:
 *
 * Returns the type object for #PclTableRecord.
 *
 * Returns: a borrowed reference to the type object for #PclTableRecord
 */
PclObject *
pcl_tablerecord_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_TABLE_RECORD, "tablerecord");
                pcl_register_singleton ("<type 'tablerecord'>", &object);
        }
        return object;
}

PclObject *
pcl_tablerecord_new (PclObject *data, PclObject *fields, glong num_keys)
{
        PclTableRecord *table_record;
        
        if (!PCL_IS_LIST (data) || !PCL_IS_TUPLE (fields) ||
                num_keys > pcl_object_measure (fields))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }
        
        table_record = pcl_object_new (PCL_TYPE_TABLE_RECORD, NULL);
        table_record->data = pcl_object_ref (data);
        table_record->fields = pcl_object_ref (fields);
        table_record->num_keys = num_keys;
        return PCL_OBJECT (table_record);
}

PclObject *
pcl_tablerecord_extract_key (PclObject *object)
{
        PclTableRecord *self = PCL_TABLE_RECORD (object);
        PclObject *key, *item;
        glong ii;

        if (!PCL_IS_TABLE_RECORD (object))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        key = pcl_tuple_new (self->num_keys);
        if (key == NULL)
                return NULL;

        for (ii = 0; ii < self->num_keys; ii++)
        {
                item = PCL_LIST_GET_ITEM (self->data, ii);
                if (item == NULL || item == PCL_FREE)
                {
                        pcl_error_set_string (
                                pcl_exception_key_error (),
                                "record has an incomplete key");
                        pcl_object_unref (key);
                        return NULL;
                }
                PCL_TUPLE_SET_ITEM (key, ii, pcl_object_ref (item));
        }
        return key;
}
