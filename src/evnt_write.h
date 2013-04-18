/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef EVNT_WRITE_H_
#define EVNT_WRITE_H_

#include "evnt_types.h"

/*
 * This function initializes the trace
 */
evnt_trace_write_t evnt_init_trace(const uint32_t);

void evnt_buffer_flush_on(evnt_trace_write_t*);
void evnt_buffer_flush_off(evnt_trace_write_t*);

void evnt_thread_safety_on(evnt_trace_write_t*);
void evnt_thread_safety_off(evnt_trace_write_t*);

void evnt_tid_recording_on(evnt_trace_write_t*);
void evnt_tid_recording_off(evnt_trace_write_t*);

void evnt_pause_recording(evnt_trace_write_t*);
void evnt_resume_recording(evnt_trace_write_t*);

void evnt_set_filename(evnt_trace_write_t*, char*);

/*
 * This function writes the recorded events from the buffer to the trace file
 */
void evnt_flush_buffer(evnt_trace_write_t*, evnt_size_t);

/*
 * This function records an event with offset only
 */
void evnt_probe_offset(evnt_trace_write_t*, int16_t);

/*
 * This function records an event without any arguments
 */
void evnt_probe0(evnt_trace_write_t*, evnt_code_t);

/*
 * This function records an event with one argument
 */
void evnt_probe1(evnt_trace_write_t*, evnt_code_t, evnt_param_t);

/*
 * This function records an event with two arguments
 */
void evnt_probe2(evnt_trace_write_t*, evnt_code_t, evnt_param_t, evnt_param_t);

/*
 * This function records an event with three arguments
 */
void evnt_probe3(evnt_trace_write_t*, evnt_code_t, evnt_param_t, evnt_param_t, evnt_param_t);

/*
 * This function records an event with four arguments
 */
void evnt_probe4(evnt_trace_write_t*, evnt_code_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t);

/*
 * This function records an event with five arguments
 */
void evnt_probe5(evnt_trace_write_t*, evnt_code_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t);

/*
 * This function records an event with six arguments
 */
void evnt_probe6(evnt_trace_write_t*, evnt_code_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t,
        evnt_param_t);

/*
 * This function records an event with seven arguments
 */
void evnt_probe7(evnt_trace_write_t*, evnt_code_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t,
        evnt_param_t, evnt_param_t);

/*
 * This function records an event with eight arguments
 */
void evnt_probe8(evnt_trace_write_t*, evnt_code_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t,
        evnt_param_t, evnt_param_t, evnt_param_t);

/*
 * This function records an event with nine arguments
 */
void evnt_probe9(evnt_trace_write_t*, evnt_code_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t,
        evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t);

/*
 * This function records an event with ten arguments
 */
void evnt_probe10(evnt_trace_write_t*, evnt_code_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t,
        evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t);

/*
 * This function records an event in a raw state, where the size is #args in the void* array
 * That helps to discover places where the application has crashed while using EZTrace
 */
void evnt_raw_probe(evnt_trace_write_t*, evnt_code_t, evnt_size_t, evnt_data_t[]);

evnt_t* get_event(evnt_trace_write_t* trace, evnt_type_t type, evnt_code_t code, int size);

#define offset_of(TYPE, MEMBER) ((size_t) &((TYPE*)0)->MEMBER)

#define EVNT_BASE_SIZE offset_of(evnt_t, parameters.regular.param)

#define ADD_ARG(_ptr_, arg) do {			\
    typeof(arg) _arg = arg;			\
    memcpy(_ptr_, &_arg, sizeof(_arg));		\
    _ptr_ = ((char*)_ptr_)+sizeof(_arg);		\
  } while(0)

#define evnt_probe_pack_0(trace, code) do {		\
    int total_size = EVNT_BASE_SIZE;			\
    evnt_t* p_evt = get_event(trace, EVNT_TYPE_PACKED, code, total_size); \
    if(p_evt){						\
      p_evt->parameters.packed.size = total_size - EVNT_BASE_SIZE;	\
    }							\
  } while(0)

#define evnt_probe_pack_1(trace, code, arg1) do {	\
    int total_size = EVNT_BASE_SIZE + sizeof(arg1);	\
    evnt_t* p_evt = get_event(trace, EVNT_TYPE_PACKED, code, total_size); \
    if(p_evt){						\
      p_evt->parameters.packed.size = total_size - EVNT_BASE_SIZE;	\
      void*_ptr_ = &p_evt->parameters.packed.param[0];			\
      ADD_ARG(_ptr_, arg1);				\
    }							\
  } while(0)

#define evnt_probe_pack_2(trace, code, arg1, arg2) do {			\
    int total_size = EVNT_BASE_SIZE + sizeof(arg1) + sizeof(arg2);	\
    evnt_t* p_evt = get_event(trace, EVNT_TYPE_PACKED, code, total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - EVNT_BASE_SIZE;			\
      void*_ptr_ = &p_evt->parameters.packed.param[0];					\
      ADD_ARG(_ptr_, arg1);						\
      ADD_ARG(_ptr_, arg2);						\
    }									\
  } while(0)

#define evnt_probe_pack_3(trace, code, arg1, arg2, arg3) do {		\
    int total_size = EVNT_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3); \
    evnt_t* p_evt = get_event(trace, EVNT_TYPE_PACKED, code, total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - EVNT_BASE_SIZE;			\
      void*_ptr_ = &p_evt->parameters.packed.param[0];					\
      ADD_ARG(_ptr_, arg1);						\
      ADD_ARG(_ptr_, arg2);						\
      ADD_ARG(_ptr_, arg3);						\
    }									\
  } while(0)

#define evnt_probe_pack_4(trace, code, arg1, arg2, arg3, arg4) do {	\
    int total_size = EVNT_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    evnt_t* p_evt = get_event(trace, EVNT_TYPE_PACKED, code, total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - EVNT_BASE_SIZE;			\
      void*_ptr_ = &p_evt->parameters.packed.param[0];					\
      ADD_ARG(_ptr_, arg1);						\
      ADD_ARG(_ptr_, arg2);						\
      ADD_ARG(_ptr_, arg3);						\
      ADD_ARG(_ptr_, arg4);						\
    }									\
  } while(0)

#define evnt_probe_pack_5(trace, code, arg1, arg2, arg3, arg4, arg5) do { \
    int total_size = EVNT_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5);						\
    evnt_t* p_evt = get_event(trace, EVNT_TYPE_PACKED, code, total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - EVNT_BASE_SIZE;			\
      void*_ptr_ = &p_evt->parameters.packed.param[0];					\
      ADD_ARG(_ptr_, arg1);						\
      ADD_ARG(_ptr_, arg2);						\
      ADD_ARG(_ptr_, arg3);						\
      ADD_ARG(_ptr_, arg4);						\
      ADD_ARG(_ptr_, arg5);						\
    }									\
  } while(0)

#define evnt_probe_pack_6(trace, code, arg1, arg2, arg3, arg4, arg5, arg6) do { \
    int total_size = EVNT_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6);				\
    evnt_t* p_evt = get_event(trace, EVNT_TYPE_PACKED, code, total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - EVNT_BASE_SIZE;			\
      void*_ptr_ = &p_evt->parameters.packed.param[0];					\
      ADD_ARG(_ptr_, arg1);						\
      ADD_ARG(_ptr_, arg2);						\
      ADD_ARG(_ptr_, arg3);						\
      ADD_ARG(_ptr_, arg4);						\
      ADD_ARG(_ptr_, arg5);						\
      ADD_ARG(_ptr_, arg6);						\
    }									\
  } while(0)

#define evnt_probe_pack_7(trace, code, arg1, arg2, arg3, arg4, arg5, arg6, arg7) do { \
    int total_size = EVNT_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7);		\
    evnt_t* p_evt = get_event(trace, EVNT_TYPE_PACKED, code, total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - EVNT_BASE_SIZE;			\
      void*_ptr_ = &p_evt->parameters.packed.param[0];					\
      ADD_ARG(_ptr_, arg1);						\
      ADD_ARG(_ptr_, arg2);						\
      ADD_ARG(_ptr_, arg3);						\
      ADD_ARG(_ptr_, arg4);						\
      ADD_ARG(_ptr_, arg5);						\
      ADD_ARG(_ptr_, arg6);						\
      ADD_ARG(_ptr_, arg7);						\
    }									\
  } while(0)

#define evnt_probe_pack_8(trace, code, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) do { \
    int total_size = EVNT_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7) + sizeof(arg8); \
    evnt_t* p_evt = get_event(trace, EVNT_TYPE_PACKED, code, total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - EVNT_BASE_SIZE;			\
      void*_ptr_ = &p_evt->parameters.packed.param[0];					\
      ADD_ARG(_ptr_, arg1);						\
      ADD_ARG(_ptr_, arg2);						\
      ADD_ARG(_ptr_, arg3);						\
      ADD_ARG(_ptr_, arg4);						\
      ADD_ARG(_ptr_, arg5);						\
      ADD_ARG(_ptr_, arg6);						\
      ADD_ARG(_ptr_, arg7);						\
      ADD_ARG(_ptr_, arg8);						\
    }									\
  } while(0)

#define evnt_probe_pack_9(trace, code, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) do { \
    int total_size = EVNT_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7) + sizeof(arg8) + sizeof(arg9); \
    evnt_t* p_evt = get_event(trace, EVNT_TYPE_PACKED, code, total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - EVNT_BASE_SIZE;			\
      void*_ptr_ = &p_evt->parameters.packed.param[0];					\
      ADD_ARG(_ptr_, arg1);						\
      ADD_ARG(_ptr_, arg2);						\
      ADD_ARG(_ptr_, arg3);						\
      ADD_ARG(_ptr_, arg4);						\
      ADD_ARG(_ptr_, arg5);						\
      ADD_ARG(_ptr_, arg6);						\
      ADD_ARG(_ptr_, arg7);						\
      ADD_ARG(_ptr_, arg8);						\
      ADD_ARG(_ptr_, arg9);						\
    }									\
  } while(0)

#define evnt_probe_pack_10(trace, code, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) do { \
    int total_size = EVNT_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7) + sizeof(arg8) + sizeof(arg9) + sizeof(arg10); \
    evnt_t* p_evt = get_event(trace, EVNT_TYPE_PACKED, code, total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - EVNT_BASE_SIZE;			\
      void*_ptr_ = &p_evt->parameters.packed.param[0];					\
      ADD_ARG(_ptr_, arg1);						\
      ADD_ARG(_ptr_, arg2);						\
      ADD_ARG(_ptr_, arg3);						\
      ADD_ARG(_ptr_, arg4);						\
      ADD_ARG(_ptr_, arg5);						\
      ADD_ARG(_ptr_, arg6);						\
      ADD_ARG(_ptr_, arg7);						\
      ADD_ARG(_ptr_, arg8);						\
      ADD_ARG(_ptr_, arg9);						\
      ADD_ARG(_ptr_, arg10);						\
    }									\
  } while(0)


/*
 * This function finalizes the trace
 */
void evnt_fin_trace(evnt_trace_write_t*);

#endif /* EVNT_WRITE_H_ */
