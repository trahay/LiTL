
AM_CFLAGS = $(FXT_CPPFLAGS)
AM_CFLAGS += -I$(top_srcdir)/src

AM_LDFLAGS = $(FXT_LDFLAGS)
AM_LDFLAGS += $(top_srcdir)/src/libevnt.la

EVNTDEPENDENCIES = $(AM_LDFLAGS)

bin_PROGRAMS = test_evnt_read test_evnt_write test_evnt_buffer

test_evnt_read_LDADD = -levnt
test_evnt_read_SOURCES = test_evnt_read.c

test_evnt_write_SOURCES = test_evnt_write.c
test_evnt_write_LDADD = -levnt

test_evnt_buffer_SOURCES = test_evnt_buffer_size.c timer.c timer.h
test_evnt_buffer_LDADD = -levnt

noinst_HEADERS = timer.h
