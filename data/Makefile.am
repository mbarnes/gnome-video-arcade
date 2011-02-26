### Process this file with automake to produce Makefile.in

desktopdir = $(datadir)/applications
desktop_in_files = gnome-video-arcade.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

gsettingsschema_in_files = org.gnome.VideoArcade.gschema.xml.in
gsettings_SCHEMAS = $(gsettingsschema_in_files:.xml.in=.xml)
@INTLTOOL_XML_NOMERGE_RULE@
@GSETTINGS_RULES@

gsettingsconvertdir = $(datadir)/GConf/gsettings
gsettingsconvert_DATA = gnome-video-arcade.convert

svgicondir = $(datadir)/icons/hicolor/scalable/apps
svgicon_DATA = gnome-video-arcade.svg

uidir = $(datadir)/gnome-video-arcade
ui_DATA = \
	gnome-video-arcade.builder	\
	gnome-video-arcade.ui

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

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
	$(desktop_in_files)		\
	$(gsettingsschema_in_files)	\
	$(svgicon_DATA)			\
	$(ui_DATA)			\
	gnome-video-arcade.convert

DISTCLEANFILES = \
	$(desktop_DATA)			\
	$(gsettings_SCHEMAS)

MAINTAINERCLEANFILES = \
	gnome-video-arcade.builder.bak

-include $(top_srcdir)/git.mk
