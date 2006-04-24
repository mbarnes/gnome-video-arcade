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

static gpointer weak_ref_parent_class = NULL;

static void
weak_ref_notify (PclWeakRef *self, PclObject *where_the_object_was)
{
        PclObject *result;

        self->object = PCL_NONE;
        if (self->callback == NULL)  /* XXX Double-check this */
                return;
        result = pcl_object_call_function (self->callback, "O", self);
        if (result == NULL)
                pcl_error_write_unraisable (self->callback);
        else
                pcl_object_unref (result);
}

static void
weak_ref_dispose (GObject *g_object)
{
        PclWeakRef *self = PCL_WEAK_REF (g_object);

        PCL_CLEAR (self->callback);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (weak_ref_parent_class)->dispose (g_object);
}

static void
weak_ref_finalize (GObject *g_object)
{
        PclWeakRef *self = PCL_WEAK_REF (g_object);

        if (self->object != PCL_NONE)
                g_object_weak_unref (
                        G_OBJECT (self->object),
                        (GWeakNotify) weak_ref_notify, self);

        /* Chain up to parent's finalize method. */
        G_OBJECT_CLASS (weak_ref_parent_class)->finalize (g_object);
}

static PclObject *
weak_ref_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        const gchar *kwlist[] = { NULL };
        PclObject *callback = NULL;
        PclObject *object;

        if (!pcl_arg_parse_tuple_and_keywords (
                        args, kwds, "O|O:weakref", kwlist, &object, &callback))
                return NULL;

        return pcl_weak_ref_new (object, callback);
}

static PclObject *
weak_ref_call (PclObject *object, PclObject *args, PclObject *kwds)
{
        PclWeakRef *self = PCL_WEAK_REF (object);
        const gchar *kwlist[] = { NULL };

        if (!pcl_arg_parse_tuple_and_keywords (
                        args, kwds, ":__call__", kwlist))
                return NULL;
        return pcl_object_ref (self->object);
}

static guint
weak_ref_hash (PclObject *object)
{
        PclWeakRef *self = PCL_WEAK_REF (object);

        if (self->hash != PCL_HASH_INVALID)
                return self->hash;
        if (self->object == PCL_NONE)
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "weak object has gone away");
                return PCL_HASH_INVALID;
        }
        self->hash = pcl_object_hash (self->object);
        return self->hash;
}

static PclObject *
weak_ref_repr (PclObject *object)
{
        PclWeakRef *self = PCL_WEAK_REF (object);

        if (self->object == PCL_NONE)
                return pcl_string_from_format (
                        "<weakref at %p; dead",
                        (gpointer) self);
        else
        {
                PclObject *repr, *tmp;
                gchar *name = NULL;

                tmp = pcl_object_get_attr_string (self->object, "__name__");
                if (tmp == NULL)
                        pcl_error_clear ();
                if (PCL_IS_STRING (tmp))
                        name = PCL_STRING_AS_STRING (tmp);
                repr = pcl_string_from_format (
                        name ? "<weakref at %p; to '%s' at %p (%s)>"
                             : "<weakref at %p; to '%s' at %p>",
                        (gpointer) self,
                        PCL_GET_TYPE_NAME (self->object),
                        (gpointer) self->object,
                        name);
                if (tmp != NULL)
                        pcl_object_unref (tmp);
                return repr;
        }
}

static gboolean
weak_ref_traverse (PclContainer *container, PclTraverseFunc func,
                   gpointer user_data)
{
        PclWeakRef *self = PCL_WEAK_REF (container);

        if (self->callback != NULL)
                if (!func (self->callback, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (weak_ref_parent_class)->
                traverse (container, func, user_data);
}

static void
weak_ref_class_init (PclWeakRefClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        weak_ref_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = weak_ref_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_weak_ref_get_type_object;
        object_class->new_instance = weak_ref_new_instance;
        object_class->call = weak_ref_call;
        object_class->hash = weak_ref_hash;
        object_class->repr = weak_ref_repr;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = weak_ref_dispose;
        g_object_class->finalize = weak_ref_finalize;
}

static void
weak_ref_init (PclWeakRef *weak_ref)
{
        weak_ref->object = PCL_NONE;
        weak_ref->hash = PCL_HASH_INVALID;
}

GType
pcl_weak_ref_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclWeakRefClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) weak_ref_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclWeakRef),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) weak_ref_init,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclWeakRef", &type_info, 0);
        }
        return type;
}

/**
 * pcl_weak_ref_get_type_object:
 *
 * Returns the type object for #PclWeakRef.
 *
 * Returns: a borrowed reference to the type object for #PclWeakRef
 */
PclObject *
pcl_weak_ref_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_WEAK_REF, "weakref");
                pcl_register_singleton ("<type 'weakref'>", &object);
        }
        return object;
}

PclObject *
pcl_weak_ref_new (PclObject *object, PclObject *callback)
{
        PclWeakRef *weak_ref;

        if (!PCL_IS_CONTAINER (object))
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "cannot create weak reference to '%s' object",
                        PCL_GET_TYPE_NAME (object));
                return NULL;
        }

        weak_ref = pcl_object_new (PCL_TYPE_WEAK_REF, NULL);
        g_object_weak_ref (
                G_OBJECT (object),
                (GWeakNotify) weak_ref_notify, weak_ref);
        weak_ref->object = object;
        if (callback == PCL_NONE)
                callback = NULL;
        else if (callback != NULL)
                pcl_object_ref (callback);
        weak_ref->callback = callback;
        return PCL_OBJECT (weak_ref);
}
