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

static gchar type_doc[] =
"type(object) -> the object's type\n\
type(name, bases, dict) -> a new type";

static gpointer type_parent_class = NULL;

/* MRO Utilities */
extern PclObject *mro_algorithm (PclType *type);
extern PclObject *mro_lookup (PclType *type, const gchar *name);

static gpointer
type_descriptor_new (GType g_type, PclType *type, const gchar *name)
{
        PclDescriptor *descriptor;

        descriptor = pcl_object_new (g_type, NULL);
        descriptor->type = pcl_object_ref (type);
        descriptor->name = pcl_string_intern_from_string (name);
        if (descriptor->name == NULL)
        {
                pcl_object_unref (descriptor);
                return NULL;
        }
        return (gpointer) descriptor;
}

static gboolean
type_add_getsets (PclType *type, PclGetSetDef *getset)
{
        PclObject *dict = type->dict;

        for (; getset->name != NULL; getset++)
        {
                gchar *name = getset->name;
                PclGetSetDescriptor *descriptor;
                PclObject *object;

                if (pcl_dict_get_item_string (dict, name) != NULL)
                        continue;
                descriptor = type_descriptor_new (
                        PCL_TYPE_GETSET_DESCRIPTOR, type, name);
                if (descriptor == NULL)
                        return FALSE;
                descriptor->getset = getset;
                object = PCL_OBJECT (descriptor);
                if (!pcl_dict_set_item_string (dict, name, object))
                        return FALSE;
                pcl_object_unref (descriptor);
        }
        return TRUE;
}

static gboolean
type_add_members (PclType *type, PclMemberDef *member)
{
        PclObject *dict = type->dict;

        for (; member->name != NULL; member++)
        {
                gchar *name = member->name;
                PclMemberDescriptor *descriptor;
                PclObject *object;

                if (pcl_dict_get_item_string (dict, name) != NULL)
                        continue;
                descriptor = type_descriptor_new (
                        PCL_TYPE_MEMBER_DESCRIPTOR, type, name);
                if (descriptor == NULL)
                        return FALSE;
                descriptor->member = member;
                object = PCL_OBJECT (descriptor);
                if (!pcl_dict_set_item_string (dict, name, object))
                        return FALSE;
                pcl_object_unref (descriptor);
        }
        return TRUE;
}

static gboolean
type_add_methods (PclType *type, PclMethodDef *method, gpointer base)
{
        PclObject *dict = type->dict;

        for (; method->name != NULL; method++)
        {
                const gchar *name = method->name;
                PclMethodDescriptor *descriptor;
                PclObject *object;

                /* If this is a wrapper method (offset > 0), make sure
                 * the function pointer being wrapped is non-NULL. */
                if (method->offset > 0 &&
                                G_STRUCT_MEMBER (gpointer *,
                                base, method->offset) == NULL)
                        continue;
                /* XXX Need more method flag checking here. */
                descriptor = type_descriptor_new (
                        PCL_TYPE_METHOD_DESCRIPTOR, type, name);
                if (descriptor == NULL)
                        return FALSE;
                descriptor->method = method;
                object = PCL_OBJECT (descriptor);
                if (!pcl_dict_set_item_string (dict, name, object))
                        return FALSE;
                pcl_object_unref (descriptor);
        }
        return TRUE;
}

static gboolean
type_add_descriptors (PclType *type, GType g_type)
{
        PclGetSetDef *getsets;
        PclMemberDef *members;
        PclMethodDef *methods;
        gpointer base;

        getsets = g_type_get_qdata (g_type, PCL_DATA_GETSETS);
        members = g_type_get_qdata (g_type, PCL_DATA_MEMBERS);
        methods = g_type_get_qdata (g_type, PCL_DATA_METHODS);

        if (G_TYPE_IS_INTERFACE (g_type))
                base = g_type_interface_peek (type->object_class, g_type);
        else
                base = type->object_class;

        if (getsets != NULL)
                if (!type_add_getsets (type, getsets))
                        return FALSE;
        if (members != NULL)
                if (!type_add_members (type, members))
                        return FALSE;
        if (methods != NULL)
                if (!type_add_methods (type, methods, base))
                        return FALSE;

        return TRUE;
}

static gboolean
type_add_subclass (PclType *type, PclType *subclass)
{
        PclObject *item;
        PclObject *list;
        PclObject *weak;
        glong ii, size;
        gboolean success;

        list = type->subclasses;
        if (list == NULL)
        {
                list = pcl_list_new (0);
                if (list == NULL)
                        return FALSE;
                type->subclasses = list;
        }
        g_assert (PCL_IS_LIST (list));
        weak = pcl_weak_ref_new (PCL_OBJECT (subclass), NULL);
        size = PCL_LIST_GET_SIZE (list);
        for (ii = 0; ii < size; ii++)
        {
                item = PCL_LIST_GET_ITEM (list, ii);
                if (PCL_WEAK_REF_GET_OBJECT (item) == PCL_NONE)
                        return pcl_list_set_item (list, ii, weak);
        }
        success = pcl_list_append (list, weak);
        pcl_object_unref (weak);
        return success;
}

static PclObject *
type_solid_base (PclObject *type)
{
        PclObject *base;

        g_assert (PCL_IS_TYPE (type));

        /* XXX Skipping extra_ivars() check */
        base = PCL_TYPE (type)->base;
        if (base != NULL)
                return type_solid_base (base);
        else
                return pcl_object_get_type_object ();
}

static PclObject *
type_best_base (PclObject *bases)
{
        /* Calculate the best base amongst multiple base classes.
         * This is the first one that's on the path to the "solid base". */

        PclObject *base;
        PclObject *candidate;
        PclObject *result = NULL;
        PclObject *winner = NULL;
        glong ii, size;

        g_assert (PCL_IS_TUPLE (bases));
        size = PCL_TUPLE_GET_SIZE (bases);
        g_assert (size > 0);

        for (ii = 0; ii < size; ii++)
        {
                base = PCL_TUPLE_GET_ITEM (bases, ii);
                if (!PCL_IS_TYPE (base))
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "bases must be types");
                        return NULL;
                }
                candidate = type_solid_base (base);
                if (winner == NULL)
                {
                        winner = candidate;
                        result = base;
                }
                else if (pcl_type_is_subtype (winner, candidate))
                        continue;
                else if (pcl_type_is_subtype (candidate, winner))
                {
                        winner = candidate;
                        result = base;
                }
                else
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "multiple bases have "
                                "instance layout conflict");
                        return NULL;
                }
        }

        g_assert (result != NULL);
        return result;
}

static void
type_dispose (GObject *g_object)
{
        PclType *self = PCL_TYPE (g_object);

        PCL_CLEAR (self->base);
        PCL_CLEAR (self->bases);
        PCL_CLEAR (self->dict);
        PCL_CLEAR (self->mro);
        PCL_CLEAR (self->subclasses);

        if (self->object_class != NULL)
        {
                g_type_class_unref (self->object_class);
                self->object_class = NULL;
        }

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (type_parent_class)->dispose (g_object);
}

static PclObject *
type_new_instance (PclObject *metatype, PclObject *args, PclObject *kwds)
{
        const gchar *kwlist[] = { "name", "bases", "dict", NULL };
        PclType *type;
        PclObject *base;
        PclObject *bases;
        PclObject *dict;
        PclObject *name;
        PclObject *winner;
        GType g_type;
        glong nargs;
        glong nbases;
        glong nkwds;
        glong ii;

        g_assert (args != NULL && PCL_IS_TUPLE (args));
        g_assert (kwds == NULL || PCL_IS_DICT (kwds));

        nargs = PCL_TUPLE_GET_SIZE (args);
        nkwds = (kwds == NULL) ? 0 : pcl_object_measure (kwds);

        /* Special case: type(x) -> type object */
        g_type = G_OBJECT_TYPE (metatype);
        if (g_type == PCL_TYPE_TYPE && nargs == 1 && nkwds == 0)
                return pcl_object_type (PCL_TUPLE_GET_ITEM (args, 0));
        if (nargs + nkwds != 3)
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "type() takes 1 or 3 arguments");
                return NULL;
        }

        if (!pcl_arg_parse_tuple_and_keywords (
                        args, kwds, "SO!O!:type", kwlist, &name,
                        pcl_tuple_get_type_object (), &bases,
                        pcl_dict_get_type_object (), &dict))
                return NULL;

        /* Determine the proper metatype to deal with this, and check
         * for metatype conflicts while we're at it.  Note that if some
         * other metatype wins to contract, it's possible that its
         * instances are not types. */
        winner = metatype;
        nbases = PCL_TUPLE_GET_SIZE (bases);
        for (ii = 0; ii < nbases; ii++)
        {
                PclObject *candidate;

                candidate =
                        PCL_GET_TYPE_OBJECT (
                        PCL_TUPLE_GET_ITEM (bases, ii));
                if (pcl_type_is_subtype (winner, candidate))
                        continue;
                if (pcl_type_is_subtype (candidate, winner))
                {
                        winner = candidate;
                        continue;
                }
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "metaclass conflict: the metaclass of a derived "
                        "class must be a (non-strict) subclass of the "
                        "metaclasses of all its bases");
                return NULL;
        }
        if (winner != metatype)
        {
                PclObjectClass *winner_class;

                winner_class = PCL_OBJECT_GET_CLASS (winner);
                if (winner_class->new_instance != type_new_instance)
                        return winner_class->new_instance (winner, args, kwds);
                metatype = winner;
        }

        /* Calculate best base, and check that all bases are type objects. */
        base = type_best_base (bases);
        if (base == NULL)
                return NULL;
        /* XXX - Skipping check of BASETYPE flag in base */

        /* XXX - Skipping support for __slots__ */

        /* Allocate the type object. */
        type = pcl_object_new (PCL_TYPE_TYPE, NULL);
        if (type == NULL)
                return NULL;

        /* XXX - Skipping initialization of type flags */

        dict = pcl_object_copy (dict);
        if (dict == NULL)
                return NULL;

        type->bases = bases;
        type->dict = dict;
        type->name = PCL_STRING_AS_STRING (name);

        if (pcl_dict_get_item_string (dict, "__module__") == NULL)
        {
                PclObject *globals;
                PclObject *module;

                globals = pcl_eval_get_globals ();
                if (globals != NULL)
                {
                        module = pcl_dict_get_item_string (
                                        globals, "__name__");
                        if (module != NULL)
                        {
                                if (!pcl_dict_set_item_string (
                                                dict, "__module__", module))
                                        return NULL;
                        }
                }
        }

        return PCL_OBJECT (type);
}

static PclObject *
type_call (PclObject *object, PclObject *args, PclObject *kwds)
{
        PclType *self = PCL_TYPE (object);
        PclObjectClass *class = self->object_class;
        PclObject *result;
        PclObject *type;

        if (class == NULL || class->new_instance == NULL)
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "cannot create '%s' instances",
                        self->name);
                return NULL;
        }

        result = class->new_instance (PCL_OBJECT (self), args, kwds);
        if (result == NULL)
                return NULL;

        /* Ugly exception: when the call was type(something),
         * don't call init_instance on the result. */
        if (PCL_OBJECT (self) == pcl_type_get_type_object () &&
                PCL_IS_TUPLE (args) && PCL_TUPLE_GET_SIZE (args) == 1 &&
                (kwds == NULL || (PCL_IS_DICT (kwds) &&
                pcl_object_measure (kwds) == 0)))
                return result;

        type = PCL_GET_TYPE_OBJECT (result);
        if (!pcl_type_is_subtype (type, PCL_OBJECT (self)))
                return result;

        class = PCL_TYPE (type)->object_class;
        if (class->init_instance == NULL)
                return result;

        if (class->init_instance (result, args, kwds))
                return result;

        pcl_object_unref (result);
        return NULL;
}

static PclObject *
type_copy (PclObject *object)
{
        return pcl_object_ref (object);
}

static PclObject *
type_dict (PclObject *object)
{
        return PCL_TYPE (object)->dict;
}

static guint
type_hash (PclObject *object)
{
        PclType *self = PCL_TYPE (object);
        return (guint) G_OBJECT_CLASS_TYPE (self->object_class);
}

static PclObject *
type_repr (PclObject *object)
{
        PclType *self = PCL_TYPE (object);
        PclObject *module;
        PclObject *name;
        PclObject *result;
        gchar *kind;

        module = pcl_object_get_attr_string (object, "__module__");
        if (module == NULL)
                pcl_error_clear ();
        else if (!PCL_IS_STRING (module))
        {
                pcl_object_unref (module);
                module = NULL;
        }
        else if (strcmp (PCL_STRING_AS_STRING (module), "__builtin__") == 0)
        {
                pcl_object_unref (module);
                module = NULL;
        }

        name = pcl_object_get_attr_string (object, "__name__");
        if (name == NULL)
                return NULL;

        if (PCL_OBJECT_GET_FLAGS (self) & PCL_OBJECT_FLAG_SINGLETON)
                kind = "type";
        else
                kind = "class";

        if (module == NULL)
                result = pcl_string_from_format (
                        "<%s '%s'>", kind, self->name);
        else
                result = pcl_string_from_format (
                        "<%s '%s.%s'>", kind,
                        PCL_STRING_AS_STRING (module),
                        PCL_STRING_AS_STRING (name));

        if (module != NULL)
                pcl_object_unref (module);
        if (name != NULL)
                pcl_object_unref (name);
        return result;
}

static gint
type_compare (PclObject *object1, PclObject *object2)
{
        return (object1 < object2) ? -1 : (object1 > object2) ? 1 : 0;
}

static PclObject *
type_get_attr (PclObject *object, const gchar *name)
{
        /* XXX More to do here... */

        /* Chain up to parent's get_attr method. */
        return PCL_OBJECT_CLASS (type_parent_class)->get_attr (object, name);
}

static gboolean
type_set_attr (PclObject *object, const gchar *name, PclObject *value)
{
        PclType *self = PCL_TYPE (object);

        if (object->flags & PCL_OBJECT_FLAG_SINGLETON)
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "can't set attributes on built-in/extension type '%s'",
                        self->name);
                return FALSE;
        }

        /* Chain up to parent's set_attr method. */
        if (!PCL_OBJECT_GET_CLASS (object)->set_attr (object, name, value))
                return FALSE;
        /* XXX return update_slot (self, name); */
        return TRUE;
}

static gboolean
type_traverse (PclContainer *container, PclTraverseFunc func,
               gpointer user_data)
{
        PclType *self = PCL_TYPE (container);

        if (self->base != NULL)
                if (!func (self->base, user_data))
                        return FALSE;
        if (self->bases != NULL)
                if (!func (self->bases, user_data))
                        return FALSE;
        if (self->dict != NULL)
                if (!func (self->dict, user_data))
                        return FALSE;
        if (self->mro != NULL)
                if (!func (self->mro, user_data))
                        return FALSE;
        if (self->subclasses != NULL)
                if (!func (self->subclasses, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (type_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
type_get_name (PclType *self, gpointer context)
{
        const gchar *cp;

        /* XXX Python checks if the type object was allocated from the heap,
         *     and if so returns PyHeapTypeObject::name. */

        cp = strrchr (self->name, '.');
        if (cp == NULL)
                cp = self->name;
        else
                cp++;
        return pcl_string_from_string (cp);
}

static gboolean
type_set_name (PclType *self, PclObject *value, gpointer context)
{
        const gchar *name;

        if (PCL_OBJECT_GET_FLAGS (self) & PCL_OBJECT_FLAG_SINGLETON)
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "can't set %s.__name__", self->name);
                return FALSE;
        }
        if (value == NULL)
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "can't delete %s.__name__", self->name);
                return FALSE;
        }
        if (!PCL_IS_STRING (value))
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "can only assign string to %s.__name__, not '%s'",
                        self->name, PCL_GET_TYPE_NAME (value));
                return FALSE;
        }
        name = PCL_STRING_AS_STRING (value);
        if (strlen (name) != (gsize) PCL_STRING_GET_SIZE (value))
        {
                pcl_error_set_string (
                        pcl_exception_value_error (),
                        "__name__ must not contain null bytes");
                return FALSE;
        }
        self->name = name;
        return TRUE;
}

static PclObject *
type_get_bases (PclType *self, gpointer context)
{
        return pcl_object_ref (self->bases);
}

static gboolean
type_set_bases (PclType *self, PclObject *value, gpointer context)
{
        if (PCL_OBJECT_GET_FLAGS (self) & PCL_OBJECT_FLAG_SINGLETON)
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "can't set %s.__bases__", self->name);
                return FALSE;
        }
        if (value == NULL)
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "can't delete %s.__bases__", self->name);
                return FALSE;
        }
        if (!PCL_IS_TUPLE (value))
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "can only assign tuple to %s.__bases__, not %s",
                        self->name, PCL_GET_TYPE_NAME (value));
                return FALSE;
        }
        if (PCL_TUPLE_GET_SIZE (value) == 0)
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "can only assign non-empty tuple to %s.__bases__, "
                        "not ()", self->name);
                return FALSE;
        }

        g_warning ("%s: Not yet working; no action taken", G_STRFUNC);
        return TRUE;
}

static PclObject *
type_get_module (PclType *self, gpointer context)
{
        PclObject *module;
        const gchar *dot;

        if (PCL_OBJECT_GET_FLAGS (self) & PCL_OBJECT_FLAG_SINGLETON)
        {
                dot = strrchr (self->name, '.');
                if (dot != NULL)
                        return pcl_string_from_string_and_size (
                                self->name, (gssize) (dot - self->name));
                return pcl_string_from_string ("__builtin__");
        }
        else
        {
                module = pcl_dict_get_item_string (self->dict, "__module__");
                if (module == NULL)
                {
                        pcl_error_set_format (
                                pcl_exception_attribute_error (),
                                "__module__");
                        return NULL;
                }
                return pcl_object_ref (module);
        }
}

static gboolean
type_set_module (PclType *self, PclObject *value, gpointer context)
{
        if (PCL_OBJECT_GET_FLAGS (self) & PCL_OBJECT_FLAG_SINGLETON)
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "can't set %s.__module__", self->name);
                return FALSE;
        }
        if (value == NULL)
        {
                pcl_error_set_format (
                        pcl_exception_type_error (),
                        "can't delete %s.__module__", self->name);
                return FALSE;
        }

        return pcl_dict_set_item_string (self->dict, "__module__", value);
}

static PclObject *
type_get_dict (PclType *self, gpointer context)
{
        if (self->dict == NULL)
                return pcl_object_ref (PCL_NONE);
        return pcl_dict_proxy_new (self->dict);
}

static PclObject *
type_get_doc (PclType *self, gpointer context)
{
        PclObject *doc;

        if ((PCL_OBJECT_GET_FLAGS (self) & PCL_OBJECT_FLAG_SINGLETON) &&
                        self->object_class->doc != NULL)
                return pcl_string_from_string (self->object_class->doc);

        doc = pcl_dict_get_item_string (self->dict, "__doc__");
        if (doc == NULL)
                return pcl_object_ref (PCL_NONE);
        if (PCL_IS_DESCRIPTOR (doc))
                return PCL_DESCRIPTOR_GET_CLASS (doc)->get (
                        PCL_DESCRIPTOR (doc), NULL, PCL_OBJECT (self));
        return pcl_object_ref (doc);
}

static PclObject *
type_method_subclasses (PclType *self)
{
        PclObject *list;
        glong ii, size;

        list = pcl_list_new (0);
        if (list == NULL)
                return NULL;
        if (self->subclasses == NULL)
                return list;
        g_assert (PCL_IS_LIST (self->subclasses));
        size = PCL_LIST_GET_SIZE (self->subclasses);
        for (ii = 0; ii < size; ii++)
        {
                PclObject *item = PCL_LIST_GET_ITEM (self->subclasses, ii);
                /* XXX assert(PyWeakref_CheckRef(item)); */
                item = PCL_WEAK_REF_GET_OBJECT (item);
                if (item != PCL_NONE && !pcl_list_append (list, item))
                {
                        pcl_object_unref (list);
                        return NULL;
                }
        }
        return list;
}

static PclObject *
type_method_mro (PclType *self)
{
        return mro_algorithm (self);
}

static void
type_class_init (PclTypeClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        type_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = type_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_type_get_type_object;
        object_class->new_instance = type_new_instance;
        object_class->call = type_call;
        object_class->copy = type_copy;
        object_class->dict = type_dict;
        object_class->hash = type_hash;
        object_class->repr = type_repr;
        object_class->compare = type_compare;
        object_class->get_attr = type_get_attr;
        object_class->set_attr = type_set_attr;
        object_class->doc = type_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = type_dispose;
}

static void
type_init (PclType *type)
{
        type->dict = pcl_dict_new ();
}

static PclGetSetDef type_getsets[] = {
        { "__name__",           (PclGetFunc) type_get_name,
                                (PclSetFunc) type_set_name },
        { "__bases__",          (PclGetFunc) type_get_bases,
                                (PclSetFunc) type_set_bases },
        { "__module__",         (PclGetFunc) type_get_module,
                                (PclSetFunc) type_set_module },
        { "__dict__",           (PclGetFunc) type_get_dict,
                                (PclSetFunc) NULL },
        { "__doc__",            (PclGetFunc) type_get_doc,
                                (PclSetFunc) NULL },
        { NULL }
};

static PclMethodDef type_methods[] = {
        { "__subclasses__",     (PclCFunction) type_method_subclasses,
                                PCL_METHOD_FLAG_NOARGS,
                                "__subclasses__() -> "
                                "list of immediate subclasses" },
        { "mro",                (PclCFunction) type_method_mro,
                                PCL_METHOD_FLAG_NOARGS,
                                "mro() -> list\n"
                                "return a type's method resolution order" },
        { NULL },
};

GType
pcl_type_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclTypeClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) type_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclType),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) type_init,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclType", &type_info, 0);
                g_type_set_qdata (
                        type, PCL_DATA_GETSETS, type_getsets);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, type_methods);
        }
        return type;
}

/**
 * pcl_type_get_type_object:
 *
 * Returns the type object for #PclType.  During runtime this is the built-in
 * object %type.
 *
 * Returns: a borrowed reference to the type object for #PclType
 */
PclObject *
pcl_type_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_TYPE, "type");
                pcl_register_singleton ("<type 'type'>", &object);
        }
        return object;
}

PclObject *
pcl_type_new (GType g_type, const gchar *name)
{
        PclType *type;
        PclObject *mro;
        GType base, *ifaces = NULL;
        guint ii, n_ifaces;
        glong jj, size;

        type = pcl_object_new (PCL_TYPE_TYPE, NULL);
        type->object_class = g_type_class_ref (g_type);
        if (g_type == PCL_TYPE_OBJECT)
                type->bases = pcl_tuple_new (0);
        else
        {
                PclObject *base_type = pcl_object_get_type_object ();
                type->bases = pcl_build_value ("(O)", base_type);
        }
        type->name = name;  /* static string */

        /* add type-specific descriptors */
        base = G_TYPE_OBJECT;
        while ((base = g_type_next_base (g_type, base)) != 0)
                if (!type_add_descriptors (type, base))
                        goto fail;
        ifaces = g_type_interfaces (g_type, &n_ifaces);
        for (ii = 0; ii < n_ifaces; ii++)
                if (!type_add_descriptors (type, ifaces[ii]))
                        goto fail;
        g_free (ifaces);

        /* set method resolution order */
        mro = mro_algorithm (type);
        if (mro == NULL)
                goto fail;
        type->mro = pcl_sequence_as_tuple (mro);
        pcl_object_unref (mro);

        /* add __doc__ to the type dictionary */
        if (pcl_dict_get_item_string (type->dict, "__doc__") == NULL)
        {
                PclObject *doc;
                if (type->object_class->doc != NULL)
                        doc = pcl_string_from_string (type->object_class->doc);
                else
                        doc = pcl_object_ref (PCL_NONE);
                pcl_dict_set_item_string (type->dict, "__doc__", doc);
                pcl_object_unref (doc);
        }

        /* link into each base class's list of subclasses */
        size = PCL_TUPLE_GET_SIZE (type->bases);
        for (jj = 0; jj < size; jj++)
        {
                PclObject *item = PCL_TUPLE_GET_ITEM (type->bases, jj);
                g_assert (PCL_IS_TYPE (item));
                if (!type_add_subclass (PCL_TYPE (item), type))
                        goto fail;
        }

        return PCL_OBJECT (type);

fail:
        pcl_object_unref (type);
        g_free (ifaces);
        return NULL;
}

PclObject *
pcl_type_get_attr (PclObject *self, const gchar *name)
{
        PclObject *result;

        if (!PCL_IS_TYPE (self))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        result = mro_lookup (PCL_TYPE (self), name);
        if (result != NULL)
        {
                if (PCL_IS_DESCRIPTOR (result))
                        result =
                                PCL_DESCRIPTOR_GET_CLASS (result)->
                                get (PCL_DESCRIPTOR (result), self,
                                PCL_GET_TYPE_OBJECT (self));
                else
                        pcl_object_ref (result);
        }
        if (result == NULL)
        {
                pcl_error_set_string (
                        pcl_exception_attribute_error (), name);
                return NULL;
        }
        return result;
}

gint
pcl_type_is_subtype (PclObject *subtype, PclObject *of)
{
        PclObject *mro;
        glong ii, size;

        if (!PCL_IS_TYPE (subtype) || !PCL_IS_TYPE (of))
        {
                pcl_error_bad_internal_call ();
                return -1;
        }

        mro = PCL_TYPE (subtype)->mro;
        g_assert (PCL_IS_TUPLE (mro));
        size = PCL_TUPLE_GET_SIZE (mro);
        for (ii = 0; ii < size; ii++)
        {
                if (PCL_TUPLE_GET_ITEM (mro, ii) == of)
                        return 1;
        }
        return 0;
}
