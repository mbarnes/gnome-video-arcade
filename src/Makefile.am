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
	gva-common.h			\
	gva-db.c			\
	gva-db.h			\
	gva-error.c			\
	gva-error.h			\
	gva-favorites.c			\
	gva-favorites.h			\
	gva-game-db.c			\
	gva-game-db.h			\
	gva-game-store.c		\
	gva-game-store.h		\
	gva-main.c			\
	gva-main.h			\
	gva-mame-process.c		\
	gva-mame-process.h		\
	gva-parser.c			\
	gva-parser.h			\
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
	gva-xmame.c			\
	gva-xmame.h			\
	main.c

gnome_video_arcade_LDADD = \
	@GLIB_LIBS@ @GTK_LIBS@ \
	@GCONF_LIBS@ @GLADE_LIBS@ \
	@SQLITE_LIBS@
