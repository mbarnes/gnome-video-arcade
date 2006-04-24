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

#ifndef PCL_CONFIG_H
#define PCL_CONFIG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>   /* For errno */
#include <math.h>    /* For HUGE_VAL, all the functions */
#include <signal.h>  /* For signal defs, sigaction(), sigprocmask() */
#include <stdarg.h>  /* For va_start(), va_arg(), va_end() */
#include <stdio.h>   /* For FILE, clearerr(), fclose(), ferror(), fflush(),
                        fileno(), fopen(), fprintf(), fread(), ftell(),
                        fseek(), stdin, stdout, stderr */
#include <stdlib.h>  /* For exit() */
#include <string.h>  /* For memchr(), memmove()*, strchr(), strcmp(), strlen(),
                        strrchr() */
#include <unistd.h>  /* For ftruncate(), getpid(), isatty() */

#include <sys/stat.h>  /* For struct stat, S_ISDIR, fstat() */
#include <sys/types.h> /* For pid_t */

#include <glib.h>          /* GLib API */
#include <gmodule.h>       /* GModule API */
#include <glib-object.h>   /* GObject API */

/* NOTE: Need memmove() from <string.h> due to the use of GLib's G_VA_COPY
 *       macro, which *may* invoke g_memmove(), which in turn will *probably*
 *       invoke memmove().  */

#define PCL_MAX_BLOCKS 50

/* Runtime Option Flags */
extern gboolean PCL_OPTION_DEBUG;
extern gboolean PCL_OPTION_PARSE_ONLY;

#endif /* PCL_CONFIG_H */
