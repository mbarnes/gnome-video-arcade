### Process this file with automake to produce Makefile.in

SUBDIRS = src docs

man_MANS = pcl.1

EXTRA_DIST = \
        HACKING         \
        $(man_MANS)     \
        pcl.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = pcl.pc
$(pkgconfig_DATA): config.status
