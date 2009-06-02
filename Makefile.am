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

MAINTAINERCLEANFILES = \
	$(srcdir)/INSTALL		\
	$(srcdir)/aclocal.m4		\
	$(srcdir)/autoscan.log		\
	$(srcdir)/compile		\
	$(srcdir)/config.guess		\
	$(srcdir)/config.h.in		\
	$(srcdir)/config.sub		\
	$(srcdir)/configure.scan	\
	$(srcdir)/depcomp		\
	$(srcdir)/install-sh		\
	$(srcdir)/ltmain.sh		\
	$(srcdir)/missing		\
	$(srcdir)/mkinstalldirs		\
	$(srcdir)/omf.make		\
	$(srcdir)/xmldocs.make		\
	$(srcdir)/gtk-doc.make		\
	$(srcdir)/gnome-doc-utils.make	\
	`find "$(srcdir)" -type -f -name Makefile.in -print`

DISTCHECK_CONFIGURE_FLAGS =		\
	--enable-gtk-doc		\
	--disable-scrollkeeper

-include $(top_srcdir)/git.mk
