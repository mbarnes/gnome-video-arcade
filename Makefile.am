### Process this file with automake to produce Makefile.in

SUBDIRS = data po src docs help

if GLADE_CATALOG
SUBDIRS += maint
endif

EXTRA_DIST = \
	config.rpath			\
	gnome-doc-utils.make		\
	intltool-extract.in		\
	intltool-merge.in		\
	intltool-update.in		\
	mkinstalldirs

DISTCLEANFILES = \
	gnome-doc-utils.make		\
	intltool-extract		\
	intltool-merge			\
	intltool-update

DISTCHECK_CONFIGURE_FLAGS =		\
	--enable-gtk-doc		\
	--disable-scrollkeeper
