/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_WRITE_H_
#define LITL_WRITE_H_

#include "litl_types.h"

/*
 * Initializes the trace buffer
 */
litl_trace_write_t litl_init_trace(const uint32_t);

/*
 * Activates buffer flush
 */
void litl_buffer_flush_on(litl_trace_write_t*);
/*
 * Deactivates buffer flush. By default, it is activated
 */
void litl_buffer_flush_off(litl_trace_write_t*);

/*
 * Activates thread safety
 */
void litl_thread_safety_on(litl_trace_write_t*);
/*
 * Deactivates thread safety
 */
void litl_thread_safety_off(litl_trace_write_t*);

/*
 * Activates recording tid
 */
void litl_tid_recording_on(litl_trace_write_t*);
/*
 * Deactivates recording tid
 */
void litl_tid_recording_off(litl_trace_write_t*);

/*
 * Pauses the event recording
 */
void litl_pause_recording(litl_trace_write_t*);
/*
 * Resumes the event recording
 */
void litl_resume_recording(litl_trace_write_t*);

/*
 * Sets a new name for the trace file
 */
void litl_set_filename(litl_trace_write_t*, char*);

/*
 * Writes the recorded events from the buffer to the trace file
 */
void litl_flush_buffer(litl_trace_write_t*, litl_size_t);

/*
 * Records an event with offset only
 */
void litl_probe_offset(litl_trace_write_t*, int16_t);


/*** Regular events ***/

/*
 * Records a regular event without parameters
 */
void litl_probe_reg_0(litl_trace_write_t*, litl_code_t);

/*
 * Records a regular event with 1 parameter
 */
void litl_probe_reg_1(litl_trace_write_t*, litl_code_t, litl_param_t);

/*
 * Records a regular event with 2 parameters
 */
void litl_probe_reg_2(litl_trace_write_t*, litl_code_t, litl_param_t,
        litl_param_t);

/*
 * Records a regular event with 3 parameters
 */
void litl_probe_reg_3(litl_trace_write_t*, litl_code_t, litl_param_t,
        litl_param_t, litl_param_t);

/*
 * Records a regular event with 4 parameters
 */
void litl_probe_reg_4(litl_trace_write_t*, litl_code_t, litl_param_t,
        litl_param_t, litl_param_t, litl_param_t);

/*
 * Records a regular event with 5 parameters
 */
void litl_probe_reg_5(litl_trace_write_t*, litl_code_t, litl_param_t,
        litl_param_t, litl_param_t, litl_param_t, litl_param_t);

/*
 * Records a regular event with 6 parameters
 */
void litl_probe_reg_6(litl_trace_write_t*, litl_code_t, litl_param_t,
        litl_param_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t);

/*
 * Records a regular event with 7 parameters
 */
void litl_probe_reg_7(litl_trace_write_t*, litl_code_t, litl_param_t,
        litl_param_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t,
        litl_param_t);

/*
 * Records a regular event with 8 parameters
 */
void litl_probe_reg_8(litl_trace_write_t*, litl_code_t, litl_param_t,
        litl_param_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t,
        litl_param_t, litl_param_t);

/*
 * Records a regular event with 9 parameters
 */
void litl_probe_reg_9(litl_trace_write_t*, litl_code_t, litl_param_t,
        litl_param_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t,
        litl_param_t, litl_param_t, litl_param_t);

/*
 * Records a regular event with 10 parameters
 */
void litl_probe_reg_10(litl_trace_write_t*, litl_code_t, litl_param_t,
        litl_param_t, litl_param_t, litl_param_t, litl_param_t, litl_param_t,
        litl_param_t, litl_param_t, litl_param_t, litl_param_t);


/*** Raw events ***/

/*
 * Records an event in a raw state, where the size is #args in the void* array.
 * That helps to discover places where the application has crashed
 */
void litl_probe_raw(litl_trace_write_t*, litl_code_t, litl_size_t,
        litl_data_t[]);


/*** Internal-use macros ***/

/*
 * For internal use only.
 * Allocates an event
 */
litl_t* __litl_get_event(litl_trace_write_t* trace, litl_type_t type,
        litl_code_t code, int size);

/*
 * For internal use only.
 * Computes the offset of MEMBER in structure TYPE
 */
#define __litl_offset_of(TYPE, MEMBER) ((size_t) &((TYPE*)0)->MEMBER)

/*
 * For internal use only.
 * Computes the offset of the first parameter in an event
 */
#define LITL_BASE_SIZE __litl_offset_of(litl_t, parameters.regular.param)

/*
 * For internal use only.
 * Adds a parameter in an event
 */
#define __LITL_ADD_ARG(_ptr_, arg) do {		\
    typeof(arg) _arg = arg;			\
    memcpy(_ptr_, &_arg, sizeof(_arg));		\
    _ptr_ = ((char*)_ptr_)+sizeof(_arg);	\
  } while(0)


/*** Packed events ***/

/*
 * Records a packed event without parameters
 */
#define litl_probe_pack_0(trace, code) do {				\
    int total_size = LITL_BASE_SIZE;					\
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
    }									\
  } while(0)

/*
 * Records a packed event with 1 parameter
 */
#define litl_probe_pack_1(trace, code, arg1) do {			\
    int total_size = LITL_BASE_SIZE + sizeof(arg1);			\
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_ADD_ARG(_ptr_, arg1);					\
    }									\
  } while(0)

/*
 * Records a packed event with 2 parameters
 */
#define litl_probe_pack_2(trace, code, arg1, arg2) do {			\
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2);	\
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_ADD_ARG(_ptr_, arg1);					\
      __LITL_ADD_ARG(_ptr_, arg2);					\
    }									\
  } while(0)

/*
 * Records a packed event with 3 parameters
 */
#define litl_probe_pack_3(trace, code, arg1, arg2, arg3) do {		\
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3); \
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_ADD_ARG(_ptr_, arg1);					\
      __LITL_ADD_ARG(_ptr_, arg2);					\
      __LITL_ADD_ARG(_ptr_, arg3);					\
    }									\
  } while(0)

/*
 * Records a packed event with 4 parameters
 */
#define litl_probe_pack_4(trace, code, arg1, arg2, arg3, arg4) do {	\
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_ADD_ARG(_ptr_, arg1);					\
      __LITL_ADD_ARG(_ptr_, arg2);					\
      __LITL_ADD_ARG(_ptr_, arg3);					\
      __LITL_ADD_ARG(_ptr_, arg4);					\
    }									\
  } while(0)

/*
 * Records a packed event with 5 parameters
 */
#define litl_probe_pack_5(trace, code, arg1, arg2, arg3, arg4, arg5) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5);						\
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_ADD_ARG(_ptr_, arg1);					\
      __LITL_ADD_ARG(_ptr_, arg2);					\
      __LITL_ADD_ARG(_ptr_, arg3);					\
      __LITL_ADD_ARG(_ptr_, arg4);					\
      __LITL_ADD_ARG(_ptr_, arg5);					\
    }									\
  } while(0)

/*
 * Records a packed event with 6 parameters
 */
#define litl_probe_pack_6(trace, code, arg1, arg2, arg3, arg4, arg5, arg6) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6);				\
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_ADD_ARG(_ptr_, arg1);					\
      __LITL_ADD_ARG(_ptr_, arg2);					\
      __LITL_ADD_ARG(_ptr_, arg3);					\
      __LITL_ADD_ARG(_ptr_, arg4);					\
      __LITL_ADD_ARG(_ptr_, arg5);					\
      __LITL_ADD_ARG(_ptr_, arg6);					\
    }									\
  } while(0)

/*
 * Records a packed event with 7 parameters
 */
#define litl_probe_pack_7(trace, code, arg1, arg2, arg3, arg4, arg5, arg6, arg7) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7);		\
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_ADD_ARG(_ptr_, arg1);					\
      __LITL_ADD_ARG(_ptr_, arg2);					\
      __LITL_ADD_ARG(_ptr_, arg3);					\
      __LITL_ADD_ARG(_ptr_, arg4);					\
      __LITL_ADD_ARG(_ptr_, arg5);					\
      __LITL_ADD_ARG(_ptr_, arg6);					\
      __LITL_ADD_ARG(_ptr_, arg7);					\
    }									\
  } while(0)

/*
 * Records a packed event with 8 parameters
 */
#define litl_probe_pack_8(trace, code, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7) + sizeof(arg8); \
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
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

/*
 * Records a packed event with 9 parameters
 */
#define litl_probe_pack_9(trace, code, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7) + sizeof(arg8) + sizeof(arg9); \
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
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

/*
 * Records a packed event with 10 parameters
 */
#define litl_probe_pack_10(trace, code, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7) + sizeof(arg8) + sizeof(arg9) + sizeof(arg10); \
    litl_t* p_evt = __litl_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
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
 * Finalizes the trace
 */
void litl_fin_trace(litl_trace_write_t*);

#endif /* LITL_WRITE_H_ */
