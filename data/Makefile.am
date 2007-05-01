### Process this file with automake to produce Makefile.in

desktopdir = $(datadir)/applications
desktop_in_files = gnome-video-arcade.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

schemadir = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = gnome-video-arcade.schemas

uidir = $(datadir)/gnome-video-arcade
ui_DATA = \
	gnome-video-arcade.glade	\
	gnome-video-arcade.ui

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	$(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)

EXTRA_DIST = \
	$(schema_DATA)			\
	$(ui_DATA)			\
	gnome-video-arcade.desktop.in

DISTCLEANFILES = \
	$(desktop_DATA)

MAINTAINERCLEANFILES = \
	gnome-video-arcade.glade.bak	\
	gnome-video-arcade.gladep.bak
