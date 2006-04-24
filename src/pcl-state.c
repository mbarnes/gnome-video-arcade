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

#define CLEAR(x) \
        G_STMT_START { \
        PclObject *tmp = PCL_OBJECT (x); \
        (x) = NULL; if (tmp != NULL) pcl_object_unref (tmp); \
        } G_STMT_END

static GSList *is_list = NULL;
static GStaticRecMutex list_mutex = G_STATIC_REC_MUTEX_INIT;
static PclThreadState *ts_current = NULL;

GFunc ts_clear = (GFunc) pcl_thread_state_clear;
GFunc ts_destroy = (GFunc) pcl_thread_state_destroy;

PclInterpreterState *
pcl_interpreter_state_new (void)
{
        PclInterpreterState *is;
        is = g_new0 (PclInterpreterState, 1);
        g_static_rec_mutex_lock (&list_mutex);
        is_list = g_slist_prepend (is_list, is);
        g_static_rec_mutex_unlock (&list_mutex);
        return is;
}

void
pcl_interpreter_state_clear (PclInterpreterState *is)
{
        g_static_rec_mutex_lock (&list_mutex);
        g_slist_foreach (is->ts_list, ts_clear, NULL);
        g_static_rec_mutex_unlock (&list_mutex);
        CLEAR (is->modules);
        CLEAR (is->sysdict);
        CLEAR (is->builtins);
}

void
pcl_interpreter_state_destroy (PclInterpreterState *is)
{
        GSList *link;
        g_static_rec_mutex_lock (&list_mutex);
        g_slist_foreach (is->ts_list, ts_destroy, NULL);
        g_slist_free (is->ts_list);
        is->ts_list = NULL;
        link = g_slist_find (is_list, is);
        if (link == NULL)
                g_error ("%s: Invalid interpreter state", G_STRFUNC);
        is_list = g_slist_delete_link (is_list, link);
        g_static_rec_mutex_unlock (&list_mutex);
        g_free (is);
}

PclThreadState *
pcl_thread_state_new (PclInterpreterState *is)
{
        PclThreadState *ts;
        ts = g_new0 (PclThreadState, 1);
        ts->is = is;
        ts->cache = g_hash_table_new_full (
                g_direct_hash, g_direct_equal,
                pcl_object_unref, pcl_object_unref);
        g_static_rec_mutex_lock (&list_mutex);
        is->ts_list = g_slist_prepend (is->ts_list, ts);
        g_static_rec_mutex_unlock (&list_mutex);
        return ts;
}

PclThreadState *
pcl_thread_state_swap (PclThreadState *ts)
{
        PclThreadState *old = ts_current;
        ts_current = ts;
        return old;
}

void
pcl_thread_state_clear (PclThreadState *ts)
{
        ts->frame = NULL;  /* weakref */

        ts->profile_function = NULL;
        ts->trace_function = NULL;
        CLEAR (ts->profile_object);
        CLEAR (ts->trace_object);

        CLEAR (ts->exc_type);
        CLEAR (ts->exc_value);
        CLEAR (ts->exc_traceback);

        CLEAR (ts->current_exc_type);
        CLEAR (ts->current_exc_value);
        CLEAR (ts->current_exc_traceback);

        CLEAR (ts->dict);
}

void
pcl_thread_state_destroy (PclThreadState *ts)
{
        PclInterpreterState *is;
        GSList *link;

        if (ts == NULL)
                g_error ("%s: Null thread state", G_STRFUNC);
        is = ts->is;
        if (is == NULL)
                g_error ("%s: Null interpreter state", G_STRFUNC);
        if (ts == ts_current)
                g_error ("%s: Thread state is still current", G_STRFUNC);
        g_static_rec_mutex_lock (&list_mutex);
        link = g_slist_find (is->ts_list, ts);
        if (link == NULL)
                g_error ("%s: Invalid thread state", G_STRFUNC);
        is->ts_list = g_slist_delete_link (is->ts_list, link);
        g_static_rec_mutex_unlock (&list_mutex);
        g_hash_table_destroy (ts->cache);
        g_free (ts);
}

PclThreadState *
pcl_thread_state_get (void)
{
        if (ts_current == NULL)
                g_error ("%s: No current thread", G_STRFUNC);
        return ts_current;
}

PclObject *
pcl_thread_state_get_dict (void)
{
        if (ts_current == NULL)
                return NULL;

        if (ts_current->dict == NULL)
                ts_current->dict = pcl_dict_new ();
        return ts_current->dict;
}

PclFrame *
pcl_thread_state_get_frame (PclThreadState *ts)
{
        g_return_val_if_fail (ts != NULL, NULL);
        return ts->frame;
}
