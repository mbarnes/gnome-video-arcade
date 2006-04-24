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
#include <signal.h>

/* XXX Platform specific */
#ifndef NSIG
#define NSIG 64         /* use a reasonable default value */
#endif

static gboolean signal_is_tripped = FALSE;
static PclObject *signal_default_handler;
static PclObject *signal_ignore_handler;
static GThread *signal_main_thread;
static pid_t signal_main_pid;

static struct {
        gboolean tripped;
        PclObject *function;
} signal_handlers[NSIG];

static gchar signal_doc[] =
"This module provides mechanisms to use signal handlers in PCL.\n\
\n\
Functions:\n\
\n\
alarm() -- cause SIGALRM after a specified time [Unix only]\n\
signal() -- set the action for a given signal\n\
getsignal() -- get the signal action for a given signal\n\
pause() -- wait until a signal arrives [Unix only]\n\
default_int_handler() -- default SIGINT handler\n\
\n\
Constants:\n\
\n\
SIG_DFL -- used to refer to the system default handler\n\
SIG_IGN -- used to ignore the signal\n\
NSIG -- number of defined signals\n\
\n\
SIGINT, SIGTERM, etc. -- signal numbers\n\
\n\
*** IMPORTANT NOTICE ***\n\
A signal handler function is called with two arguments:\n\
the first is the signal number, the second is the interrupted stack frame.";

static gboolean
signal_check_with_arg (gpointer unused)
{
        return pcl_error_check_signals ();
}

static void
signal_handler (gint sig)
{
        struct sigaction sa;

        if (getpid () == signal_main_pid)
        {
                signal_is_tripped = TRUE;
                signal_handlers[sig].tripped = TRUE;
                pcl_add_pending_call (signal_check_with_arg, NULL);
        }

        sa.sa_handler = signal_handler;
        sigemptyset (&sa.sa_mask);
        sa.sa_flags = 0;

        /* XXX Not checking return value. */
        sigaction (sig, &sa, NULL);
}

static PclObject *
signal_alarm (PclObject *self, PclObject *args)
{
        gint seconds;

        if (!pcl_arg_parse_tuple (args, "i:alarm", &seconds))
                return NULL;
        /* alarm() returns the number of seconds remaining. */
        return pcl_int_from_long ((glong) alarm (seconds));
}

static gchar signal_alarm_doc[] =
"alarm(seconds)\n\
\n\
Arrange for SIGALRM to arrive after the given number of seconds.";

static PclObject *
signal_default_int_handler (PclObject *self, PclObject *args)
{
        pcl_error_set_none (pcl_exception_keyboard_interrupt ());
        return NULL;
}

static gchar signal_default_int_handler_doc[] =
"default_int_handler(...)\n\
\n\
The default handler for SIGINT installed by PCL.\n\
It raises KeyboardInterrupt.";

static PclObject *
signal_getsignal (PclObject *self, PclObject *args)
{
        gint sig;

        if (!pcl_arg_parse_tuple (args, "i:getsignal", &sig))
                return NULL;
        if (sig < 1 || sig >= NSIG)
        {
                pcl_error_set_string (
                        pcl_exception_value_error (),
                        "signal number out of range");
                return NULL;
        }
        return pcl_object_ref (signal_handlers[sig].function);
}

static gchar signal_getsignal_doc[] =
"getsignal(sig) -> action\n\
\n\
Return the current action for the given signal.  The return value can be:\n\
SIG_IGN -- if the signal is being ignored\n\
SIG_DFL -- if the default action for the signal is in effect\n\
None -- if an unknown handler is in effect\n\
anything else -- the callable PCL object used as a handler";

static PclObject *
signal_pause (PclObject *self)
{
        /* XXX Py_BEGIN_ALLOW_THREADS */
        (void) pause();
        /* XXX Py_END_ALLOW_THREADS */

        if (!pcl_error_check_signals ())
                return NULL;
        return pcl_object_ref (PCL_NONE);
}

static gchar signal_pause_doc[] =
"pause()\n\
\n\
Wait until a signal arrives.";

static PclObject *
signal_signal (PclObject *self, PclObject *args)
{
        struct sigaction sa;
        PclObject *object;
        PclObject *old_handler;
        gint sig;

        if (!pcl_arg_parse_tuple (args, "iO:signal", &sig, &object))
                return NULL;

        if (g_thread_self () != signal_main_thread)
        {
                pcl_error_set_string (
                        pcl_exception_value_error (),
                        "signal only works in main thread");
                return NULL;
        }

        if (sig < 1 || sig >= NSIG)
        {
                pcl_error_set_string (
                        pcl_exception_value_error (),
                        "signal number out of range");
                return NULL;
        }

        if (object == signal_ignore_handler)
                sa.sa_handler = SIG_IGN;
        else if (object == signal_default_handler)
                sa.sa_handler = SIG_DFL;
        else if (!PCL_IS_CALLABLE (object))
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "signal handler must be signal.SIG_IGN, "
                        "signal.SIG_DFL, or a callable object");
                return NULL;
        }
        else
                sa.sa_handler = signal_handler;
        sigemptyset (&sa.sa_mask);
        sa.sa_flags = 0;
        if (sigaction (sig, &sa, NULL) == SIG_ERR)
        {
                pcl_error_set_from_errno (pcl_exception_runtime_error ());
                return NULL;
        }

        old_handler = signal_handlers[sig].function;
        signal_handlers[sig].tripped = FALSE;
        signal_handlers[sig].function = pcl_object_ref (object);
        return old_handler;
}

static gchar signal_signal_doc[] =
"signal(sig, action) -> action\n\
\n\
Set the action for the given signal.  The action can be SIG_DFL,\n\
SIG_IGN, or a callable PCL object.  The previous action is returned.\n\
See getsignal() for possible return values.\n\
\n\
*** IMPORTANT NOTICE ***\n\
A signal handler function is called with two arguments:\n\
the first is the signal number, the second is the interrupted stack frame.";

static PclMethodDef signal_methods[] = {
        { "alarm",                      (PclCFunction) signal_alarm,
                                        PCL_METHOD_FLAG_VARARGS,
                                        signal_alarm_doc },
        { "default_int_handler",        (PclCFunction)
                                        signal_default_int_handler,
                                        PCL_METHOD_FLAG_VARARGS,
                                        signal_default_int_handler_doc },
        { "getsignal",                  (PclCFunction) signal_getsignal,
                                        PCL_METHOD_FLAG_VARARGS,
                                        signal_getsignal_doc },
        { "pause",                      (PclCFunction) signal_pause,
                                        PCL_METHOD_FLAG_NOARGS,
                                        signal_pause_doc },
        { "signal",                     (PclCFunction) signal_signal,
                                        PCL_METHOD_FLAG_VARARGS,
                                        signal_signal_doc },
        { NULL }
};

/* Declared in pcl-error.h */
gboolean
pcl_error_check_signals (void)
{
        PclObject *frame;
        gint ii;

        if (!signal_is_tripped)
                return TRUE;

        if (g_thread_self () != signal_main_thread)
                return TRUE;

        frame = (PclObject *) pcl_eval_get_frame ();
        if (frame == NULL)
                frame = PCL_NONE;

        for (ii = 1; ii < NSIG; ii++)
        {
                if (signal_handlers[ii].tripped)
                {
                        PclObject *args;
                        PclObject *result = NULL;

                        signal_handlers[ii].tripped = FALSE;
                        args = pcl_build_value ("(iO)", ii, frame);
                        if (args != NULL)
                        {
                                result = pcl_eval_call_object (
                                        signal_handlers[ii].function, args);
                                pcl_object_unref (args);
                        }
                        if (result == NULL)
                                return FALSE;
                        pcl_object_unref (result);
                }
        }

        signal_is_tripped = FALSE;
        return TRUE;
}

/* Declared in pcl-error.h */
void
pcl_error_set_interrupt (void)
{
        signal_is_tripped = TRUE;
        signal_handlers[SIGINT].tripped = TRUE;
        pcl_add_pending_call (signal_check_with_arg, NULL);
}

/* Declared in pcl-error.h */
gboolean
pcl_os_interrupt_occurred (void)
{
        if (signal_handlers[SIGINT].tripped)
        {
                if (g_thread_self () != signal_main_thread)
                        return FALSE;
                signal_handlers[SIGINT].tripped = FALSE;
                return TRUE;
        }
        return FALSE;
}

PclObject *
_pcl_signal_init (void)
{
        PclObject *dict;
        PclObject *module;
        PclObject *object;
        PclObject *sigint;
        gint ii;

        signal_main_thread = g_thread_self ();
        signal_main_pid = getpid ();

        module = pcl_module_init ("signal", signal_methods, signal_doc);
        dict = pcl_module_get_dict (module);

        object = pcl_long_from_void_ptr ((gpointer) SIG_DFL);
        signal_default_handler = pcl_object_ref (object);
        pcl_module_add_object (module, "SIG_DFL", object);

        object = pcl_long_from_void_ptr ((gpointer) SIG_IGN);
        signal_ignore_handler = pcl_object_ref (object);
        pcl_module_add_object (module, "SIG_IGN", object);

        pcl_module_add_int (module, "NSIG", NSIG);

        sigint = pcl_dict_get_item_string (dict, "default_int_handler");

        signal_handlers[0].tripped = FALSE;
        for (ii = 1; ii < NSIG; ii++)
        {
                struct sigaction sa;

                object = PCL_NONE;
                signal_handlers[ii].tripped = FALSE;
                if (sigaction (ii, NULL, &sa) == 0)
                {
                        if (sa.sa_handler == SIG_DFL)
                                object = signal_default_handler;
                        if (sa.sa_handler == SIG_IGN)
                                object = signal_ignore_handler;
                }
                signal_handlers[ii].function = pcl_object_ref (object);
        }
        if (signal_handlers[SIGINT].function == signal_default_handler)
        {
                struct sigaction sa;

                sa.sa_handler = signal_handler;
                sigemptyset (&sa.sa_mask);
                sa.sa_flags = 0;
                /* XXX Not saving old SIGINT handler. */
                if (sigaction (SIGINT, &sa, NULL) == 0)
                {
                        pcl_object_ref (sigint);
                        pcl_object_unref (signal_handlers[SIGINT].function);
                        signal_handlers[SIGINT].function = sigint;
                }
        }

#ifdef SIGABRT
        pcl_module_add_int (module, "SIGABRT", SIGABRT);
#endif
#ifdef SIGALRM
        pcl_module_add_int (module, "SIGALRM", SIGALRM);
#endif
#ifdef SIGBUS
        pcl_module_add_int (module, "SIGBUS", SIGBUS);
#endif
#ifdef SIGCHLD
        pcl_module_add_int (module, "SIGCHLD", SIGCHLD);
#endif
#ifdef SIGCONT
        pcl_module_add_int (module, "SIGCONT", SIGCONT);
#endif
#ifdef SIGFPE
        pcl_module_add_int (module, "SIGFPE", SIGFPE);
#endif
#ifdef SIGHUP
        pcl_module_add_int (module, "SIGHUP", SIGHUP);
#endif
#ifdef SIGILL
        pcl_module_add_int (module, "SIGILL", SIGILL);
#endif
#ifdef SIGINT
        pcl_module_add_int (module, "SIGINT", SIGINT);
#endif
#ifdef SIGKILL
        pcl_module_add_int (module, "SIGKILL", SIGKILL);
#endif
#ifdef SIGPIPE
        pcl_module_add_int (module, "SIGPIPE", SIGPIPE);
#endif
#ifdef SIGPOLL
        pcl_module_add_int (module, "SIGPOLL", SIGPOLL);
#endif
#ifdef SIGPROF
        pcl_module_add_int (module, "SIGPROF", SIGPROF);
#endif
#ifdef SIGQUIT
        pcl_module_add_int (module, "SIGQUIT", SIGQUIT);
#endif
#ifdef SIGRTMIN
        pcl_module_add_int (module, "SIGRTMIN", SIGRTMIN);
#endif
#ifdef SIGRTMAX
        pcl_module_add_int (module, "SIGRTMAX", SIGRTMAX);
#endif
#ifdef SIGSEGV
        pcl_module_add_int (module, "SIGSEGV", SIGSEGV);
#endif
#ifdef SIGSTOP
        pcl_module_add_int (module, "SIGSTOP", SIGSTOP);
#endif
#ifdef SIGSYS
        pcl_module_add_int (module, "SIGSYS", SIGSYS);
#endif
#ifdef SIGTERM
        pcl_module_add_int (module, "SIGTERM", SIGTERM);
#endif
#ifdef SIGTRAP
        pcl_module_add_int (module, "SIGTRAP", SIGTRAP);
#endif
#ifdef SIGTSTP
        pcl_module_add_int (module, "SIGTSTP", SIGTSTP);
#endif
#ifdef SIGTTIN
        pcl_module_add_int (module, "SIGTTIN", SIGTTIN);
#endif
#ifdef SIGTTOU
        pcl_module_add_int (module, "SIGTTOU", SIGTTOU);
#endif
#ifdef SIGURG
        pcl_module_add_int (module, "SIGURG", SIGURG);
#endif
#ifdef SIGUSR1
        pcl_module_add_int (module, "SIGUSR1", SIGUSR1);
#endif
#ifdef SIGUSR2
        pcl_module_add_int (module, "SIGUSR2", SIGUSR2);
#endif
#ifdef SIGVTALRM
        pcl_module_add_int (module, "SIGVTALRM", SIGVTALRM);
#endif
#ifdef SIGXCPU
        pcl_module_add_int (module, "SIGXCPU", SIGXCPU);
#endif
#ifdef SIGXFSZ
        pcl_module_add_int (module, "SIGXFSZ", SIGXFSZ);
#endif

        return module;
}
