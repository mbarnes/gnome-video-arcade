### Process this file with automake to produce Makefile.in

desktopdir = $(datadir)/applications
desktop_in_files = gnome-video-arcade.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

schemadir = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = gnome-video-arcade.schemas

svgicondir = $(datadir)/icons/hicolor/scalable/apps
svgicon_DATA = gnome-video-arcade.svg

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

uidir = $(datadir)/gnome-video-arcade
ui_DATA = \
	gnome-video-arcade.builder	\
	gnome-video-arcade.ui

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	$(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)
else
install-data-local:
endif

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

EXTRA_DIST = \
	$(schema_DATA)			\
	$(svgicon_DATA)			\
	$(ui_DATA)			\
	gnome-video-arcade.desktop.in

DISTCLEANFILES = \
	$(desktop_DATA)

MAINTAINERCLEANFILES = \
	gnome-video-arcade.builder.bak

-include $(top_srcdir)/git.mk
