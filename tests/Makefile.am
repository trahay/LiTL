# Copyright © Télécom SudParis.
# See COPYING in top-level directory.

AM_CFLAGS = -W -Wall -Wextra -I$(top_srcdir)/src -L$(top_srcdir)/src

AM_LDFLAGS = $(top_builddir)/src/liblitl.la
#LITLDEPENDENCIES = $(AM_LDFLAGS)

CTESTS = test_litl_write \
         test_litl_write_multiple \
         test_litl_write_concurent \
         test_litl_write_multiple_threads \
         test_litl_read \
         test_litl_pause \
         test_litl_buffer \
         test_litl_trace_size \
         test_litl_mapping_to_fxt

check_PROGRAMS = $(CTESTS)
TESTS = $(CTESTS)

test_litl_write_SOURCES = test_litl_write.c

test_litl_write_multiple_SOURCES = test_litl_write_multiple.c
test_litl_write_multiple_LDFLAGS = -lpthread -llitl

test_litl_write_multiple_threads_SOURCES = test_litl_write_multiple_threads.c
test_litl_write_multiple_threads_LDFLAGS = -lpthread -llitl

test_litl_write_concurent_SOURCES = test_litl_write_concurent.c
test_litl_write_concurent_LDFLAGS = -lpthread -llitl

test_litl_read_SOURCES = test_litl_read.c

test_litl_buffer_SOURCES = test_litl_buffer_size.c

test_litl_pause_SOURCES = test_litl_pause.c

test_litl_trace_size_SOURCES = test_litl_trace_size.c

test_litl_mapping_to_fxt_SOURCES = test_litl_mapping_to_fxt.c
test_litl_mapping_to_fxt_LDFLAGS = -lpthread -llitl 

clean-local:
	rm -rf Makefile.in Makefile.bk
	rm -rf test_litl_write test_litl_write_multiple test_litl_write_concurent \
	       test_litl_read test_litl_pause test_litl_buffer \
	       test_litl_trace_size test_litl_mapping_to_fxt
