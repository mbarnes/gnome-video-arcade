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

static PclObject *
mapping_method_clear (PclObject *self)
{
        if (!pcl_mapping_clear (self))
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

static PclObject *
mapping_method_copy (PclObject *self)
{
        return pcl_object_copy (self);
}

static PclObject *
mapping_method_fromkeys (PclObject *class, PclObject *args)
{
        PclObject *instance;
        PclObject *iterator;
        PclObject *next;
        PclObject *sequence;
        PclObject *value = PCL_NONE;

        if (!pcl_arg_unpack_tuple (args, "fromkeys", 1, 2, &sequence, &value))
                return NULL;

        instance = pcl_object_call_object (class, NULL);
        if (instance == NULL)
                return NULL;

        iterator = pcl_object_iterate (sequence);
        if (iterator == NULL)
        {
                pcl_object_unref (instance);
                return NULL;
        }

        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                if (!pcl_object_set_item (instance, next, value))
                {
                        pcl_object_unref (next);
                        break;
                }
                pcl_object_unref (next);
        }

        pcl_object_unref (iterator);
        if (pcl_error_occurred ())
        {
                pcl_object_unref (instance);
                return NULL;
        }
        return instance;
}

static PclObject *
mapping_method_get (PclObject *self, PclObject *args)
{
        PclObject *def = PCL_NONE;
        PclObject *key;
        PclObject *value;

        if (!pcl_arg_unpack_tuple (args, "get", 1, 2, &key, &def))
                return NULL;

        /* mappings are subscriptable, so this is safe */
        value = pcl_object_get_item (self, key);
        if (value == NULL && pcl_error_exception_matches (
                pcl_exception_key_error ()))
        {
                pcl_error_clear ();
                value = pcl_object_ref (def);
        }
        return value;
}

static PclObject *
mapping_method_has_key (PclObject *self, PclObject *key)
{
        gint result;

        /* mappings are iterable, so this is safe */
        result = pcl_object_contains (self, key);
        if (result < 0)
                return NULL;
        return pcl_bool_from_boolean ((gboolean) result);
}

static PclObject *
mapping_method_items (PclObject *self)
{
        return pcl_mapping_items (self);
}

static PclObject *
mapping_method_keys (PclObject *self)
{
        return pcl_mapping_keys (self);
}

static PclObject *
mapping_method_pop (PclObject *self, PclObject *args)
{
        PclObject *def = NULL;
        PclObject *key;
        PclObject *value;

        if (!pcl_arg_unpack_tuple (args, "pop", 1, 2, &key, &def))
                return NULL;

        /* mappings are measurable, so this is safe */
        if (pcl_object_measure (self) == 0)
        {
                if (def != NULL)
                        return pcl_object_ref (def);
                pcl_error_set_format (
                        pcl_exception_key_error (),
                        "pop(): %s is empty",
                        PCL_GET_TYPE_NAME (self));
                return NULL;
        }

        /* mappings are subscriptable, so this is safe */
        value = pcl_object_get_item (self, key);
        if (value == NULL && pcl_error_exception_matches (
                pcl_exception_key_error ()))
        {
                if (def != NULL)
                {
                        pcl_error_clear ();
                        return pcl_object_ref (def);
                }
                return NULL;
        }

        /* mappings are subscriptable, and those that have pop() are
         * mutable, so this is safe */
        if (!pcl_object_del_item (self, key))
        {
                pcl_object_unref (value);
                return NULL;
        }

        return value;
}

static PclObject *
mapping_method_popitem (PclObject *self)
{
        return pcl_mapping_pop_item (self);
}

static PclObject *
mapping_method_setdefault (PclObject *self, PclObject *args)
{
        PclObject *def = PCL_NONE;
        PclObject *key;
        PclObject *value;

        if (!pcl_arg_unpack_tuple (args, "setdefault", 1, 2, &key, &def))
                return NULL;

        /* mappings are subscriptable, so this is safe */
        value = pcl_object_get_item (self, key);
        if (value == NULL && pcl_error_exception_matches (
                pcl_exception_key_error ()))
        {
                pcl_error_clear ();
                if (!pcl_dict_set_item (self, key, def))
                        return NULL;
                value = pcl_object_ref (def);
        }
        return value;
}

static PclObject *
mapping_method_update (PclObject *self, PclObject *args, PclObject *kwds)
{
        PclObject *from = NULL;

        if (!pcl_arg_unpack_tuple (args, "update", 0, 1, &from))
                return NULL;

        if (from != NULL && !pcl_mapping_merge (self, from, TRUE))
                return NULL;
        if (kwds != NULL && !pcl_mapping_merge (self, kwds, TRUE))
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

static PclObject *
mapping_method_values (PclObject *self)
{
        return pcl_mapping_values (self);
}

#define WRAPS(func)     G_STRUCT_OFFSET (PclMappingIface, func)

static PclMethodDef mapping_methods[] = {
        { "clear",              (PclCFunction) mapping_method_clear,
                                PCL_METHOD_FLAG_NOARGS,
                                "D.clear() -> "
                                "None.  Remove all items from D.",
                                WRAPS (clear) },
        { "copy",               (PclCFunction) mapping_method_copy,
                                PCL_METHOD_FLAG_NOARGS,
                                "D.copy()-> a shallow copy of D" },
        { "fromkeys",           (PclCFunction) mapping_method_fromkeys,
                                PCL_METHOD_FLAG_VARARGS |
                                PCL_METHOD_FLAG_CLASS,
                                "dict.fromkeys(S[.v]) -> "
                                "New dict with keys from S and values equal "
                                "to v.\n"
                                "v defaults to None." },
        { "get",                (PclCFunction) mapping_method_get,
                                PCL_METHOD_FLAG_VARARGS,
                                "D.get(k[,d]) -> "
                                "D[k] if k in D, else d.  "
                                "d defaults to None." },
        { "has_key",            (PclCFunction) mapping_method_has_key,
                                PCL_METHOD_FLAG_ONEARG,
                                "D.has_key(k) -> "
                                "True if D has a key k, else False" },
        { "items",              (PclCFunction) mapping_method_items,
                                PCL_METHOD_FLAG_NOARGS,
                                "D.items() -> "
                                "an iterator over the (key, value) items "
                                "of D",
                                WRAPS (items) },
        { "keys",               (PclCFunction) mapping_method_keys,
                                PCL_METHOD_FLAG_NOARGS,
                                "D.keys() -> "
                                "an iterator over the keys of D",
                                WRAPS (keys) },
        { "pop",                (PclCFunction) mapping_method_pop,
                                PCL_METHOD_FLAG_VARARGS,
                                "D.pop(k[,d]) -> "
                                "v, remove specified key and return the "
                                "corresponding value\n"
                                "If key is not found, d is return if "
                                "given, otherwise KeyError is raised" },
        { "popitem",            (PclCFunction) mapping_method_popitem,
                                PCL_METHOD_FLAG_NOARGS,
                                "D.popitem() -> "
                                "(k, v), remove and return some "
                                "(key, value) pair as a\n"
                                "2-tuple; but raise KeyError if D is empty",
                                WRAPS (pop_item) },
        { "setdefault",         (PclCFunction) mapping_method_setdefault,
                                PCL_METHOD_FLAG_VARARGS,
                                "D.setdefault(k[,d]) ->"
                                "D.get(k,d), also set D[k]=d if k not in D" },
        { "update",             (PclCFunction) mapping_method_update,
                                PCL_METHOD_FLAG_VARARGS |
                                PCL_METHOD_FLAG_KEYWORDS,
                                "D.update(E, **F) -> "
                                "None.  Update D from E and F: "
                                "for k in E: D[k] = E[k]\n"
                                "(if E is mapping else: for (k, v) in E: "
                                "D[k] = v) then: for k in F: D[k] = F[k]" },
        { "values",             (PclCFunction) mapping_method_values,
                                PCL_METHOD_FLAG_NOARGS,
                                "D.values() -> "
                                "an iterator over the values of D",
                                WRAPS (values) },
        { NULL }
};

#undef WRAPS

GType
pcl_mapping_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclMappingIface),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) NULL,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        0,     /* instance_size */
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                       G_TYPE_INTERFACE, "PclMapping", &type_info, 0);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, mapping_methods);
        }
        return type;
}

PclObject *
pcl_mapping_keys (PclObject *object)
{
        PclMappingIface *iface;

        iface = PCL_MAPPING_GET_IFACE (object);
        if (iface != NULL && iface->keys != NULL)
                return iface->keys ((PclMapping *) object);

        pcl_error_bad_internal_call ();
        return NULL;
}

PclObject *
pcl_mapping_values (PclObject *object)
{
        PclMappingIface *iface;

        iface = PCL_MAPPING_GET_IFACE (object);
        if (iface != NULL && iface->values != NULL)
                return iface->values ((PclMapping *) object);

        pcl_error_bad_internal_call ();
        return NULL;
}

PclObject *
pcl_mapping_items (PclObject *object)
{
        PclMappingIface *iface;

        iface = PCL_MAPPING_GET_IFACE (object);
        if (iface != NULL && iface->items != NULL)
                return iface->items ((PclMapping *) object);

        pcl_error_bad_internal_call ();
        return NULL;
}

gboolean
pcl_mapping_clear (PclObject *object)
{
        PclMappingIface *iface;

        iface = PCL_MAPPING_GET_IFACE (object);
        if (iface != NULL && iface->clear != NULL)
                return iface->clear ((PclMapping *) object);

        pcl_error_bad_internal_call ();
        return FALSE;
}

PclObject *
pcl_mapping_pop_item (PclObject *object)
{
        PclMappingIface *iface;

        iface = PCL_MAPPING_GET_IFACE (object);
        if (iface != NULL && iface->pop_item != NULL)
                return iface->pop_item ((PclMapping *) object);

        pcl_error_bad_internal_call ();
        return NULL;
}

gboolean
pcl_mapping_merge (PclObject *object, PclObject *from, gboolean override)
{
        PclObject *iterator;
        PclObject *next;
        gboolean from_mapping;
        gint index = 0;

        if (!PCL_IS_MAPPING (object) || !PCL_IS_ITERABLE (from))
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        iterator = pcl_object_iterate (from);
        if (iterator == NULL)
                return FALSE;

        from_mapping = PCL_IS_MAPPING (from);
        while ((next = pcl_iterator_next (iterator)) != NULL)
        {
                PclObject *key;
                PclObject *value;

                if (from_mapping)
                {
                        key = pcl_object_ref (next);
                        value = pcl_object_get_item (from, key);
                        if (value == NULL)
                                goto fail;
                }
                else
                {
                        glong length;

                        if (!PCL_IS_SEQUENCE (next))
                        {
                                pcl_error_set_format (
                                        pcl_exception_type_error (),
                                        "cannot convert %s update sequence "
                                        "element #%d to a sequence",
                                        PCL_GET_TYPE_NAME (object), index);
                                goto fail;
                        }

                        length = pcl_object_measure (next);
                        if (length != 2)
                        {
                                pcl_error_set_format (
                                        pcl_exception_value_error (),
                                        "%s update sequence element #%d "
                                        "has length %u; 2 is required",
                                        PCL_GET_TYPE_NAME (object),
                                        index, length);
                                goto fail;
                        }

                        key = pcl_sequence_get_item (next, 0);
                        if (key == NULL)
                                goto fail;
                        value = pcl_sequence_get_item (next, 1);
                        if (value == NULL)
                        {
                                pcl_object_unref (key);
                                goto fail;
                        }
                }

                if (override || !pcl_object_contains (object, key))
                {
                        if (!pcl_object_set_item (object, key, value))
                        {
                                pcl_object_unref (key);
                                pcl_object_unref (value);
                                goto fail;
                        }
                }

                pcl_object_unref (key);
                pcl_object_unref (value);
                pcl_object_unref (next);
                index++;
        }

        pcl_object_unref (iterator);
        return (pcl_error_occurred () == NULL);

fail:
        pcl_object_unref (iterator);
        pcl_object_unref (next);
        return FALSE;
}
