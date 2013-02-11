# Copyright © Télécom SudParis.
# See COPYING in top-level directory.

AM_CFLAGS = $(FXT_CPPFLAGS)
AM_CFLAGS += -I$(top_srcdir)/src

AM_LDFLAGS = $(FXT_LDFLAGS)
AM_LDFLAGS += $(top_srcdir)/src/libevnt.la

EVNTDEPENDENCIES = $(AM_LDFLAGS)

CTESTS = test_evnt_write \
         test_evnt_read \
         test_evnt_buffer
	 
check_PROGRAMS = $(CTESTS)
TESTS = $(CTESTS)

test_evnt_write_SOURCES = test_evnt_write.c
test_evnt_read_SOURCES = test_evnt_read.c
test_evnt_buffer_SOURCES = test_evnt_buffer_size.c

clean-local:
	rm -rf Makefile.in Makefile.bk
	