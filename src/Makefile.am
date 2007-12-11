### Process this file with automake to produce Makefile.in

bin_PROGRAMS = gnome-video-arcade

AM_CFLAGS = \
	-Wall \
	@GLIB_CFLAGS@ @GTK_CFLAGS@ \
	@GCONF_CFLAGS@ @GLADE_CFLAGS@ \
	@GNOME_CFLAGS@ @SQLITE_CFLAGS@

# The DATADIR, LIBDIR, PREFIX, and SYSCONFDIR definitions
# are only needed for libgnome.  GLib handles it better.
AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Gva\" \
	-DDATADIR=\""$(datadir)"\" \
	-DLIBDIR=\""$(libdir)"\" \
	-DPREFIX=\""$(prefix)"\" \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DLOCALEDIR=\""$(datadir)/locale/"\"

AM_LDFLAGS = \
	-export-dynamic

gnome_video_arcade_SOURCES = \
	gconf-bridge.c			\
	gconf-bridge.h			\
	gva-audit.c			\
	gva-audit.h			\
	gva-categories.h		\
	gva-categories.c		\
	gva-cell-renderer-pixbuf.c	\
	gva-cell-renderer-pixbuf.h	\
	gva-column-manager.c		\
	gva-column-manager.h		\
	gva-columns.c			\
	gva-columns.h			\
	gva-common.h			\
	gva-db.c			\
	gva-db.h			\
	gva-error.c			\
	gva-error.h			\
	gva-favorites.c			\
	gva-favorites.h			\
	gva-game-store.c		\
	gva-game-store.h		\
	gva-history.c			\
	gva-history.h			\
	gva-main.c			\
	gva-main.h			\
	gva-mame-common.c		\
	gva-mame-common.h		\
	gva-mame-process.c		\
	gva-mame-process.h		\
	gva-mame.h			\
	gva-play-back.c			\
	gva-play-back.h			\
	gva-preferences.c		\
	gva-preferences.h		\
	gva-process.c			\
	gva-process.h			\
	gva-properties.h		\
	gva-properties.c		\
	gva-search.c			\
	gva-search.h			\
	gva-time.c			\
	gva-time.h			\
	gva-tree-view.c			\
	gva-tree-view.h			\
	gva-ui.c			\
	gva-ui.h			\
	gva-util.c			\
	gva-util.h			\
	main.c

EXTRA_gnome_video_arcade_SOURCES = \
	gva-mame-sdlmame.c		\
	gva-mame-xmame.c

gnome_video_arcade_LDADD = \
	@GLIB_LIBS@ @GTK_LIBS@ \
	@GCONF_LIBS@ @GLADE_LIBS@ \
	@GNOME_LIBS@ @MAME_BACKEND@ \
	@SQLITE_LIBS@

gnome_video_arcade_DEPENDENCIES = \
	@MAME_BACKEND@
