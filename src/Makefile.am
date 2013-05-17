# Copyright © Télécom SudParis.
# See COPYING in top-level directory.

#AM_CFLAGS = $(FXT_CPPFLAGS)
#AM_LDFLAGS = $(FXT_LDFLAGS)

bin_PROGRAMS = evnt_read litl_merge
evnt_read_SOURCES = evnt_types.h evnt_read.h evnt_read.c evnt_macro.h evnt_macro.c
litl_merge_SOURCES = evnt_types.h litl_merge.h litl_merge.c

lib_LTLIBRARIES = libevnt.la

AM_CFLAGS = -W -Wall -Wextra -I$(top_srcdir)/src

libevnt_la_CPPFLAGS = $(AM_CPPFLAGS) $(AM_CFLAGS)
libevnt_la_LDFLAGS = $(AM_LDFLAGS) -lm
libevnt_la_SOURCES = evnt_types.h evnt_write.h evnt_write.c evnt_read.h evnt_read.c evnt_macro.h evnt_macro.c timer.h timer.c

include_HEADERS = evnt_types.h evnt_write.h evnt_read.h litl_merge.h evnt_macro.h timer.h

clean-local:
	rm -rf Makefile.bk
