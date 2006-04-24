### Process this file with automake to produce Makefile.in

bin_PROGRAMS = pcl
lib_LTLIBRARIES = libpcl.la

AM_CFLAGS = -pedantic
AM_YFLAGS = -d

AM_CPPFLAGS = \
        -DG_LOG_DOMAIN=\"Pcl\"                  \
        -DPCL_EXEC_PREFIX=\"$(exec_prefix)\"    \
        -DPCL_PREFIX=\"$(prefix)\"              \
        -I$(top_srcdir)                         \
        -I$(top_srcdir)/src                     \
        -I$(top_builddir)                       \
        $(PCL_DEBUG_FLAGS)                      \
        $(DEPENDENCIES_CFLAGS)

BUILT_SOURCES = pcl-parser.h

libpcl_la_SOURCES = \
        pcl.c                   	\
        pcl-arg.c               	\
        pcl-attr-getter.c       	\
        pcl-bool.c              	\
	pcl-callable-iterator.c		\
        pcl-cell.c              	\
	pcl-class-method.c		\
        pcl-code.c              	\
        pcl-compiler.c          	\
        pcl-complex.c           	\
        pcl-container.c         	\
        pcl-descriptor.c        	\
        pcl-dict.c              	\
        pcl-dict-proxy.c        	\
        pcl-ellipsis.c          	\
        pcl-enumerate.c         	\
        pcl-error.c             	\
        pcl-eval.c              	\
        pcl-exception.c         	\
        pcl-factory.c           	\
        pcl-file.c              	\
        pcl-float.c             	\
        pcl-frame.c             	\
        pcl-free.c              	\
        pcl-function.c          	\
        pcl-generator.c         	\
        pcl-getset-descriptor.c 	\
        pcl-import.c            	\
	pcl-instance-method.c		\
        pcl-int.c               	\
        pcl-item-getter.c       	\
        pcl-iterator.c          	\
        pcl-lexer.l             	\
        pcl-list.c              	\
	pcl-long.c			\
        pcl-main.c              	\
        pcl-mapping.c           	\
        pcl-member-descriptor.c 	\
        pcl-member.c            	\
        pcl-method-descriptor.c 	\
        pcl-method.c            	\
        pcl-mod-builtin.c       	\
        pcl-mod-exceptions.c    	\
        pcl-mod-gc.c            	\
        pcl-mod-math.c          	\
        pcl-mod-operator.c      	\
        pcl-mod-signal.c                \
        pcl-mod-sys.c           	\
        pcl-module.c            	\
        pcl-mro.c               	\
        pcl-node.c              	\
        pcl-node.h              	\
        pcl-none.c              	\
        pcl-not-implemented.c   	\
        pcl-number.c            	\
        pcl-opcode.h            	\
        pcl-object.c            	\
	pcl-optimizer.c			\
        pcl-parse-info.c        	\
        pcl-parser.y            	\
        pcl-property.c          	\
        pcl-range.c             	\
        pcl-reversed.c          	\
        pcl-run.c               	\
        pcl-scope.c             	\
        pcl-scope.h             	\
	pcl-sequence-iterator.c		\
        pcl-sequence.c          	\
        pcl-set.c               	\
        pcl-slice.c             	\
        pcl-state.c             	\
	pcl-static-method.c		\
        pcl-string.c            	\
        pcl-super.c             	\
        pcl-symbol-table.c      	\
        pcl-symbol-table.h      	\
        pcl-table.c             	\
        pcl-tablerecord.c       	\
        pcl-traceback.c         	\
        pcl-tuple.c             	\
        pcl-type.c              	\
        pcl-util.c              	\
        pcl-weak-ref.c          	\
        pcl-zip.c               	\
        glib-ext.c              	\
        glib-ext.h

pclincludedir=$(includedir)/pcl
pclinclude_HEADERS = \
        pcl.h                   	\
        pcl-arg.h               	\
        pcl-attr-getter.h       	\
        pcl-bool.h              	\
	pcl-callable-iterator.h		\
        pcl-cell.h              	\
	pcl-class-method.h		\
        pcl-code.h              	\
        pcl-compiler.h          	\
        pcl-complex.h           	\
        pcl-config.h            	\
        pcl-container.h         	\
        pcl-descriptor.h        	\
        pcl-dict.h              	\
        pcl-dict-proxy.h        	\
        pcl-ellipsis.h          	\
        pcl-enumerate.h         	\
        pcl-error.h             	\
        pcl-eval.h              	\
        pcl-exception.h         	\
        pcl-factory.h           	\
        pcl-file.h              	\
        pcl-float.h             	\
        pcl-frame.h             	\
        pcl-free.h              	\
        pcl-function.h          	\
        pcl-generator.h         	\
        pcl-getset-descriptor.h 	\
        pcl-import.h            	\
	pcl-instance-method.h		\
        pcl-int.h               	\
        pcl-item-getter.h       	\
        pcl-iterator.h          	\
        pcl-list.h              	\
	pcl-long.h			\
        pcl-main.h              	\
        pcl-mapping.h           	\
        pcl-member-descriptor.h 	\
        pcl-member.h            	\
        pcl-method-descriptor.h 	\
        pcl-method.h            	\
        pcl-mod-gc.h            	\
        pcl-mod-sys.h           	\
        pcl-module.h            	\
        pcl-none.h              	\
        pcl-not-implemented.h   	\
        pcl-number.h            	\
        pcl-object.h            	\
        pcl-parse-info.h        	\
        pcl-property.h          	\
        pcl-range.h             	\
        pcl-reversed.h          	\
        pcl-run.h               	\
	pcl-sequence-iterator.h		\
        pcl-sequence.h          	\
        pcl-set.h               	\
        pcl-slice.h             	\
        pcl-state.h             	\
	pcl-static-method.h		\
        pcl-string.h            	\
        pcl-super.h             	\
        pcl-table.h             	\
        pcl-tablerecord.h       	\
        pcl-traceback.h         	\
        pcl-tuple.h             	\
        pcl-type.h              	\
        pcl-util.h              	\
        pcl-weak-ref.h          	\
        pcl-zip.h

libpcl_la_LIBADD = @DEPENDENCIES_LIBS@ @LEXLIB@
libpcl_la_LDFLAGS = \
        -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
        -export-dynamic $(LIBTOOL_EXPORT_OPTIONS)

pcl_SOURCES = main.c
pcl_LDADD = libpcl.la

EXTRA_DIST = \
        .gdbinit graph.pcl

CLEANFILES = \
        pcl-parser.output

# These require special tools to rebuild.
MAINTAINERCLEANFILES = \
        pcl-parser.c            \
        pcl-parser.h            \
        pcl-lexer.c
