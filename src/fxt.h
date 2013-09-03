/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

/**
 *  \file fxt.h
 *  \brief fxt Provides a mapping of the LiTL APIs into the FxT APIs for easier
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

typedef litl_trace_write_t* fxt_t;

static fxt_t __trace;

/* BEGIN -- Recording functions */
#define fut_setup(buffer_size, keymask, threadid) do {          \
      __trace = litl_write_init_trace(buffer_size);             \
      litl_write_pause_recording(__trace);                      \
    }while(0)

// finalizing traces
#define fut_endup(filename) do {                                \
      litl_write_finalize_trace(__trace);                       \
    }while(0)
#define fut_done(void) do {                                     \
    } while(0)

#define fut_set_filename(filename) do {                         \
      litl_write_set_filename(__trace, filename);               \
      litl_write_resume_recording(__trace);                     \
    } while(0)

#define enable_fut_flush() do {                                 \
      litl_write_buffer_flush_on(__trace);                      \
    } while(0)

#define disable_fut_flush() do {                                \
      litl_write_buffer_flush_off(__trace);                     \
    } while(0)

#define fut_enable_tid_logging() do {                           \
      litl_write_tid_recording_on(__trace);                     \
    } while(0)

#define fut_disable_tid_logging() do {                          \
      litl_write_tid_recording_off(__trace);                    \
    } while(0)

/* END -- Recording functions */

/* BEGIN -- Events */
#define FUT_DO_PROBE0(code) litl_write_probe_pack_0(__trace, code)
#define FUT_DO_PROBE1(code, arg1) litl_write_probe_pack_1(__trace, code, arg1)
#define FUT_DO_PROBE2(code, arg1, arg2) litl_write_probe_pack_2(__trace, code, arg1, arg2)
#define FUT_DO_PROBE3(code, arg1, arg2, arg3) litl_write_probe_pack_3(__trace, code, arg1, arg2, arg3)
#define FUT_DO_PROBE4(code, arg1, arg2, arg3, arg4) litl_write_probe_pack_4(__trace, code, arg1, arg2, arg3, arg4)
#define FUT_DO_PROBE5(code, arg1, arg2, arg3, arg4, arg5) litl_write_probe_pack_5(__trace, code, arg1, arg2, arg3, arg4, arg5)
#define FUT_DO_PROBE6(code, arg1, arg2, arg3, arg4, arg5, arg6) litl_write_probe_pack_6(__trace, code, arg1, arg2, arg3, arg4, arg5, arg6)
#define FUT_DO_PROBE(code, ...) litl_write_probe_pack_0(__trace, code)
#define FUT_DO_PROBESTR(code, str) litl_write_probe_raw(__trace, code, strlen(str), str)

/* END -- Events */

#endif /* FXT_H_ */
