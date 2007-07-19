### Process this file with automake to produce Makefile.in

bin_PROGRAMS = gnome-video-arcade

AM_CFLAGS = \
	-Werror \
	@GLIB_CFLAGS@ @GTK_CFLAGS@ \
	@GCONF_CFLAGS@ @GLADE_CFLAGS@ \
	@SQLITE_CFLAGS@

AM_CPPFLAGS = \
	-DDATADIR=\""$(datadir)"\" \
	-DLOCALEDIR=\""$(datadir)/locale/"\" \
	-DG_LOG_DOMAIN=\"Gva\"

gnome_video_arcade_SOURCES = \
	gva-cell-renderer-pixbuf.c	\
	gva-cell-renderer-pixbuf.h	\
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
	@MAME_BACKEND@ @SQLITE_LIBS@

gnome_video_arcade_DEPENDENCIES = \
	@MAME_BACKEND@
