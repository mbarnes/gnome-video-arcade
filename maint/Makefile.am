### Process this file with automake to produce Makefile.in

gladegva_LTLIBRARIES = libgladegva.la
gladegvadir = `$(PKGCONFIG) --variable=moduledir gladeui-1.0`

libgladegva_la_CFLAGS = \
	@GLIB_CFLAGS@ @GTK_CFLAGS@ \
	@GCONF_CFLAGS@

libgladegva_la_SOURCES = \
	$(top_srcdir)/src/gva-column-manager.c

libgladegva_la_LDFLAGS = \
	-module -avoid-version

libgladegva_la_LIBADD = \
	@GLIB_LIBS@ @GTK_LIBS@ \
	@GCONF_LIBS@

gladecatalogdir = `$(PKGCONFIG) --variable=catalogdir gladeui-1.0`
gladecatalog_DATA = gva.xml

EXTRA_DIST = \
	$(gladecatalog_DATA)
