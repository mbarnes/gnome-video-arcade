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

void
factory_toggle_notify (gpointer data, GObject *object, gboolean is_last_ref)
{
        PclFactory *factory = data;
        PclGCList *gc_link;

        if (PCL_IS_CONTAINER (object))
                gc_link = &PCL_CONTAINER (object)->gc;
        else
                gc_link = NULL;

        if (is_last_ref)
        {
                /* all other references have been dropped */
                if (factory->limit == 0 || factory->length < factory->limit)
                {
                        /* release all references to other objects
                         * note, don't call g_object_run_dispose();
                         * it increments object's reference count */
                        G_OBJECT_GET_CLASS (object)->dispose (object);

                        /* save the object in the free list */
                        factory->list =
                                g_slist_prepend (
                                factory->list, object);
                        factory->length++;

                        /* disable GC tracking (if applicable) */
                        if (gc_link != NULL)
                                pcl_gc_list_remove (gc_link);
                }
                else
                {
                        /* free list is full; we have to let it go */
                        g_object_remove_toggle_ref (
                                object, factory_toggle_notify, factory);
                }
        }
        else
        {
                /* a free object is being recycled */
                factory->list =
                        g_slist_delete_link (
                        factory->list, factory->list);
                factory->length--;

                /* enable GC tracking (if applicable) */
                if (gc_link != NULL)
                        pcl_gc_list_insert (gc_link);
        }
}

gpointer
pcl_factory_order (PclFactory *factory)
{
        gpointer object;

        g_return_val_if_fail (factory != NULL, NULL);

        if (factory->list != NULL)
                return pcl_object_ref (factory->list->data);

        object = pcl_object_new (factory->type, NULL);
        PCL_OBJECT (object)->flags |= PCL_OBJECT_FLAG_RECYCLABLE;
        g_object_add_toggle_ref (object, factory_toggle_notify, factory);
        return object;
}

void
pcl_factory_close (PclFactory *factory)
{
        GSList *link;

        g_return_if_fail (factory != NULL);

        for (link = factory->list; link != NULL; link = link->next)
                g_object_remove_toggle_ref (
                        link->data, factory_toggle_notify, factory);

        g_slist_free (factory->list);
        factory->length = 0;
}
