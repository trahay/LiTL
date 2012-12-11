
FXTCFLAGS=$(FXT_CPPFLAGS)
FXTLDFLAGS=$(FXT_LDFLAGS)

SUBDIRS = src \
	  	  tests

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = evnt.pc

EXTRA_DIST = \
ChangeLog \
test/Makefile.am \
src/Makefile.am

include_HEADERS = $(top_srcdir)/src/evnt_types.h
