### Process this file with autoconf to produce a configure script

AC_PREREQ([2.54])

m4_define([gva_version], 0.0.1)

AC_INIT([GNOME Video Arcade], [gva_version],
        [matthew@barnes.net], gnome-video-arcade)

AC_CONFIG_SRCDIR(src/main.c)
AC_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

GVA_VERSION=gva_version
AC_SUBST(GVA_VERSION)

GCONF_MODULES="gconf-2.0 >= 2.0.0"
PKG_CHECK_MODULES(GCONF, $GCONF_MODULES)
AC_SUBST(GCONF_CFLAGS)
AC_SUBST(GCONF_LIBS)

GLADE_MODULES="libglade-2.0 >= 2.0.0"
PKG_CHECK_MODULES(GLADE, $GLADE_MODULES)
AC_SUBST(GLADE_CFLAGS)
AC_SUBST(GLADE_LIBS)

# Header Checks
AC_HEADER_STDC
AC_CHECK_HEADERS(wordexp.h)

# Program Checks
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_INTLTOOL
AC_PATH_PROG(GCONFTOOL, gconftool-2)

AM_GCONF_SOURCE_2

# Localization
AH_TEMPLATE(GETTEXT_PACKAGE, [Package name for gettext])
GETTEXT_PACKAGE=gnome-video-arcade
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE")
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT

AC_CONFIG_FILES([Makefile data/Makefile src/Makefile po/Makefile.in])
AC_OUTPUT
