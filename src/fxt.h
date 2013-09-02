/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

/**
 *  \file fxt.h
 *  \brief fxt Provides mapping of the LiTL APIs into the FxT APIs for easier
 *  usage of LiTL in FxT-related applications
 *
 *  \authors
 *    Developers are: \n
 *        Roman Iakymchuk   -- roman.iakymchuk@telecom-sudparis.eu \n
 *        Francois Trahay   -- francois.trahay@telecom-sudparis.eu \n
 */

#ifndef FXT_H_
#define FXT_H_

#include <string.h>

#include "litl_types.h"
#include "litl_write.h"
#include "litl_read.h"

typedef struct litl_trace_write_t *fxt_t;

fxt_t litl_trace;

/* BEGIN -- Recording events*/
// here is the main difference, see eztrace_core.c line:255
int fut_setup(unsigned int nints, unsigned int keymask, unsigned int threadid);

// finalizing traces
#define fut_endup(filename) litl_fin_trace(litl_trace);
#define fut_done(void) do {                            \
    } while(0)

#define fut_set_filename(filename) do {                \
      litl_write_set_filename(trace, filename);        \
    } while(0)

#define enable_fut_flush() do {                        \
      litl_write_buffer_flush_on(trace);               \
    } while(0)

#define disable_fut_flush() do {                       \
      litl_write_buffer_flush_off(trace);              \
    } while(0)

#define fut_enable_tid_logging() do {                  \
      litl_write_tid_recording_on(trace);              \
    } while(0)

#define fut_disable_tid_logging() do {                 \
      litl_write_tid_recording_off(trace);             \
    } while(0)

/* BEGIN -- Events */

#define FUT_DO_PROBE0(code) litl_probe_pack_0(litl_trace, code)
#define FUT_DO_PROBE1(code, arg1) litl_probe_pack_1(litl_trace, code, arg1)
#define FUT_DO_PROBE2(code, arg1, arg2) litl_probe_pack_2(litl_trace, code, arg1, arg2)
#define FUT_DO_PROBE3(code, arg1, arg2, arg3) litl_probe_pack_3(litl_trace, code, arg1, arg2, arg3)
#define FUT_DO_PROBE4(code, arg1, arg2, arg3, arg4) litl_probe_pack_4(litl_trace, code, arg1, arg2, arg3, arg4)
#define FUT_DO_PROBE5(code, arg1, arg2, arg3, arg4, arg5) litl_probe_pack_5(litl_trace, code, arg1, arg2, arg3, arg4, arg5)
#define FUT_DO_PROBE6(code, arg1, arg2, arg3, arg4, arg5, arg6) litl_probe_pack_6(litl_trace, code, arg1, arg2, arg3, arg4, arg5, arg6)
#define FUT_DO_PROBE(code, ...) litl_probe_pack_0(litl_trace, code)
#define FUT_DO_PROBESTR(code, str) litl_probe_raw(litl_trace, code, strlen(str), str)

/* END -- Events */

/* END -- Recording events */

#endif /* FXT_H_ */
