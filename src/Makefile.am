# Copyright © Télécom SudParis.
# See COPYING in top-level directory.

bin_PROGRAMS = litl_read litl_merge litl_split
litl_read_SOURCES = litl_types.h litl_macro.h litl_macro.c litl_read.h litl_read.c
litl_merge_SOURCES = litl_types.h litl_queue.h litl_queue.c litl_merge.h litl_merge.c
litl_split_SOURCES = litl_types.h litl_split.h litl_split.c

lib_LTLIBRARIES = liblitl.la

AM_CFLAGS = -W -Wall -Wextra -I$(top_srcdir)/src

liblitl_la_CPPFLAGS = $(AM_CPPFLAGS) $(AM_CFLAGS)
liblitl_la_LDFLAGS = $(AM_LDFLAGS) -lm
liblitl_la_SOURCES = litl_types.h litl_write.h litl_write.c litl_read.h litl_read.c litl_macro.h litl_macro.c \
                     litl_timer.h litl_timer.c

include_HEADERS = litl_types.h litl_write.h litl_read.h litl_merge.h litl_split.h litl_queue.h litl_macro.h \
                  litl_timer.h

clean-local:
	rm -rf Makefile.bk
