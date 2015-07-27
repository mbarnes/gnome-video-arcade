### Process this file with autoconf to produce a configure script

AC_PREREQ([2.54])

m4_define([gva_version], 0.8.4)

AC_INIT([GNOME Video Arcade], [gva_version],
        [mbarnes@redhat.com], gnome-video-arcade)

AC_CONFIG_MACRO_DIR(m4)
AC_CONFIG_HEADER(config.h)
AC_CONFIG_SRCDIR(src/main.c)

AM_INIT_AUTOMAKE([1.11 no-dist-gzip dist-xz])
AM_MAINTAINER_MODE

GVA_VERSION=gva_version
AC_SUBST(GVA_VERSION)

AC_SUBST(abs_top_builddir)

# Automake 1.11 - Silent Build Rules
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Dependencies

# Keep these two definitions in agreement.
m4_define([glib_minimum_version], [2.28])
m4_define([glib_encoded_version], [GLIB_VERSION_2_28])

# Keep these two definitions in agreement.
m4_define([gdk_minimum_version], [3.0])
m4_define([gdk_encoded_version], [GDK_VERSION_3_0])

# Keep these two definitions in agreement.
m4_define([soup_minimum_version], [2.34])
m4_define([soup_encoded_version], [SOUP_VERSION_2_34])

PKG_CHECK_MODULES(GLIB, [gio-2.0 >= glib_minimum_version])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(GTK, [gtk+-3.0 >= gdk_minimum_version])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

PKG_CHECK_MODULES(SOUP, [libsoup-2.4 >= soup_minimum_version])
AC_SUBST(SOUP_CFLAGS)
AC_SUBST(SOUP_LIBS)

PKG_CHECK_MODULES(SQLITE, [sqlite3 >= 3.0])
AC_SUBST(SQLITE_CFLAGS)
AC_SUBST(SQLITE_LIBS)

# Warn about API usage that violates our minimum requirements.
DEP_CPPFLAGS="$DEP_CPPFLAGS -DGLIB_VERSION_MAX_ALLOWED=glib_encoded_version"
DEP_CPPFLAGS="$DEP_CPPFLAGS -DGDK_VERSION_MAX_ALLOWED=gdk_encoded_version"
DEP_CPPFLAGS="$DEP_CPPFLAGS -DSOUP_VERSION_MAX_ALLOWED=soup_encoded_version"

# These will suppress warnings about newly-deprecated symbols.  Ideally
# these settings should match our minimum requirements and we will clean
# up any new deprecation warnings after bumping our minimum requirements.
# But if the warnings get to be overwhelming, use fixed versions instead.
DEP_CPPFLAGS="$DEP_CPPFLAGS -DGLIB_VERSION_MIN_REQUIRED=glib_encoded_version"
DEP_CPPFLAGS="$DEP_CPPFLAGS -DGDK_VERSION_MIN_REQUIRED=gdk_encoded_version"
DEP_CPPFLAGS="$DEP_CPPFLAGS -DSOUP_VERSION_MIN_REQUIRED=soup_encoded_version"

AC_SUBST(DEP_CPPFLAGS)

# Needed for the monospace font name.
GDS_MODULES="gsettings-desktop-schemas"
PKG_CHECK_MODULES(GSETTINGS_DESKTOP_SCHEMAS, $GDS_MODULES)

# --with-wnck=[yes|no]
AC_ARG_WITH([wnck],
        [AC_HELP_STRING([--with-wnck],
        [manage game windows with libwnck [default=yes]])],
        with_wnck="$withval", with_wnck="yes")
if test "$with_wnck" = "yes"; then
WNCK_MODULES="libwnck-3.0 >= 2.91.6"
PKG_CHECK_MODULES(WNCK, $WNCK_MODULES)
AC_DEFINE_UNQUOTED(HAVE_WNCK, 1,
        [Define to 1 if you are using libwnck])
else
WNCK_CFLAGS=
WNCK_LIBS=
fi
AC_SUBST(WNCK_CFLAGS)
AC_SUBST(WNCK_LIBS)

# --with-glade-catalog=[yes|no]
AC_ARG_WITH([glade-catalog],
        [AC_HELP_STRING([--with-glade-catalog],
                [install the catalog files for Glade 3 ]
                [(for maintainers only) [default=no]])],
        with_catalog="$withval", with_catalog="no")
if test "$with_catalog" = "yes"; then
        GLADEUI_MODULES="gladeui-2.0 >= 3.10.0"
        PKG_CHECK_MODULES(GLADEUI, $GLADEUI_MODULES)
fi
AM_CONDITIONAL(GLADE_CATALOG, test $with_catalog = yes)

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
AC_PATH_PROGS([MAME], [mame sdlmame],, [$PATH:/usr/local/games:/usr/games])
if test "x$MAME" != "x"; then
        MAME_PROGRAM=$MAME
        MAME_BACKEND='gva-mame-sdlmame.$(OBJEXT)'
else
        AC_ARG_VAR([XMAME], [XMAME command])
        AC_PATH_PROG([XMAME], [xmame],, [$PATH:/usr/local/games:/usr/games])
        if test "x$XMAME" != "x"; then
                MAME_PROGRAM=$XMAME
                MAME_BACKEND='gva-mame-xmame.$(OBJEXT)'
        fi
fi
if test "x$MAME" = "x"; then
        AC_MSG_ERROR([

        Unable to find a MAME program.

        Please install either sdlmame or xmame, or specify the
        MAME command with a MAME or XMAME environment variable.

        e.g. XMAME=/path/to/program/xmame.x11 ./configure
])
fi
AC_SUBST([MAME_BACKEND])
AC_DEFINE_UNQUOTED(MAME_PROGRAM, "$MAME_PROGRAM", [Location of the MAME program])

# Localization
AH_TEMPLATE(GETTEXT_PACKAGE, [Package name for gettext])
GETTEXT_PACKAGE=gnome-video-arcade
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE")
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT

# Miscellaneous
GLIB_GSETTINGS
GTK_DOC_CHECK([1.6], [--flavour no-tmpl])
YELP_HELP_INIT

# Optional Features

# --with-category-file=PATH
AC_MSG_CHECKING([for category file])
AC_ARG_WITH([category-file],
        [AC_HELP_STRING([--with-category-file=PATH],
        [path to category file (usually named 'catver.ini')])],
        with_category="$withval", with_category="no")
if test "$with_category" = "yes"; then
        AC_MSG_ERROR([--with-category-file missing PATH argument])
fi
if test "$with_category" != "no"; then
        AC_DEFINE_UNQUOTED(CATEGORY_FILE, "$with_category",
                [Location of the category file])
fi
AC_MSG_RESULT($with_category)

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

# --with-nplayers-file=PATH
AC_MSG_CHECKING([for number of players file])
AC_ARG_WITH([nplayers-file],
        [AC_HELP_STRING([--with-nplayers-file=PATH],
        [path to number of players file (usually named 'nplayers.ini')])],
        with_nplayers="$withval", with_nplayers="no")
if test "$with_nplayers" != "no"; then
        AC_DEFINE_UNQUOTED(NPLAYERS_FILE, "$with_nplayers",
                [Location of the number of players file])
fi
AC_MSG_RESULT($with_nplayers)

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

echo
echo "    Configuration Summary"
echo "    ---------------------"
echo
echo "    MAME Program    : $MAME_PROGRAM"
echo "    Category File   : $with_category"
echo "    History File    : $with_history"
echo "    NPlayers File   : $with_nplayers"
echo "    Use libwnck     : $with_wnck"
echo
