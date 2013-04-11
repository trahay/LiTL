/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef EVNT_READ_H_
#define EVNT_READ_H_

#include "evnt_types.h"

/*
 * This function sets the buffer size for reading the trace file
 */
void set_read_buffer_size(const uint32_t);

/*
 * This function opens a trace and reads the first portion of data to the buffer
 */
evnt_buffer_t evnt_open_trace(const char*);

/*
 * This function reads information contained in the trace header
 */
evnt_info_t* evnt_get_trace_header(evnt_block_t*);

/*
 * This function returns the current trace, FILE pointer, and the current position in the file
 */
evnt_block_t evnt_get_block(evnt_buffer_t);

/*
 * This function resets the trace
 */
void evnt_reset_trace(evnt_block_t*);

/*
 * This function reads an event
 */
evnt_t* evnt_read_event(evnt_block_t*);

/*
 * This function searches for the next event
 */
evnt_t* evnt_next_event(evnt_block_t*);

/*
 * This function closes the trace and frees the buffer
 */
void evnt_close_trace(evnt_block_t*);



#define GET_ARG_REGULAR(ptr, arg) do {		\
    arg = *(evnt_param_t*)ptr;			\
    (evnt_param_t*)ptr++;			\
  } while(0)


#define GET_ARG_PACKED(ptr, arg) do {		\
    memcpy(&arg, ptr, sizeof(arg));		\
    ptr = ((char*)ptr)+sizeof(arg);		\
  } while(0)

#define GET_ARG(evt, ptr, arg)			\
  do {						\
    if(evt->type == EVENT_TYPE_REGULAR)		\
      GET_ARG_REGULAR(ptr, arg);		\
    else					\
      GET_ARG_PACKED(ptr, arg);			\
  } while(0)


#define INIT_PTR(evt, ptr)				\
  do {							\
    if(evt->type == EVENT_TYPE_REGULAR)			\
      ptr = &evt->parameters.regular.param[0];		\
    else						\
      ptr = &evt->parameters.packed.param[0];			\
  } while(0)

#define evnt_get_param_1(p_evt, arg1)		\
  do {						\
    void* ptr;					\
    INIT_PTR(p_evt, ptr);			\
    GET_ARG(p_evt, ptr, arg1);			\
  } while(0)

#define evnt_get_param_2(p_evt, arg1, arg2)	\
  do {						\
    void* ptr;					\
    INIT_PTR(p_evt, ptr);			\
    GET_ARG(p_evt, ptr, arg1);			\
    GET_ARG(p_evt, ptr, arg2);			\
  } while(0)

#define evnt_get_param_3(p_evt, arg1, arg2, arg3)	\
  do {							\
    void* ptr;						\
    INIT_PTR(p_evt, ptr);				\
    GET_ARG(p_evt, ptr, arg1);				\
    GET_ARG(p_evt, ptr, arg2);				\
    GET_ARG(p_evt, ptr, arg3);				\
  } while(0)

#define evnt_get_param_4(p_evt, arg1, arg2, arg3, arg4)	\
  do {							\
    void* ptr;						\
    INIT_PTR(p_evt, ptr);				\
    GET_ARG(p_evt, ptr, arg1);				\
    GET_ARG(p_evt, ptr, arg2);				\
    GET_ARG(p_evt, ptr, arg3);				\
    GET_ARG(p_evt, ptr, arg4);				\
  } while(0)

#define evnt_get_param_5(p_evt, arg1, arg2, arg3, arg4, arg5)	\
  do {								\
    void* ptr;							\
    INIT_PTR(p_evt, ptr);					\
    GET_ARG(p_evt, ptr, arg1);					\
    GET_ARG(p_evt, ptr, arg2);					\
    GET_ARG(p_evt, ptr, arg3);					\
    GET_ARG(p_evt, ptr, arg4);					\
    GET_ARG(p_evt, ptr, arg5);					\
  } while(0)

#define evnt_get_param_6(p_evt, arg1, arg2, arg3, arg4, arg5, arg6)	\
  do {									\
    void* ptr;								\
    INIT_PTR(p_evt, ptr);						\
    GET_ARG(p_evt, ptr, arg1);						\
    GET_ARG(p_evt, ptr, arg2);						\
    GET_ARG(p_evt, ptr, arg3);						\
    GET_ARG(p_evt, ptr, arg4);						\
    GET_ARG(p_evt, ptr, arg5);						\
    GET_ARG(p_evt, ptr, arg6);						\
  } while(0)

#define evnt_get_param_7(p_evt, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
  do {									\
    void* ptr;								\
    INIT_PTR(p_evt, ptr);						\
    GET_ARG(p_evt, ptr, arg1);						\
    GET_ARG(p_evt, ptr, arg2);						\
    GET_ARG(p_evt, ptr, arg3);						\
    GET_ARG(p_evt, ptr, arg4);						\
    GET_ARG(p_evt, ptr, arg5);						\
    GET_ARG(p_evt, ptr, arg6);						\
    GET_ARG(p_evt, ptr, arg7);						\
  } while(0)

#define evnt_get_param_8(p_evt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
  do {									\
    void* ptr;								\
    INIT_PTR(p_evt, ptr);						\
    GET_ARG(p_evt, ptr, arg1);						\
    GET_ARG(p_evt, ptr, arg2);						\
    GET_ARG(p_evt, ptr, arg3);						\
    GET_ARG(p_evt, ptr, arg4);						\
    GET_ARG(p_evt, ptr, arg5);						\
    GET_ARG(p_evt, ptr, arg6);						\
    GET_ARG(p_evt, ptr, arg7);						\
    GET_ARG(p_evt, ptr, arg8);						\
  } while(0)

#define evnt_get_param_9(p_evt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) \
  do {									\
    void* ptr;								\
    INIT_PTR(p_evt, ptr);						\
    GET_ARG(p_evt, ptr, arg1);						\
    GET_ARG(p_evt, ptr, arg2);						\
    GET_ARG(p_evt, ptr, arg3);						\
    GET_ARG(p_evt, ptr, arg4);						\
    GET_ARG(p_evt, ptr, arg5);						\
    GET_ARG(p_evt, ptr, arg6);						\
    GET_ARG(p_evt, ptr, arg7);						\
    GET_ARG(p_evt, ptr, arg8);						\
    GET_ARG(p_evt, ptr, arg9);						\
  } while(0)

#define evnt_get_param_10(p_evt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) \
  do {									\
    void* ptr;								\
    INIT_PTR(p_evt, ptr);						\
    GET_ARG(p_evt, ptr, arg1);						\
    GET_ARG(p_evt, ptr, arg2);						\
    GET_ARG(p_evt, ptr, arg3);						\
    GET_ARG(p_evt, ptr, arg4);						\
    GET_ARG(p_evt, ptr, arg5);						\
    GET_ARG(p_evt, ptr, arg6);						\
    GET_ARG(p_evt, ptr, arg7);						\
    GET_ARG(p_evt, ptr, arg8);						\
    GET_ARG(p_evt, ptr, arg9);						\
    GET_ARG(p_evt, ptr, arg10);						\
  } while(0)

#endif /* EVNT_READ_H_ */
