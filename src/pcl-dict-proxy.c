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

static gpointer dict_proxy_parent_class = NULL;

static void
dict_proxy_dispose (GObject *g_object)
{
        PclDictProxy *self = PCL_DICT_PROXY (g_object);

        PCL_CLEAR (self->dict);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (dict_proxy_parent_class)->dispose (g_object);
}

static PclObject *
dict_proxy_str (PclObject *object)
{
        PclDictProxy *self = PCL_DICT_PROXY (object);
        return pcl_object_str (self->dict);
}

static gint
dict_proxy_compare (PclObject *object, PclObject *other)
{
        PclDictProxy *self = PCL_DICT_PROXY (object);
        return pcl_object_compare (self->dict, other);
}

static gint
dict_proxy_contains (PclObject *object, PclObject *value)
{
        PclDictProxy *self = PCL_DICT_PROXY (object);
        return pcl_object_contains (self->dict, value);
}

static PclObject *
dict_proxy_iterate (PclObject *object)
{
        PclDictProxy *self = PCL_DICT_PROXY (object);
        return pcl_object_iterate (self->dict);
}

static glong
dict_proxy_measure (PclObject *object)
{
        PclDictProxy *self = PCL_DICT_PROXY (object);
        return pcl_object_measure (self->dict);
}

static PclObject *
dict_proxy_get_item (PclObject *object, PclObject *subscript)
{
        PclDictProxy *self = PCL_DICT_PROXY (object);
        return pcl_object_get_item (self->dict, subscript);
}

static gboolean
dict_proxy_traverse (PclContainer *container, PclTraverseFunc func,
                     gpointer user_data)
{
        PclDictProxy *self = PCL_DICT_PROXY (container);

        if (self->dict != NULL)
                if (!func (self->dict, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (dict_proxy_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
dict_proxy_mapping_keys (PclDictProxy *self)
{
        return pcl_mapping_keys (self->dict);
}

static PclObject *
dict_proxy_mapping_values (PclDictProxy *self)
{
        return pcl_mapping_values (self->dict);
}

static PclObject *
dict_proxy_mapping_items (PclDictProxy *self)
{
        return pcl_mapping_items (self->dict);
}

static void
dict_proxy_mapping_init (PclMappingIface *iface)
{
        iface->keys = (PclMappingKeys) dict_proxy_mapping_keys;
        iface->values = (PclMappingValues) dict_proxy_mapping_values;
        iface->items = (PclMappingItems) dict_proxy_mapping_items;
}

static void
dict_proxy_class_init (PclDictProxyClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        dict_proxy_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = dict_proxy_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_dict_proxy_get_type_object;
        object_class->str = dict_proxy_str;
        object_class->compare = dict_proxy_compare;
        object_class->contains = dict_proxy_contains;
        object_class->iterate = dict_proxy_iterate;
        object_class->measure = dict_proxy_measure;
        object_class->get_item = dict_proxy_get_item;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = dict_proxy_dispose;
}

GType
pcl_dict_proxy_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclDictProxyClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) dict_proxy_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclDictProxy),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                static const GInterfaceInfo mapping_info = {
                        (GInterfaceInitFunc) dict_proxy_mapping_init,
                        (GInterfaceFinalizeFunc) NULL,
                        NULL   /* interface_data */
                };

                /* ITERABLE, MEASURABLE, and SUBSCRIPTABLE are prerequisites
                 * for MAPPING */
                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclDictProxy", &type_info, 0);
                g_type_add_interface_static (
                        type, PCL_TYPE_MAPPING, &mapping_info);
        }
        return type;
}

/**
 * pcl_dict_proxy_get_type_object:
 *
 * Returns the type object for #PclDictProxy.
 *
 * Returns: a borrowed reference to the type object for #PclDictProxy
 */
PclObject *
pcl_dict_proxy_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_DICT_PROXY, "dictproxy");
                pcl_register_singleton ("<type 'dictproxy'>", &object);
        }
        return object;
}

PclObject *
pcl_dict_proxy_new (PclObject *dict)
{
        PclDictProxy *dict_proxy;

        dict_proxy = pcl_object_new (PCL_TYPE_DICT_PROXY, NULL);
        dict_proxy->dict = pcl_object_ref (dict);
        return PCL_OBJECT (dict_proxy);
}
