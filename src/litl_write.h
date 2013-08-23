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

static litl_trace_write_t* __trace __attribute__ ((__unused__));

/*
 * Initializes the trace buffer
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
 * \return A pointer to the trace.
 *         NULL in case of failure.
 */
void litl_write_init_trace(const uint32_t buf_size);

/**
 * \ingroup litl_write_init
 * \brief Activates buffer flush
 */
void litl_write_buffer_flush_on();

/**
 * \ingroup litl_write_init
 * \brief Deactivates buffer flush. By default, it is activated
 */
void litl_write_buffer_flush_off();

/**
 * \ingroup litl_write_init
 * \brief Activates thread safety
 */
void litl_write_thread_safety_on();

/**
 * \ingroup litl_write_init
 * \brief Deactivates thread safety
 */
void litl_write_thread_safety_off();

/**
 * \ingroup litl_write_init
 * \brief Activates recording tid
 */
void litl_write_tid_recording_on();

/**
 * \ingroup litl_write_init
 * \brief Deactivates recording tid
 */
void litl_write_tid_recording_off();

/**
 * \ingroup litl_write_init
 * \brief Pauses the event recording
 */
void litl_write_pause_recording();

/**
 * \ingroup litl_write_init
 * \brief Resumes the event recording
 */
void litl_write_resume_recording();

/**
 * \ingroup litl_write_init
 * \brief Sets a new name for the trace file
 */
void litl_write_set_filename(char* filename);

/*** Regular events ***/

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event without parameters
 */
void litl_write_probe_reg_0(litl_code_t code);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 1 parameter
 */
void litl_write_probe_reg_1(litl_code_t code, litl_param_t param1);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 2 parameters
 */
void litl_write_probe_reg_2(litl_code_t code, litl_param_t param1,
        litl_param_t param2);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 3 parameters
 */
void litl_write_probe_reg_3(litl_code_t code, litl_param_t param1,
        litl_param_t param2, litl_param_t param3);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 4 parameters
 */
void litl_write_probe_reg_4(litl_code_t code, litl_param_t param1,
        litl_param_t param2, litl_param_t param3, litl_param_t param4);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 5 parameters
 */
void litl_write_probe_reg_5(litl_code_t code, litl_param_t param1,
        litl_param_t param2, litl_param_t param3, litl_param_t param4,
        litl_param_t param5);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 6 parameters
 */
void litl_write_probe_reg_6(litl_code_t code, litl_param_t param1,
        litl_param_t param2, litl_param_t param3, litl_param_t param4,
        litl_param_t param5, litl_param_t param6);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 7 parameters
 */
void litl_write_probe_reg_7(litl_code_t code, litl_param_t param1,
        litl_param_t param2, litl_param_t param3, litl_param_t param4,
        litl_param_t param5, litl_param_t param6, litl_param_t param7);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 8 parameters
 */
void litl_write_probe_reg_8(litl_code_t code, litl_param_t param1,
        litl_param_t param2, litl_param_t param3, litl_param_t param4,
        litl_param_t param5, litl_param_t param6, litl_param_t param7,
        litl_param_t param8);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 9 parameters
 */
void litl_write_probe_reg_9(litl_code_t code, litl_param_t param1,
        litl_param_t param2, litl_param_t param3, litl_param_t param4,
        litl_param_t param5, litl_param_t param6, litl_param_t param7,
        litl_param_t param8, litl_param_t param9);

/**
 * \ingroup litl_write_reg
 * \brief Records a regular event with 10 parameters
 */
void litl_write_probe_reg_10(litl_code_t code, litl_param_t param1,
        litl_param_t param2, litl_param_t param3, litl_param_t param4,
        litl_param_t param5, litl_param_t param6, litl_param_t param7,
        litl_param_t param8, litl_param_t param9, litl_param_t param10);

/*** Raw events ***/

/**
 * \ingroup litl_write_raw
 * \brief Records an event in a raw state, where the size is #args in the void* array.
 * That helps to discover places where the application has crashed
 */
void litl_write_probe_raw(litl_code_t code, litl_size_t size,
        litl_data_t data[]);

/*** Internal-use macros ***/

/**
 * For internal use only.
 * Allocates an event
 */
litl_t* __litl_write_get_event(litl_type_t type, litl_code_t code, int size);

/**
 * For internal use only.
 * Adds a parameter in an event
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
 */
#define litl_write_probe_pack_0(code) do {				\
    int total_size = LITL_BASE_SIZE;					\
    litl_t* p_evt = __litl_write_get_event(LITL_TYPE_PACKED, code, total_size); \
    if(p_evt){								\
      p_evt->parameters.packed.size = total_size - LITL_BASE_SIZE;	\
    }									\
  } while(0)

/**
 * \ingroup litl_write_pack
 * \brief Records a packed event with 1 parameter
 */
#define litl_write_probe_pack_1(code, arg1) do {			\
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
 */
#define litl_write_probe_pack_2(code, arg1, arg2) do {			\
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2);	\
    litl_t* p_evt = __litl_write_get_event(LITL_TYPE_PACKED, code, total_size); \
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
 */
#define litl_write_probe_pack_3(code, arg1, arg2, arg3) do {		\
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3); \
    litl_t* p_evt = __litl_write_get_event(LITL_TYPE_PACKED, code, total_size); \
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
 */
#define litl_write_probe_pack_4(code, arg1, arg2, arg3, arg4) do {	\
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    litl_t* p_evt = __litl_write_get_event(LITL_TYPE_PACKED, code, total_size); \
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
 */
#define litl_write_probe_pack_5(code, arg1, arg2, arg3, arg4, arg5) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5);						\
    litl_t* p_evt = __litl_write_get_event(LITL_TYPE_PACKED, code, total_size); \
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
 */
#define litl_write_probe_pack_6(code, arg1, arg2, arg3, arg4, arg5, arg6) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6);				\
    litl_t* p_evt = __litl_write_get_event(LITL_TYPE_PACKED, code, total_size); \
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
 */
#define litl_write_probe_pack_7(code, arg1, arg2, arg3, arg4, arg5, arg6, arg7) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7);		\
    litl_t* p_evt = __litl_write_get_event(LITL_TYPE_PACKED, code, total_size); \
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
 */
#define litl_write_probe_pack_8(code, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7) + sizeof(arg8); \
    litl_t* p_evt = __litl_write_get_event(LITL_TYPE_PACKED, code, total_size); \
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
 */
#define litl_write_probe_pack_9(code, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7) + sizeof(arg8) + sizeof(arg9); \
    litl_t* p_evt = __litl_write_get_event(LITL_TYPE_PACKED, code, total_size); \
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
 */
#define litl_write_probe_pack_10(code, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) do { \
    int total_size = LITL_BASE_SIZE + sizeof(arg1) + sizeof(arg2) + sizeof(arg3) + sizeof(arg4); \
    total_size += sizeof(arg5) + sizeof(arg6) + sizeof(arg7) + sizeof(arg8) + sizeof(arg9) + sizeof(arg10); \
    litl_t* p_evt = __litl_write_get_event(LITL_TYPE_PACKED, code, total_size); \
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
 */
void litl_write_finalize_trace();

#endif /* LITL_WRITE_H_ */
