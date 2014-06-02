# Copyright © Télécom SudParis.
# See COPYING in top-level directory.

AM_CFLAGS = -W -Wall -Wextra -I$(top_srcdir)/src -L$(top_srcdir)/src

AM_LDFLAGS = $(top_builddir)/src/liblitl.la
AM_LDADD = $(top_builddir)/src/liblitl.la
#LITLDEPENDENCIES = $(AM_LDFLAGS)

CTESTS = test_litl_write \
         test_litl_read \
         test_litl_write_concurent \
         test_litl_write_multiple_threads \
         test_litl_write_multiple_applications \
         test_litl_pause \
         test_litl_write_flush \
         test_litl_read_flush \
         test_litl_write_concurent_flush \
         test_litl_write_multiple_threads_flush \
         test_litl_write_multiple_applications_flush \
         test_litl_pause_flush \
         test_litl_buffer \
         test_litl_trace_size \
         test_litl_mapping_to_fxt

check_PROGRAMS = $(CTESTS)
TESTS = $(CTESTS)

test_litl_write_SOURCES = test_litl_write.c

test_litl_write_concurent_SOURCES = test_litl_write_concurent.c
test_litl_write_concurent_LDFLAGS = -lpthread $(AM_LDFLAGS)

test_litl_write_multiple_threads_SOURCES = test_litl_write_multiple_threads.c
test_litl_write_multiple_threads_LDFLAGS = -lpthread $(AM_LDFLAGS)

test_litl_write_multiple_applications_SOURCES = test_litl_write_multiple_applications.c
test_litl_write_multiple_applications_LDFLAGS = -lpthread $(AM_LDFLAGS)

test_litl_read_SOURCES = test_litl_read.c

test_litl_pause_SOURCES = test_litl_pause.c

test_litl_buffer_SOURCES = test_litl_buffer_size.c

test_litl_trace_size_SOURCES = test_litl_trace_size.c

test_litl_mapping_to_fxt_SOURCES = test_litl_mapping_to_fxt.c
test_litl_mapping_to_fxt_LDFLAGS = -lpthread $(AM_LDFLAGS)


test_litl_write_flush_SOURCES = $(test_litl_write_SOURCES)
test_litl_read_flush_SOURCES = $(test_litl_read_SOURCES)
test_litl_write_concurent_flush_SOURCES = $(test_litl_write_concurent_SOURCES)
test_litl_write_multiple_threads_flush_SOURCES = $(test_litl_write_multiple_threads_SOURCES)
test_litl_write_multiple_applications_flush_SOURCES = $(test_litl_write_multiple_applications_SOURCES)
test_litl_pause_flush_SOURCES = $(test_litl_pause_SOURCES)

test_litl_write_flush_CFLAGS = -DLITL_TESTBUFFER_FLUSH=1 $(AM_CFLAGS)
test_litl_read_flush_CLFAGS = -DLITL_TESTBUFFER_FLUSH=1 $(AM_CFLAGS
test_litl_write_concurent_flush_CFLAGS = -DLITL_TESTBUFFER_FLUSH=1 $(AM_CFLAGS)
test_litl_write_multiple_threads_flush_CFLAGS = -DLITL_TESTBUFFER_FLUSH=1 $(AM_CFLAGS)
test_litl_write_multiple_applications_flush_CFLAGS = -DLITL_TESTBUFFER_FLUSH=1 $(AM_CFLAGS)
test_litl_pause_flush_CFLAGS = -DLITL_TESTBUFFER_FLUSH=1 $(AM_CFLAGS)

test_litl_write_multiple_threads_flush_LDFLAGS = $(test_litl_write_multiple_threads_LDFLAGS)
test_litl_write_multiple_applications_flush_LDFLAGS = $(test_litl_write_multiple_applications_LDFLAGS)
test_litl_write_concurent_flush_LDFLAGS = $(test_litl_write_concurent_LDFLAGS)

clean-local:
	rm -rf Makefile.in Makefile.bk
	rm -rf test_litl_write test_litl_write_concurent \
	       test_litl_write_multiple_threads test_litl_read test_litl_pause \
	       test_litl_write_multiple_applications test_litl_buffer \
	       test_litl_trace_size test_litl_mapping_to_fxt
