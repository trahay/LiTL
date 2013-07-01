/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_WRITE_H_
#define LITL_WRITE_H_

#include "litl_types.h"

/*
 * This function initializes the trace
 */
litl_trace_write_t litl_init_trace(const uint32_t);

void litl_buffer_flush_on(litl_trace_write_t*);
void litl_buffer_flush_off(litl_trace_write_t*);

void litl_thread_safety_on(litl_trace_write_t*);
void litl_thread_safety_off(litl_trace_write_t*);

void litl_tid_recording_on(litl_trace_write_t*);
void litl_tid_recording_off(litl_trace_write_t*);

void litl_pause_recording(litl_trace_write_t*);
void litl_resume_recording(litl_trace_write_t*);

void litl_set_filename(litl_trace_write_t*, char*);


/*** Regular events ***/

/*
 * This function writes the recorded events from the buffer to the trace file
 */
void litl_flush_buffer(litl_trace_write_t*, litl_size_t);

/*
 * This function records an event with offset only
 */
void litl_probe_offset(litl_trace_write_t*, int16_t);

/*
 * This function records an event without any arguments
 */
void litl_probe0(litl_trace_write_t*, litl_code_t);

/*
 * This function records an event with one argument
 */
void litl_probe1(litl_trace_write_t*, litl_code_t, litl_param_t);

/*
 * This function records an event with two arguments
 */
void litl_probe2(litl_trace_write_t*, litl_code_t, litl_param_t, litl_param_t);

/*
 * This function records an event with three arguments
 */
void litl_probe3(litl_trace_write_t*, litl_code_t, litl_param_t, litl_param_t, litl_param_t);

/*
 * This function records an event with four arguments
 */
void litl_probe4(litl_trace_write_t*, litl_code_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t);

/*
 * This function records an event with five arguments
 */
void litl_probe5(litl_trace_write_t*, litl_code_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t,
        litl_param_t);

/*
 * This function records an event with six arguments
 */
void litl_probe6(litl_trace_write_t*, litl_code_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t,
        litl_param_t);

/*
 * This function records an event with seven arguments
 */
void litl_probe7(litl_trace_write_t*, litl_code_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t,
        litl_param_t, litl_param_t);

/*
 * This function records an event with eight arguments
 */
void litl_probe8(litl_trace_write_t*, litl_code_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t,
        litl_param_t, litl_param_t, litl_param_t);

/*
 * This function records an event with nine arguments
 */
void litl_probe9(litl_trace_write_t*, litl_code_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t,
        litl_param_t, litl_param_t, litl_param_t, litl_param_t);

/*
 * This function records an event with ten arguments
 */
void litl_probe10(litl_trace_write_t*, litl_code_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t,
        litl_param_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t);



/*** Raw events ***/

/*
 * This function records an event in a raw state, where the size is #args in the void* array
 * That helps to discover places where the application has crashed while using EZTrace
 */
void litl_raw_probe(litl_trace_write_t*, litl_code_t, litl_size_t, litl_data_t[]);



/*** Internal-use macros ***/

/* Allocates an event.
 * For internal use only.
 */
litl_t* __litl_get_event(litl_trace_write_t* trace, litl_type_t type, litl_code_t code, int size);

/* Computes the offset of MEMBER in structure TYPE.
 * For internal use only.
 */
#define __litl_offset_of(TYPE, MEMBER) ((size_t) &((TYPE*)0)->MEMBER)

/* Computes the offset of the first parameter in an event.
 * For internal use only.
 */
#define LITL_BASE_SIZE __litl_offset_of(litl_t, parameters.regular.param)

/* Adds a parameter in an event.
 * For internal use only
 */
#define __LITL_ADD_ARG(_ptr_, arg) do {		\
    typeof(arg) _arg = arg;			\
    memcpy(_ptr_, &_arg, sizeof(_arg));		\
    _ptr_ = ((char*)_ptr_)+sizeof(_arg);	\
  } while(0)


/*** Packed events ***/


/* Records a packed event */
#define litl_probe_pack_0(trace, code) do {				\
    int total_size = LITL_BASE_SIZE;					\
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
    }									\
  } while(0)

/* Records a packed event with 1 parameter */
#define litl_probe_pack_1(trace, code, arg1) do {			\
    int total_size = LITL_BASE_SIZE + sizeof(arg1);			\
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void*_ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_ADD_ARG(_ptr_, arg1);					\
    }									\
  } while(0)

/* Records a packed event with 2 parameters */
#define litl_probe_pack_2(trace, code, arg1, arg2) do {			\
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2);	\
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void*_ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_ADD_ARG(_ptr_, arg1);					\
      __LITL_ADD_ARG(_ptr_, arg2);					\
    }									\
  } while(0)

/* Records a packed event with 3 parameters */
#define litl_probe_pack_3(trace, code, arg1, arg2, arg3) do {		\
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3); \
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void*_ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_ADD_ARG(_ptr_, arg1);					\
      __LITL_ADD_ARG(_ptr_, arg2);					\
      __LITL_ADD_ARG(_ptr_, arg3);					\
    }									\
  } while(0)

/* Records a packed event with 4 parameters */
#define litl_probe_pack_4(trace, code, arg1, arg2, arg3, arg4) do {	\
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void*_ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_ADD_ARG(_ptr_, arg1);					\
      __LITL_ADD_ARG(_ptr_, arg2);					\
      __LITL_ADD_ARG(_ptr_, arg3);					\
      __LITL_ADD_ARG(_ptr_, arg4);					\
    }									\
  } while(0)

/* Records a packed event with 5 parameters */
#define litl_probe_pack_5(trace, code, arg1, arg2, arg3, arg4, arg5) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5);						\
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void*_ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_ADD_ARG(_ptr_, arg1);					\
      __LITL_ADD_ARG(_ptr_, arg2);					\
      __LITL_ADD_ARG(_ptr_, arg3);					\
      __LITL_ADD_ARG(_ptr_, arg4);					\
      __LITL_ADD_ARG(_ptr_, arg5);					\
    }									\
  } while(0)

/* Records a packed event with 6 parameters */
#define litl_probe_pack_6(trace, code, arg1, arg2, arg3, arg4, arg5, arg6) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6);				\
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void*_ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_ADD_ARG(_ptr_, arg1);					\
      __LITL_ADD_ARG(_ptr_, arg2);					\
      __LITL_ADD_ARG(_ptr_, arg3);					\
      __LITL_ADD_ARG(_ptr_, arg4);					\
      __LITL_ADD_ARG(_ptr_, arg5);					\
      __LITL_ADD_ARG(_ptr_, arg6);					\
    }									\
  } while(0)

/* Records a packed event with 7 parameters */
#define litl_probe_pack_7(trace, code, arg1, arg2, arg3, arg4, arg5, arg6, arg7) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7);		\
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void*_ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_ADD_ARG(_ptr_, arg1);					\
      __LITL_ADD_ARG(_ptr_, arg2);					\
      __LITL_ADD_ARG(_ptr_, arg3);					\
      __LITL_ADD_ARG(_ptr_, arg4);					\
      __LITL_ADD_ARG(_ptr_, arg5);					\
      __LITL_ADD_ARG(_ptr_, arg6);					\
      __LITL_ADD_ARG(_ptr_, arg7);					\
    }									\
  } while(0)

/* Records a packed event with 8 parameters */
#define litl_probe_pack_8(trace, code, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7) + sizeof(arg8); \
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void*_ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_ADD_ARG(_ptr_, arg1);					\
      __LITL_ADD_ARG(_ptr_, arg2);					\
      __LITL_ADD_ARG(_ptr_, arg3);					\
      __LITL_ADD_ARG(_ptr_, arg4);					\
      __LITL_ADD_ARG(_ptr_, arg5);					\
      __LITL_ADD_ARG(_ptr_, arg6);					\
      __LITL_ADD_ARG(_ptr_, arg7);					\
      __LITL_ADD_ARG(_ptr_, arg8);					\
    }									\
  } while(0)

/* Records a packed event with 9 parameters */
#define litl_probe_pack_9(trace, code, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7) + sizeof(arg8) + sizeof(arg9); \
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void*_ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_ADD_ARG(_ptr_, arg1);					\
      __LITL_ADD_ARG(_ptr_, arg2);					\
      __LITL_ADD_ARG(_ptr_, arg3);					\
      __LITL_ADD_ARG(_ptr_, arg4);					\
      __LITL_ADD_ARG(_ptr_, arg5);					\
      __LITL_ADD_ARG(_ptr_, arg6);					\
      __LITL_ADD_ARG(_ptr_, arg7);					\
      __LITL_ADD_ARG(_ptr_, arg8);					\
      __LITL_ADD_ARG(_ptr_, arg9);					\
    }									\
  } while(0)

/* Records a packed event with 10 parameters */
#define litl_probe_pack_10(trace, code, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7) + sizeof(arg8) + sizeof(arg9) + sizeof(arg10); \
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void*_ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_ADD_ARG(_ptr_, arg1);					\
      __LITL_ADD_ARG(_ptr_, arg2);					\
      __LITL_ADD_ARG(_ptr_, arg3);					\
      __LITL_ADD_ARG(_ptr_, arg4);					\
      __LITL_ADD_ARG(_ptr_, arg5);					\
      __LITL_ADD_ARG(_ptr_, arg6);					\
      __LITL_ADD_ARG(_ptr_, arg7);					\
      __LITL_ADD_ARG(_ptr_, arg8);					\
      __LITL_ADD_ARG(_ptr_, arg9);					\
      __LITL_ADD_ARG(_ptr_, arg10);					\
    }									\
  } while(0)

/*
 * This function finalizes the trace
 */
void litl_fin_trace(litl_trace_write_t*);

#endif /* LITL_WRITE_H_ */
