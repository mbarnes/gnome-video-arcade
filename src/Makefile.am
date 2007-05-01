### Process this file with automake to produce Makefile.in

bin_PROGRAMS = gnome-video-arcade

AM_CFLAGS = \
	-Werror -pedantic-errors \
	@GCONF_CFLAGS@ @GLADE_CFLAGS@

AM_CPPFLAGS = \
	-DDATADIR=\""$(datadir)"\" \
	-DLOCALEDIR=\""$(datadir)/locale/"\" \
	-DG_LOG_DOMAIN=\"Gva\"

gnome_video_arcade_SOURCES = \
	gva-actions.h			\
	gva-actions.c			\
	gva-common.h			\
	gva-game-store.c		\
	gva-game-store.h		\
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
	@GCONF_LIBS@ @GLADE_LIBS@
