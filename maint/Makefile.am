### Process this file with automake to produce Makefile.in

gladecatalog_DATA = gva.xml
gladecatalogdir = `$(PKG_CONFIG) --variable=catalogdir gladeui-2.0`

gladepixmap_DATA = \
	widget-gva-column-manager.png \
	widget-gva-mute-button.png
gladepixmapdir = `$(PKG_CONFIG) --variable=pixmapdir gladeui-2.0`/hicolor/22x22/actions

gladegva_LTLIBRARIES = libgladegva.la
gladegvadir = `$(PKG_CONFIG) --variable=moduledir gladeui-2.0`

libgladegva_la_CPPFLAGS = $(DEP_CPPFLAGS)

libgladegva_la_CFLAGS = \
	@GLIB_CFLAGS@ @GTK_CFLAGS@ @SOUP_CFLAGS@

libgladegva_la_SOURCES = \
	$(top_srcdir)/src/gva-column-manager.c \
	$(top_srcdir)/src/gva-mute-button.c

libgladegva_la_LDFLAGS = \
	-module -avoid-version

libgladegva_la_LIBADD = \
	@GLIB_LIBS@ @GTK_LIBS@ @SOUP_LIBS@

EXTRA_DIST = \
	$(gladecatalog_DATA) \
	$(gladepixmap_DATA)

-include $(top_srcdir)/git.mk
