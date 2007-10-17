### Process this file with autoconf to produce a configure script

AC_PREREQ([2.54])

m4_define([gva_version], 0.5.0)

AC_INIT([GNOME Video Arcade], [gva_version],
        [mbarnes@redhat.com], gnome-video-arcade)

AC_CONFIG_MACRO_DIR(m4)
AC_CONFIG_HEADER(config.h)
AC_CONFIG_SRCDIR(src/main.c)

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

GLADE_MODULES="libglade-2.0 >= 2.6.0"
PKG_CHECK_MODULES(GLADE, $GLADE_MODULES)
AC_SUBST(GLADE_CFLAGS)
AC_SUBST(GLADE_LIBS)

# --with-gnome=[yes|no]
AC_ARG_WITH([gnome],
        [AC_HELP_STRING([--with-gnome],
        [require GNOME libraries [default=yes]])],
        with_gnome="$withval", with_gnome="yes")
if test "$with_gnome" = "yes"; then
GNOME_MODULES="libgnomeui-2.0 >= 2.14.0"
PKG_CHECK_MODULES(GNOME, $GNOME_MODULES)
AC_DEFINE_UNQUOTED(WITH_GNOME, 1,
        [Define to 1 if you are linking against GNOME libraries])
else
GNOME_CFLAGS=
GNOME_LIBS=
fi
AC_SUBST(GNOME_CFLAGS)
AC_SUBST(GNOME_LIBS)

SQLITE_MODULES="sqlite3 >= 3.0.0"
PKG_CHECK_MODULES(SQLITE, $SQLITE_MODULES)
AC_SUBST(SQLITE_CFLAGS)
AC_SUBST(SQLITE_LIBS)

if test "$USE_MAINTAINER_MODE" = "yes"; then
GLADEUI_MODULES="gladeui-1.0 >= 3.0.0"
PKG_CHECK_MODULES(GLADEUI, $GLADEUI_MODULES)
fi

# Header Checks
AC_HEADER_STDC
AC_CHECK_HEADERS(wordexp.h)

# Program Checks
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_INTLTOOL
AC_PROG_LIBTOOL
AC_PATH_PROG(BASENAME, basename)
AC_PATH_PROG(GCONFTOOL, gconftool-2)

# MAME Program
AC_ARG_VAR([MAME], [MAME command])
AC_PATH_PROGS([MAME], [sdlmame xmame],, [$PATH:/usr/local/games:/usr/games])
case `$BASENAME $MAME` in
        sdlmame*) MAME_BACKEND='gva-mame-sdlmame.$(OBJEXT)' ;;
        xmame*)   MAME_BACKEND='gva-mame-xmame.$(OBJEXT)' ;;
        *)        AC_MSG_ERROR([

        Unable to find a MAME program.

        Please install either sdlmame or xmame, or specify
        the MAME command with a MAME environment variable.

        e.g. MAME=/path/to/program/xmame.x11 ./configure
]) ;;
esac
AC_SUBST([MAME_BACKEND])
AC_DEFINE_UNQUOTED(MAME_PROGRAM, "$MAME", [Location of the MAME program])

# Localization
AH_TEMPLATE(GETTEXT_PACKAGE, [Package name for gettext])
GETTEXT_PACKAGE=gnome-video-arcade
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE")
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT

# Miscellaneous
AM_GCONF_SOURCE_2
GTK_DOC_CHECK(1.6)
GNOME_DOC_INIT

# Optional Features

# --with-history-file=PATH
AC_MSG_CHECKING([for arcade history file])
AC_ARG_WITH([history-file],
        [AC_HELP_STRING([--with-history-file=PATH],
        [path to arcade history file (usually named 'history.dat')])],
        with_history="$withval", with_history="no")
if test "$with_history" = "yes"; then
        AC_MSG_ERROR([--with-history-file missing PATH argument])
fi
if test "$with_history" != "no"; then
        AC_DEFINE_UNQUOTED(HISTORY_FILE, "$with_history",
                [Location of the arcade history file])
fi
AC_MSG_RESULT($with_history)

AC_CONFIG_FILES([
        Makefile
        data/Makefile
        docs/Makefile
        docs/reference/Makefile
        docs/reference/version.xml
        help/Makefile
        maint/Makefile
        po/Makefile.in
        src/Makefile
])
AC_OUTPUT
