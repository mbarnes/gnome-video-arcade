### Process this file with autoconf to produce a configure script

AC_PREREQ([2.54])

m4_define([gva_version], 0.3.0)

AC_INIT([GNOME Video Arcade], [gva_version],
        [mbarnes@redhat.com], gnome-video-arcade)

AC_CONFIG_SRCDIR(src/main.c)
AC_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

GVA_VERSION=gva_version
AC_SUBST(GVA_VERSION)

# Dependencies
GLIB_MODULES="glib-2.0 >= 2.12"
PKG_CHECK_MODULES(GLIB, $GLIB_MODULES)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

GTK_MODULES="gtk+-2.0 >= 2.10"
PKG_CHECK_MODULES(GTK, $GTK_MODULES)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

GCONF_MODULES="gconf-2.0 >= 2.0.0"
PKG_CHECK_MODULES(GCONF, $GCONF_MODULES)
AC_SUBST(GCONF_CFLAGS)
AC_SUBST(GCONF_LIBS)

GLADE_MODULES="libglade-2.0 >= 2.0.0"
PKG_CHECK_MODULES(GLADE, $GLADE_MODULES)
AC_SUBST(GLADE_CFLAGS)
AC_SUBST(GLADE_LIBS)

SQLITE_MODULES="sqlite3 >= 3.0.0"
PKG_CHECK_MODULES(SQLITE, $SQLITE_MODULES)
AC_SUBST(SQLITE_CFLAGS)
AC_SUBST(SQLITE_LIBS)

# Header Checks
AC_HEADER_STDC
AC_CHECK_HEADERS(wordexp.h)

# Program Checks
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_INTLTOOL
AC_PROG_LIBTOOL
AC_PATH_PROG(GCONFTOOL, gconftool-2)

# Localization
AH_TEMPLATE(GETTEXT_PACKAGE, [Package name for gettext])
GETTEXT_PACKAGE=gnome-video-arcade
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE")
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT

# Miscellaneous
AM_GCONF_SOURCE_2

# Optional Features

# --with-history-file=PATH
AC_MSG_CHECKING([for arcade history file])
AC_ARG_WITH([history-file],
        AC_HELP_STRING([--with-history-file=PATH],
        [path to arcade history file (usually named 'history.dat')]),
        with_history="$withval", with_history="no")
if test "x${with_history}" = "xyes"; then
        AC_MSG_ERROR([--with-history-file missing PATH argument])
fi
if test "x${with_history}" != "xno"; then
        AC_DEFINE_UNQUOTED(HISTORY_FILE, "$with_history",
                [Location of the arcade history file])
fi
AC_MSG_RESULT($with_history)

AC_CONFIG_FILES([Makefile data/Makefile src/Makefile po/Makefile.in])
AC_OUTPUT
