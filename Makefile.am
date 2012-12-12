
FXTCFLAGS=$(FXT_CPPFLAGS)
FXTLDFLAGS=$(FXT_LDFLAGS)

SUBDIRS = src \
	  	  tests

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = evnt.pc

EXTRA_DIST = ChangeLog

clean-local:
	rm -rf Makefile.in aclocal.m4 autom4te.cache/ config.log evnt.pc libtool
	