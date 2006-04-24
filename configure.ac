# Process this file with autoconf to produce a configure script.

# Autoconf 2.53b introduced AC_GNU_SOURCE.  This macro defines _GNU_SOURCE
# if we're using GNU's C Library implementation (glibc), which includes a
# number of non-standard extensions.  One of those extensions is the ability
# to fetch backtrace information, which we use in <pcl.c> for debugging.

AC_PREREQ([2.54])

# Making releases:
#       pcl_micro_version += 1
#       pcl_interface_age += 1
#       pcl_binary_age += 1
# If any functions have been added:
#       set pcl_interface_age to 0
# If backwards compatibility has been broken:
#       set pcl_binary_age and pcl_interface_age to 0
m4_define([pcl_major_version], [0])
m4_define([pcl_minor_version], [0])
m4_define([pcl_micro_version], [24])
m4_define([pcl_interface_age], [0])
m4_define([pcl_binary_age],
          [m4_eval(100 * pcl_minor_version + pcl_micro_version)])
m4_define([pcl_version],
          [pcl_major_version.pcl_minor_version.pcl_micro_version])

# Libtool version related macros
m4_define([pcl_lt_release],
          [pcl_major_version.pcl_minor_version])
m4_define([pcl_lt_current],
          [m4_eval(100 * pcl_minor_version + pcl_micro_version - pcl_interface_age)])
m4_define([pcl_lt_revision],
          [pcl_interface_age])
m4_define([pcl_lt_age],
          [m4_eval(pcl_binary_age - pcl_interface_age)])
m4_define([pcl_lt_current_minus_age],
          [m4_eval(pcl_lt_current - pcl_lt_age)])

# If the minor version number is odd, then we want debugging.
# Otherwise we only want minimal debugging support.
# XXX Disable this until we get to version 0.1.
#m4_define([pcl_debug_default],
#          [m4_if(m4_eval(pcl_minor_version % 2), [1], [yes], [minimum])])
m4_define([pcl_debug_default], [minimum])

# Required versions of other packages
m4_define([glib_required_version], [2.8.0])
m4_define([gmodule_required_version], [glib_required_version])
m4_define([gobject_required_version], [glib_required_version])
m4_define([gthread_required_version], [glib_required_version])

AC_INIT([Predicate Constraint Language], [pcl_version],
        [matthew.f.barnes@boeing.com], [pcl])

AC_CONFIG_SRCDIR([src/pcl.h])

# Save this value here, since automake will set CFLAGS later.
cflags_set=${CFLAGS+set}

AM_INIT_AUTOMAKE([no-define])

AM_CONFIG_HEADER([config.h])

PCL_MAJOR_VERSION=pcl_major_version
PCL_MINOR_VERSION=pcl_minor_version
PCL_MICRO_VERSION=pcl_micro_version
PCL_INTERFACE_AGE=pcl_interface_age
PCL_BINARY_AGE=pcl_binary_age
PCL_VERSION=pcl_version

AC_SUBST(PCL_MAJOR_VERSION)
AC_SUBST(PCL_MINOR_VERSION)
AC_SUBST(PCL_MICRO_VERSION)
AC_SUBST(PCL_INTERFACE_AGE)
AC_SUBST(PCL_BINARY_AGE)
AC_SUBST(PCL_VERSION)

AC_DEFINE(
        PCL_MAJOR_VERSION, [pcl_major_version],
        [Define to the PCL major version])
AC_DEFINE(
        PCL_MINOR_VERSION, [pcl_minor_version],
        [Define to the PCL minor version])
AC_DEFINE(
        PCL_MICRO_VERSION, [pcl_micro_version],
        [Define to the PCL micro version])
AC_DEFINE(
        PCL_INTERFACE_AGE, [pcl_interface_age],
        [Define to the PCL interface age])
AC_DEFINE(
        PCL_BINARY_AGE, [pcl_binary_age],
        [Define to the PCL binary age])

# Libtool versioning
LT_RELEASE=pcl_lt_release
LT_CURRENT=pcl_lt_current
LT_REVISION=pcl_lt_revision
LT_AGE=pcl_lt_age
LT_CURRENT_MINUS_AGE=pcl_lt_current_minus_age
AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)
AC_SUBST(LT_CURRENT_MINUS_AGE)

AM_MAINTAINER_MODE

# Libtool option to control which symbols are exported.
# Right now, symbols starting with _ are not exported.
LIBTOOL_EXPORT_OPTIONS='-export-symbols-regex "^[[^_]].*"'
AC_SUBST(LIBTOOL_EXPORT_OPTIONS)

# Package Options
AC_ARG_ENABLE([debug],
        AC_HELP_STRING([--enable-debug=@<:@no/minimum/yes@:>@],
                       [turn on debugging @<:@default=pcl_debug_Default@]),,
        [enable_debug=pcl_debug_default])
AC_ARG_ENABLE([profiling],
        AC_HELP_STRING([--enable-profiling],
                       [instrument code for gprof [default=no]]),,
        [enable_profiling=no])

AC_DEFINE_UNQUOTED(
        PCL_COMPILED_WITH_DEBUGGING, ["${enable_debug}"],
        [Whether PCL was compiled with debugging enabled])

# Header checks
AC_GNU_SOURCE
AC_HEADER_STDC

# Library checks
AC_CHECK_LIB(m, acos)
AC_CHECK_LIB(curses, initscr)
AC_CHECK_LIB(readline, readline)

# Program checks
AC_PROG_CC
AM_PROG_LEX
AC_PROG_YACC
AC_PROG_INSTALL

# Initialize libtool
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

# System services
#AC_SYS_LARGEFILE

# Checks to cover Flex/Bison generated code
# (suggested by autoscan)
AC_CHECK_FUNCS([memset])
AC_CHECK_HEADERS([inttypes.h stddef.h stdlib.h string.h unistd.h])
AC_C_CONST
AC_FUNC_MALLOC
AC_TYPE_SIZE_T

# XXX All this shell script crap is pretty kludgy.  Problem is I'm not sure of
#     the "right" way to manage compiler options.  What appears below is sort
#     of a mix of the way GLib does it and the way Python does it.

CFLAGS=""  # kludge
if test x$cflags_set != xset ; then
        case " $CFLAGS " in
        *[[\ \	]]-g[[\ \	]]*) ;;
        *) CFLAGS="$CFLAGS -g" ;;
        esac
fi

if test "x$enable_debug" = "xyes" ; then
        PCL_DEBUG_FLAGS="-DG_ENABLE_DEBUG"
else
        case " $CFLAGS " in
        *[[\ \	]]-O3[[\ \	]]*) ;;
        *) CFLAGS="$CFLAGS -O3" ;;
        esac
        PCL_DEBUG_FLAGS="-DG_DISABLE_CAST_CHECKS"
        if test "x$enable_debug" = "xno"; then
                PCL_DEBUG_FLAGS="$PCL_DEBUG_FLAGS -DG_DISABLE_ASSERT"
                PCL_DEBUG_FLAGS="$PCL_DEBUG_FLAGS -DG_DISABLE_CHECKS"
        fi
fi
AC_SUBST(PCL_DEBUG_FLAGS)

if test "x$enable_profiling" = "xyes" ; then
        case " $CFLAGS " in
        *[[\ \	]]-pg[[\ \	]]*) ;;
        *) CFLAGS="$CFLAGS -pg"
        esac
        case " $LDFLAGS " in
        *[[\ \	]]-pg[[\ \	]]*) ;;
        *) LDFLAGS="$LDFLAGS -pg"
        esac
        case " $LDFLAGS " in
        *[[\ \	]]-static[[\ \	]]*) ;;
        *) LDFLAGS="$LDFLAGS -static"
        esac
fi

if test "x$GCC" = "xyes"; then
        case " $CFLAGS " in
        *[[\ \	]]-Wall[[\ \	]]*) ;;
        *) CFLAGS="$CFLAGS -Wall" ;;
        esac
        case " $CFLAGS " in
        *[[\ \	]]-Wstrict-prototypes[[\ \	]]*) ;;
        *) CFLAGS="$CFLAGS -Wstrict-prototypes" ;;
        esac
fi

PKG_CHECK_MODULES(DEPENDENCIES,
        [glib-2.0 >= glib_required_version]
        [gmodule-2.0 >= gmodule_required_version]
        [gobject-2.0 >= gobject_required_version]
        [gthread-2.0 >= gthread_required_version])
AC_SUBST(DEPENDENCIES_CFLAGS)
AC_SUBST(DEPENDENCIES_LIBS)
GTK_DOC_CHECK(1.4)

AC_CONFIG_FILES([
pcl.pc
Makefile
src/Makefile
docs/Makefile
docs/reference/Makefile
docs/reference/version.xml
])

AC_OUTPUT
