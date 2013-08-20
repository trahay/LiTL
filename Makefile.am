# Copyright © Télécom SudParis.
# See COPYING in top-level directory.

SUBDIRS = src \
          utils \
	  	  tests

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = litl.pc

EXTRA_DIST = ChangeLog

clean-local:
	rm -rf aclocal.m4 autom4te.cache/ config.log litl.pc libtool
	