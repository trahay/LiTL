/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef EVNT_READ_H_
#define EVNT_READ_H_

#include "evnt_types.h"

/*
 * This function opens a trace and reads the first portion of data to the buffer
 */
evnt_trace_read_t *evnt_open_trace(const char*);

/*
 * This function sets the buffer size
 */
void evnt_set_buffer_size(evnt_trace_read_t*, const evnt_size_t);

/*
 * This function returns the buffer size
 */
evnt_size_t evnt_get_buffer_size(evnt_trace_read_t*);

/*
 * This function initializes the header structure
 */
void evnt_init_trace_header(evnt_trace_read_t*);

/*
 * This function return a pointer to the header
 */
evnt_header_t* evnt_get_trace_header(evnt_trace_read_t*);

/*
 * This function initializes buffer of each recorded thread. A buffer per thread.
 */
void evnt_init_buffers(evnt_trace_read_t*);

/*
 * This function resets the trace
 */
void evnt_reset_trace(evnt_trace_read_t*, evnt_size_t);

/*
 * This function reads an event
 */
read_evnt_t* evnt_read_event(evnt_trace_read_t*, evnt_size_t);

/*
 * This function searches for the next event inside the buffer
 */
read_evnt_t* evnt_next_buffer_event(evnt_trace_read_t*, evnt_size_t);

/*
 * This function searches for the next event inside the trace
 */
read_evnt_t* evnt_next_trace_event(evnt_trace_read_t*);

/*
 * This function closes the trace and frees the allocated memory
 */
void evnt_close_trace(evnt_trace_read_t*);



#define GET_ARG_REGULAR(_ptr_, arg) do {		\
    arg = *(evnt_param_t*)_ptr_;			\
    (evnt_param_t*)_ptr_++;			\
  } while(0)


#define GET_ARG_PACKED(_ptr_, arg) do {		\
    memcpy(&arg, _ptr_, sizeof(arg));		\
    _ptr_ = ((char*)_ptr_)+sizeof(arg);		\
  } while(0)

#define GET_ARG(evt, _ptr_, arg)			\
  do {						\
    if(EVENT_GET_TYPE(evt) == EVNT_TYPE_REGULAR)	\
      GET_ARG_REGULAR(_ptr_, arg);		\
    else					\
      GET_ARG_PACKED(_ptr_, arg);			\
  } while(0)


#define INIT_PTR(evt, _ptr_)				\
  do {							\
    if(EVENT_GET_TYPE(evt) == EVNT_TYPE_REGULAR)		\
      _ptr_ = &EVENT_REGULAR(evt)->param[0];	\
    else						\
      _ptr_ = &EVENT_PACKED(evt)->param[0];			\
  } while(0)

#define evnt_get_param_1(p_evt, arg1)		\
  do {						\
    void* _ptr_;					\
    INIT_PTR(p_evt, _ptr_);			\
    GET_ARG(p_evt, _ptr_, arg1);			\
  } while(0)

#define evnt_get_param_2(p_evt, arg1, arg2)	\
  do {						\
    void* _ptr_;					\
    INIT_PTR(p_evt, _ptr_);			\
    GET_ARG(p_evt, _ptr_, arg1);			\
    GET_ARG(p_evt, _ptr_, arg2);			\
  } while(0)

#define evnt_get_param_3(p_evt, arg1, arg2, arg3)	\
  do {							\
    void* _ptr_;						\
    INIT_PTR(p_evt, _ptr_);				\
    GET_ARG(p_evt, _ptr_, arg1);				\
    GET_ARG(p_evt, _ptr_, arg2);				\
    GET_ARG(p_evt, _ptr_, arg3);				\
  } while(0)

#define evnt_get_param_4(p_evt, arg1, arg2, arg3, arg4)	\
  do {							\
    void* _ptr_;						\
    INIT_PTR(p_evt, _ptr_);				\
    GET_ARG(p_evt, _ptr_, arg1);				\
    GET_ARG(p_evt, _ptr_, arg2);				\
    GET_ARG(p_evt, _ptr_, arg3);				\
    GET_ARG(p_evt, _ptr_, arg4);				\
  } while(0)

#define evnt_get_param_5(p_evt, arg1, arg2, arg3, arg4, arg5)	\
  do {								\
    void* _ptr_;							\
    INIT_PTR(p_evt, _ptr_);					\
    GET_ARG(p_evt, _ptr_, arg1);					\
    GET_ARG(p_evt, _ptr_, arg2);					\
    GET_ARG(p_evt, _ptr_, arg3);					\
    GET_ARG(p_evt, _ptr_, arg4);					\
    GET_ARG(p_evt, _ptr_, arg5);					\
  } while(0)

#define evnt_get_param_6(p_evt, arg1, arg2, arg3, arg4, arg5, arg6)	\
  do {									\
    void* _ptr_;								\
    INIT_PTR(p_evt, _ptr_);						\
    GET_ARG(p_evt, _ptr_, arg1);						\
    GET_ARG(p_evt, _ptr_, arg2);						\
    GET_ARG(p_evt, _ptr_, arg3);						\
    GET_ARG(p_evt, _ptr_, arg4);						\
    GET_ARG(p_evt, _ptr_, arg5);						\
    GET_ARG(p_evt, _ptr_, arg6);						\
  } while(0)

#define evnt_get_param_7(p_evt, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
  do {									\
    void* _ptr_;								\
    INIT_PTR(p_evt, _ptr_);						\
    GET_ARG(p_evt, _ptr_, arg1);						\
    GET_ARG(p_evt, _ptr_, arg2);						\
    GET_ARG(p_evt, _ptr_, arg3);						\
    GET_ARG(p_evt, _ptr_, arg4);						\
    GET_ARG(p_evt, _ptr_, arg5);						\
    GET_ARG(p_evt, _ptr_, arg6);						\
    GET_ARG(p_evt, _ptr_, arg7);						\
  } while(0)

#define evnt_get_param_8(p_evt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
  do {									\
    void* _ptr_;								\
    INIT_PTR(p_evt, _ptr_);						\
    GET_ARG(p_evt, _ptr_, arg1);						\
    GET_ARG(p_evt, _ptr_, arg2);						\
    GET_ARG(p_evt, _ptr_, arg3);						\
    GET_ARG(p_evt, _ptr_, arg4);						\
    GET_ARG(p_evt, _ptr_, arg5);						\
    GET_ARG(p_evt, _ptr_, arg6);						\
    GET_ARG(p_evt, _ptr_, arg7);						\
    GET_ARG(p_evt, _ptr_, arg8);						\
  } while(0)

#define evnt_get_param_9(p_evt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) \
  do {									\
    void* _ptr_;								\
    INIT_PTR(p_evt, _ptr_);						\
    GET_ARG(p_evt, _ptr_, arg1);						\
    GET_ARG(p_evt, _ptr_, arg2);						\
    GET_ARG(p_evt, _ptr_, arg3);						\
    GET_ARG(p_evt, _ptr_, arg4);						\
    GET_ARG(p_evt, _ptr_, arg5);						\
    GET_ARG(p_evt, _ptr_, arg6);						\
    GET_ARG(p_evt, _ptr_, arg7);						\
    GET_ARG(p_evt, _ptr_, arg8);						\
    GET_ARG(p_evt, _ptr_, arg9);						\
  } while(0)

#define evnt_get_param_10(p_evt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) \
  do {									\
    void* _ptr_;								\
    INIT_PTR(p_evt, _ptr_);						\
    GET_ARG(p_evt, _ptr_, arg1);						\
    GET_ARG(p_evt, _ptr_, arg2);						\
    GET_ARG(p_evt, _ptr_, arg3);						\
    GET_ARG(p_evt, _ptr_, arg4);						\
    GET_ARG(p_evt, _ptr_, arg5);						\
    GET_ARG(p_evt, _ptr_, arg6);						\
    GET_ARG(p_evt, _ptr_, arg7);						\
    GET_ARG(p_evt, _ptr_, arg8);						\
    GET_ARG(p_evt, _ptr_, arg9);						\
    GET_ARG(p_evt, _ptr_, arg10);						\
  } while(0)

#endif /* EVNT_READ_H_ */
