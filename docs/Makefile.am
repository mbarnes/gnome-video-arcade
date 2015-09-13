### Process this file with automake to produce Makefile.in

SUBDIRS = reference

man_MANS = gnome-video-arcade.1

gnome-video-arcade.1: gnome-video-arcade.1.in
	$(AM_V_GEN) sed -e "s|[@]TIMESTAMP[@]|$(shell date '+%B %Y')|" \
	< $< > $@.tmp && mv $@.tmp $@

EXTRA_DIST = gnome-video-arcade.1.in

CLEANFILES = $(man_MANS)

-include $(top_srcdir)/git.mk
