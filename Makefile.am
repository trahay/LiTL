
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

clean-local:
	rm -rf INSTALL Makefile Makefile.in aclocal.m4 autom4te.cache/ compile config.guess config.log \
		   config.status config.sub configure depcomp evnt.pc install-sh libtool ltmain.sh missing
	