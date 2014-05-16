/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <unistd.h>

#include "litl_timer.h"
#include "litl_tools.h"
#include "litl_write.h"

/*
 * Adds a header to the trace file with the information regarding:
 *   - OS
 *   - Processor type
 *   - Version of LiTL
 */
static void __litl_write_add_trace_header(litl_write_trace_t* trace) {
  struct utsname uts;

  // allocate memory for the trace header
  trace->header_ptr = (litl_buffer_t) malloc(trace->header_size);
  if (!trace->header_ptr) {
    perror("Could not allocate memory for the trace header!");
    exit(EXIT_FAILURE);
  }
  trace->header = trace->header_ptr;

  if (uname(&uts) < 0)
    perror("Could not use uname()!");

  // add a general header
  // version of LiTL
  sprintf((char*) ((litl_general_header_t *) trace->header)->litl_ver, "%s",
          VERSION);
  // system information
  sprintf((char*) ((litl_general_header_t *) trace->header)->sysinfo,
          "%s %s %s %s %s", uts.sysname, uts.nodename, uts.release, uts.version,
          uts.machine);
  // a number of processes
  ((litl_general_header_t *) trace->header)->nb_processes = 1;
  // move pointer
  trace->header += sizeof(litl_general_header_t);

  // add a process-specific header
  // by default one trace file contains events only of one process
  char* filename = strrchr(trace->filename, '/');
  filename++;
  sprintf((char*) ((litl_process_header_t *) trace->header)->process_name, "%s",
          filename);
  ((litl_process_header_t *) trace->header)->nb_threads = trace->nb_threads;
  ((litl_process_header_t *) trace->header)->header_nb_threads =
    trace->nb_threads;
  ((litl_process_header_t *) trace->header)->buffer_size = trace->buffer_size;
  ((litl_process_header_t *) trace->header)->trace_size = 0;
  ((litl_process_header_t *) trace->header)->offset =
    sizeof(litl_general_header_t) + sizeof(litl_process_header_t);

  // header_size stores the position of nb_threads in the trace file
  trace->header_size = sizeof(litl_general_header_t)
    + 256 * sizeof(litl_data_t);
  // move pointer
  trace->header += sizeof(litl_process_header_t);
}

/*
 * To create trace->buffer_ptr and trace->buffer
 */
static void __litl_write_init_var(litl_write_trace_t* trace) {
  pthread_key_create(&trace->index, NULL );
}

/*
 * Initializes the trace buffer
 */
litl_write_trace_t* litl_write_init_trace(const litl_size_t buf_size) {
  litl_med_size_t i;
  litl_write_trace_t* trace;

  trace = (litl_write_trace_t*) malloc(sizeof(litl_write_trace_t));
  if (!trace) {
    perror("Could not allocate memory for the trace!");
    exit(EXIT_FAILURE);
  }

  // set variables
  trace->filename = NULL;
  trace->general_offset = 0;
  trace->is_header_flushed = 0;

  // set the buffer size using the environment variable.
  //   If the variable is not specified, use the provided value
  char* str = getenv("LITL_BUFFER_SIZE");
  if (str != NULL )
    trace->buffer_size = atoi(str);
  else
    trace->buffer_size = buf_size;

  trace->is_buffer_full = 0;
  trace->nb_allocated_buffers = 256;
  trace->buffers = malloc(
      sizeof(litl_write_buffer_t*) * trace->nb_allocated_buffers);
  if (!trace->buffers) {
    perror("Could not allocate memory for the threads!");
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < trace->nb_allocated_buffers; i++) {
    // initialize the array already_flushed
    trace->buffers[i] = malloc(sizeof(litl_write_buffer_t));
    if (!trace->buffers[i]) {
      perror("Could not allocate memory for a thread\n");
      exit(EXIT_FAILURE);
    }
    trace->buffers[i]->already_flushed = 0;

    // initialize tids by zeros; this is needed for __is_tid and __find_slot
    trace->buffers[i]->tid = 0;
  }
  trace->nb_threads = 0;

  // initialize the timing mechanism
  litl_time_initialize();

  // a jump function is needed 'cause it is not possible to pass args to
  //   the calling function through pthread_once
  void __init() {
    __litl_write_init_var(trace);
  }
  trace->index_once = (pthread_once_t) PTHREAD_ONCE_INIT;
  pthread_once(&trace->index_once, __init);

  // set trace->allow_buffer_flush using the environment variable.
  //   By default the buffer flushing is disabled
  litl_write_buffer_flush_off(trace);
  str = getenv("LITL_BUFFER_FLUSH");
  if (str) {
    if(strcmp(str, "0") == 0)
      litl_write_buffer_flush_off(trace);
    else
      litl_write_buffer_flush_on(trace);
  }

  // set trace->allow_thread_safety using the environment variable.
  //   By default thread safety is enabled
  litl_write_thread_safety_on(trace);
  str = getenv("LITL_THREAD_SAFETY");
  if (str && (strcmp(str, "0") == 0))
    litl_write_thread_safety_off(trace);

  if (trace->allow_thread_safety)
    pthread_mutex_init(&trace->lock_litl_flush, NULL );
  pthread_mutex_init(&trace->lock_buffer_init, NULL );

  // set trace->allow_tid_recording using the environment variable.
  //   By default tid recording is enabled
  litl_write_tid_recording_on(trace);
  str = getenv("LITL_TID_RECORDING");
  if (str && (strcmp(str, "0") == 0))
    litl_write_tid_recording_off(trace);

  trace->is_recording_paused = 0;
  trace->is_litl_initialized = 1;

  return trace;
}

/*
 * Computes the size of data in the trace header
 */
static litl_size_t __litl_write_get_header_size(litl_write_trace_t* trace) {
  return (trace->header - trace->header_ptr);
}

/*
 * Computes the size of data in buffer
 */
static litl_size_t __litl_write_get_buffer_size(litl_write_trace_t* trace,
                                                litl_med_size_t pos) {
  return (trace->buffers[pos]->buffer - trace->buffers[pos]->buffer_ptr);
}

/*
 * Activates buffer flush
 */
void litl_write_buffer_flush_on(litl_write_trace_t* trace) {
  trace->allow_buffer_flush = 1;
}

/*
 * Deactivates buffer flush. By default, it is activated
 */
void litl_write_buffer_flush_off(litl_write_trace_t* trace) {
  trace->allow_buffer_flush = 0;
}

/*
 * Activate thread safety. By default it is deactivated
 */
void litl_write_thread_safety_on(litl_write_trace_t* trace) {
  trace->allow_thread_safety = 1;
}

/*
 * Deactivates thread safety
 */
void litl_write_thread_safety_off(litl_write_trace_t* trace) {
  trace->allow_thread_safety = 0;
}

/*
 * Activates recording tid. By default it is deactivated
 */
void litl_write_tid_recording_on(litl_write_trace_t* trace) {
  trace->allow_tid_recording = 1;
}

/*
 * Deactivates recording tid
 */
void litl_write_tid_recording_off(litl_write_trace_t* trace) {
  trace->allow_tid_recording = 0;
}

/*
 * Pauses the event recording
 */
void litl_write_pause_recording(litl_write_trace_t* trace) {
  if (trace)
    trace->is_recording_paused = 1;
}

/*
 * Resumes the event recording
 */
void litl_write_resume_recording(litl_write_trace_t* trace) {
  if (trace)
    trace->is_recording_paused = 0;
}

/*
 * Sets a new name for the trace file
 */
void litl_write_set_filename(litl_write_trace_t* trace, char* filename) {
  if (trace->filename) {
    if (trace->is_header_flushed)
      fprintf(
          stderr,
          "Warning: changing the trace file name to %s after some events have been saved in file %s\n",
          filename, trace->filename);
    free(trace->filename);
  }

  // check whether the file name was set. If no, set it by default trace name.
  if (filename == NULL )
    sprintf(filename, "/tmp/%s_%s", getenv("USER"), "litl_log_1");

  if (asprintf(&trace->filename, "%s", filename) == -1) {
    perror("Error: Cannot set the filename for recording events!\n");
    exit(EXIT_FAILURE);
  }
}

/*
 * Records an event with offset only
 */
static void __litl_write_probe_offset(litl_write_trace_t* trace,
                                      litl_med_size_t index) {
  if (!trace->is_litl_initialized || trace->is_recording_paused
    || trace->is_buffer_full)
    return;
  // litl_t* cur_ptr = litl_cmpxchg((uint8_t**) &trace->buffer[index],
  //    LITL_BASE_SIZE + sizeof(litl_param_t));
  litl_t* cur_ptr = (litl_t *) trace->buffers[index]->buffer;

  cur_ptr->time = 0;
  cur_ptr->code = LITL_OFFSET_CODE;
  cur_ptr->type = LITL_TYPE_REGULAR;
  cur_ptr->parameters.offset.nb_params = 1;
  cur_ptr->parameters.offset.offset = 0;
  trace->buffers[index]->buffer += LITL_BASE_SIZE + sizeof(litl_param_t);
}

/*
 * Writes the recorded events from the buffer to the trace file
 */
static void __litl_write_flush_buffer(litl_write_trace_t* trace,
                                      litl_med_size_t index) {
  int res __attribute__ ((__unused__));
  litl_offset_t offset, header_size;

  if (!trace->is_litl_initialized)
    return;

  if (trace->allow_thread_safety)
    pthread_mutex_lock(&trace->lock_litl_flush);

  if (!trace->is_header_flushed) {
    // check whether the trace file can be opened
    if ((trace->f_handle = open(trace->filename, O_WRONLY | O_CREAT, 0644))
      < 0) {
      fprintf(stderr, "Cannot open %s\n", trace->filename);
      exit(EXIT_FAILURE);
    }

    // add a header to the trace file
    trace->header_size = sizeof(litl_general_header_t)
      + sizeof(litl_process_header_t)
      + (trace->nb_threads + 1) * sizeof(litl_thread_pair_t);
    __litl_write_add_trace_header(trace);

    // add information about each working thread: (tid, offset)
    litl_med_size_t i;
    for (i = 0; i < trace->nb_threads; i++) {
      ((litl_thread_pair_t *) trace->header)->tid = trace->buffers[i]->tid;
      ((litl_thread_pair_t *) trace->header)->offset = 0;

      trace->header += sizeof(litl_thread_pair_t);

      // save the position of offset inside the trace file
      trace->buffers[i]->offset = __litl_write_get_header_size(trace)
        - sizeof(litl_offset_t);
      trace->buffers[i]->already_flushed = 1;
    }

    // offset indicates the position of offset to the next slot of
    //   pairs (tid, offset) within the trace file
    trace->header_offset = __litl_write_get_header_size(trace);

    // specify the last slot of pairs (offset == 0)
    ((litl_thread_pair_t *) trace->header)->tid = 0;
    ((litl_thread_pair_t *) trace->header)->offset = 0;
    trace->header += sizeof(litl_thread_pair_t);

    // write the trace header to the trace file
    if (write(trace->f_handle, trace->header_ptr,
              __litl_write_get_header_size(trace)) == -1) {
      perror(
          "Flushing the buffer. Could not write measured data to the trace file!");
      exit(EXIT_FAILURE);
    }

    trace->general_offset = __litl_write_get_header_size(trace);

    trace->header_nb_threads = trace->nb_threads;
    trace->threads_offset = 0;
    trace->nb_slots = 0;

    trace->is_header_flushed = 1;
  }

  header_size = sizeof(litl_general_header_t) + sizeof(litl_process_header_t);
  // handle the situation when some threads start after the header was flushed
  if (!trace->buffers[index]->already_flushed) {

    // when more buffers to store threads information is required
    if (trace->nb_threads
      > (trace->header_nb_threads + NBTHREADS * trace->nb_slots)) {

      // updated the offset from the previous slot
      lseek(trace->f_handle, trace->header_offset + sizeof(litl_tid_t),
            SEEK_SET);
      offset = trace->general_offset - header_size;
      res = write(trace->f_handle, &offset, sizeof(litl_offset_t));

      // reserve a new slot for pairs (tid, offset)
      trace->header_offset = trace->general_offset;
      trace->threads_offset = trace->header_offset;
      trace->general_offset += (NBTHREADS + 1) * sizeof(litl_thread_pair_t);

      trace->nb_slots++;
    }

    // add a new pair (tid, offset)
    lseek(trace->f_handle, trace->header_offset, SEEK_SET);
    res = write(trace->f_handle, &trace->buffers[index]->tid,
                sizeof(litl_tid_t));
    offset = trace->general_offset - header_size;
    res = write(trace->f_handle, &offset, sizeof(litl_offset_t));

    // add an indicator to specify the last slot of pairs (offset == 0)
    // TODO: how to optimize this and write only once at the end of the slot
    offset = 0;
    res = write(trace->f_handle, &offset, sizeof(litl_tid_t));
    res = write(trace->f_handle, &offset, sizeof(litl_offset_t));

    trace->header_offset += sizeof(litl_thread_pair_t);
    trace->buffers[index]->already_flushed = 1;

    // updated the number of threads
    // TODO: perform update only once 'cause there is duplication
    lseek(trace->f_handle, trace->header_size, SEEK_SET);
    res = write(trace->f_handle, &trace->nb_threads, sizeof(litl_med_size_t));
    lseek(trace->f_handle, trace->general_offset, SEEK_SET);
  } else {
    // update the previous offset of the current thread,
    //   updating the location in the file
    lseek(trace->f_handle, trace->buffers[index]->offset, SEEK_SET);
    offset = trace->general_offset - header_size;
    res = write(trace->f_handle, &offset, sizeof(litl_offset_t));
    lseek(trace->f_handle, trace->general_offset, SEEK_SET);
  }

  // add an event with offset
  __litl_write_probe_offset(trace, index);
  if (write(trace->f_handle, trace->buffers[index]->buffer_ptr,
            __litl_write_get_buffer_size(trace, index)) == -1) {
    perror(
        "Flushing the buffer. Could not write measured data to the trace file!");
    exit(EXIT_FAILURE);
  }

  // update the general_offset
  trace->general_offset += __litl_write_get_buffer_size(trace, index);
  // update the current offset of the thread
  trace->buffers[index]->offset = trace->general_offset - sizeof(litl_offset_t);

  if (trace->allow_thread_safety)
    pthread_mutex_unlock(&trace->lock_litl_flush);

  trace->buffers[index]->buffer = trace->buffers[index]->buffer_ptr;
}

/*
 * Checks whether the trace buffer was allocated. If no, then allocate
 *    the buffer and, for otherwise too, returns the position of
 *    the thread buffer in the array buffer_ptr/buffer.
 */
static void __litl_write_allocate_buffer(litl_write_trace_t* trace) {
  litl_med_size_t* pos;

  // thread safe region
  pthread_mutex_lock(&trace->lock_buffer_init);

  pos = malloc(sizeof(litl_med_size_t));
  *pos = trace->nb_threads;
  pthread_setspecific(trace->index, pos);
  trace->nb_threads++;

  if (*pos >= trace->nb_allocated_buffers) {
    // We need to allocate a bigger array of buffers
    void* ptr = realloc(
        trace->buffers,
        trace->nb_allocated_buffers * 2 * sizeof(litl_write_buffer_t*));
    if (!ptr) {
      perror("LiTL failed to reallocate memory for threads!\n");
      exit(EXIT_FAILURE);
    }

    trace->buffers = ptr;
    unsigned i;
    for (i = trace->nb_allocated_buffers; i < 2 * trace->nb_allocated_buffers;
        i++) {
      trace->buffers[i] = malloc(sizeof(litl_write_buffer_t));
      if (!trace->buffers[i]) {
        perror("Could not allocate memory for a thread\n!");
        exit(EXIT_FAILURE);
      }
      trace->buffers[i]->already_flushed = 0;
    }
    trace->nb_allocated_buffers *= 2;
  }

  trace->buffers[*pos]->tid = CUR_TID;
  trace->buffers[*pos]->already_flushed = 0;

  pthread_mutex_unlock(&trace->lock_buffer_init);

  trace->buffers[*pos]->buffer_ptr = malloc(
      trace->buffer_size + __litl_get_reg_event_size(LITL_MAX_PARAMS)
        + __litl_get_reg_event_size(1));
  if (!trace->buffers[*pos]->buffer_ptr) {
    perror("Could not allocate memory buffer for the thread\n!");
    exit(EXIT_FAILURE);
  }

  // touch the memory so that it is allocated for real (otherwise, this may
  //    cause performance issues on NUMA machines)
  memset(trace->buffers[*pos]->buffer_ptr, 1, 1);
  trace->buffers[*pos]->buffer = trace->buffers[*pos]->buffer_ptr;
}

/*
 * For internal use only.
 * Allocates an event
 */
litl_t* __litl_write_get_event(litl_write_trace_t* trace, litl_type_t type,
                               litl_code_t code, int size) {

  if (trace && trace->is_litl_initialized && !trace->is_recording_paused
    && !trace->is_buffer_full) {

    // find the thread index
    litl_med_size_t *p_index = pthread_getspecific(trace->index);
    if (!p_index) {
      __litl_write_allocate_buffer(trace);
      p_index = pthread_getspecific(trace->index);
    }
    litl_med_size_t index = *(litl_med_size_t *) p_index;

    litl_write_buffer_t *p_buffer = trace->buffers[index];

    // is there enough space in the buffer?
    if (__litl_write_get_buffer_size(trace, index) < trace->buffer_size) {
      // there is enough space for this event
      litl_t* cur_ptr = (litl_t*) p_buffer->buffer;
      p_buffer->buffer += size;

      // fill the event
      cur_ptr->time = litl_get_time();
      cur_ptr->code = code;
      cur_ptr->type = type;

      switch (type) {
      case LITL_TYPE_REGULAR:
        cur_ptr->parameters.regular.nb_params = size;
        break;
      case LITL_TYPE_RAW:
        cur_ptr->parameters.raw.size = size;
        break;
      case LITL_TYPE_PACKED:
        cur_ptr->parameters.packed.size = size;
        break;
      default:
        fprintf(stderr, "Unknown event type %d\n", type);
        abort();
      }
      return cur_ptr;
    } else if (trace->allow_buffer_flush) {

      // not enough space. flush the buffer and retry
      __litl_write_flush_buffer(trace, index);
      return __litl_write_get_event(trace, type, code, size);
    } else {
      // not enough space, but flushing is disabled so just stop recording
      trace->is_buffer_full = 1;
      return NULL ;
    }
  }
  return NULL ;
}


/* Common function for recording a regular event.
 * This function fills all the fiels except for the parameters
 */
static litl_t* __litl_write_probe_reg_common(litl_write_trace_t* trace,
					     litl_code_t code,
					     unsigned nb_params) {
  if (!trace->is_litl_initialized || trace->is_recording_paused
    || trace->is_buffer_full)
    return NULL;

  if (pthread_getspecific(trace->index) == NULL )
    __litl_write_allocate_buffer(trace);

  litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
      trace->index);
  if (__litl_write_get_buffer_size(trace, index) < trace->buffer_size) {
    litl_t* cur_ptr = (litl_t *) trace->buffers[index]->buffer;

    cur_ptr->time = litl_get_time();
    cur_ptr->code = code;
    cur_ptr->type = LITL_TYPE_REGULAR;

    cur_ptr->parameters.regular.nb_params = nb_params;
    trace->buffers[index]->buffer += LITL_BASE_SIZE + (nb_params * sizeof(litl_param_t));

    return cur_ptr;
  } else {
    /* buffer is full */
    if (trace->allow_buffer_flush) {

      /* flush the buffer to disk */
      __litl_write_flush_buffer(trace, index);
      return __litl_write_probe_reg_common(trace, code, nb_params);
    } else
      /* stop recording events */
      trace->is_buffer_full = 1;
    return NULL;
  }

}

/*
 * Records a regular event without any arguments
 */
litl_t* litl_write_probe_reg_0(litl_write_trace_t* trace, litl_code_t code) {
  litl_t *cur_ptr = __litl_write_probe_reg_common(trace, code, 0);
  return cur_ptr;
}

/*
 * Records a regular event with one argument
 */
litl_t* litl_write_probe_reg_1(litl_write_trace_t* trace, litl_code_t code,
			       litl_param_t param1) {
  litl_t *cur_ptr = __litl_write_probe_reg_common(trace, code, 1);
  if(cur_ptr) {
    cur_ptr->parameters.regular.param[0] = param1;
  }
  return cur_ptr;
}

/*
 * Records a regular event with two arguments
 */
litl_t* litl_write_probe_reg_2(litl_write_trace_t* trace, litl_code_t code,
                            litl_param_t param1, litl_param_t param2) {
  litl_t *cur_ptr = __litl_write_probe_reg_common(trace, code, 2);
  if(cur_ptr) {
    cur_ptr->parameters.regular.param[0] = param1;
    cur_ptr->parameters.regular.param[1] = param2;
  }
  return cur_ptr;
}

/*
 * Records a regular event with three arguments
 */
litl_t* litl_write_probe_reg_3(litl_write_trace_t* trace, litl_code_t code,
                            litl_param_t param1, litl_param_t param2,
                            litl_param_t param3) {
  litl_t *cur_ptr = __litl_write_probe_reg_common(trace, code, 3);
  if(cur_ptr) {
    cur_ptr->parameters.regular.param[0] = param1;
    cur_ptr->parameters.regular.param[1] = param2;
    cur_ptr->parameters.regular.param[2] = param3;
  }
  return cur_ptr;
}

/*
 * Records a regular event with four arguments
 */
litl_t* litl_write_probe_reg_4(litl_write_trace_t* trace, litl_code_t code,
                            litl_param_t param1, litl_param_t param2,
                            litl_param_t param3, litl_param_t param4) {
  litl_t *cur_ptr = __litl_write_probe_reg_common(trace, code, 4);
  if(cur_ptr) {
    cur_ptr->parameters.regular.param[0] = param1;
    cur_ptr->parameters.regular.param[1] = param2;
    cur_ptr->parameters.regular.param[2] = param3;
    cur_ptr->parameters.regular.param[3] = param4;
  }
  return cur_ptr;
}

/*
 * Records a regular event with five arguments
 */
litl_t* litl_write_probe_reg_5(litl_write_trace_t* trace, litl_code_t code,
                            litl_param_t param1, litl_param_t param2,
                            litl_param_t param3, litl_param_t param4,
                            litl_param_t param5) {
  litl_t *cur_ptr = __litl_write_probe_reg_common(trace, code, 5);
  if(cur_ptr) {
    cur_ptr->parameters.regular.param[0] = param1;
    cur_ptr->parameters.regular.param[1] = param2;
    cur_ptr->parameters.regular.param[2] = param3;
    cur_ptr->parameters.regular.param[3] = param4;
    cur_ptr->parameters.regular.param[4] = param5;
  }
  return cur_ptr;
}

/*
 * Records a regular event with six arguments
 */
litl_t* litl_write_probe_reg_6(litl_write_trace_t* trace, litl_code_t code,
                            litl_param_t param1, litl_param_t param2,
                            litl_param_t param3, litl_param_t param4,
                            litl_param_t param5, litl_param_t param6) {
  litl_t *cur_ptr = __litl_write_probe_reg_common(trace, code, 6);
  if(cur_ptr) {
    cur_ptr->parameters.regular.param[0] = param1;
    cur_ptr->parameters.regular.param[1] = param2;
    cur_ptr->parameters.regular.param[2] = param3;
    cur_ptr->parameters.regular.param[3] = param4;
    cur_ptr->parameters.regular.param[4] = param5;
    cur_ptr->parameters.regular.param[5] = param6;
  }
  return cur_ptr;
}

/*
 * Records a regular event with seven arguments
 */
litl_t* litl_write_probe_reg_7(litl_write_trace_t* trace, litl_code_t code,
                            litl_param_t param1, litl_param_t param2,
                            litl_param_t param3, litl_param_t param4,
                            litl_param_t param5, litl_param_t param6,
                            litl_param_t param7) {
  litl_t *cur_ptr = __litl_write_probe_reg_common(trace, code, 7);
  if(cur_ptr) {
    cur_ptr->parameters.regular.param[0] = param1;
    cur_ptr->parameters.regular.param[1] = param2;
    cur_ptr->parameters.regular.param[2] = param3;
    cur_ptr->parameters.regular.param[3] = param4;
    cur_ptr->parameters.regular.param[4] = param5;
    cur_ptr->parameters.regular.param[5] = param6;
    cur_ptr->parameters.regular.param[6] = param7;
  }
  return cur_ptr;
}

/*
 * Records a regular event with eight arguments
 */
litl_t* litl_write_probe_reg_8(litl_write_trace_t* trace, litl_code_t code,
                            litl_param_t param1, litl_param_t param2,
                            litl_param_t param3, litl_param_t param4,
                            litl_param_t param5, litl_param_t param6,
                            litl_param_t param7, litl_param_t param8) {
  litl_t *cur_ptr = __litl_write_probe_reg_common(trace, code, 8);
  if(cur_ptr) {
    cur_ptr->parameters.regular.param[0] = param1;
    cur_ptr->parameters.regular.param[1] = param2;
    cur_ptr->parameters.regular.param[2] = param3;
    cur_ptr->parameters.regular.param[3] = param4;
    cur_ptr->parameters.regular.param[4] = param5;
    cur_ptr->parameters.regular.param[5] = param6;
    cur_ptr->parameters.regular.param[6] = param7;
    cur_ptr->parameters.regular.param[7] = param8;
  }
  return cur_ptr;
}

/*
 * Records a regular event with nine arguments
 */
litl_t* litl_write_probe_reg_9(litl_write_trace_t* trace, litl_code_t code,
                            litl_param_t param1, litl_param_t param2,
                            litl_param_t param3, litl_param_t param4,
                            litl_param_t param5, litl_param_t param6,
                            litl_param_t param7, litl_param_t param8,
                            litl_param_t param9) {
  litl_t *cur_ptr = __litl_write_probe_reg_common(trace, code, 9);
  if(cur_ptr) {
    cur_ptr->parameters.regular.param[0] = param1;
    cur_ptr->parameters.regular.param[1] = param2;
    cur_ptr->parameters.regular.param[2] = param3;
    cur_ptr->parameters.regular.param[3] = param4;
    cur_ptr->parameters.regular.param[4] = param5;
    cur_ptr->parameters.regular.param[5] = param6;
    cur_ptr->parameters.regular.param[6] = param7;
    cur_ptr->parameters.regular.param[7] = param8;
    cur_ptr->parameters.regular.param[8] = param9;
  }
  return cur_ptr;
}

/*
 * Records a regular event with ten arguments
 */
litl_t* litl_write_probe_reg_10(litl_write_trace_t* trace, litl_code_t code,
                             litl_param_t param1, litl_param_t param2,
                             litl_param_t param3, litl_param_t param4,
                             litl_param_t param5, litl_param_t param6,
                             litl_param_t param7, litl_param_t param8,
                             litl_param_t param9, litl_param_t param10) {
  litl_t *cur_ptr = __litl_write_probe_reg_common(trace, code, 10);
  if(cur_ptr) {
    cur_ptr->parameters.regular.param[0] = param1;
    cur_ptr->parameters.regular.param[1] = param2;
    cur_ptr->parameters.regular.param[2] = param3;
    cur_ptr->parameters.regular.param[3] = param4;
    cur_ptr->parameters.regular.param[4] = param5;
    cur_ptr->parameters.regular.param[5] = param6;
    cur_ptr->parameters.regular.param[6] = param7;
    cur_ptr->parameters.regular.param[7] = param8;
    cur_ptr->parameters.regular.param[8] = param9;
    cur_ptr->parameters.regular.param[9] = param10;
  }
  return cur_ptr;
}

/*
 * Records an event in a raw state, where the size is #args in the void* array.
 * That helps to discover places where the application has crashed
 */
litl_t* litl_write_probe_raw(litl_write_trace_t* trace, litl_code_t code,
                          litl_size_t size, litl_data_t data[]) {
  if (!trace->is_litl_initialized || trace->is_recording_paused
    || trace->is_buffer_full)
    return NULL;

  // specify explicitly the end of the string
  data[size] = '\0';
  size++;

  if (pthread_getspecific(trace->index) == NULL )
    __litl_write_allocate_buffer(trace);

  litl_med_size_t i, index;
  index = *(litl_med_size_t *) pthread_getspecific(trace->index);

  litl_t* cur_ptr = (litl_t *) trace->buffers[index]->buffer;
  // needs to be done outside of the if statement 'cause of undefined size of
  //    the string that may cause segfault
  trace->buffers[index]->buffer += LITL_BASE_SIZE + 7 + size;

  if (__litl_write_get_buffer_size(trace, index) < trace->buffer_size) {
    cur_ptr->time = litl_get_time();
    cur_ptr->code = code;
    cur_ptr->type = LITL_TYPE_RAW;
    cur_ptr->parameters.raw.size = size;
    if (size > 0)
      for (i = 0; i < size; i++)
        cur_ptr->parameters.raw.data[i] = data[i];

    return cur_ptr;

  } else {
    /* the buffer is full */
    if (trace->allow_buffer_flush) {
      // if there is not enough size we reset back the buffer pointer
      trace->buffers[index]->buffer -= LITL_BASE_SIZE + 7 + size;

      __litl_write_flush_buffer(trace, index);
      return litl_write_probe_raw(trace, code, size, data);
    } else {
      // this applies only when the flushing is off
      trace->is_buffer_full = 1;
      return NULL;
    }
  }
}

/*
 * This function finalizes the trace
 */
void litl_write_finalize_trace(litl_write_trace_t* trace) {
  litl_med_size_t i;
  if(!trace)
    return;

  for (i = 0; i < trace->nb_threads; i++)
    __litl_write_flush_buffer(trace, i);

  close(trace->f_handle);
  trace->f_handle = -1;

  for (i = 0; i < trace->nb_allocated_buffers; i++)
    if (trace->buffers[i]->tid != 0) {
      free(trace->buffers[i]->buffer_ptr);
    } else {
      break;
    }

  if (trace->allow_thread_safety) {
    pthread_mutex_destroy(&trace->lock_litl_flush);
  }
  pthread_mutex_destroy(&trace->lock_buffer_init);

  free(trace->filename);
  trace->filename = NULL;
  trace->is_litl_initialized = 0;
  trace->is_header_flushed = 0;
  free(trace);
}
