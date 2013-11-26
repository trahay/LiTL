/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

/**
 *  \file litl_write.h
 *  \brief litl_write Provides a set of functions for recording events in a
 *  trace file
 *
 *  \authors
 *    Developers are : \n
 *        Roman Iakymchuk   -- roman.iakymchuk@telecom-sudparis.eu \n
 *        Francois Trahay   -- francois.trahay@telecom-sudparis.eu \n
 */

#ifndef LITL_WRITE_H_
#define LITL_WRITE_H_

#include "litl_types.h"

/**
 * \defgroup litl_write LiTL Writing Functions
 */

/**
 * \defgroup litl_write_init Initialization Functions
 * \ingroup litl_write
 */

/**
 * \defgroup litl_write_reg Functions for Recording Regular Events
 * \ingroup litl_write
 */

/**
 * \defgroup litl_write_raw Functions for Recording Raw Events
 * \ingroup litl_write
 */

/**
 * \defgroup litl_write_pack Functions for Recording Packed Events
 * \ingroup litl_write
 */

/**
 * \ingroup litl_write_init
 * \brief Initializes the trace buffer
 * \param buf_size A buffer size (in Byte)
 * \return A pointer to the event recording object.
 *         NULL in case of failure
 */
litl_write_trace_t* litl_write_init_trace(const uint32_t buf_size);

/**
 * \ingroup litl_write_init
 * \brief Enable buffer flush. By default, it is disabled
 * \param trace A pointer to the event recording object
 */
void litl_write_buffer_flush_on(litl_write_trace_t* trace);

/**
 * \ingroup litl_write_init
 * \brief Disable buffer flush.
 * \param trace A pointer to the event recording object
 */
void litl_write_buffer_flush_off(litl_write_trace_t* trace);

/**
 * \ingroup litl_write_init
 * \brief Enable thread safety
 * \param trace A pointer to the event recording object
 */
void litl_write_thread_safety_on(litl_write_trace_t* trace);

/**
 * \ingroup litl_write_init
 * \brief Disable thread safety. By default, it is enabled
 * \param trace A pointer to the event recording object
 */
void litl_write_thread_safety_off(litl_write_trace_t* trace);

/**
 * \ingroup litl_write_init
 * \brief Enable recording tid
 */
void litl_write_tid_recording_on(litl_write_trace_t* trace);

/**
 * \ingroup litl_write_init
 * \brief Disable recording tid. By default, it is enabled
 * \param trace A pointer to the event recording object
 */
void litl_write_tid_recording_off(litl_write_trace_t* trace);

/**
 * \ingroup litl_write_init
 * \brief Pauses the event recording
 * \param trace A pointer to the event recording object
 */
void litl_write_pause_recording(litl_write_trace_t* trace);

/**
 * \ingroup litl_write_init
 * \brief Resumes the event recording
 * \param trace A pointer to the event recording object
 */
void litl_write_resume_recording(litl_write_trace_t* trace);

/**
 * \ingroup litl_write_init
 * \brief Sets a new name for the trace file
 * \param trace A pointer to the event recording object
 * \param filename A new file name
 */
void litl_write_set_filename(litl_write_trace_t* trace, char* filename);

/*** Regular events ***/

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event without parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \return a pointer to the event that was recorded or NULL in case of error
 */
litl_t* litl_write_probe_reg_0(litl_write_trace_t* trace, litl_code_t code);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 1 parameter
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param param1 1st parameter for this event
 * \return a pointer to the event that was recorded or NULL in case of error
 */
litl_t* litl_write_probe_reg_1(litl_write_trace_t* trace, litl_code_t code,
			       litl_param_t param1);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 2 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \return a pointer to the event that was recorded or NULL in case of error
 */
litl_t* litl_write_probe_reg_2(litl_write_trace_t* trace, litl_code_t code,
			       litl_param_t param1, litl_param_t param2);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 3 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \return a pointer to the event that was recorded or NULL in case of error
 */
litl_t* litl_write_probe_reg_3(litl_write_trace_t* trace, litl_code_t code,
			       litl_param_t param1, litl_param_t param2,
			       litl_param_t param3);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 4 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \return a pointer to the event that was recorded or NULL in case of error
 */
litl_t* litl_write_probe_reg_4(litl_write_trace_t* trace, litl_code_t code,
			       litl_param_t param1, litl_param_t param2,
			       litl_param_t param3, litl_param_t param4);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 5 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5th parameter for this event
 * \return a pointer to the event that was recorded or NULL in case of error
 */
litl_t* litl_write_probe_reg_5(litl_write_trace_t* trace, litl_code_t code,
			       litl_param_t param1, litl_param_t param2,
			       litl_param_t param3, litl_param_t param4,
			       litl_param_t param5);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 6 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5th parameter for this event
 * \param param6 6th parameter for this event
 * \return a pointer to the event that was recorded or NULL in case of error
 */
litl_t* litl_write_probe_reg_6(litl_write_trace_t* trace, litl_code_t code,
			       litl_param_t param1, litl_param_t param2,
			       litl_param_t param3, litl_param_t param4,
			       litl_param_t param5, litl_param_t param6);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 7 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5th parameter for this event
 * \param param6 6th parameter for this event
 * \param param7 7th parameter for this event
 * \return a pointer to the event that was recorded or NULL in case of error
 */
litl_t* litl_write_probe_reg_7(litl_write_trace_t* trace, litl_code_t code,
			       litl_param_t param1, litl_param_t param2,
			       litl_param_t param3, litl_param_t param4,
			       litl_param_t param5, litl_param_t param6,
			       litl_param_t param7);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 8 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5th parameter for this event
 * \param param6 6th parameter for this event
 * \param param7 7th parameter for this event
 * \param param8 8th parameter for this event
 * \return a pointer to the event that was recorded or NULL in case of error
 */
litl_t* litl_write_probe_reg_8(litl_write_trace_t* trace, litl_code_t code,
			       litl_param_t param1, litl_param_t param2,
			       litl_param_t param3, litl_param_t param4,
			       litl_param_t param5, litl_param_t param6,
			       litl_param_t param7, litl_param_t param8);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 9 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5th parameter for this event
 * \param param6 6th parameter for this event
 * \param param7 7th parameter for this event
 * \param param8 8th parameter for this event
 * \param param9 9th parameter for this event
 * \return a pointer to the event that was recorded or NULL in case of error
 */
litl_t* litl_write_probe_reg_9(litl_write_trace_t* trace, litl_code_t code,
			       litl_param_t param1, litl_param_t param2,
			       litl_param_t param3, litl_param_t param4,
			       litl_param_t param5, litl_param_t param6,
			       litl_param_t param7, litl_param_t param8,
			       litl_param_t param9);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 10 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
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
 * \return a pointer to the event that was recorded or NULL in case of error
 */
litl_t* litl_write_probe_reg_10(litl_write_trace_t* trace, litl_code_t code,
				litl_param_t param1, litl_param_t param2,
				litl_param_t param3, litl_param_t param4,
				litl_param_t param5, litl_param_t param6,
				litl_param_t param7, litl_param_t param8,
				litl_param_t param9, litl_param_t param10);

/*** Raw events ***/

/**
 * \ingroup litl_write_raw
 * \brief Records an event with data in a string format
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param size Size (in Bytes) of the data to store
 * \param data Data to store with this event
 * \return a pointer to the event that was recorded or NULL in case of error
 */
litl_t* litl_write_probe_raw(litl_write_trace_t* trace, litl_code_t code,
			     litl_size_t size, litl_data_t data[]);

/*** Internal-use macros ***/

/**
 * \ingroup litl_write_pack
 * \brief For internal use only. Allocates an event
 * \param trace A pointer to the event recording object
 * \param type An event type
 * \param code An event code
 * \param size Size of the event (in Bytes)
 * \return The allocated event or NULL in case of error
 */
litl_t* __litl_write_get_event(litl_write_trace_t* trace, litl_type_t type,
                               litl_code_t code, int size);

/**
 * \ingroup litl_write_pack
 * \brief For internal use only. Adds a parameter to a packed event
 * \param ptr A pointer to an event where the parameter should be stored
 * \param param A parameter to store
 */
#define __LITL_WRITE_ADD_ARG(ptr, param) do {	\
    typeof(param) _param = param;		\
    memcpy(ptr, &_param, sizeof(_param));	\
    ptr = ((char*) ptr)+sizeof(_param);		\
  } while(0)

/*** Packed events ***/

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event without parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
 */
#define litl_write_probe_pack_0(trace,					\
				code,					\
				retval) do {				\
    int total_size = LITL_BASE_SIZE;					\
    litl_t* p_evt = __litl_write_get_event(trace,			\
					   LITL_TYPE_PACKED,		\
					   code, total_size);		\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
    }									\
    retval = p_evt;							\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 1 parameter
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param param1 1st parameter for this event
 */
#define litl_write_probe_pack_1(trace,					\
				code,					\
				param1,					\
				retval)					\
  do {									\
    int total_size = LITL_BASE_SIZE + sizeof(param1);			\
    litl_t* p_evt = __litl_write_get_event(trace,			\
					   LITL_TYPE_PACKED,		\
					   code,			\
					   total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, param1);				\
    }									\
    retval = p_evt;							\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 2 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 */
#define litl_write_probe_pack_2(trace,					\
				code,					\
				param1,					\
				param2,					\
				retval)					\
  do {									\
    int total_size = LITL_BASE_SIZE + sizeof(param1) + sizeof(param2);	\
    litl_t* p_evt = __litl_write_get_event(trace,			\
					   LITL_TYPE_PACKED,		\
					   code,			\
					   total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, param1);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param2);				\
    }									\
    retval = p_evt;							\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 3 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 */
#define litl_write_probe_pack_3(trace,					\
				code,					\
				param1,					\
				param2,					\
				param3,					\
				retval) do {				\
    int total_size = LITL_BASE_SIZE + sizeof(param1) + sizeof(param2) +	\
      sizeof(param3);							\
    litl_t* p_evt = __litl_write_get_event(trace,			\
					   LITL_TYPE_PACKED,		\
					   code,			\
					   total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, param1);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param2);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param3);				\
    }									\
    retval = p_evt;							\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 4 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 */
#define litl_write_probe_pack_4(trace,					\
				code,					\
				param1,					\
				param2,					\
				param3,					\
				param4,					\
				retval) do {				\
    int total_size = LITL_BASE_SIZE + sizeof(param1) + sizeof(param2) + \
      sizeof(param3) + sizeof(param4);					\
    litl_t* p_evt = __litl_write_get_event(trace,			\
					   LITL_TYPE_PACKED,		\
					   code,			\
					   total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, param1);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param2);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param3);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param4);				\
    }									\
    retval = p_evt;							\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 5 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5th parameter for this event
 */
#define litl_write_probe_pack_5(trace,					\
				code,					\
				param1,					\
				param2,					\
				param3,					\
				param4,					\
				param5,					\
				retval) do {				\
    int total_size = LITL_BASE_SIZE + sizeof(param1) + sizeof(param2) + \
      sizeof(param3) + sizeof(param4) +sizeof(param5);			\
    litl_t* p_evt = __litl_write_get_event(trace,			\
					   LITL_TYPE_PACKED,		\
					   code,			\
					   total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, param1);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param2);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param3);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param4);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param5);				\
    }									\
    retval = p_evt;							\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 6 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5th parameter for this event
 * \param param6 6th parameter for this event
 */
#define litl_write_probe_pack_6(trace,					\
				code,					\
				param1,					\
				param2,					\
				param3,					\
				param4,					\
				param5,					\
				param6,					\
				retval) do {				\
    int total_size = LITL_BASE_SIZE + sizeof(param1) + sizeof(param2) + \
      sizeof(param3) + sizeof(param4) + sizeof(param5) + sizeof(param6); \
    litl_t* p_evt = __litl_write_get_event(trace,			\
					   LITL_TYPE_PACKED,		\
					   code,			\
					   total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, param1);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param2);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param3);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param4);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param5);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param6);				\
    }									\
    retval = p_evt;							\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 7 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5th parameter for this event
 * \param param6 6th parameter for this event
 * \param param7 7th parameter for this event
 */
#define litl_write_probe_pack_7(trace,					\
				code,					\
				param1,					\
				param2,					\
				param3,					\
				param4,					\
				param5,					\
				param6,					\
				param7,					\
				retval) do {				\
    int total_size = LITL_BASE_SIZE + sizeof(param1) + sizeof(param2) + \
      sizeof(param3) + sizeof(param4) + sizeof(param5) + sizeof(param6) \
      + sizeof(param7);							\
    litl_t* p_evt = __litl_write_get_event(trace,			\
					   LITL_TYPE_PACKED,		\
					   code,			\
					   total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, param1);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param2);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param3);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param4);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param5);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param6);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param7);				\
    }									\
    retval = p_evt;							\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 8 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
 * \param param1 1st parameter for this event
 * \param param2 2nd parameter for this event
 * \param param3 3rd parameter for this event
 * \param param4 4th parameter for this event
 * \param param5 5th parameter for this event
 * \param param6 6th parameter for this event
 * \param param7 7th parameter for this event
 * \param param8 8th parameter for this event
 */
#define litl_write_probe_pack_8(trace,					\
				code,					\
				param1,					\
				param2,					\
				param3,					\
				param4,					\
				param5,					\
				param6,					\
				param7,					\
				param8,					\
				retval) do {				\
    int total_size = LITL_BASE_SIZE + sizeof(param1) + sizeof(param2) + \
      sizeof(param3) + sizeof(param4) + sizeof(param5) + sizeof(param6) \
      + sizeof(param7) + sizeof(param8);				\
    litl_t* p_evt = __litl_write_get_event(trace,			\
					   LITL_TYPE_PACKED,		\
					   code,			\
					   total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, param1);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param2);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param3);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param4);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param5);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param6);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param7);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param8);				\
    }									\
    retval = p_evt;							\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 9 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
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
#define litl_write_probe_pack_9(trace,					\
				code,					\
				param1,					\
				param2,					\
				param3,					\
				param4,					\
				param5,					\
				param6,					\
				param7,					\
				param8,					\
				param9,					\
				retval) do {				\
    int total_size = LITL_BASE_SIZE + sizeof(param1) + sizeof(param2) + \
      sizeof(param3) + sizeof(param4)  + sizeof(param5) + sizeof(param6) \
      + sizeof(param7) + sizeof(param8) + sizeof(param9);		\
    litl_t* p_evt = __litl_write_get_event(trace,			\
					   LITL_TYPE_PACKED,		\
					   code,			\
					   total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, param1);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param2);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param3);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param4);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param5);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param6);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param7);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param8);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param9);				\
    }									\
    retval = p_evt;							\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 10 parameters
 * \param trace A pointer to the event recording object
 * \param code An event code
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
#define litl_write_probe_pack_10(trace,					\
				 code,					\
				 param1,				\
				 param2,				\
				 param3,				\
				 param4,				\
				 param5,				\
				 param6,				\
				 param7,				\
				 param8,				\
				 param9,				\
				 param10,				\
				 retval) do {				\
    int total_size = LITL_BASE_SIZE + sizeof(param1) + sizeof(param2) + \
      sizeof(param3) + sizeof(param4) + sizeof(param5) + sizeof(param6) + \
      sizeof(param7) + sizeof(param8) + sizeof(param9) + sizeof(param10); \
    litl_t* p_evt = __litl_write_get_event(trace,			\
					   LITL_TYPE_PACKED,		\
					   code,			\
					   total_size);			\
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
      void* _ptr_ = &p_evt->parameters.packed.param[0];			\
      __LITL_WRITE_ADD_ARG(_ptr_, param1);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param2);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param3);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param4);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param5);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param6);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param7);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param8);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param9);				\
      __LITL_WRITE_ADD_ARG(_ptr_, param10);				\
    }									\
    retval = p_evt;							\
  } while(0)

/**
 * \ingroup litl_write_init
 * \brief Finalizes the trace
 * \param trace A pointer to the event recording object
 */
void litl_write_finalize_trace(litl_write_trace_t* trace);

#endif /* LITL_WRITE_H_ */
