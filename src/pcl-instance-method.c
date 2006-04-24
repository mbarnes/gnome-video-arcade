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

static gchar instance_method_doc[] =
"instancemethod(callable, instance, class)\n\
\n\
Create an instance method object.";

static PclFactory instance_method_factory;

static gpointer instance_method_parent_class = NULL;

/* MRO Utilities */
extern PclObject *mro_lookup (PclType *type, const gchar *name);

static void
instance_method_dispose (GObject *g_object)
{
        PclInstanceMethod *self = PCL_INSTANCE_METHOD (g_object);

        PCL_CLEAR (self->callable);
        PCL_CLEAR (self->instance);
        PCL_CLEAR (self->owner);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (instance_method_parent_class)->dispose (g_object);
}

static PclObject *
instance_method_new_instance (PclObject *type, PclObject *args,
                              PclObject *kwds)
{
        PclObject *callable;
        PclObject *instance;
        PclObject *owner = NULL;

        if (!pcl_arg_no_keywords ("instancemethod()", kwds))
                return NULL;

        if (!pcl_arg_unpack_tuple (args, "instancemethod", 2, 3,
                        &callable, &instance, &owner))
                return NULL;
        if (!PCL_IS_CALLABLE (callable))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "first argument must be callable");
                return NULL;
        }
        if (instance == PCL_NONE)
                instance = NULL;
        if (instance == NULL && owner == NULL)
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "unbound methods must have non-NULL owner");
                return NULL;
        }

        return pcl_instance_method_new (callable, instance, owner);
}

/* helper for instance_method_call() */
static void
get_class_name (PclObject *class, gchar *buf, gsize bufsize)
{
        PclObject *name;

        g_assert (bufsize > 1);
        strcpy (buf, "?");  /* default outcome */
        if (class == NULL)
                return;
        name = pcl_object_get_attr_string (class, "__name__");
        if (name == NULL)
        {
                pcl_error_clear ();
                return;
        }
        if (PCL_IS_STRING (name))
        {
                strncpy (buf, PCL_STRING_AS_STRING (name), bufsize);
                buf[bufsize - 1] = '\0';
        }
        pcl_object_unref (name);
}

/* helper for instance_method_call() */
static void
get_instance_class_name (PclObject *instance, gchar *buf, gsize bufsize)
{
        PclObject *class;

        if (instance == NULL)
        {
                g_assert (bufsize > strlen ("nothing"));
                strcpy (buf, "nothing");
                return;
        }

        class = pcl_object_get_attr_string (instance, "__class__");
        if (class == NULL)
        {
                pcl_error_clear ();
                class = PCL_GET_TYPE_OBJECT (instance);
                pcl_object_ref (class);
        }
        get_class_name (class, buf, bufsize);
        pcl_object_unref (class);
}

static PclObject *
instance_method_call (PclObject *object, PclObject *args, PclObject *kwds)
{
        PclInstanceMethod *self = PCL_INSTANCE_METHOD (object);
        PclObject *callable = self->callable;
        PclObject *instance = self->instance;
        PclObject *owner = self->owner;
        PclObject *result;

        if (instance == NULL)
        {
                gboolean is_instance = FALSE;
                if (PCL_TUPLE_GET_SIZE (args) >= 1)
                        instance = PCL_TUPLE_GET_ITEM (args, 0);
                if (instance != NULL)
                {
                        is_instance = pcl_object_is_instance (instance, owner);
                        if (is_instance < 0)
                                return NULL;
                }
                if (!is_instance)
                {
                        gchar typebuf[256];
                        gchar instbuf[256];
                        get_class_name (
                                owner, typebuf, sizeof (typebuf));
                        get_instance_class_name (
                                instance, instbuf, sizeof (instbuf));
                        pcl_error_set_format (
                                pcl_exception_type_error (),
                                "unbound method %s%s must be called with "
                                "%s instance as first argument "
                                "(got %s%s instead)",
                                pcl_eval_get_func_name (callable),
                                pcl_eval_get_func_desc (callable),
                                typebuf,
                                instbuf,
                                instance == NULL ? "" : " instance");
                        return NULL;
                }
                pcl_object_ref (args);
        }
        else
        {
                glong ii, size = PCL_TUPLE_GET_SIZE (args);
                PclObject *new_args = pcl_tuple_new (size + 1);
                if (new_args == NULL)
                        return NULL;
                pcl_object_ref (instance);
                PCL_TUPLE_SET_ITEM (new_args, 0, instance);
                for (ii = 0; ii < size; ii++)
                {
                        PclObject *item = PCL_TUPLE_GET_ITEM (args, ii);
                        pcl_object_ref (item);
                        PCL_TUPLE_SET_ITEM (new_args, ii + 1, item);
                }
                args = new_args;
        }

        result = pcl_object_call (callable, args, kwds);
        pcl_object_unref (args);
        return result;
}

static guint
instance_method_hash (PclObject *object)
{
        PclInstanceMethod *self = PCL_INSTANCE_METHOD (object);
        guint x, y;

        if (self->instance == NULL)
                x = pcl_object_hash (PCL_NONE);
        else
                x = pcl_object_hash (self->instance);
        if (x == PCL_HASH_INVALID)
                return PCL_HASH_INVALID;
        y = pcl_object_hash (self->callable);
        if (y == PCL_HASH_INVALID)
                return PCL_HASH_INVALID;
        return x ^ y;
}

static PclObject *
instance_method_repr (PclObject *object)
{
        PclInstanceMethod *self = PCL_INSTANCE_METHOD (object);
        PclObject *callable = self->callable;
        PclObject *callable_name = NULL;
        PclObject *instance = self->instance;
        PclObject *owner = self->owner;
        PclObject *owner_name = NULL;
        PclObject *result = NULL;

        callable_name = pcl_object_get_attr_string (callable, "__name__");
        if (callable_name == NULL)
        {
                if (!pcl_error_exception_matches (
                                pcl_exception_attribute_error ()))
                        return NULL;
                pcl_error_clear ();
        }
        else if (!PCL_IS_STRING (callable_name))
        {
                pcl_object_unref (callable_name);
                callable_name = NULL;
        }

        if (owner != NULL)
        {
                owner_name = pcl_object_get_attr_string (owner, "__name__");
                if (owner_name == NULL)
                {
                        if (!pcl_error_exception_matches (
                                        pcl_exception_attribute_error ()))
                                return NULL;
                        pcl_error_clear ();
                }
                else if (!PCL_IS_STRING (owner_name))
                {
                        pcl_object_unref (owner_name);
                        owner_name = NULL;
                }
        }

        if (instance == NULL)
                result = pcl_string_from_format (
                        "<unbound method %s.%s>",
                        (owner_name == NULL) ? "?" :
                                PCL_STRING_AS_STRING (owner_name),
                        (callable_name == NULL) ? "?" :
                                PCL_STRING_AS_STRING (callable_name));
        else
        {
                PclObject *instance_repr;
                instance_repr = pcl_object_repr (instance);
                if (instance_repr == NULL)
                        goto fail;
                if (!PCL_IS_STRING (instance_repr))
                {
                        pcl_object_unref (instance_repr);
                        goto fail;
                }
                result = pcl_string_from_format (
                        "<bound method %s.%s of %s>",
                        (owner_name == NULL) ? "?" :
                                PCL_STRING_AS_STRING (owner_name),
                        (callable_name == NULL) ? "?" :
                                PCL_STRING_AS_STRING (callable_name),
                        PCL_STRING_AS_STRING (instance_repr));
                pcl_object_unref (instance_repr);
        }

fail:
        if (callable_name != NULL)
                pcl_object_unref (callable_name);
        if (owner_name != NULL)
                pcl_object_unref (owner_name);
        return result;
}

static gint
instance_method_compare (PclObject *object1, PclObject *object2)
{
        PclInstanceMethod *im1, *im2;

        if (!PCL_IS_INSTANCE_METHOD (object1))
                return 2;
        if (!PCL_IS_INSTANCE_METHOD (object2))
                return 2;

        im1 = PCL_INSTANCE_METHOD (object1);
        im2 = PCL_INSTANCE_METHOD (object2);

        if (im1->instance != im2->instance)
                return (im1->instance < im2->instance) ? -1 : 1;
        return pcl_object_compare (im1->callable, im2->callable);
}

static PclObject *
instance_method_get_attr (PclObject *object, const gchar *name)
{
        PclInstanceMethod *self = PCL_INSTANCE_METHOD (object);
        PclObject *type = PCL_GET_TYPE_OBJECT (object);
        PclObject *result;

        result = mro_lookup (PCL_TYPE (type), name);
        if (result != NULL)
        {
                PclDescriptorClass *class;

                if (!PCL_IS_DESCRIPTOR (result))
                        return pcl_object_ref (result);
                class = PCL_DESCRIPTOR_GET_CLASS (result);
                return class->get (PCL_DESCRIPTOR (result), object, type);
        }

        return pcl_object_get_attr_string (self->callable, name);
}

static gboolean
instance_method_traverse (PclContainer *container, PclTraverseFunc func,
                          gpointer user_data)
{
        PclInstanceMethod *self = PCL_INSTANCE_METHOD (container);

        if (self->callable != NULL)
                if (!func (self->callable, user_data))
                        return FALSE;
        if (self->instance != NULL)
                if (!func (self->instance, user_data))
                        return FALSE;
        if (self->owner != NULL)
                if (!func (self->instance, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (instance_method_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
instance_method_get (PclDescriptor *descriptor,
                     PclObject *object, PclObject *type)
{
        PclInstanceMethod *self = PCL_INSTANCE_METHOD (descriptor);

        /* Don't rebind an already bound method, or an unbound method of a
         * class that's not a base class of the type argument. */

        if (self->instance != NULL)
                return pcl_object_ref (self);  /* already bound */
        if (self->owner != NULL && type != NULL)
        {
                gboolean is_subclass;
                is_subclass = pcl_object_is_subclass (type, self->owner);
                if (is_subclass < 0)
                        return NULL;
                if (!is_subclass)
                        return pcl_object_ref (self);
        }
        return pcl_instance_method_new (self->callable, object, type);
}

static PclObject *
instance_method_get_doc (PclInstanceMethod *self, gpointer context)
{
        return pcl_object_get_attr_string (self->callable, "__doc__");
}

static void
instance_method_class_init (PclInstanceMethodClass *class)
{
        PclDescriptorClass *descriptor_class;
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        instance_method_parent_class = g_type_class_peek_parent (class);

        descriptor_class = PCL_DESCRIPTOR_CLASS (class);
        descriptor_class->get = instance_method_get;

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = instance_method_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_instance_method_get_type_object;
        object_class->new_instance = instance_method_new_instance;
        object_class->call = instance_method_call;
        object_class->hash = instance_method_hash;
        object_class->repr = instance_method_repr;
        object_class->compare = instance_method_compare;
        object_class->get_attr = instance_method_get_attr;
        object_class->doc = instance_method_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = instance_method_dispose;
}

static PclGetSetDef instance_method_getsets[] = {
        { "__doc__",            (PclGetFunc) instance_method_get_doc,
                                (PclSetFunc) NULL },
        { NULL }
};

GType
pcl_instance_method_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclInstanceMethodClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) instance_method_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclInstanceMethod),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_DESCRIPTOR, "PclInstanceMethod",
                        &type_info, 0);
                g_type_set_qdata (
                        type, PCL_DATA_GETSETS, instance_method_getsets);
        }
        return type;
}

/**
 * pcl_instance_method_get_type_object:
 *
 * Returns the type object for #PclInstanceMethod.
 *
 * Returns: a borrowed reference to the type object for #PclInstanceMethod
 */
PclObject *
pcl_instance_method_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (
                        PCL_TYPE_INSTANCE_METHOD, "instancemethod");
                pcl_register_singleton ("<type 'instancemethod'>", &object);
        }
        return object;
}

PclObject *
pcl_instance_method_new (PclObject *callable, PclObject *instance,
                         PclObject *owner)
{
        PclInstanceMethod *instance_method;

        if (!PCL_IS_CALLABLE (callable))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        pcl_object_ref (callable);
        if (instance != NULL)
                pcl_object_ref (instance);
        if (owner != NULL)
                pcl_object_ref (owner);

        instance_method = pcl_factory_order (&instance_method_factory);
        pcl_object_ref (callable);
        instance_method->callable = callable;
        instance_method->instance = instance;
        instance_method->owner = owner;
        return PCL_OBJECT (instance_method);
}

void
_pcl_instance_method_init (void)
{
        instance_method_factory.type = PCL_TYPE_INSTANCE_METHOD;
}

void
_pcl_instance_method_fini (void)
{
        pcl_factory_close (&instance_method_factory);
}
