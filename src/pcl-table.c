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

static gchar table_doc[] =
"XXX Document me!";

static gpointer table_parent_class = NULL;

static PclObject *
table_record_build (PclTable *self, PclObject *args, 
                    PclObject *kwds, PclObject *fill)
{
        PclObject *record = NULL, *value, *key;
        PclObject *record_list = NULL, *iterator, *next;
        glong index, i;
        
        if (fill == NULL)
                fill = PCL_NONE;

        /* Iterating through the argument tuple */
        if (args != NULL)
        {
                glong n_args = pcl_object_measure (args);

                if (n_args > self->num_fields)
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "too many arguments");
                        return NULL;
                }
                iterator = pcl_object_iterate (args);
                if ((next = pcl_iterator_next (iterator)) != NULL)
                {
                        if (PCL_IS_TABLE_RECORD (next))
                        {
                                if (PCL_TABLE_RECORD (next)->fields != 
                                        self->all_fields)
                                {
                                        pcl_error_set_string (
                                                pcl_exception_value_error (),
                                                "foreign record argument");
                                        pcl_object_unref (iterator);
                                        pcl_object_unref (next);
                                        return NULL;
                                }
                                if (n_args != 1 || kwds != NULL)
                                {
                                        pcl_error_set_string (
                                                pcl_exception_type_error (),
                                                "arguments after record arg");
                                        pcl_object_unref (iterator);
                                        pcl_object_unref (next);
                                        return NULL;
                                }
                                pcl_object_unref (iterator);
                                /* return steals the reference to next */
                                return next;
                        }
                        else
                        {
                                record_list = pcl_list_new (self->num_fields);
                                /* Keys default to NULL, others to fill */
                                for (i = self->num_keys; 
                                        i < self->num_fields; i++)
                                {
                                        pcl_object_ref (fill);
                                        PCL_LIST_SET_ITEM (record_list,i,fill);
                                }
                                record = pcl_tablerecord_new (
                                        record_list, self->all_fields,
                                        self->num_keys);
                                index = 0;
                                do
                                {
                                        if (pcl_object_hash (next) ==
                                                PCL_HASH_INVALID)
                                        {
                                                pcl_object_unref (next);
                                                pcl_object_unref (record_list);
                                                pcl_object_unref (record);
                                                pcl_object_unref (iterator);
                                                return NULL;
                                        }
                                
                                        /* Steals reference to next */
                                        pcl_list_set_item (
                                                record_list, index++, next);
                                }
                                while ((next = pcl_iterator_next (iterator)) != 
                                        NULL);
                        }
                }
                pcl_object_unref (iterator);
        }
        if (record == NULL)
        {
                record_list = pcl_list_new (self->num_fields);
                /* Keys default to NULL, others to fill */
                for (i = self->num_keys; i < self->num_fields; i++)
                {
                        pcl_object_ref (fill);
                        PCL_LIST_SET_ITEM (record_list, i, fill);
                }
                record = pcl_tablerecord_new (record_list,
                                              self->all_fields,
                                              self->num_keys);
        }
        /* Iterating through the keywords dictionary */
        if (kwds)
        {
                glong pos = 0;

                while (pcl_dict_next (kwds, &pos, &key, &value))
                {
                        index = pcl_sequence_index (self->all_fields, key);
                
                        if (index < 0)
                        {
                                pcl_error_clear ();
                                pcl_error_set_format (
                                        pcl_exception_key_error (),
                                        "%s is not a field name",
                                        pcl_object_repr (key));
                                pcl_object_unref (record_list);
                                pcl_object_unref (record);
                                return NULL;
                        }
                        if (pcl_object_hash (value) == PCL_HASH_INVALID)
                        {
                                pcl_object_unref (record_list);
                                pcl_object_unref (record);
                                return NULL;
                        }
                        /* steals the reference */
                        pcl_object_ref (value);
                        if (!pcl_list_set_item (record_list, index, value))
                        {
                                pcl_object_unref (record_list);
                                pcl_object_unref (record);
                                return NULL;
                        }
                }
        }
        pcl_object_unref (record_list);
        return record;
}

static void
table_dispose (GObject *g_object)
{
        PclTable *self = PCL_TABLE (g_object);

        PCL_CLEAR (self->key_fields);
        PCL_CLEAR (self->all_fields);
        PCL_CLEAR (self->records);
        PCL_CLEAR (self->lookups);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (table_parent_class)->dispose (g_object);
}

static PclObject *
table_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        PclObject *table;
        PclObject *keys = NULL;
        PclObject *data = NULL;
 
        if (!pcl_arg_parse_tuple (args, "O|O:table", &keys, &data))
                return NULL;

        if (PCL_IS_TABLE (keys))
                return pcl_object_clone (keys);
        else if (PCL_IS_STRING (keys))
        {
                keys = pcl_build_value ("(O)", keys);
                if (keys == NULL)
                        return NULL;
        }
        else
                pcl_object_ref (keys);
 
        if (data == NULL)
        {
                data = pcl_tuple_new (0);
                if (data == NULL)
                {
                        pcl_object_unref (keys);
                        return NULL;
                }
        }
        else if (PCL_IS_STRING (data))
        {
                data = pcl_build_value ("(O)", data);
                if (data == NULL)
                {
                        pcl_object_unref (keys);
                        return NULL;
                }
        }
        else
                pcl_object_ref (data);
 
        table = pcl_table_new (keys, data);
        pcl_object_unref (data);
        pcl_object_unref (keys);
        return table;
}

static PclObject *
table_clone (PclObject *object)
{
        PclTable *self = PCL_TABLE (object);
        PclTable *clone = pcl_object_new (PCL_TYPE_TABLE, NULL);
        PclThreadState *ts = pcl_thread_state_get ();

        g_hash_table_insert (
                ts->cache,
                pcl_object_ref (object),
                pcl_object_ref (clone));
                             
        clone->num_keys = self->num_keys;
        clone->num_fields = self->num_fields;

        if (self->key_fields != NULL)
                clone->key_fields = pcl_object_clone (self->key_fields);
        if (self->all_fields != NULL)
                clone->all_fields = pcl_object_clone (self->all_fields);
        if (self->records != NULL)
                clone->records = pcl_object_clone (self->records);
        if (self->lookups != NULL)
                clone->lookups = pcl_object_clone (self->lookups);
        
        return PCL_OBJECT (clone);
}

static gboolean
table_print (PclObject *object, FILE *stream, PclPrintFlags flags)
{
        PclTable *self = PCL_TABLE (object);
        PclObject *iterator;
        PclObject *next;

        if (!(flags & PCL_PRINT_FLAG_RAW))
        {
                PclObject* repr = pcl_object_repr (object);
                fprintf (stream, pcl_string_as_string (repr));
                pcl_object_unref (repr);
                return TRUE;
        }

        iterator = pcl_object_iterate (self->all_fields);
        if (iterator == NULL)
                return FALSE;
        
        fprintf (stream, "<");
        if ((next = pcl_iterator_next (iterator)) != NULL)
        {
                if  (!pcl_object_print (next, stream, 0))
                {
                        pcl_object_unref (iterator);
                        pcl_object_unref (next);
                        return FALSE;
                }
                pcl_object_unref (next);

                while ((next = pcl_iterator_next (iterator)) != NULL)
                {
                        fprintf (stream, ", ");
                        if (!pcl_object_print (next, stream, 0))
                        {
                                pcl_object_unref (iterator);
                                pcl_object_unref (next);
                                return FALSE;
                        }
                        pcl_object_unref (next);
                }
        }
        fprintf (stream, ">");
        pcl_object_unref (iterator);

        if (pcl_error_occurred ())
                return FALSE;
        
        iterator = pcl_mapping_values (self->records);
        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                fprintf (stream, "\n");
                if (!pcl_object_print (next, stream, 0))
                {
                        pcl_object_unref (next);
                        pcl_object_unref (iterator);
                        return FALSE;
                }
                pcl_object_unref (next);
        }
        pcl_object_unref (iterator);
        return TRUE;
}

static PclObject *
table_repr (PclObject *object)
{
        return pcl_string_from_format (
                "<table object at %p>", (gpointer) object);
}

static PclObject *
table_str (PclObject *object)
{
        PclTable *self = PCL_TABLE (object);
        PclObject *iterator, *next;
        PclObject *str = pcl_string_from_string ("<");
        PclObject *comma = pcl_string_from_string (", ");
        PclObject *newline = pcl_string_from_string ("\n");
        
        iterator = pcl_object_iterate (self->all_fields);
        if (iterator == NULL)
                return NULL;

        next = pcl_iterator_next (iterator);
        if (next != NULL)
        {
                pcl_string_concat_and_del (&str, pcl_object_repr (next));
                pcl_object_unref (next);

                while ((next = pcl_iterator_next (iterator)) != NULL)
                {
                        pcl_string_concat (&str, comma);
                        pcl_string_concat_and_del (&str, pcl_object_repr(next));
                        pcl_object_unref (next);
                }
        }
        pcl_string_concat_and_del (&str, pcl_string_from_string (">"));
        pcl_object_unref (iterator);
        
        iterator = pcl_mapping_values (self->records);
        if (iterator == NULL)
                return NULL;

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                pcl_string_concat (&str, newline);
                pcl_string_concat_and_del (&str, pcl_object_repr(next));
                pcl_object_unref (next);
        }
        
        pcl_object_unref (iterator);
        pcl_object_unref (comma);
        pcl_object_unref (newline);
        return str;
}

static gint
table_contains (PclObject *object, PclObject *value)
{
        PclTable *self = PCL_TABLE (object);
        return pcl_object_contains (self->records, value);
}

static PclObject *
table_iterate (PclObject *object)
{
        PclTable *self = PCL_TABLE (object);
        return pcl_mapping_values (self->records);
}

static glong
table_measure (PclObject *object)
{
        PclTable *self = PCL_TABLE (object);
        return pcl_object_measure (self->records);
}

static gboolean
table_traverse (PclContainer *container, PclTraverseFunc func,
                gpointer user_data)
{
        PclTable *self = PCL_TABLE (container);

        if (self->key_fields != NULL)
                if (!func (self->key_fields, user_data))
                        return FALSE;
        if (self->all_fields != NULL)
                if (!func (self->all_fields, user_data))
                        return FALSE;
        if (self->records != NULL)
                if (!func (self->records, user_data))
                        return FALSE;
        if (self->lookups != NULL)
                if (!func (self->lookups, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (table_parent_class)->
                traverse (container, func, user_data);
}

static gboolean
table_lookups_insert_item (PclObject *lookup,
                           PclObject *key,
                           PclObject *record)
{
        PclObject *lookup_list;
        gboolean success;
        
        switch (pcl_dict_has_key (lookup, key))
        {
        case 1:
                /* Add to listing */
                lookup_list = pcl_dict_get_item (lookup, key);
                g_assert (lookup_list);
                pcl_list_append (lookup_list, record);
                return TRUE;
        case 0:
                /* Create new listing */
                lookup_list = pcl_list_new (1);
                pcl_object_ref (record);
                PCL_LIST_SET_ITEM (lookup_list, 0, record);
                success = pcl_dict_set_item (lookup, key, lookup_list);
                pcl_object_unref (lookup_list);
                return success;
        default:
                /* Internal error */
                return FALSE;
        }
}

static gboolean
table_lookups_insert_record (PclTable *self, PclObject *record)
{
        PclObject *it, *name, *item, *lookup;
        gint index = 0;
        
        it = pcl_object_iterate (self->all_fields);
        while ((name = pcl_iterator_next (it)) != NULL)
        {
                lookup = pcl_dict_get_item (self->lookups, name);
                pcl_object_unref (name);
                g_assert (lookup != NULL);
                g_assert (index < self->num_fields);
                item = PCL_LIST_GET_ITEM (PCL_TABLE_RECORD (record)->data, 
                                          index++);
                
                if (!table_lookups_insert_item (lookup, item, record))
                {
                        pcl_object_unref (it);
                        return FALSE;
                }
        }
        pcl_object_unref (it);
        return TRUE;
}

static gboolean
table_lookups_delete_item (PclObject *lookup,
                           PclObject *key,
                           PclObject *record)
{
        PclObject *lookup_list;
        gint index;
        if (pcl_dict_has_key (lookup, key) == 1)
        {
                /* Remove from listing */
                lookup_list = pcl_dict_get_item (lookup, key);
                index = pcl_object_measure (lookup_list) - 1;
                for (; index >= 0; index--)
                {
                        if (record == PCL_LIST_GET_ITEM(lookup_list, index))
                        {
                                pcl_list_del_item (lookup_list, index);
                                break;
                        }
                }
                        
                /* If listing is now empty, delete it */
                if (pcl_object_measure (lookup_list) == 0)
                        if (!pcl_dict_del_item (lookup, key))
                                return FALSE;
                        
                return TRUE;
        }
        /* Internal error */
        return FALSE;
}

static gboolean
table_lookups_delete_record (PclTable *self, PclObject *record)
{
        PclObject *it, *name, *item, *lookup;
        gint index = 0;
        
        it = pcl_object_iterate (self->all_fields);
        while ((name = pcl_iterator_next (it)) != NULL)
        {
                lookup = pcl_dict_get_item (self->lookups, name);
                pcl_object_unref (name);
                g_assert (lookup != NULL);
                g_assert (index < self->num_fields);
                item = PCL_LIST_GET_ITEM (PCL_TABLE_RECORD (record)->data,
                                          index++);
                
                if (!table_lookups_delete_item (lookup, item, record))
                {
                        pcl_object_unref (it);
                        return FALSE;
                }
        }
        pcl_object_unref (it);
        return TRUE;
}

static PclObject *
table_update_record (PclTable *self, PclObject *record, PclObject *kwds)
{
        /* XXX Needs better error checking */
        PclObject *value, *old_value; 
        PclObject *field, *lookup, *lookup_list;
        glong index1, index2;
        glong pos = 0;
        
        if (kwds == NULL)
        {
                pcl_error_set_string (pcl_exception_type_error (),
                                      "nothing to update");
                return NULL;
        }
        
        while (pcl_dict_next (kwds, &pos, &field, &value))
        {
                index1 = pcl_sequence_index (self->all_fields, field);

                if (index1 < 0)
                {
                        pcl_error_set_format (
                                pcl_exception_key_error (),
                                "%s is not a field name",
                                pcl_object_repr (field));
                        return NULL;
                }
                if (index1 < self->num_keys)
                {
                        pcl_error_set_string (
                                pcl_exception_key_error (),
                                "key fields are immutable");
                        return NULL;
                }
                if (pcl_object_hash (value) == PCL_HASH_INVALID)
                        return NULL;
 
                lookup = pcl_dict_get_item (self->lookups, field);
                old_value = pcl_list_get_item (PCL_TABLE_RECORD (record)->data,
                                               index1);
                lookup_list = pcl_dict_get_item (lookup, old_value);
                if (lookup_list != NULL)
                {
                        index2 = pcl_sequence_index (lookup_list, record);
                        if (index2 >= 0)
                                pcl_list_del_item(lookup_list, index2);
                        if (pcl_object_measure (lookup_list) == 0)
                                pcl_dict_del_item (lookup, old_value);
                }
                pcl_list_set_item (PCL_TABLE_RECORD (record)->data, 
                                   index1, 
                                   pcl_object_ref (value));
                lookup_list = pcl_dict_get_item (lookup, value);
                if (lookup_list != NULL)
                {
                        pcl_list_append (lookup_list, record);
                }
                else
                {
                        lookup_list = pcl_list_new (1);
                        PCL_LIST_SET_ITEM (lookup_list, 
                                           0, 
                                           pcl_object_ref (record));
                        pcl_dict_set_item (lookup, value, lookup_list);
                }
        }

        return pcl_object_ref (PCL_NONE);
}

static PclObject *
table_method_fields (PclTable *self)
{
        return pcl_object_ref (self->all_fields);
}

static PclObject *
table_method_key_fields (PclTable *self)
{
        return pcl_object_ref (self->key_fields);
}

static PclObject *
table_method_insert (PclTable *self, PclObject *args, PclObject *kwds)
{
        PclObject *record;
        PclObject *key;
        
        if ((record = table_record_build (self, args, kwds, PCL_NONE)) == NULL)
                return NULL;

        if ((key = pcl_tablerecord_extract_key (record)) != NULL)
        {
                switch (pcl_dict_has_key (self->records, key))
                {
                case 1:
                        pcl_error_set_string (pcl_exception_key_error (),
                                              "record already exists in table");
                        break;
                case 0:
                        pcl_dict_set_item (self->records, key, record);
                        table_lookups_insert_record (self, record);
                        pcl_object_unref (record);
                        pcl_object_unref (key);                        
                        return pcl_object_ref (PCL_NONE);
                default:
                        /* Exception string set when has_key returns -1 */
                        break;
                }
                pcl_object_unref (key);
        }
        /* Exception string set when extract_key returns NULL */
        pcl_object_unref (record);
        return NULL;
}

static PclObject *
table_method_delete (PclTable *self, PclObject *args, PclObject *kwds)
{
        PclObject *record;
        PclObject *key;
        
        if ((record = table_record_build (self, args, kwds, PCL_NONE)) == NULL)
                return NULL;
                
        if ((key = pcl_tablerecord_extract_key (record)) != NULL)
        {
                switch (pcl_dict_has_key (self->records, key))
                {
                case 0:
                        pcl_error_set_string (pcl_exception_key_error (),
                                              "record does not exist in table");
                        break;
                case 1:
                        pcl_object_unref (record);
                        record = pcl_dict_get_item (self->records, key);
                        pcl_object_ref (record);
                        pcl_dict_del_item (self->records, key);
                        table_lookups_delete_record (self, record);
                        pcl_object_unref (record);
                        pcl_object_unref (key);
                        return pcl_object_ref (PCL_NONE);
                default:
                        /* Exception string set when has_key returns -1 */
                        break;
                }
                pcl_object_unref (key);
        }
        /* Exception string set when extract_key returns NULL */
        pcl_object_unref (record);
        return NULL;
}

static PclObject *
table_method_update (PclTable *self, PclObject *args, PclObject *kwds)
{
        PclObject *key, *record;
        
        if (pcl_object_measure (args) != 1)
        {
                pcl_error_set_string (pcl_exception_type_error (),
                                      "update accepts one positional argument");
                return NULL;
        }
        record = PCL_TUPLE_GET_ITEM (args, 0);
        if (!PCL_IS_TABLE_RECORD (record))
        {
                pcl_error_set_string (pcl_exception_type_error (),
                                      "first argument must be a record");
                return NULL;
        }
        if (PCL_TABLE_RECORD (record)->fields != self->all_fields)
        {
                pcl_error_set_string (pcl_exception_value_error (),
                                      "record does not belong to this table");
                return NULL;
        }
        
        if ((key = pcl_tablerecord_extract_key (record)) != NULL)
        {
                switch (pcl_dict_has_key (self->records, key))
                {
                case 0:
                        pcl_error_set_string (pcl_exception_key_error (),
                                              "record does not exist in table");
                        break;
                case 1:
                        record = pcl_dict_get_item (self->records, key);
                        pcl_object_unref (key);
                        return table_update_record (self, record, kwds);
                default:
                        /* Exception string set when has_key returns -1 */
                        break;                        
                }
        }
        /* Exception string set when extract_key returns NULL */
        pcl_object_unref (key);
        return NULL;
}

static PclObject *
table_method_find (PclTable *self, PclObject *args, PclObject *kwds)
{
        gboolean first_time;
        gint index, temp_int, size, i;
        PclObject *find_list = NULL, *item, *record, *record_list = NULL;
        PclObject *key, *lookup_list, *lookup, *rec;
        
        /* Quick check for common call */
        if (pcl_object_measure (args) == 0 && kwds == NULL)
        {
                PclObject *iterator;

                iterator = pcl_mapping_values (self->records);
                if (iterator == NULL)
                        return NULL;
                record_list = pcl_list_from_iterator (iterator);
                pcl_object_unref (iterator);
                return record_list;
        }

        record = table_record_build (self, args, kwds, PCL_FREE);
        if (record == NULL)
                return NULL;
 
        key = pcl_tablerecord_extract_key (record);
        if (key == NULL)
                pcl_error_clear ();
        else
        { 
                if (pcl_dict_has_key (self->records, key) > 0)
                {
                        item = pcl_dict_get_item (self->records, key);
                        record_list = PCL_TABLE_RECORD (item)->data;
                        find_list = PCL_TABLE_RECORD (record)->data;
                        /* Make sure record matches the rest of
                           the query fields */
                        for (i = self->num_keys; i < self->num_fields; i++)
                        {
                                if (PCL_LIST_GET_ITEM(find_list, i) != 
                                        PCL_FREE &&
                                    PCL_LIST_GET_ITEM(find_list, i) != 
                                        PCL_LIST_GET_ITEM(record_list, i))
                                {
                                        pcl_object_unref (record);
                                        pcl_object_unref (key);
                                        return pcl_list_new (0);                                        
                                }
                        }
                        record_list = pcl_list_new (1);
                        PCL_LIST_SET_ITEM (record_list, 0, pcl_object_ref (item));
                        pcl_object_unref (record);
                        pcl_object_unref (key);
                        return record_list;
                }
                pcl_object_unref (record);
                pcl_object_unref (key);
                return pcl_list_new (0);
        }
        
        /* Building list from lookup tables */
        first_time = TRUE;
        find_list = PCL_TABLE_RECORD (record)->data;
        for (index = 0; index < self->num_fields; index++)
        {
                item = PCL_LIST_GET_ITEM (find_list, index);
                if (item != PCL_FREE && item != NULL)
                {
                        key = PCL_TUPLE_GET_ITEM (self->all_fields, index);
                        lookup = pcl_dict_get_item (self->lookups, key);
                        g_assert (lookup);
                        if ((temp_int = pcl_dict_has_key (lookup, item)) == 1)
                        {
                                lookup_list = pcl_dict_get_item (lookup, item);
                        }
                        else
                        {
                                if (!first_time)
                                        pcl_object_unref (record_list);
                                pcl_object_unref (record);
                                if (temp_int == 0)
                                        return pcl_list_new (0);
                                return NULL;
                        }
                        
                        if (first_time)
                        {
                                first_time = FALSE;
                                record_list = pcl_object_copy(lookup_list);
                        }
                        else
                        {
                                size = pcl_object_measure (record_list);
                                for (i = 0; i < size; )
                                {
                                        rec = PCL_LIST_GET_ITEM (record_list, 
                                                                 i);
                                        if (pcl_object_contains (
                                                lookup_list, rec))
                                        {
                                                i++;
                                        }
                                        else
                                        {
                                                pcl_list_del_item (record_list,
                                                                   i);
                                                size--;
                                        }
                                }
                        }
                        if (pcl_object_measure (record_list) == 0)
                        {
                                pcl_object_unref (record);
                                return record_list;
                        }
                }
        }
        pcl_object_unref (record);

        if (first_time)
        {
                PclObject *iterator;

                iterator = pcl_mapping_values (self->records);
                if (iterator == NULL)
                        return NULL;
                record_list = pcl_list_from_iterator (iterator);
                pcl_object_unref (iterator);
        }
        return record_list;
}

static void
table_class_init (PclTableClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        table_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = table_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_table_get_type_object;
        object_class->new_instance = table_new_instance;
        object_class->clone = table_clone;
        object_class->print = table_print;
        object_class->repr = table_repr;
        object_class->str = table_str;
        object_class->contains = table_contains;
        object_class->iterate = table_iterate;
        object_class->measure = table_measure;
        object_class->doc = table_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = table_dispose;
}

static PclMethodDef table_methods[] = {
        { "fields",             (PclCFunction) table_method_fields,
                                PCL_METHOD_FLAG_NOARGS },
        { "key_fields",         (PclCFunction) table_method_key_fields,
                                PCL_METHOD_FLAG_NOARGS },
        { "insert",             (PclCFunction) table_method_insert,
                                PCL_METHOD_FLAG_VARARGS |
                                PCL_METHOD_FLAG_KEYWORDS },
        { "delete",             (PclCFunction) table_method_delete,
                                PCL_METHOD_FLAG_VARARGS |
                                PCL_METHOD_FLAG_KEYWORDS },
        { "update",             (PclCFunction) table_method_update,
                                PCL_METHOD_FLAG_VARARGS |
                                PCL_METHOD_FLAG_KEYWORDS },
        { "find",               (PclCFunction) table_method_find,
                                PCL_METHOD_FLAG_VARARGS |
                                PCL_METHOD_FLAG_KEYWORDS },
        { NULL }
};

GType
pcl_table_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclTableClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) table_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclTable),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclTable", &type_info, 0);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, table_methods);
        }
        return type;
}

/**
 * pcl_table_get_type_object:
 *
 * Returns the type object for #PclTable.  During runtime this is the built-in
 * object %table.
 *
 * Returns: a borrowed reference to the type object for #PclTable
 */
PclObject *
pcl_table_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_TABLE, "table");
                pcl_register_singleton ("<type 'table'>", &object);
        }
        return object;
}

PclObject *
pcl_table_new (PclObject *keys, PclObject *data)
{
        PclTable *table;
        glong ii;
 
        if (!PCL_IS_TUPLE (keys) || !PCL_IS_TUPLE (data))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "invalid argument list");
                return NULL;
        }

        table = pcl_object_new (PCL_TYPE_TABLE, NULL);
        table->key_fields = pcl_object_copy (keys);
        table->num_keys = PCL_TUPLE_GET_SIZE (table->key_fields);
        table->all_fields = pcl_sequence_concat (keys, data);
        table->num_fields = PCL_TUPLE_GET_SIZE (table->all_fields);
        table->records = pcl_dict_new ();
        table->lookups = pcl_dict_new ();
        
        for (ii = 0; ii < table->num_fields; ii++)
        {
                PclObject *dict;
                PclObject *name;

                name = PCL_TUPLE_GET_ITEM (table->all_fields, ii);

                if (!PCL_IS_STRING (name))
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "field name must be a string");
                        pcl_object_unref (table);
                        return NULL;
                }

                if (pcl_object_contains (table->lookups, name) != 0)
                {
                        if (!pcl_error_occurred ())
                                pcl_error_set_format (
                                        pcl_exception_value_error (),
                                        "duplication field name '%s'",
                                        PCL_STRING_AS_STRING (name));
                        pcl_object_unref (table);
                        return NULL;
                }

                dict = pcl_dict_new ();
                if (dict == NULL)
                {
                        pcl_object_unref (table);
                        return NULL;
                }
                pcl_dict_set_item (table->lookups, name, dict);
                pcl_object_unref (dict);
        }

        return PCL_OBJECT (table);
}
