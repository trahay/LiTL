/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_READ_H_
#define LITL_READ_H_

/**
 *  \file litl_read.h
 *  \brief litl_read Provides a set of functions for reading events from a
 *  regular trace file or an archive of traces
 *
 *  \authors
 *    Developers are: \n
 *        Roman Iakymchuk   -- roman.iakymchuk@telecom-sudparis.eu \n
 *        Francois Trahay   -- francois.trahay@telecom-sudparis.eu \n
 */

#include "litl_types.h"

/**
 * \defgroup litl_read LiTL Reading Functions
 */

/**
 * \defgroup litl_read_init Initialization Functions
 * \ingroup litl_read
 */

/**
 * \defgroup litl_read_main Functions for Reading Events
 * \ingroup litl_read
 */

/**
 * \defgroup litl_read_process Functions for Processing Events
 * \ingroup litl_read
 */

/**
 * \ingroup litl_read_init
 * \brief Opens a trace and reads the first portion of data (trace header) to the buffer
 * \param filename A filename
 * \return A pointer to the trace object. NULL in case of failure
 */
litl_read_trace_t *litl_read_open_trace(const char* filename);

/**
 * \ingroup litl_read_init
 * \brief Initializes the event reading structure
 * \param trace A pointer to the trace object
 */
void litl_read_init_processes(litl_read_trace_t* trace);

/**
 * \ingroup litl_read_init
 * \brief Returns a pointer to the trace header
 * \param trace A pointer to the trace object
 * \return A pointer to the trace header
 */
litl_general_header_t* litl_read_get_trace_header(litl_read_trace_t* trace);

/**
 * \ingroup litl_read_init
 * \brief Returns a pointer to the process header
 * \param process A pointer to the process object
 * \return A pointer to the trace header
 */
litl_process_header_t* litl_read_get_process_header(
        litl_read_process_t* process);

/**
 * \ingroup litl_read_init
 * \brief Sets the buffer size
 * \param trace A pointer to the trace object
 * \param buf_size A buffer size (in Byte)
 */
void litl_read_set_buffer_size(litl_read_trace_t* trace,
        const litl_size_t buf_size);

/**
 * \ingroup litl_read_init
 * \brief Returns the buffer size
 * \param trace A pointer to the trace object
 * \return A buffer size (in Byte)
 */
litl_size_t litl_read_get_buffer_size(litl_read_trace_t* trace);

/**
 * \ingroup litl_read_main
 * \brief Resets the trace pointer
 * \param trace A pointer to the trace object
 * \param process A pointer to the process object
 */
void litl_read_reset_process(litl_read_trace_t* trace,
        litl_read_process_t* process);

/**
 * \ingroup litl_read_main
 * \brief Reads the next event from a trace
 * \param trace A pointer to the trace object
 * \param process A pointer to the process object
 */
litl_read_event_t* litl_read_next_process_event(litl_read_trace_t* trace,
        litl_read_process_t* process);

/**
 * \ingroup litl_read_main
 * \brief Reads the next event from a trace file
 * \param trace A pointer to the trace object
 */
litl_read_event_t* litl_read_next_event(litl_read_trace_t* trace);

/**
 * \ingroup litl_read_main
 * \brief Closes the trace and frees the allocated memory
 * \param trace A pointer to the trace object
 */
void litl_read_finalize_trace(litl_read_trace_t* trace);

/*** Internal-use macros ***/

/*
 * For internal use only
 * Initializes a pointer for browsing the parameters of an event
 */
#define __LITL_READ_INIT_PTR(evt, _ptr_)                        \
  do {                                                          \
    if(LITL_READ_GET_TYPE(evt) == LITL_TYPE_REGULAR)            \
      _ptr_ = &LITL_REGULAR(evt)->param[0];                     \
    else                                                        \
      _ptr_ = &LITL_PACKED(evt)->param[0];                      \
  } while(0)

/*
 * For internal use only
 * Returns the next parameter in an event
 */
#define __LITL_READ_GET_ARG(evt, _ptr_, arg)                    \
  do {                                                          \
    if(LITL_READ_GET_TYPE(evt) == LITL_TYPE_REGULAR)            \
      __LITL_READ_GET_ARG_REGULAR(_ptr_, arg);                  \
    else                                                        \
      __LITL_READ_GET_ARG_PACKED(_ptr_, arg);                   \
  } while(0)

/*
 * For internal use only
 * Returns the next parameter in a regular event
 */
#define __LITL_READ_GET_ARG_REGULAR(_ptr_, arg) do {	        \
    arg = *(litl_param_t*)_ptr_;		                        \
    (litl_param_t*)_ptr_++;			                            \
  } while(0)

/*
 * For internal use only
 * Returns the next parameter in a packed event
 */
#define __LITL_READ_GET_ARG_PACKED(_ptr_, arg) do {	            \
    memcpy(&arg, _ptr_, sizeof(arg));		                    \
    _ptr_ = ((char*)_ptr_)+sizeof(arg);		                    \
  } while(0)

/*** functions for reading events ***/

/**
 * \ingroup litl_read_process
 * \brief Returns a current event of a given thread
 * \param process An event reading object
 * \param thread_index An index of a given thread
 */
#define LITL_READ_GET_CUR_EVENT_PER_THREAD(process, thread_index) (&(process)->threads[(thread_index)]->cur_event)
/**
 * \ingroup litl_read_process
 * \brief Returns a current event of a given trace
 * \param process An event reading object
 */
#define LITL_READ_GET_CUR_EVENT(process) LITL_READ_GET_CUR_EVENT_PER_THREAD(process, (process)->cur_index)

/**
 * \ingroup litl_read_process
 * \brief Returns a thread id of a given event
 * \param read_event An event
 */
#define LITL_READ_GET_TID(read_event) (read_event)->tid
/**
 * \ingroup litl_read_process
 * \brief Returns a time stamp of a given event
 * \param read_event An event
 */
#define LITL_READ_GET_TIME(read_event) (read_event)->event->time
/**
 * \ingroup litl_read_process
 * \brief Returns a type of a given event
 * \param read_event An event
 */
#define LITL_READ_GET_TYPE(read_event) (read_event)->event->type
/**
 * \ingroup litl_read_process
 * \brief Returns a code of a given event
 * \param read_event An event
 */
#define LITL_READ_GET_CODE(read_event) (read_event)->event->code

/**
 * \ingroup litl_read_process
 * \brief Returns a size and parameters in the string format of a raw event
 * \param read_event An event
 */
#define LITL_READ_RAW(read_event) (&(read_event)->event->parameters.raw)
/**
 * \ingroup litl_read_process
 * \brief Returns a size and a list of parameters of a regular event
 * \param read_event An event
 */
#define LITL_READ_REGULAR(read_event) (&(read_event)->event->parameters.regular)
/**
 * \ingroup litl_read_process
 * \brief Returns a size and a list of parameters of a packed event
 * \param read_event An event
 */
#define LITL_READ_PACKED(read_event) (&(read_event)->event->parameters.packed)
/**
 * \ingroup litl_read_process
 * \brief Returns a size and an offset of an event of type offset
 * \param read_event An event
 */
#define LITL_READ_OFFSET(read_event) (&(read_event)->event->parameters.offset)

/**
 * \ingroup litl_read_process
 * \brief Assigns the first parameter of p_evt to param1
 * \param p_evt A pointer to an event
 * \param param1 1st parameter for this event
 */
#define litl_read_get_param_1(p_evt, param1)		            \
  do {						                                    \
    void* _ptr_;				                                \
    __LITL_READ_INIT_PTR(p_evt, _ptr_);			                \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param1);		            \
  } while(0)

/**
 * \ingroup litl_read_process
 * \brief Assigns the first 2 parameters of p_evt to param1 and param2
 * \param p_evt A pointer to an event
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 */
#define litl_read_get_param_2(p_evt, param1, param2)	        \
  do {						                                    \
    void* _ptr_;				                                \
    __LITL_READ_INIT_PTR(p_evt, _ptr_);			                \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param1);		            \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param2);		            \
  } while(0)

/**
 * \ingroup litl_read_process
 * \brief Assigns the first 3 parameters of p_evt to param1, ..., param3
 * \param p_evt A pointer to an event
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 */
#define litl_read_get_param_3(p_evt, param1, param2, param3)	\
  do {							                                \
    void* _ptr_;					                            \
    __LITL_READ_INIT_PTR(p_evt, _ptr_);				            \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param1);			        \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param2);			        \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param3);			        \
  } while(0)

/**
 * \ingroup litl_read_process
 * \brief Assigns the first 4 parameters of p_evt to param1, ..., param4
 * \param p_evt A pointer to an event
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 */
#define litl_read_get_param_4(p_evt, param1, param2, param3, param4)	\
  do {							                                \
    void* _ptr_;					                            \
    __LITL_READ_INIT_PTR(p_evt, _ptr_);				            \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param1);			        \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param2);			        \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param3);			        \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param4);			        \
  } while(0)

/**
 * \ingroup litl_read_process
 * \brief Assigns the first 5 parameters of p_evt to param1, ..., param5
 * \param p_evt A pointer to an event
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5th parameter for this event
 */
#define litl_read_get_param_5(p_evt, param1, param2, param3, param4, param5)	\
  do {								                            \
    void* _ptr_;						                        \
    __LITL_READ_INIT_PTR(p_evt, _ptr_);					        \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param1);				    \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param2);				    \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param3);				    \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param4);				    \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param5);				    \
  } while(0)

/**
 * \ingroup litl_read_process
 * \brief Assigns the first 6 parameters of p_evt to param1, ..., param6
 * \param p_evt A pointer to an event
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5th parameter for this event
 * \param param6 6th parameter for this event
 */
#define litl_read_get_param_6(p_evt, param1, param2, param3, param4, param5, param6)	\
  do {									                        \
    void* _ptr_;							                    \
    __LITL_READ_INIT_PTR(p_evt, _ptr_);						    \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param1);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param2);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param3);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param4);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param5);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param6);					\
  } while(0)

/**
 * \ingroup litl_read_process
 * \brief Assigns the first 7 parameters of p_evt to param1, ..., param7
 * \param p_evt A pointer to an event
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5th parameter for this event
 * \param param6 6th parameter for this event
 * \param param7 7th parameter for this event
 */
#define litl_read_get_param_7(p_evt, param1, param2, param3, param4, param5, param6, param7) \
  do {									                        \
    void* _ptr_;							                    \
    __LITL_READ_INIT_PTR(p_evt, _ptr_);						    \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param1);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param2);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param3);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param4);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param5);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param6);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param7);					\
  } while(0)

/**
 * \ingroup litl_read_process
 * \brief Assigns the first 8 parameters of p_evt to param1, ..., param8
 * \param p_evt A pointer to an event
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5th parameter for this event
 * \param param6 6th parameter for this event
 * \param param7 7th parameter for this event
 * \param param8 8th parameter for this event
 */
#define litl_read_get_param_8(p_evt, param1, param2, param3, param4, param5, param6, param7, param8) \
  do {									                        \
    void* _ptr_;							                    \
    __LITL_READ_INIT_PTR(p_evt, _ptr_);						    \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param1);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param2);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param3);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param4);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param5);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param6);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param7);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param8);					\
  } while(0)

/**
 * \ingroup litl_read_process
 * \brief Assigns the first 9 parameters of p_evt to param1, ..., param9
 * \param p_evt A pointer to an event
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5th parameter for this event
 * \param param6 6th parameter for this event
 * \param param7 7th parameter for this event
 * \param param8 8th parameter for this event
 * \param param9 9th parameter for this event
 */
#define litl_read_get_param_9(p_evt, param1, param2, param3, param4, param5, param6, param7, param8, param9) \
  do {									                        \
    void* _ptr_;							                    \
    __LITL_READ_INIT_PTR(p_evt, _ptr_);						    \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param1);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param2);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param3);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param4);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param5);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param6);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param7);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param8);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param9);					\
  } while(0)

/**
 * \ingroup litl_read_process
 * \brief Assigns the first 10 parameters of p_evt to param1, ..., param10
 * \param p_evt A pointer to an event
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5th parameter for this event
 * \param param6 6th parameter for this event
 * \param param7 7th parameter for this event
 * \param param8 8th parameter for this event
 * \param param9 9th parameter for this event
 * \param param10 10th parameter for this event
 */
#define litl_read_get_param_10(p_evt, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10) \
  do {									                        \
    void* _ptr_;							                    \
    __LITL_READ_INIT_PTR(p_evt, _ptr_);						    \
    __LITL_READ_GET_ARG(p_evt, _ptr_, param1);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param2);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param3);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param4);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param5);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param6);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param7);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param8);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param9);					\
    __LITL_READ_GET_ARG(p_evt, _ptr_, param10);				    \
  } while(0)

#endif /* LITL_READ_H_ */
