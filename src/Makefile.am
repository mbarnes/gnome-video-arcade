### Process this file with automake to produce Makefile.in

bin_PROGRAMS = gnome-video-arcade

AM_CFLAGS = \
	-Wall \
	@GLIB_CFLAGS@ @GTK_CFLAGS@ @GIT_CFLAGS@ @GSTREAMER_CFLAGS@ \
	@SOUP_CFLAGS@ @SQLITE_CFLAGS@ @WNCK_CFLAGS@

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Gva\" \
	-DTOPSRCDIR=\""$(abs_top_builddir)"\" \
	-DLOCALEDIR=\""$(datadir)/locale/"\"

AM_LDFLAGS = \
	-export-dynamic

gnome_video_arcade_SOURCES = \
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
	gva-input-file.c		\
	gva-input-file.h		\
	gva-main.c			\
	gva-main.h			\
	gva-mame-common.c		\
	gva-mame-common.h		\
	gva-mame-process.c		\
	gva-mame-process.h		\
	gva-mame.h			\
	gva-music-button.c		\
	gva-music-button.h		\
	gva-mute-button.c		\
	gva-mute-button.h		\
	gva-nplayers.c			\
	gva-nplayers.h			\
	gva-play-back.c			\
	gva-play-back.h			\
	gva-preferences.c		\
	gva-preferences.h		\
	gva-process.c			\
	gva-process.h			\
	gva-properties.c		\
	gva-properties.h		\
	gva-screen-saver.c		\
	gva-screen-saver.h		\
	gva-time.c			\
	gva-time.h			\
	gva-tree-view.c			\
	gva-tree-view.h			\
	gva-ui.c			\
	gva-ui.h			\
	gva-util.c			\
	gva-util.h			\
	gva-wnck.c			\
	gva-wnck.h			\
	main.c

EXTRA_gnome_video_arcade_SOURCES = \
	gva-mame-sdlmame.c		\
	gva-mame-xmame.c

gnome_video_arcade_LDADD = \
	@GLIB_LIBS@ @GTK_LIBS@ @GIT_LIBS@ \
	@GSTREAMER_LIBS@ @MAME_BACKEND@ \
	@SOUP_LIBS@ @SQLITE_LIBS@ @WNCK_LIBS@

gnome_video_arcade_DEPENDENCIES = \
	@MAME_BACKEND@

-include $(top_srcdir)/git.mk
