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

#define GC_NUM_GENERATIONS      (G_N_ELEMENTS (gc_generations))
#define GC_GEN_HEAD(n)          (&gc_generations[n].head)

#define GC_REF_STATE_REACHABLE  0
#define GC_REF_STATE_SEARCHING  1
#define GC_REF_STATE_CANDIDATE  2

typedef struct _GC_Generation GC_Generation;

struct _GC_Generation {
        PclGCList head;
        gint threshold;         /* collection threshold */
        gint count;             /* count of allocations or collections
                                   of younger generations */
};

/* Linked lists of PclContainer objects */
static GC_Generation gc_generations[] = {
        /* PclGCList,                           threshold,      count */
        { { GC_GEN_HEAD (0), GC_GEN_HEAD (0) },       700,          0 },
        { { GC_GEN_HEAD (1), GC_GEN_HEAD (1) },        10,          0 },
        { { GC_GEN_HEAD (2), GC_GEN_HEAD (2) },        10,          0 }
};

/* XXX This structure is declared privately in GObject, but GObject's public
 *     API doesn't provide a way to iterate over the weak reference list.  So
 *     we're forced to copy this here and keep it in sync with GObject. */
typedef struct {
        GObject *object;
        guint n_weak_refs;
        struct {
                GWeakNotify notify;
                gpointer data;
        } weak_refs[1];  /* flexible array */
} WeakRefStack;

/* XXX Each object's WeakRefStack is stored in the object's GData structure
 *     under the name "GObject-weak-references", but that name is not in
 *     GObject's API so we're not supposed to know that either. */
#define WEAK_REFS "GObject-weak-references"
static GQuark quark_weak_refs = 0;

static gchar gc_doc[] =
"This module provides access to the garbage collector for reference cycles.\n\
\n\
enable() -- Enable automatic garbage collection.\n\
disable() -- Disable automatic garbage collection.\n\
isenabled() -- Returns true if automatic collection is enabled.\n\
collect() -- Do a full collection right now.\n\
set_debug() -- Set debugging flags.\n\
get_debug() -- Get debugging flags.\n\
set_threshold() -- Set the collection thresholds.\n\
get_threshold() -- Return the current collection thresholds.\n\
get_objects() -- Return a list of all objects tracked by the collector.\n\
get_referrers() -- Return the list of objects that refer to an object.\n\
get_referents() -- Return the list of objects that an object refers to.\n";

static gboolean gc_enabled = TRUE;
static gboolean gc_collecting = FALSE;
static PclObject *gc_garbage = NULL;
static guint gc_debug_flags = 0;

static void
gc_list_init (PclGCList *list)
{
        list->prev = list;
        list->next = list;
}

static glong
gc_list_size (PclGCList *list)
{
        PclGCList *link;
        glong size = 0;

        for (link = list->next; link != list; link = link->next)
                size++;
        return size;
}

static gboolean
gc_list_is_empty (PclGCList *list)
{
        return (list->next == list);
}

static void
gc_list_move (PclGCList *link, PclGCList *list)
{
        PclGCList *new_prev;
        PclGCList *current_prev = link->prev;
        PclGCList *current_next = link->next;

        /* unlink from current list */
        current_prev->next = current_next;
        current_next->prev = current_prev;

        /* relink at end of new list */
        new_prev = link->prev = list->prev;
        new_prev->next = list->prev = link;
        link->next = list;
}

static void
gc_list_merge (PclGCList *from_list, PclGCList *to_list)
{
        PclGCList *to_tail;
        g_assert (from_list != to_list);
        if (!gc_list_is_empty (from_list))
        {
                to_tail = to_list->prev;
                to_tail->next = from_list->next;
                to_tail->next->prev = to_tail;
                to_list->prev = from_list->prev;
                to_list->prev->next = to_list;
        }
        gc_list_init (from_list);
}

static gboolean
gc_traverse (PclGCList *link, PclTraverseFunc func, gpointer data)
{
        PclContainerClass *class;

        class = PCL_CONTAINER_GET_CLASS (link->object);
        if (class != NULL && class->traverse != NULL)
                return class->traverse (link->object, func, data);
        return FALSE;
}

/* traversal callback for gc_select_candidates() */
static gboolean
gc_visit_unref (gpointer object, gpointer data)
{
        if (PCL_IS_CONTAINER (object))
        {
                PclGCList *link = &PCL_CONTAINER (object)->gc;
                if (link->ref_state == GC_REF_STATE_SEARCHING)
                {
                        g_assert (link->ref_count > 0);
                        link->ref_count--;
                }
        }
        return TRUE;
}

/* traversal callback for gc_select_candidates() */
static gboolean
gc_visit_reachable (gpointer object, gpointer data)
{
        PclGCList *generation = data;

        if (PCL_IS_CONTAINER (object))
        {
                PclGCList *link = &PCL_CONTAINER (object)->gc;
                if (link->ref_state == GC_REF_STATE_SEARCHING)
                {
                        /* This is in the candidate selection list, but the
                         * traversal hasn't yet gotten to it.  All we need
                         * to do is indicate that it's reachable. */
                        if (link->ref_count == 0)
                                link->ref_count = 1;
                }
                else if (link->ref_state == GC_REF_STATE_CANDIDATE)
                {
                        /* This had a reference count of 0 when the traversal
                         * got to it, but turns out it's reachable after all.
                         * Move it back to the candidate selection list, and
                         * the traversal will eventually get to it again. */
                        gc_list_move (link, generation);
                        link->ref_state = GC_REF_STATE_SEARCHING;
                        link->ref_count = 1;
                }
        }
        return TRUE;
}

static void
gc_select_candidates (PclGCList *list, PclGCList *candidates)
{
        PclGCList *link;

        for (link = list->next; link != list; link = link->next)
        {
                /* Start with GObject's reference count. */
                GObject *object = link->object;
                g_assert (G_IS_OBJECT (object));
                g_assert (link->ref_state == GC_REF_STATE_REACHABLE);
                link->ref_state = GC_REF_STATE_SEARCHING;
                link->ref_count = object->ref_count;
                g_assert (link->ref_count > 0);

                /* If the object is recyclable (i.e. a factory is tracking
                 * it with a toggle reference), subtract one to account for
                 * the toggle reference. */
                if (PCL_OBJECT_GET_FLAGS (object) & PCL_OBJECT_FLAG_RECYCLABLE)
                        link->ref_count--;
        }
        for (link = list->next; link != list; link = link->next)
        {
                /* Subtract internal references. */
                gc_traverse (link, gc_visit_unref, NULL);
        }

        link = list->next;
        while (link != list)
        {
                PclGCList *next;

                if (link->ref_count > 0)
                {
                        /* This is definitely reachable from outside the set.
                         * Mark it as such, and traverse its pointers to find
                         * any other objects that may be reachable from it. */
                        link->ref_state = GC_REF_STATE_REACHABLE;
                        gc_traverse (link, gc_visit_reachable, list);
                        next = link->next;
                }
                else
                {
                        /* This *may* be unreachable.  To make progress,
                         * assume it is.  The object isn't directly reachable
                         * from any object we've already traversed, but may be
                         * reachable from an object we haven't gotten to yet.
                         * gc_visit_reachable() will move this back into the
                         * set if that's so, and we'll see it again. */
                        next = link->next;
                        gc_list_move (link, candidates);
                        link->ref_state = GC_REF_STATE_CANDIDATE;
                }

                link = next;
        }
}

/* traversal callback for gc_select_finalizers() */
static gboolean
gc_visit_move (gpointer object, gpointer data)
{
        PclGCList *finalizers = data;

        if (PCL_IS_CONTAINER (object))
        {
                PclGCList *link = &PCL_CONTAINER (object)->gc;
                if (link->ref_state == GC_REF_STATE_CANDIDATE)
                {
                        link->ref_state = GC_REF_STATE_REACHABLE;
                        gc_list_move (link, finalizers);
                }
        }
        return TRUE;
}

static void
gc_select_finalizers (PclGCList *list, PclGCList *finalizers)
{
        PclGCList *link, *next;

        for (link = list->next; link != list; link = next)
        {
                next = link->next;
                g_assert (link->ref_state == GC_REF_STATE_CANDIDATE);
                /* FIXME - Don't know how I'm going to do finalizers yet,
                 *         so skip the detection part for now.  The calling
                 *         function will think we just didn't find any. */
                if (FALSE)
                {
                        link->ref_state = GC_REF_STATE_REACHABLE;
                        gc_list_move (link, finalizers);
                }
        }

        /* Move objects that are reachable from finalizers, from the
         * candidates set to the finalizers set. */
        for (link = finalizers->next; link != finalizers; link = link->next)
                gc_traverse (link, gc_visit_move, finalizers);
}

static gint
gc_handle_weakrefs (PclGCList *list, PclGCList *generation)
{
        PclGCList wrcb_to_call; /* weakrefs with callbacks to call */
        PclGCList *link, *next;
        PclWeakRef *wr;
        gint num_freed = 0;

        gc_list_init (&wrcb_to_call);

        /* Clear all weakrefs to the objects in 'list'.  If such a weakref also
         * has a callback, move it into 'wrcb_to_call' if the callback needs to
         * be invoked.  Note that we cannot invoke any callbacks until all
         * weakrefs to unreachable objects are cleared, lest the callback
         * resurrect an unreachable object via a still-active weakref.  We
         * make another pass over 'wrcb_to_call', invoking callbacks, after
         * this pass completes. */
        for (link = list->next; link != list; link = next)
        {
                WeakRefStack *wstack;
                guint ii;

                g_assert (link->ref_state == GC_REF_STATE_CANDIDATE);

                next = link->next;

                /* XXX Hacking on GObject's privates here.  How naughty! */
                wstack = g_object_get_qdata (link->object, quark_weak_refs);
                if (wstack == NULL || wstack->n_weak_refs == 0)
                        continue;

                for (ii = 0; ii < wstack->n_weak_refs; ii++)
                {
                        PclGCList *gc;
                        gpointer object;

                        object = wstack->weak_refs[ii].data;
                        g_assert (PCL_IS_WEAK_REF (object));

                        wr = PCL_WEAK_REF (object);
                        wr->object = PCL_NONE;
                        if (wr->callback == NULL)
                                continue;

                        /* Headache time.  An object in 'list' is going away,
                         * and is weakly referenced by 'wr', which has a
                         * callback.  Should the callback be invoked?  If 'wr'
                         * is also trash, no.
                         *
                         * 1. There's no need to call it.  The object and the
                         *    weakref are both going away, so it's legitimate
                         *    to pretend the weakref is going away first.  The
                         *    user has to ensure a weakref outlives its
                         *    referent if they want a guarantee that the 'wr'
                         *    callback will get invoked.
                         *
                         * 2. It may be catastrophic to call it.  If the
                         *    callback is also in cyclic trash (CT), then
                         *    although the CT is unreachable from outside the
                         *    current generation, CT may be reachable from the
                         *    callback.  Then the callback could resurrect
                         *    insane objects.
                         *
                         * Since the callback is never needed and may be unsafe
                         * in this case, 'wr' is simply left in the unreachable
                         * set.  Note that because we already set the referent
                         * to PCL_NONE, its callback will never trigger.
                         *
                         * OTOH, if 'wr' isn't part of CT, we should invoke the
                         * callback: the weakref outlived the trash.  Note that
                         * since 'wr' isn't CT in this case, its callback can't
                         * be CT either -- 'wr' acted as an external root to
                         * this generation, and therefore its callback did too.
                         * So nothing in CT is reachable from the callback
                         * either, so it's hard to imagine how calling it later
                         * could create a problem for us.  'wr' is moved to
                         * 'wrcb_to_call' in this case. */

                        gc = &PCL_CONTAINER (object)->gc;
                        if (gc->ref_state == GC_REF_STATE_CANDIDATE)
                                continue;
                        g_assert (gc->ref_state == GC_REF_STATE_REACHABLE);

                        pcl_object_ref (object);
                        gc_list_move (gc, &wrcb_to_call);
                }
        }

        /* Invoke the callbacks we decided to honor.  It's safe to invoke them
         * because they can't reference unreachable objects. */
        while (!gc_list_is_empty (&wrcb_to_call))
        {
                PclObject *result;

                link = wrcb_to_call.next;
                g_assert (link->ref_state == GC_REF_STATE_REACHABLE);
                g_assert (PCL_IS_WEAK_REF (link->object));
                wr = PCL_WEAK_REF (link->object);
                g_assert (wr->callback != NULL);

                result = pcl_object_call_function (wr->callback, "O", wr);
                if (result == NULL)
                        pcl_error_write_unraisable (wr->callback);
                else
                        pcl_object_unref (result);

                pcl_object_unref (wr);

                /* If object is still alive, move it. */
                if (wrcb_to_call.next == link)
                        gc_list_move (link, generation);
                else
                        num_freed++;
        }

        return num_freed;
}

static void
gc_delete_garbage (PclGCList *collectable, PclGCList *generation)
{
        while (!gc_list_is_empty (collectable))
        {
                PclGCList *link = collectable->next;
                g_object_run_dispose (link->object);
                if (collectable->next == link)
                {
                        /* Object is still alive so move it into an older
                         * garbage generation.  It may die later. */
                        gc_list_move (link, generation);
                        link->ref_state = GC_REF_STATE_REACHABLE;
                }
        }
}

static gboolean
gc_handle_finalizers (PclGCList *finalizers, PclGCList *generation)
{
        PclGCList *link;

        g_assert (gc_garbage != NULL);

        for (link = finalizers->next; link != finalizers; link = link->next)
        {
                PclObject *object = link->object;

                /* XXX Missing call to has_finalizer(). */
                if (gc_debug_flags & PCL_GC_DEBUG_SAVEALL)
                {
                        if (!pcl_list_append (gc_garbage, object))
                                return FALSE;
                }
        }
        gc_list_merge (finalizers, generation);
        return TRUE;
}

static glong
gc_do_collect (gint generation)
{
        glong noc = 0;          /* # objects collected */
        glong nnc = 0;          /* # unreachable objects not collected */
        PclGCList *young;       /* the generation we are examining */
        PclGCList *old;         /* the next older generation */
        PclGCList *link;        /* list iterator */
        PclGCList candidates;   /* candidates for deletion */
        PclGCList finalizers;   /* objects with, and reachable from, __del__ */
        gint ii;

        if (gc_debug_flags & PCL_GC_DEBUG_STATS)
        {
                g_message ("GC: Collecting generation %d...", generation);
                for (ii = 0; ii < GC_NUM_GENERATIONS; ii++)
                {
                        glong size = gc_list_size (GC_GEN_HEAD (ii));
                        g_message ("GC: Generation %d has %ld object%s",
                                ii, size, size != 1 ? "s" : "");
                }
        }

        /* Update collection and allocation counters. */
        if (generation + 1 < GC_NUM_GENERATIONS)
                gc_generations[generation + 1].count += 1;
        for (ii = 0; ii <= generation; ii++)
                gc_generations[ii].count = 0;

        /* Merge younger generations with one we are currently collecting. */
        for (ii = 0; ii < generation; ii++)
                gc_list_merge (GC_GEN_HEAD (ii), GC_GEN_HEAD (generation));

        /* handy references */
        young = GC_GEN_HEAD (generation);
        if (generation < GC_NUM_GENERATIONS - 1)
                old = GC_GEN_HEAD (generation + 1);
        else
                old = young;

        /* Objects in the PclContainer set are reachable from outside the
         * set if they have a reference count greater than 0 when all the
         * references within the set are taken into account.  Those which
         * are not reachable are candidates for deletion.  Separate them
         * from the reachable objects. */
        gc_list_init (&candidates);
        gc_select_candidates (young, &candidates);

        /* Move reachable objects to next generation. */
        if (young != old)
                gc_list_merge (young, old);

        /* All of the candidates are garbage, but objects reachable from
         * finalizers can't safely be deleted.  PCL programmers should take
         * care not to create such things.  For PCL, finalizers means class
         * instances with __del__ methods.  Separate the finalizers and the
         * objects reachable from them. */
        gc_list_init (&finalizers);
        gc_select_finalizers (&candidates, &finalizers);

        /* Collect statistics on collectable objects found and print debugging
         * information. */
        for (link = candidates.next; link != &candidates; link = link->next)
        {
                noc++;
                if (gc_debug_flags & PCL_GC_DEBUG_COLLECTABLE)
                        g_message ("GC: Collectable <'%s' object at %p>",
                                PCL_GET_TYPE_NAME (link->object),
                                link->object);
        }

        /* Clear weakrefs and invoke callbacks as necessary. */
        noc += gc_handle_weakrefs (&candidates, old);

        /* Call dispose() on the remaining candidates.  This will cause the
         * reference cycles to be broken.  It may also cause some objects in
         * finalizers to be freed. */
        gc_delete_garbage (&candidates, old);

        /* Collect statistics on uncollectable objects found and print
         * debugging information. */
        for (link = finalizers.next; link != &finalizers; link = link->next)
        {
                nnc++;
                if (gc_debug_flags & PCL_GC_DEBUG_UNCOLLECTABLE)
                        pcl_sys_write_stderr (
                                "GC: Uncollectable <'%s' object at %p>",
                                PCL_GET_TYPE_NAME (link->object),
                                link->object);
        }
        if (gc_debug_flags & PCL_GC_DEBUG_STATS)
        {
                if (noc == 0 && nnc == 0)
                        g_message ("GC: Done");
                else
                        g_message ("GC: Done, "
                                "%ld unreachable, %ld uncollectable",
                                noc + nnc, nnc);
        }

        /* Append instances in the uncollectable set to a PCL reachable list
         * of garbage.  The programmer has to deal with this if they insist
         * on creating this type of structure. */
        gc_handle_finalizers (&finalizers, old);

        if (pcl_error_occurred ())
        {
                pcl_error_write_unraisable (
                        pcl_string_from_string ("garbage collection"));
                g_error ("unexpected exception during garbage collection");
        }

        return 0;
}

static glong
gc_collect_generations (void)
{
        glong n_unreachable = 0;
        gint ii;

        /* Find the oldest generation (highest numbered) where the count
         * exceeds the threshold.  Objects in that generation and generations
         * younger than it will be collected. */
        for (ii = GC_NUM_GENERATIONS - 1; ii >= 0; ii--)
        {
                if (gc_generations[ii].count > gc_generations[ii].threshold)
                {
                        g_debug ("Collecting garbage in generation[%d]", ii);
                        n_unreachable = gc_do_collect (ii);
                        g_debug ("Collected %ld object(s)", n_unreachable);
                        break;
                }
        }

        return n_unreachable;
}

glong
pcl_gc_collect (void)
{
        glong n_unreachable;

        if (gc_collecting)
                n_unreachable = 0;
        else
        {
                gc_collecting = TRUE;
                n_unreachable = gc_do_collect (GC_NUM_GENERATIONS - 1);
                gc_collecting = FALSE;
        }

        return n_unreachable;
}

void
pcl_gc_list_insert (PclGCList *link)
{
        g_return_if_fail (link != NULL);

        gc_generations[0].count++;
        if (gc_generations[0].count > gc_generations[0].threshold)
        {
                /* additional constraints */
                if (gc_enabled && gc_generations[0].threshold > 0 &&
                        !gc_collecting && !pcl_error_occurred ())
                {
                        gc_collecting = TRUE;
                        gc_collect_generations ();
                        gc_collecting = FALSE;
                }
        }

        link->next = GC_GEN_HEAD (0);
        link->prev = GC_GEN_HEAD (0)->prev;
        link->prev->next = GC_GEN_HEAD (0)->prev = link;
}

void
pcl_gc_list_remove (PclGCList *link)
{
        g_return_if_fail (link != NULL);

        /* This is critical for factory-produced objects. */
        link->ref_state = GC_REF_STATE_REACHABLE;

        if (link->prev != NULL && link->next != NULL)
        {
                link->prev->next = link->next;
                link->next->prev = link->prev;
                link->prev = link->next = NULL;

                if (gc_generations[0].count > 0)
                        gc_generations[0].count--;
        }
}

/*****************************************************************************/

static PclObject *
gc_collect (PclObject *self)
{
        return pcl_int_from_long (pcl_gc_collect ());
}

static gchar gc_collect_doc[] =
"collect() -> n\n\
\n\
Run a full collection.  The number of unreachable objects is returned.";

static PclObject *
gc_disable (PclObject *self)
{
        if (gc_enabled)
                g_debug ("Disabling automatic garbage collection");
        gc_enabled = FALSE;
        return pcl_object_ref (PCL_NONE);
}

static gchar gc_disable_doc[] =
"disable() -> None\n\
\n\
Disable automatic garbage collection.";

static PclObject *
gc_enable (PclObject *self)
{
        if (!gc_enabled)
                g_debug ("Enabling automatic garbage collection");
        gc_enabled = TRUE;
        return pcl_object_ref (PCL_NONE);
}

static gchar gc_enable_doc[] =
"enable() -> None\n\
\n\
Enable automatic garbage collection.";

static PclObject *
gc_get_debug (PclObject *self)
{
        return pcl_build_value ("i", gc_debug_flags);
}

static gchar gc_get_debug_doc[] =
"get_debug() -> flags\n\
\n\
Get the garbage collection debugging flags.";

static PclObject *
gc_get_objects (PclObject *self)
{
        PclObject *result;
        gint ii;

        result = pcl_list_new (0);
        if (result == NULL)
                return NULL;
        for (ii = 0; ii < GC_NUM_GENERATIONS; ii++)
        {
                PclGCList *link;
                PclGCList *list;

                list = GC_GEN_HEAD (ii);
                for (link = list->next; link != list; link = link->next)
                {
                        PclObject *object;
                        object = link->object;
                        if (object == result)
                                continue;
                        if (!pcl_list_append (result, object))
                        {
                                pcl_object_unref (result);
                                return NULL;
                        }
                }
        }
        return result;
}

static gchar gc_get_objects_doc[] =
"get_objects() -> [...]\n\
\n\
Return a list of objects tracked by the collector (excluding the list\n\
returned).";

/* traversal callback for gc_get_referents() */
static gboolean
gc_visit_referents (gpointer object, gpointer list)
{
        return pcl_list_append (list, object);
}

static PclObject *
gc_get_referents (PclObject *self, PclObject *args)
{
        PclObject *result;
        glong ii, size;

        result = pcl_list_new (0);
        if (result == NULL)
                return NULL;
        size = PCL_TUPLE_GET_SIZE (args);
        for (ii = 0; ii < size; ii++)
        {
                PclContainerClass *class;
                PclObject *object;

                object = PCL_TUPLE_GET_ITEM (args, ii);
                if (!PCL_IS_CONTAINER (object))
                        continue;
                class = PCL_CONTAINER_GET_CLASS (object);
                if (class == NULL || class->traverse == NULL)
                        continue;
                if (!class->traverse (PCL_CONTAINER (object),
                        gc_visit_referents, result))
                {
                        pcl_object_unref (result);
                        return NULL;
                }
        }
        return result;
}

static gchar gc_get_referents_doc[] =
"get_referents(*objs) -> list\n\
\n\
Return the list of objects that are directly referred to by objs.";

/* traversal callback for gc_get_referrers() */
static gboolean
gc_visit_referrers (gpointer object, gpointer args)
{
        /* XXX Here's a case for why the traverse function should return a
         *     gint instead of a gboolean.  This won't work as written! */
        glong ii, size;

        size = PCL_TUPLE_GET_SIZE (args);
        for (ii = 0; ii < size; ii++)
                if (PCL_TUPLE_GET_ITEM (args, ii) == object)
                        return TRUE;  /* found */
        return FALSE;  /* not found */
}

static PclObject *
gc_get_referrers (PclObject *self, PclObject *args)
{
        PclObject *result;
        gint ii;

        result = pcl_list_new (0);
        if (result == NULL)
                return NULL;
        for (ii = 0; ii < GC_NUM_GENERATIONS; ii++)
        {
                PclGCList *link;
                PclGCList *list;

                list = GC_GEN_HEAD (ii);
                for (link = list->next; link != list; link = link->next)
                {
                        PclContainerClass *class;
                        PclObject *object;

                        object = link->object;
                        if (object == args || object == result)
                                continue;
                        class = PCL_CONTAINER_GET_CLASS (object);
                        if (class == NULL || class->traverse == NULL)
                                continue;
                        if (class->traverse (PCL_CONTAINER (object),
                                gc_visit_referrers, args))
                        {
                                if (!pcl_list_append (result, object))
                                {
                                        pcl_object_unref (result);
                                        return NULL;
                                }
                        }
                }
        }
        return result;
}

static gchar gc_get_referrers_doc[] =
"get_referrers(*objs) -> list\n\
\n\
Return the list of objects that directly refer to any of objs.";

static PclObject *
gc_get_threshold (PclObject *self)
{
        return pcl_build_value ("(iii)",
                        gc_generations[0].threshold,
                        gc_generations[1].threshold,
                        gc_generations[2].threshold);
}

static gchar gc_get_threshold_doc[] =
"get_threshold() -> (threshold0, threshold1, threshold2)\n\
\n\
Return the current collection thresholds.";

static PclObject *
gc_isenabled (PclObject *self)
{
        return pcl_bool_from_boolean (gc_enabled);
}

static gchar gc_isenabled_doc[] =
"isenabled() -> status\n\
\n\
Returns true if automatic garbage collection is enabled.";

static PclObject *
gc_set_debug (PclObject *self, PclObject *args)
{
        /* XXX Format code should be 'I' for unsigned int, not 'i'. */
        if (!pcl_arg_parse_tuple (args, "i:set_debug", &gc_debug_flags))
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

static gchar gc_set_debug_doc[] =
"set_debug(flags) -> None\n\
\n\
Set the garbage collection debugging flags.  Debugging information is\n\
written to sys.stderr.\n\
\n\
flags is an integer and can have the following bits turned on:\n\
\n\
  DEBUG_STATS - Print statistics during collection.\n\
  DEBUG_COLLECTABLE - Print collectable objects found.\n\
  DEBUG_UNCOLLECTABLE - Print unreachable but collectable objects found.\n\
  DEBUG_SAVEALL - Save objects to gc.garbage rather than freeing them.\n\
  DEBUG_LEAK - Debug leaking programs (everything but STATS).\n";

static PclObject *
gc_set_threshold (PclObject *self, PclObject *args)
{
        gint ii;

        if (!pcl_arg_parse_tuple (args, "i|ii:set_threshold",
                                &gc_generations[0].threshold,
                                &gc_generations[1].threshold,
                                &gc_generations[2].threshold))
                return NULL;

        for (ii = 2; ii < GC_NUM_GENERATIONS; ii++)
        {
                /* generations higher than 2 get the same threshold */
                gc_generations[ii].threshold = gc_generations[2].threshold;
        }

        return pcl_object_ref (PCL_NONE);
}

static gchar gc_set_threshold_doc[] =
"set_threshold(threshold0, [threshold1, threshold2]) -> None\n\
\n\
Sets the collection thresholds.  Setting threshold0 to zero disables\n\
collection.";

static PclMethodDef gc_methods[] = {
        { "collect",            (PclCFunction) gc_collect,
                                PCL_METHOD_FLAG_NOARGS,
                                gc_collect_doc },
        { "disable",            (PclCFunction) gc_disable,
                                PCL_METHOD_FLAG_NOARGS,
                                gc_disable_doc },
        { "enable",             (PclCFunction) gc_enable,
                                PCL_METHOD_FLAG_NOARGS,
                                gc_enable_doc },
        { "get_debug",          (PclCFunction) gc_get_debug,
                                PCL_METHOD_FLAG_NOARGS,
                                gc_get_debug_doc },
        { "get_objects",        (PclCFunction) gc_get_objects,
                                PCL_METHOD_FLAG_NOARGS,
                                gc_get_objects_doc },
        { "get_referents",      (PclCFunction) gc_get_referents,
                                PCL_METHOD_FLAG_VARARGS,
                                gc_get_referents_doc },
        { "get_referrers",      (PclCFunction) gc_get_referrers,
                                PCL_METHOD_FLAG_VARARGS,
                                gc_get_referrers_doc },
        { "get_threshold",      (PclCFunction) gc_get_threshold,
                                PCL_METHOD_FLAG_NOARGS,
                                gc_get_threshold_doc },
        { "isenabled",          (PclCFunction) gc_isenabled,
                                PCL_METHOD_FLAG_NOARGS,
                                gc_isenabled_doc },
        { "set_debug",          (PclCFunction) gc_set_debug,
                                PCL_METHOD_FLAG_VARARGS,
                                gc_set_debug_doc },
        { "set_threshold",      (PclCFunction) gc_set_threshold,
                                PCL_METHOD_FLAG_VARARGS,
                                gc_set_threshold_doc },
        { NULL }
};

PclObject *
_pcl_gc_init (void)
{
        PclObject *module;
        const gchar *env_string;

        env_string = g_getenv ("PCL_GC_DEBUG");
        if (env_string != NULL)
        {
                static GDebugKey debug_keys[] = {
                        { "stats",         PCL_GC_DEBUG_STATS },
                        { "collectable",   PCL_GC_DEBUG_COLLECTABLE },
                        { "uncollectable", PCL_GC_DEBUG_UNCOLLECTABLE },
                        { "saveall",       PCL_GC_DEBUG_SAVEALL },
                        { "leak",          PCL_GC_DEBUG_LEAK }
                };

                gc_debug_flags = g_parse_debug_string (
                        env_string, debug_keys, G_N_ELEMENTS (debug_keys));
                env_string = NULL;
        }

        module = pcl_module_init ("gc", gc_methods, gc_doc);
        if (module == NULL)
                return NULL;

        if (gc_garbage == NULL)
        {
                gc_garbage = pcl_list_new (0);
                if (gc_garbage == NULL)
                        return NULL;
        }

        if (!pcl_module_add_object (module, "garbage", gc_garbage))
                return NULL;

#define ADD_INT(NAME) \
        if (!pcl_module_add_int (module, #NAME, PCL_GC_##NAME)) \
                return NULL;

        ADD_INT (DEBUG_STATS);
        ADD_INT (DEBUG_COLLECTABLE);
        ADD_INT (DEBUG_UNCOLLECTABLE);
        ADD_INT (DEBUG_SAVEALL);
        ADD_INT (DEBUG_LEAK);

#undef ADD_INT

        g_debug ("Automatic garbage collection is %s",
                (gc_enabled) ? "enabled" : "disabled");

        quark_weak_refs = g_quark_from_static_string (WEAK_REFS);

        return module;
}
