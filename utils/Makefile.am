# Copyright © Télécom SudParis.
# See COPYING in top-level directory.

AM_CFLAGS = -W -Wall -Wextra -I$(top_srcdir)/src
AM_LDFLAGS = -L$(top_builddir)/src -llitl

bin_PROGRAMS = litl_print 

# litl_merge litl_split

litl_print_SOURCES = litl_print.c

litl_merge_SOURCES = litl_merge.c

litl_split_SOURCES = litl_split.c

clean-local:
	rm -rf Makefile.bk
	rm -rf litl_print litl_merge litl_split
