/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

/**
 *  \file litl_write.h
 *  \brief
 *  litl_write provides a set of functions for recording events in a trace file.
 *
 *  \authors
 *    Developers are : \n
 *        Roman Iakymchuk   - roman.iakymchuk@telecom-sudparis.eu \n
 *        Francois Trahay   - francois.trahay@telecom-sudparis.eu \n
 */

#ifndef LITL_WRITE_H_
#define LITL_WRITE_H_

#include "litl_types.h"

/**
 * \defgroup litl_write Writing functions
 */

/**
 * \defgroup litl_write_init Initialization functions
 * \ingroup litl_write
 */

/**
 * \defgroup litl_write_reg Functions for recording regular events
 * \ingroup litl_write
 */

/**
 * \defgroup litl_write_raw Functions for recording raw events
 * \ingroup litl_write
 */

/**
 * \defgroup litl_write_pack Functions for recording packed events
 * \ingroup litl_write
 */

/**
 * \ingroup litl_write_init
 * \brief Initializes the trace buffer
 * \param buf_size Buffer size (in Byte)
 * \return A pointer to the event recording object
 *         NULL in case of failure.
 */
litl_trace_write_t* litl_write_init_trace(const uint32_t buf_size);

/**
 * \ingroup litl_write_init
 * \brief Activates buffer flush
 * \param trace A pointer to the event recording object
 */
void litl_write_buffer_flush_on(litl_trace_write_t* trace);

/**
 * \ingroup litl_write_init
 * \brief Deactivates buffer flush. By default, it is activated
 * \param trace A pointer to the event recording object
 */
void litl_write_buffer_flush_off(litl_trace_write_t* trace);

/**
 * \ingroup litl_write_init
 * \brief Activates thread safety
 * \param trace A pointer to the event recording object
 */
void litl_write_thread_safety_on(litl_trace_write_t* trace);

/**
 * \ingroup litl_write_init
 * \brief Deactivates thread safety
 * \param trace A pointer to the event recording object
 */
void litl_write_thread_safety_off(litl_trace_write_t* trace);

/**
 * \ingroup litl_write_init
 * \brief Activates recording tid
 */
void litl_write_tid_recording_on(litl_trace_write_t* trace);

/**
 * \ingroup litl_write_init
 * \brief Deactivates recording tid
 * \param trace A pointer to the event recording object
 */
void litl_write_tid_recording_off(litl_trace_write_t* trace);

/**
 * \ingroup litl_write_init
 * \brief Pauses the event recording
 * \param trace A pointer to the event recording object
 */
void litl_write_pause_recording(litl_trace_write_t* trace);

/**
 * \ingroup litl_write_init
 * \brief Resumes the event recording
 * \param trace A pointer to the event recording object
 */
void litl_write_resume_recording(litl_trace_write_t* trace);

/**
 * \ingroup litl_write_init
 * \brief Sets a new name for the trace file
 * \param trace A pointer to the event recording object
 * \param filename New file name
 */
void litl_write_set_filename(litl_trace_write_t* trace, char* filename);

/*** Regular events ***/

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event without parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 */
void litl_write_probe_reg_0(litl_trace_write_t* trace, litl_code_t code);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 1 parameter
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param param1 1st parameter for this event
 */
void litl_write_probe_reg_1(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 2 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 */
void litl_write_probe_reg_2(litl_trace_write_t*, litl_code_t code,
        litl_param_t param1, litl_param_t param2);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 3 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 */
void litl_write_probe_reg_3(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1, litl_param_t param2, litl_param_t param3);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 4 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 */
void litl_write_probe_reg_4(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1, litl_param_t param2, litl_param_t param3,
        litl_param_t param4);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 5 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5h parameter for this event
 */
void litl_write_probe_reg_5(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1, litl_param_t param2, litl_param_t param3,
        litl_param_t param4, litl_param_t param5);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 6 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5h parameter for this event
 * \param param6 6th parameter for this event
 */
void litl_write_probe_reg_6(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1, litl_param_t param2, litl_param_t param3,
        litl_param_t param4, litl_param_t param5, litl_param_t param6);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 7 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5h parameter for this event
 * \param param6 6th parameter for this event
 * \param param7 7th parameter for this event
 */
void litl_write_probe_reg_7(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1, litl_param_t param2, litl_param_t param3,
        litl_param_t param4, litl_param_t param5, litl_param_t param6,
        litl_param_t param7);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 8 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5h parameter for this event
 * \param param6 6th parameter for this event
 * \param param7 7th parameter for this event
 * \param param8 8th parameter for this event
 */
void litl_write_probe_reg_8(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1, litl_param_t param2, litl_param_t param3,
        litl_param_t param4, litl_param_t param5, litl_param_t param6,
        litl_param_t param7, litl_param_t param8);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 9 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5h parameter for this event
 * \param param6 6th parameter for this event
 * \param param7 7th parameter for this event
 * \param param8 8th parameter for this event
 * \param param9 9th parameter for this event
 */
void litl_write_probe_reg_9(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1, litl_param_t param2, litl_param_t param3,
        litl_param_t param4, litl_param_t param5, litl_param_t param6,
        litl_param_t param7, litl_param_t param8, litl_param_t param9);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 10 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5h parameter for this event
 * \param param6 6th parameter for this event
 * \param param7 7th parameter for this event
 * \param param8 8th parameter for this event
 * \param param9 9th parameter for this event
 * \param param10 10th parameter for this event
 */
void litl_write_probe_reg_10(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1, litl_param_t param2, litl_param_t param3,
        litl_param_t param4, litl_param_t param5, litl_param_t param6,
        litl_param_t param7, litl_param_t param8, litl_param_t param9,
        litl_param_t param10);

/*** Raw events ***/

/**
 * \ingroup litl_write_raw
 * \brief Records an event with additional data.
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param size Size (in Bytes) of the data to store
 * \param data Data to store with this event
 */
void litl_write_probe_raw(litl_trace_write_t* trace, litl_code_t code,
        litl_size_t size, litl_data_t data[]);

/*** Internal-use macros ***/

/**
 * For internal use only.
 * Allocates an event
 * \param trace A pointer to the event recording object
 * \param type Event type
 * \param code Event code
 * \param size Size of the event (in Bytes)
 * \return The allocated event or NULL in case of error.
 */
litl_t* __litl_write_get_event(litl_trace_write_t* trace, litl_type_t type,
        litl_code_t code, int size);

/**
 * For internal use only.
 * Adds a parameter in an event
 * \param _ptr_ Address where the parameter should be stored
 * \param arg Parameter to store
 */
#define __LITL_WRITE_ADD_ARG(_ptr_, arg) do {		\
    typeof(arg) _arg = arg;			\
    memcpy(_ptr_, &_arg, sizeof(_arg));		\
    _ptr_ = ((char*)_ptr_)+sizeof(_arg);	\
  } while(0)

/*** Packed events ***/

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event without parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 */
#define litl_write_probe_pack_0(trace, code) do {				\
    int total_size = LITL_BASE_SIZE;					\
    litl_t* p_evt = __litl_write_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
    }									\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 1 parameter
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param arg1 1st parameter for this event
 */
#define litl_write_probe_pack_1(trace, code, arg1) do {			\
    int total_size = LITL_BASE_SIZE + sizeof(arg1);			\
    litl_t* p_evt = __litl_write_get_event(LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, arg1);					\
    }									\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 2 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param arg1 1st parameter for this event
 * \param arg2 2nd parameter for this event
 */
#define litl_write_probe_pack_2(trace, code, arg1, arg2) do {			\
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2);	\
    litl_t* p_evt = __litl_write_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, arg1);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg2);					\
    }									\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 3 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param arg1 1st parameter for this event
 * \param arg2 2nd parameter for this event
 * \param arg3 3rd parameter for this event
 */
#define litl_write_probe_pack_3(trace, code, arg1, arg2, arg3) do {		\
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3); \
    litl_t* p_evt = __litl_write_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, arg1);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg2);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg3);					\
    }									\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 4 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param arg1 1st parameter for this event
 * \param arg2 2nd parameter for this event
 * \param arg3 3rd parameter for this event
 * \param arg4 4th parameter for this event
 */
#define litl_write_probe_pack_4(trace, code, arg1, arg2, arg3, arg4) do {	\
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    litl_t* p_evt = __litl_write_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, arg1);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg2);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg3);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg4);					\
    }									\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 5 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param arg1 1st parameter for this event
 * \param arg2 2nd parameter for this event
 * \param arg3 3rd parameter for this event
 * \param arg4 4th parameter for this event
 * \param arg5 5h parameter for this event
 */
#define litl_write_probe_pack_5(trace, code, arg1, arg2, arg3, arg4, arg5) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5);						\
    litl_t* p_evt = __litl_write_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, arg1);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg2);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg3);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg4);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg5);					\
    }									\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 6 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param arg1 1st parameter for this event
 * \param arg2 2nd parameter for this event
 * \param arg3 3rd parameter for this event
 * \param arg4 4th parameter for this event
 * \param arg5 5h parameter for this event
 * \param arg6 6th parameter for this event
 */
#define litl_write_probe_pack_6(trace, code, arg1, arg2, arg3, arg4, arg5, arg6) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6);				\
    litl_t* p_evt = __litl_write_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, arg1);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg2);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg3);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg4);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg5);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg6);					\
    }									\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 7 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param arg1 1st parameter for this event
 * \param arg2 2nd parameter for this event
 * \param arg3 3rd parameter for this event
 * \param arg4 4th parameter for this event
 * \param arg5 5h parameter for this event
 * \param arg6 6th parameter for this event
 * \param arg7 7th parameter for this event
 */
#define litl_write_probe_pack_7(trace, code, arg1, arg2, arg3, arg4, arg5, arg6, arg7) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7);		\
    litl_t* p_evt = __litl_write_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, arg1);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg2);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg3);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg4);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg5);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg6);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg7);					\
    }									\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 8 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param arg1 1st parameter for this event
 * \param arg2 2nd parameter for this event
 * \param arg3 3rd parameter for this event
 * \param arg4 4th parameter for this event
 * \param arg5 5h parameter for this event
 * \param arg6 6th parameter for this event
 * \param arg7 7th parameter for this event
 * \param arg8 8th parameter for this event
 */
#define litl_write_probe_pack_8(trace, code, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7) + sizeof(arg8); \
    litl_t* p_evt = __litl_write_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, arg1);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg2);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg3);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg4);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg5);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg6);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg7);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg8);					\
    }									\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 9 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param arg1 1st parameter for this event
 * \param arg2 2nd parameter for this event
 * \param arg3 3rd parameter for this event
 * \param arg4 4th parameter for this event
 * \param arg5 5h parameter for this event
 * \param arg6 6th parameter for this event
 * \param arg7 7th parameter for this event
 * \param arg8 8th parameter for this event
 * \param arg9 9th parameter for this event
 */
#define litl_write_probe_pack_9(trace, code, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7) + sizeof(arg8) + sizeof(arg9); \
    litl_t* p_evt = __litl_write_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, arg1);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg2);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg3);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg4);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg5);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg6);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg7);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg8);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg9);					\
    }									\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 10 parameters
 * \param trace A pointer to the event recording object
 * \param code Event code
 * \param arg1 1st parameter for this event
 * \param arg2 2nd parameter for this event
 * \param arg3 3rd parameter for this event
 * \param arg4 4th parameter for this event
 * \param arg5 5h parameter for this event
 * \param arg6 6th parameter for this event
 * \param arg7 7th parameter for this event
 * \param arg8 8th parameter for this event
 * \param arg9 9th parameter for this event
 * \param arg10 10th parameter for this event
 */
#define litl_write_probe_pack_10(trace, code, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7) + sizeof(arg8) + sizeof(arg9) + sizeof(arg10); \
    litl_t* p_evt = __litl_write_get_event(trace, LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, arg1);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg2);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg3);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg4);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg5);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg6);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg7);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg8);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg9);					\
      __LITL_WRITE_ADD_ARG(_ptr_, arg10);					\
    }									\
  } while(0)

/**
 * \ingroup litl_write_init
 * \brief Finalizes the trace
 * \param trace A pointer to the event recording object
 */
void litl_write_finalize_trace(litl_trace_write_t* trace);

#endif /* LITL_WRITE_H_ */
