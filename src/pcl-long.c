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

static gpointer long_parent_class = NULL;

static PclObject *
long_format (PclLong *self, gint base, gboolean addL)
{
        /* TODO */
        g_assert (base >= 2 && base <= 36);
        return pcl_object_ref (PCL_NONE);
}

static void
long_finalize (GObject *g_object)
{
        PclLong *self = PCL_LONG (g_object);

        g_array_free (self->array, TRUE);

        /* Chain up to parent's finalize method. */
        G_OBJECT_CLASS (long_parent_class)->finalize (g_object);
}

static PclObject *
long_repr (PclObject *object)
{
        PclLong *self = PCL_LONG (object);
        return long_format (self, 10, TRUE);
}

static PclObject *
long_str (PclObject *object)
{
        PclLong *self = PCL_LONG (object);
        return long_format (self, 10, FALSE);
}

static PclObject *
long_method_getnewargs (PclObject *self)
{
        return pcl_build_value ("(N)", pcl_object_copy (self));
}

static void
long_class_init (PclLongClass *class)
{
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        long_parent_class = g_type_class_peek_parent (class);

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_long_get_type_object;
        object_class->repr = long_repr;
        object_class->str = long_str;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->finalize = long_finalize;
}

static void
long_init (PclLong *long_obj)
{
        long_obj->array = g_array_new (FALSE, TRUE, sizeof (gshort));
}

static PclMethodDef long_methods[] = {
        { "__getnewargs__",     (PclCFunction) long_method_getnewargs,
                                PCL_METHOD_FLAG_NOARGS },
        { NULL }
};

GType
pcl_long_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclLongClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) long_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclLong),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) long_init,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_OBJECT, "PclLong", &type_info, 0);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, long_methods);
        }
        return type;
}

/**
 * pcl_long_get_type_object:
 *
 * Returns the type object for #PclLong.  During runtime this is the built-in
 * object %long.
 *
 * Returns: a borrowed reference to the type object for #PclLong
 */
PclObject *
pcl_long_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_LONG, "long");
                pcl_register_singleton ("<type 'long'>", &object);
        }
        return object;
}

PclObject *
pcl_long_from_void_ptr (gpointer pointer)
{
        g_assert (sizeof (gpointer) <= sizeof (glong));
        return pcl_int_from_long ((glong) pointer);
}
