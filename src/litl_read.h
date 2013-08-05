/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_READ_H_
#define LITL_READ_H_

#include "litl_types.h"

/*
 * Opens a trace and reads the first portion of data to the buffer
 */
litl_trace_read_t *litl_open_trace(const char*);

/*
 * Initializes the archive's traces
 */
void litl_init_traces(litl_trace_read_t*);

/*
 * Returns a pointer to the header
 */
litl_header_t* litl_get_trace_header(litl_trace_read_t*);

/*
 * Sets the buffer size
 */
void litl_set_buffer_size(litl_trace_read_t*, const litl_size_t);

/*
 * Returns the buffer size
 */
litl_size_t litl_get_buffer_size(litl_trace_read_t*);

/*
 * Resets the trace
 */
void litl_reset_trace(litl_trace_read_t*, litl_size_t);

/*
 * Searches for the next event inside the trace
 */
litl_read_t* litl_next_trace_event(litl_trace_read_t*, litl_size_t);

/*
 * Reads the next event either from an archive or a regular trace
 */
litl_read_t* litl_next_event(litl_trace_read_t*);

/*
 * Closes the trace and frees the allocated memory
 */
void litl_close_trace(litl_trace_read_t*);

/*** Internal-use macros ***/

/*
 * For internal use only
 * Returns the next parameter in a regular event
 */
#define __LITL_GET_ARG_REGULAR(_ptr_, arg) do {	\
    arg = *(litl_param_t*)_ptr_;		\
    (litl_param_t*)_ptr_++;			\
  } while(0)

/*
 * For internal use only
 * Returns the next parameter in a packed event
 */
#define __LITL_GET_ARG_PACKED(_ptr_, arg) do {	\
    memcpy(&arg, _ptr_, sizeof(arg));		\
    _ptr_ = ((char*)_ptr_)+sizeof(arg);		\
  } while(0)

/*
 * For internal use only
 * Returns the next parameter in an event
 */
#define __LITL_GET_ARG(evt, _ptr_, arg)		\
  do {						\
    if(LITL_GET_TYPE(evt) == LITL_TYPE_REGULAR)	\
      __LITL_GET_ARG_REGULAR(_ptr_, arg);	\
    else					\
      __LITL_GET_ARG_PACKED(_ptr_, arg);	\
  } while(0)

/*
 * For internal use only
 * Initializes a pointer for browsing the parameters of an event
 */
#define __LITL_INIT_PTR(evt, _ptr_)		\
  do {						\
    if(LITL_GET_TYPE(evt) == LITL_TYPE_REGULAR)	\
      _ptr_ = &LITL_REGULAR(evt)->param[0];	\
    else					\
      _ptr_ = &LITL_PACKED(evt)->param[0];	\
  } while(0)

/*** functions for reading events ***/

#define LITL_GET_CUR_EVENT_PER_THREAD(_trace_, _thread_index_) (&(_trace_)->buffers[(_thread_index_)].cur_event)
#define LITL_GET_CUR_EVENT(_trace_) LITL_GET_CUR_EVENT_PER_THREAD(_trace_, (trace)->cur_index)

#define LITL_GET_TID(_read_event_) (_read_event_)->tid
#define LITL_GET_TIME(_read_event_) (_read_event_)->event->time
#define LITL_GET_TYPE(_read_event_) (_read_event_)->event->type
#define LITL_GET_CODE(_read_event_) (_read_event_)->event->code

#define LITL_RAW(_read_event_) (&(_read_event_)->event->parameters.raw)
#define LITL_REGULAR(_read_event_) (&(_read_event_)->event->parameters.regular)
#define LITL_PACKED(_read_event_) (&(_read_event_)->event->parameters.packed)
#define LITL_OFFSET(_read_event_) (&(_read_event_)->event->parameters.offset)

/*
 * Assigns the first parameter of p_evt to arg1
 */
#define litl_get_param_1(p_evt, arg1)		\
  do {						\
    void* _ptr_;				\
    __LITL_INIT_PTR(p_evt, _ptr_);			\
    __LITL_GET_ARG(p_evt, _ptr_, arg1);		\
  } while(0)

/*
 * Assigns the first 2 parameters of p_evt to arg1 and arg2
 */
#define litl_get_param_2(p_evt, arg1, arg2)	\
  do {						\
    void* _ptr_;				\
    __LITL_INIT_PTR(p_evt, _ptr_);			\
    __LITL_GET_ARG(p_evt, _ptr_, arg1);		\
    __LITL_GET_ARG(p_evt, _ptr_, arg2);		\
  } while(0)

/*
 * Assigns the first 3 parameters of p_evt to arg1, ..., arg3
 */
#define litl_get_param_3(p_evt, arg1, arg2, arg3)	\
  do {							\
    void* _ptr_;					\
    __LITL_INIT_PTR(p_evt, _ptr_);				\
    __LITL_GET_ARG(p_evt, _ptr_, arg1);			\
    __LITL_GET_ARG(p_evt, _ptr_, arg2);			\
    __LITL_GET_ARG(p_evt, _ptr_, arg3);			\
  } while(0)

/*
 * Assigns the first 4 parameters of p_evt to arg1, ..., arg4
 */
#define litl_get_param_4(p_evt, arg1, arg2, arg3, arg4)	\
  do {							\
    void* _ptr_;					\
    __LITL_INIT_PTR(p_evt, _ptr_);				\
    __LITL_GET_ARG(p_evt, _ptr_, arg1);			\
    __LITL_GET_ARG(p_evt, _ptr_, arg2);			\
    __LITL_GET_ARG(p_evt, _ptr_, arg3);			\
    __LITL_GET_ARG(p_evt, _ptr_, arg4);			\
  } while(0)

/*
 * Assigns the first 5 parameters of p_evt to arg1, ..., arg5
 */
#define litl_get_param_5(p_evt, arg1, arg2, arg3, arg4, arg5)	\
  do {								\
    void* _ptr_;						\
    __LITL_INIT_PTR(p_evt, _ptr_);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg1);				\
    __LITL_GET_ARG(p_evt, _ptr_, arg2);				\
    __LITL_GET_ARG(p_evt, _ptr_, arg3);				\
    __LITL_GET_ARG(p_evt, _ptr_, arg4);				\
    __LITL_GET_ARG(p_evt, _ptr_, arg5);				\
  } while(0)

/*
 * Assigns the first 6 parameters of p_evt to arg1, ..., arg6
 */
#define litl_get_param_6(p_evt, arg1, arg2, arg3, arg4, arg5, arg6)	\
  do {									\
    void* _ptr_;							\
    __LITL_INIT_PTR(p_evt, _ptr_);						\
    __LITL_GET_ARG(p_evt, _ptr_, arg1);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg2);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg3);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg4);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg5);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg6);					\
  } while(0)

/*
 * Assigns the first 7 parameters of p_evt to arg1, ..., arg7
 */
#define litl_get_param_7(p_evt, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
  do {									\
    void* _ptr_;							\
    __LITL_INIT_PTR(p_evt, _ptr_);						\
    __LITL_GET_ARG(p_evt, _ptr_, arg1);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg2);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg3);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg4);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg5);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg6);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg7);					\
  } while(0)

/*
 * Assigns the first 8 parameters of p_evt to arg1, ..., arg8
 */
#define litl_get_param_8(p_evt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
  do {									\
    void* _ptr_;							\
    __LITL_INIT_PTR(p_evt, _ptr_);						\
    __LITL_GET_ARG(p_evt, _ptr_, arg1);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg2);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg3);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg4);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg5);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg6);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg7);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg8);					\
  } while(0)

/*
 * Assigns the first 9 parameters of p_evt to arg1, ..., arg9
 */
#define litl_get_param_9(p_evt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) \
  do {									\
    void* _ptr_;							\
    __LITL_INIT_PTR(p_evt, _ptr_);						\
    __LITL_GET_ARG(p_evt, _ptr_, arg1);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg2);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg3);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg4);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg5);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg6);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg7);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg8);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg9);					\
  } while(0)

/*
 * Assigns the first 10 parameters of p_evt to arg1, ..., arg10
 */
#define litl_get_param_10(p_evt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) \
  do {									\
    void* _ptr_;							\
    __LITL_INIT_PTR(p_evt, _ptr_);						\
    __LITL_GET_ARG(p_evt, _ptr_, arg1);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg2);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg3);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg4);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg5);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg6);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg7);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg8);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg9);					\
    __LITL_GET_ARG(p_evt, _ptr_, arg10);				\
  } while(0)

#endif /* LITL_READ_H_ */
