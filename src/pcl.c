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

#define MAX_EXIT_FUNCS 32
static gint n_exit_funcs = 0;
static GVoidFunc exit_funcs[MAX_EXIT_FUNCS];

static gboolean initialized = FALSE;
static GLogFunc old_log_handler = NULL;
static GData *singletons = NULL;

/* External Declarations */
extern void _pcl_int_init (void);
extern void _pcl_int_fini (void);
extern void _pcl_dict_init (void);
extern void _pcl_dict_fini (void);
extern void _pcl_list_init (void);
extern void _pcl_list_fini (void);
extern void _pcl_float_init (void);
extern void _pcl_float_fini (void);
extern void _pcl_frame_init (void);
extern void _pcl_frame_fini (void);
extern void _pcl_tuple_init (void);
extern void _pcl_tuple_fini (void);
extern void _pcl_method_init (void);
extern void _pcl_method_fini (void);
extern void _pcl_instance_method_init (void);
extern void _pcl_instance_method_fini (void);
extern PclObject *_pcl_builtin_init (void);
extern PclObject *_pcl_exceptions_init (void);
extern PclObject *_pcl_gc_init (void);
extern PclObject *_pcl_signal_init (void);
extern PclObject *_pcl_sys_init (void);

extern GData *internal_strings;

static void
call_exit_funcs (void)
{
        while (n_exit_funcs > 0)
                (*exit_funcs[--n_exit_funcs]) ();

        fflush (stdout);
        fflush (stderr);
}

static void
log_handler (const gchar *log_domain, GLogLevelFlags log_level,
             const gchar *message, gpointer user_data)
{
        if (PCL_OPTION_DEBUG || !(log_level & G_LOG_LEVEL_DEBUG))
                g_log_default_handler (
                        log_domain, log_level, message, user_data);
        if (PCL_OPTION_DEBUG && (log_level & G_LOG_FLAG_FATAL))
                g_on_error_query (NULL);
}

static void
singleton_destroy_notify (gpointer *p_object)
{
        if (*p_object != NULL)
        {
                gpointer object = *p_object;
                g_nullify_pointer (p_object);
                g_object_run_dispose (object);
                pcl_object_unref (object);
        }
}

static void
singleton_weak_notify (const gchar *name, gpointer where_the_object_was)
{
        g_debug ("Finalizing object %s at %p", name, where_the_object_was);
}

static void
init_main (void)
{
        PclObject *module, *globals;

        if ((module = pcl_import_add_module ("__main__")) == NULL)
                g_error ("Can't create __main__ module");
        globals = pcl_module_get_dict (module);
        if (pcl_dict_get_item_string (globals, "__builtins__") == NULL)
        {
                PclObject *binmod = pcl_import_module ("__builtin__");
                pcl_dict_set_item_string (globals, "__builtins__", binmod);
                pcl_object_unref (binmod);
        }
}

void
pcl_initialize (void)
{
        PclInterpreterState *is;
        PclThreadState *ts;
        PclObject *binmod, *sysmod;
        const gchar *error_message;

        if (initialized)
                return;
        initialized = TRUE;

        error_message = glib_check_version (
                        GLIB_MAJOR_VERSION,
                        GLIB_MINOR_VERSION,
                        GLIB_MICRO_VERSION);
        if (error_message != NULL)
                g_error ("%s", error_message);

        if (!g_thread_supported ())
                g_thread_init (NULL);
        g_type_init ();

        old_log_handler = g_log_set_default_handler (log_handler, NULL);

        g_debug ("Linked against GLib %u.%u.%u",
                glib_major_version,
                glib_minor_version,
                glib_micro_version);

        g_datalist_init (&singletons);

        is = pcl_interpreter_state_new ();
        if (is == NULL)
                g_error ("%s: can't make first interpreter", G_STRFUNC);

        ts = pcl_thread_state_new (is);
        if (ts == NULL)
                g_error ("%s: can't make first thread", G_STRFUNC);

        pcl_thread_state_swap (ts);

        _pcl_int_init ();
        _pcl_dict_init ();
        _pcl_list_init ();
        _pcl_float_init ();
        _pcl_frame_init ();
        _pcl_tuple_init ();
        _pcl_method_init ();
        _pcl_instance_method_init ();

        is->modules = pcl_dict_new ();
        if (is->modules == NULL)
                g_error ("%s: can't make modules dictionary", G_STRFUNC);

        /* initialize __builtin__ module */
        binmod = _pcl_builtin_init ();
        if (binmod == NULL)
                g_error ("%s: can't initialize __builtin__", G_STRFUNC);
        is->builtins = pcl_object_ref (pcl_module_get_dict (binmod));

        /* initialize sys module */
        sysmod = _pcl_sys_init ();
        if (sysmod == NULL)
                g_error ("%s: can't initialize sys", G_STRFUNC);
        is->sysdict = pcl_object_ref (pcl_module_get_dict (sysmod));
        pcl_sys_set_path (g_getenv ("PCLPATH"));
        pcl_dict_set_item_string (is->sysdict, "modules", is->modules);

        _pcl_exceptions_init ();
        _pcl_gc_init ();
        _pcl_signal_init ();

        /* initialize __main__ module */
        init_main ();
}

void
pcl_finalize (void)
{
        PclInterpreterState *is;
        PclThreadState *ts;

        if (!initialized)
                return;

        call_exit_funcs ();
        initialized = FALSE;

        ts = pcl_thread_state_get ();
        is = ts->is;

        /* destroy all modules */
        pcl_import_cleanup ();
        
        /* collect garbage */
        pcl_gc_collect ();

        /* finalize singleton objects */
        g_datalist_clear (&singletons);

        /* clear interpreter state */
        pcl_interpreter_state_clear (is);

        /* destroy current thread */
        pcl_thread_state_swap (NULL);
        pcl_interpreter_state_destroy (is);

        /* cleanup cached objects */
        _pcl_int_fini ();
        _pcl_dict_fini ();
        _pcl_list_fini ();
        _pcl_float_fini ();
        _pcl_frame_fini ();
        _pcl_tuple_fini ();
        _pcl_method_fini ();
        _pcl_instance_method_fini ();

        /* clear internal strings cache (external to pcl-string.c)*/
        if (internal_strings != NULL)
                g_datalist_clear (&internal_strings);
        
        g_assert (old_log_handler != NULL);
        g_log_set_default_handler (old_log_handler, NULL);

        initialized = FALSE;
}

void
pcl_exit (gint status)
{
        pcl_finalize ();
        exit (status);
}

gboolean
pcl_at_exit (GVoidFunc func)
{
        if (n_exit_funcs >= MAX_EXIT_FUNCS)
                return FALSE;
        exit_funcs[n_exit_funcs++] = func;
        return TRUE;
}

void
pcl_register_singleton (const gchar *name, gpointer *p_object)
{
        g_return_if_fail (name != NULL && p_object != NULL);

        if (*p_object == NULL)
                g_error ("%s (\"%s\", NULL)", G_STRFUNC, name);

        if (g_datalist_get_data (&singletons, name) == NULL)
        {
                g_datalist_set_data_full (
                        &singletons, name, p_object,
                        (GDestroyNotify) singleton_destroy_notify);
                g_debug ("First instance of %s at %p", name, *p_object);
        }
        else
                g_warning ("Reinstantiating %s at %p", name, *p_object);

        g_object_weak_ref (*p_object,
                (GWeakNotify) singleton_weak_notify, (gchar *) name);
        PCL_OBJECT (*p_object)->flags |= PCL_OBJECT_FLAG_SINGLETON;
}
