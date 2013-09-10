# Copyright © Télécom SudParis.
# See COPYING in top-level directory.

AM_CFLAGS = -W -Wall -Wextra -I$(top_srcdir)/src

lib_LTLIBRARIES = liblitl.la

liblitl_la_CPPFLAGS = $(AM_CPPFLAGS) $(AM_CFLAGS) -g
liblitl_la_LDFLAGS = $(AM_LDFLAGS) -lm -g
liblitl_la_SOURCES = litl_types.h litl_tools.h litl_tools.c \
					 litl_timer.h litl_timer.c \
                     litl_write.h litl_write.c
#                     litl_read.h litl_read.c \
#                     litl_merge.h litl_merge.c \
#                     litl_split.h litl_split.c

include_HEADERS = litl_types.h litl_tools.h litl_timer.h \
                  litl_write.h 
                  #litl_read.h \
                  #litl_merge.h litl_split.h \
                  #fxt.h fut.h

clean-local:
	rm -rf Makefile.bk
