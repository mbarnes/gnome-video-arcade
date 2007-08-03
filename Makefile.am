### Process this file with automake to produce Makefile.in

SUBDIRS = data po src docs

if MAINTAINER_MODE
SUBDIRS += maint
endif

EXTRA_DIST = \
	config.rpath			\
	intltool-extract.in		\
	intltool-merge.in		\
	intltool-update.in		\
	mkinstalldirs

DISTCLEANFILES = \
	intltool-extract		\
	intltool-merge			\
	intltool-update

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc
