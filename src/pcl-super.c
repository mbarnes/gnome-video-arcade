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

static gchar super_doc[] =
"super(type) -> unbound super object\n\
super(type, obj) -> bound super object; requires isinstance(obj, type)\n\
super(type, type2) -> bound super object; requires issubclass(type2, type)\n\
Typical use to call a cooperative superclass method:\n\
class C(B):\n\
    def meth(self, arg):\n\
        super(C, self).meth(arg)";

static gpointer super_parent_class = NULL;

static PclObject *
super_check (PclObject *type, PclObject *object)
{
        PclObject *object_class;
        PclObject *object_type;

        /* Check that a super() call makes sense.  Return a type object.
         *
         * object can be a class, or an instance of one:
         *
         * - If it is a class, it must be a subclass of 'type'.  This case is
         *   used for class methods; the return value is object.
         *
         * - If it is an instance, it must be an instance of 'type'.  This is
         *   the normal case; the return value is object.__class__.
         *
         * But... when object is an instance, we want to allow for the case
         * where type(object) is not a subclass of type, but object.__class__
         * is!  This will allow using super() with a proxy for object.
         */

        /* check for first bullet above (special case) */
        if (PCL_IS_TYPE (object) && pcl_type_is_subtype (object, type))
                return pcl_object_ref (object);

        /* normal case */
        object_type = PCL_GET_TYPE_OBJECT (object);
        if (pcl_type_is_subtype (object_type, type))
                return pcl_object_ref (object_type);

        /* try the slow way */
        object_class = pcl_object_get_attr_string (object, "__class__");
        if (PCL_IS_TYPE (object_class) && object_class != object_type)
                if (pcl_type_is_subtype (object_class, type))
                        return object_class;

        if (object_class == NULL)
                pcl_error_clear ();
        else
                pcl_object_unref (object_class);
        pcl_error_set_string (
                pcl_exception_type_error (),
                "super(type, obj): "
                "obj must be an instance of subtype of type");
        return NULL;
}

PclObject *
super_new (PclObject *type, PclObject *object)
{
        PclSuper *super;
        PclObject *object_type = NULL;

        g_assert (PCL_IS_TYPE (type));

        if (object == PCL_NONE)
                object = NULL;
        if (object != NULL)
        {
                object_type = super_check (type, object);
                if (object_type == NULL)
                        return NULL;
                pcl_object_ref (object);
        }

        super = pcl_object_new (PCL_TYPE_SUPER, NULL);
        super->type = pcl_object_ref (type);
        super->object = object;
        super->object_type = object_type;
        return PCL_OBJECT (super);
}
static void
super_dispose (GObject *g_object)
{
        PclSuper *self = PCL_SUPER (g_object);

        PCL_CLEAR (self->type);
        PCL_CLEAR (self->object);
        PCL_CLEAR (self->object_type);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (super_parent_class)->dispose (g_object);
}

static PclObject *
super_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        PclObject *super_type;
        PclObject *object = NULL;

        if (!pcl_arg_no_keywords ("super()", kwds))
                return NULL;

        if (!pcl_arg_parse_tuple (args, "O!|O:super",
                        pcl_type_get_type_object (), &super_type, &object))
                return NULL;

        return super_new (super_type, object);
}

static PclObject *
super_repr (PclObject *object)
{
        PclSuper *self = PCL_SUPER (object);
        const gchar *class_name;

        if (self->type != NULL)
                class_name = PCL_TYPE (self->type)->name;
        else
                class_name = NULL;

        if (self->object_type != NULL)
                return pcl_string_from_format (
                        "<super: <class '%s'>, <%s object>>",
                        class_name, PCL_TYPE (self->object_type)->name);
        else
                return pcl_string_from_format (
                        "<super: <class '%s'>, NULL>", class_name);
}

static PclObject *
super_get_attr (PclObject *object, const gchar *name)
{
        PclSuper *self = PCL_SUPER (object);
        gboolean skip = (self->object_type == NULL);

        if (!skip)
        {
                /* We want __class__ to return the class of the super object,
                 * not the class of self->object. */
                skip = (strcmp (name, "__class__") == 0);
        }

        if (!skip)
        {
                PclObject *(*get) (PclDescriptor *, PclObject *, PclObject *);

                PclObject *mro;
                PclObject *result;
                PclObject *temp;
                glong ii, size;

                mro = PCL_TYPE (self->object_type)->mro;
                result = NULL;

                if (mro == NULL)
                        size = 0;
                else
                {
                        g_assert (PCL_IS_TUPLE (mro));
                        size = PCL_TUPLE_GET_SIZE (mro);
                }
                for (ii = 0; ii < size; ii++)
                        if (self->type == PCL_TUPLE_GET_ITEM (mro, ii))
                                break;
                while (++ii < size)
                {
                        temp = PCL_TUPLE_GET_ITEM (mro, ii);
                        if (!PCL_IS_TYPE (temp))
                                continue;
                        result = pcl_dict_get_item_string (
                                PCL_TYPE (temp)->dict, name);
                        if (result == NULL)
                                continue;
                        if (!PCL_IS_DESCRIPTOR (result))
                                return pcl_object_ref (result);
                        get = PCL_DESCRIPTOR_GET_CLASS (result)->get;
                        if (get == NULL)
                                return pcl_object_ref (result);
                        if (self->object != self->object_type)
                                temp = self->object;
                        else
                                temp = NULL;
                        return get (PCL_DESCRIPTOR (result),
                                temp, self->object_type);
                }
        }

        /* Chain up to parent's get_attr method. */
        return PCL_OBJECT_CLASS (super_parent_class)->get_attr (object, name);
}

static gboolean
super_traverse (PclContainer *container, PclTraverseFunc func,
                gpointer user_data)
{
        PclSuper *self = PCL_SUPER (container);

        if (self->type != NULL)
                if (!func (self->type, user_data))
                        return FALSE;
        if (self->object != NULL)
                if (!func (self->object, user_data))
                        return FALSE;
        if (self->object_type != NULL)
                if (!func (self->object_type, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (super_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
super_get (PclDescriptor *descriptor, PclObject *object, PclObject *type)
{
        PclSuper *self = PCL_SUPER (descriptor);

        /* Not binding to an object, or already bound. */
        if (object == NULL || object == PCL_NONE || self->object != NULL)
                return pcl_object_ref (self);

        /* If self is not an instance of a subclass of super, call its type. */
        if (self->type != pcl_super_get_type_object ())
                return pcl_object_call_function (
                                self->type, "OO", self->type, object);

        return super_new (self->type, object);
}

static void
super_class_init (PclSuperClass *class)
{
        PclDescriptorClass *descriptor_class;
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        super_parent_class = g_type_class_peek_parent (class);

        descriptor_class = PCL_DESCRIPTOR_CLASS (class);
        descriptor_class->get = super_get;

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = super_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_super_get_type_object;
        object_class->new_instance = super_new_instance;
        object_class->repr = super_repr;
        object_class->get_attr = super_get_attr;
        object_class->doc = super_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = super_dispose;
}

GType
pcl_super_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclSuperClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) super_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclSuper),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_DESCRIPTOR, "PclSuper", &type_info, 0);
        }
        return type;
}

/**
 * pcl_super_get_type_object:
 *
 * Returns the type object for #PclSuper.  During runtime this is the built-in
 * object %super.
 *
 * Returns: a borrowed reference to the type object for #PclSuper
 */
PclObject *
pcl_super_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_SUPER, "super");
                pcl_register_singleton ("<type 'super'>", &object);
        }
        return object;
}
