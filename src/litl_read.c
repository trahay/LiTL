/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdlib.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "litl_macro.h"
#include "litl_read.h"

static char* __input_filename = "trace";

static void __usage(int argc __attribute__((unused)), char **argv) {
    fprintf(stderr, "Usage: %s [-f input_filename] \n", argv[0]);
    printf("       -?, -h:    Display this help and exit\n");
}

static void __parse_args(int argc, char **argv) {
    int i;

    for (i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-f") == 0)) {
            __input_filename = argv[++i];
        } else if ((strcmp(argv[i], "-h") || strcmp(argv[i], "-?")) == 0) {
            __usage(argc, argv);
            exit(-1);
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option %s\n", argv[i]);
            __usage(argc, argv);
            exit(-1);
        }
    }

    if (strcmp(__input_filename, "trace") == 0) {
        __usage(argc, argv);
        exit(-1);
    }
}

/*
 * Initializes the archive header
 */
static void __init_header(litl_trace_read_t* arch) {
    int res;

    // at first, the header is small 'cause it stores only nb_traces and
    //   is_trace_archive values
    arch->header_size = sizeof(litl_tiny_size_t) + sizeof(litl_med_size_t);
    arch->header_buffer = (litl_buffer_t) malloc(arch->header_size);
    if (!arch->header_buffer) {
        perror("Could not allocate memory for the header!");
        exit(EXIT_FAILURE);
    }

    // read the archive header
    res = read(arch->f_handle, arch->header_buffer, arch->header_size);
    // If the end of file is reached, then all data are read; res = 0.
    //   Otherwise, res equals the number of elements (= 1) or the error
    //   occurred and res = -1.
    if (res == -1) {
        perror("Could not read the trace header!");
        exit(EXIT_FAILURE);
    }

    // get the number of traces
    arch->is_trace_archive =
            ((litl_header_t *) arch->header_buffer)->is_trace_archive;
    if (arch->is_trace_archive) {
        // Yes, we work with an archive of trace. So, we increase
        //   the header size and relocate the header buffer
        arch->nb_traces = ((litl_header_t *) arch->header_buffer)->nb_threads;
        arch->header_size = arch->nb_traces * sizeof(litl_header_triples_t);
        printf("header_size = %d\n", arch->header_size);
        arch->header_buffer = (litl_buffer_t) realloc(arch->header_buffer,
                arch->header_size);

        // read the archive header
        res = read(arch->f_handle, arch->header_buffer, arch->header_size);
    } else {
        arch->nb_traces = 1;
    }
}

/*
 * Initializes the trace header, meaning it reads chunks with all pairs
 */
static void __init_trace_header(litl_trace_read_t* arch,
        litl_trace_read_process_t* trace) {

    // init the header structure
    // the minimum size: size of the header with all information and
    //   one pair (tid, offset)
    trace->header_size = sizeof(litl_header_t) + sizeof(litl_header_tids_t);
    trace->header_buffer_ptr = (litl_buffer_t) malloc(trace->header_size);
    if (!trace->header_buffer_ptr) {
        perror("Could not allocate memory for the trace header!");
        exit(EXIT_FAILURE);
    }

    // read the header
    lseek(arch->f_handle, trace->triples->offset, SEEK_SET);
    int res = read(arch->f_handle, trace->header_buffer_ptr,
            trace->header_size);
    if (res == -1) {
        perror("Could not read the trace header!");
        exit(EXIT_FAILURE);
    }
    trace->header = (litl_header_t *) trace->header_buffer_ptr;

    // check if the buffer is large enough to store the header
    litl_med_size_t nb_threads =
            (trace->header->nb_threads > NBTHREADS) ? trace->header->nb_threads :
                    NBTHREADS;
    if (trace->header->nb_threads > 1) {
        trace->header_buffer_ptr = (litl_buffer_t) realloc(
                trace->header_buffer_ptr,
                sizeof(litl_header_t)
                        + (nb_threads + 1) * sizeof(litl_header_tids_t));

        lseek(arch->f_handle, trace->triples->offset, SEEK_SET);
        trace->header_size = sizeof(litl_header_t)
                + (trace->header->header_nb_threads + 1)
                        * sizeof(litl_header_tids_t);
        int res = read(arch->f_handle, trace->header_buffer_ptr,
                trace->header_size);
        if (res == -1) {
            perror("Could not read the trace header!");
            exit(EXIT_FAILURE);
        }
        trace->header = (litl_header_t *) trace->header_buffer_ptr;
    }

    trace->header_buffer = trace->header_buffer_ptr + sizeof(litl_header_t);
}

/*
 * Reads another portion of pairs(tid, offset) from the trace file
 */
static void __next_pairs_buffer(litl_trace_read_t* arch,
        litl_trace_read_process_t* trace, litl_offset_t offset) {

    lseek(arch->f_handle, offset, SEEK_SET);

    litl_med_size_t nb_threads =
            (trace->nb_buffers - trace->header->header_nb_threads) > NBTHREADS ? NBTHREADS :
                    (trace->nb_buffers - trace->header->header_nb_threads);

    int res = read(arch->f_handle,
            trace->header_buffer_ptr + sizeof(litl_header_t),
            (nb_threads + 1) * sizeof(litl_header_tids_t));
    trace->header_buffer = trace->header_buffer_ptr + sizeof(litl_header_t);

    if (res == -1) {
        perror(
                "Could not read the next part of pairs (tid, offset) from the trace file!");
        exit(EXIT_FAILURE);
    }
}

/*
 * Initializes buffer of each recorded thread. One buffer per thread.
 */
static void __init_buffers(litl_trace_read_t* arch,
        litl_trace_read_process_t* trace) {
    litl_med_size_t i, size;
    litl_header_tids_t *tids;

    size = sizeof(litl_header_tids_t);
    // init nb_buffers and allocate memory
    trace->nb_buffers = trace->header->nb_threads;
    trace->buffers = (litl_trace_read_thread_t*) malloc(
            trace->nb_buffers * sizeof(litl_trace_read_thread_t));

    // increase a bit the buffer size 'cause of the possible event's tail and
    //   the offset
    trace->buffer_size = trace->header->buffer_size
            + get_reg_event_size(LITL_MAX_PARAMS) + get_reg_event_size(0);

    for (i = 0; i < trace->nb_buffers; i++) {
        /* read pairs (tid, offset) */
        tids = (litl_header_tids_t *) trace->header_buffer;

        // deal with slots of pairs
        if ((tids->tid == 0) && (tids->offset != 0)) {
            __next_pairs_buffer(arch, trace,
                    trace->triples->offset + tids->offset);
            tids = (litl_header_tids_t *) trace->header_buffer;
        }

        // end of reading pairs
        if ((tids->tid == 0) && (tids->offset == 0))
            break;

        trace->buffers[i].tids = (litl_header_tids_t*) malloc(
                sizeof(litl_header_tids_t));
        trace->buffers[i].tids->tid = tids->tid;
        trace->buffers[i].tids->offset = tids->offset;

        trace->header_buffer += size;

        /* read chunks of data */
        trace->buffers[i].buffer_size = trace->buffer_size;
        trace->buffers[i].buffer_ptr = (litl_buffer_t) malloc(
                trace->buffer_size);

        // use offsets in order to access a chuck of data that corresponds to
        //   each thread
        lseek(arch->f_handle,
                trace->triples->offset + trace->buffers[i].tids->offset,
                SEEK_SET);
        int res = read(arch->f_handle, trace->buffers[i].buffer_ptr,
                trace->buffer_size);

        // If the end of file is reached, then all data are read. So, res is 0.
        // Otherwise, res is either an error or the number of elements, which is 1.
        if (res == -1) {
            perror(
                    "Could not read the first partition of data from the trace file!");
            exit(EXIT_FAILURE);
        }

        trace->buffers[i].buffer = trace->buffers[i].buffer_ptr;
        trace->buffers[i].tracker = trace->buffers[i].buffer_size;
        trace->buffers[i].offset = 0;
    }
}

/*
 * Opens an archive of traces
 */
litl_trace_read_t *litl_open_trace(const char* filename) {
    litl_trace_read_t *arch = malloc(sizeof(litl_trace_read_t));

    // open a trace file
    if ((arch->f_handle = open(filename, O_RDONLY)) < 0) {
        fprintf(stderr, "Cannot open %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // init the archive header
    __init_header(arch);

    return arch;
}

/*
 * Initializes the archive's traces
 */
void litl_init_traces(litl_trace_read_t* arch) {

    arch->traces = (litl_trace_read_process_t *) malloc(
            arch->nb_traces * sizeof(litl_trace_read_process_t));

    if (arch->is_trace_archive) {
        // archive of traces
        litl_med_size_t i, size;
        size = sizeof(litl_header_triples_t);

        for (i = 0; i < arch->nb_traces; i++) {
            // TODO: implement this loop as recursion in order to deal with
            //       archives of archives
            // read triples that contain offset from the beginning of the archive
            arch->traces[i].triples =
                    (litl_header_triples_t *) arch->header_buffer;

            arch->header_buffer += size;

            arch->traces[i].cur_index = -1;
            arch->traces[i].initialized = 0;

            // init the trace header
            __init_trace_header(arch, &arch->traces[i]);

            // init buffers of events: one buffer per thread
            __init_buffers(arch, &arch->traces[i]);
        }
    } else {
        // regular trace
        arch->traces->triples = (litl_header_triples_t *) malloc(
                sizeof(litl_header_triples_t));
        arch->traces->triples->offset = 0;

        arch->traces->cur_index = -1;
        arch->traces->initialized = 0;

        // init the trace header
        __init_trace_header(arch, arch->traces);

        // init buffers of events: one buffer per thread
        __init_buffers(arch, arch->traces);
    }
}

/*
 * Returns a pointer to the trace header
 */
litl_header_t* litl_get_trace_header(litl_trace_read_t* arch) {
    return arch->traces[0].header;
}

/*
 * Sets the buffer size
 */
void litl_set_buffer_size(litl_trace_read_t* arch, const litl_size_t buf_size) {
    litl_med_size_t i;

    for (i = 0; i < arch->nb_traces; i++)
        arch->traces[i].buffer_size = buf_size;
}

/*
 * Returns the buffer size
 */
litl_size_t litl_get_buffer_size(litl_trace_read_t* arch) {
    return arch->traces[0].buffer_size;
}

/*
 * Reads another portion of data from the trace file to the buffer
 */
static void __next_buffer(litl_trace_read_t* arch, litl_med_size_t trace_index,
        litl_med_size_t buffer_index) {
    litl_trace_read_process_t* trace = &arch->traces[trace_index];

    lseek(arch->f_handle,
            trace->triples->offset + trace->buffers[buffer_index].tids->offset,
            SEEK_SET);
    trace->buffers[buffer_index].offset = 0;

    int res = read(arch->f_handle, trace->buffers[buffer_index].buffer_ptr,
            trace->buffer_size);

    // If the end of file is reached, then all data are read. So, res is 0.
    // Otherwise, res is either an error or the number of elements, which is 1.
    if (res == -1) {
        perror("Could not read the next part of the trace file!");
        exit(EXIT_FAILURE);
    }

    trace->buffers[buffer_index].buffer =
            trace->buffers[buffer_index].buffer_ptr;
    trace->buffers[buffer_index].tracker = trace->buffers[buffer_index].offset
            + trace->buffer_size;
}

/*
 * Resets the trace buffer
 */
void litl_reset_trace(litl_trace_read_t* arch, litl_med_size_t trace_index) {
    litl_med_size_t i;
    litl_trace_read_process_t* trace = &arch->traces[trace_index];

    for (i = 0; i < trace->nb_buffers; i++)
        trace->buffers[i].buffer = trace->buffers[i].buffer_ptr;
}

/*
 * Reads an event
 */
static litl_read_t* __read_event(litl_trace_read_t* arch,
        litl_med_size_t trace_index, litl_med_size_t buffer_index) {
    uint8_t to_be_loaded;
    litl_t* event;
    litl_buffer_t* buffer;
    litl_trace_read_process_t* trace = &arch->traces[trace_index];

    buffer = (litl_buffer_t*) trace->buffers[buffer_index].buffer;
    to_be_loaded = 0;

    if (!buffer) {
        trace->buffers[buffer_index].cur_event.event = NULL;
        return NULL ;
    }

    event = (litl_t *) buffer;

    // While reading events from the buffer, there can be two situations:
    // 1. The situation when the buffer contains exact number of events;
    // 2. The situation when only a part of the last event is loaded.
    // Check whether the main four components (tid, time, code, nb_params) are
    //   loaded.
    // Check whether all arguments are loaded.
    // If any of these cases is not true, the next part of the trace plus
    // the current event is loaded to the buffer
    litl_size_t remaining_size = trace->buffers[buffer_index].tracker
            - trace->buffers[buffer_index].offset;
    if (remaining_size < get_reg_event_size(0)) {
        // this event is truncated. We can't even read the nb_param field
        to_be_loaded = 1;
    } else {
        // The nb_param (or size) field is available. Let's check whether
        //   the event is truncated
        litl_med_size_t event_size = get_gen_event_size(event);
        if (remaining_size < event_size)
            to_be_loaded = 1;
    }

    // event that stores tid and offset
    if (event->code == LITL_OFFSET_CODE) {
        if (event->parameters.offset.offset != 0) {
            trace->buffers[buffer_index].tids->offset =
                    event->parameters.offset.offset;
            to_be_loaded = 1;
        } else {
            trace->buffers[buffer_index].buffer = NULL;
            *buffer = NULL;
            trace->buffers[buffer_index].cur_event.event = NULL;
            return NULL ;
        }
    }

    // fetch the next block of data from the trace
    if (to_be_loaded) {
        __next_buffer(arch, trace_index, buffer_index);
        buffer = &trace->buffers[buffer_index].buffer;
        event = (litl_t *) *buffer;
    }

    // move pointer to the next event and update __offset
    unsigned evt_size = get_gen_event_size(event);
    trace->buffers[buffer_index].buffer += evt_size;
    trace->buffers[buffer_index].offset += evt_size;

    trace->buffers[buffer_index].cur_event.event = event;
    trace->buffers[buffer_index].cur_event.tid =
            trace->buffers[buffer_index].tids->tid;
    return &trace->buffers[buffer_index].cur_event;
}

/*
 * Searches for the next event inside the trace
 */
litl_read_t* litl_next_trace_event(litl_trace_read_t* arch,
        litl_med_size_t trace_index) {
    litl_med_size_t i;
    litl_time_t min_time = -1;
    litl_trace_read_process_t* trace = &arch->traces[trace_index];

    if (!trace->initialized) {
        for (i = 0; i < trace->nb_buffers; i++)
            __read_event(arch, trace_index, i);

        trace->cur_index = -1;
        trace->initialized = 1;
    }

    // read the next event from the buffer
    if (trace->cur_index != -1)
        __read_event(arch, trace_index, trace->cur_index);

    int found = 0;
    for (i = 0; i < trace->nb_buffers; i++) {
        litl_read_t *evt = LITL_GET_CUR_EVENT_PER_THREAD(trace, i);
        if ( evt && evt->event && (LITL_GET_TIME(evt) < min_time)) {
            found = 1;
            min_time = LITL_GET_TIME(evt);
            trace->cur_index = i;
        }
    }

    if (found)
        return LITL_GET_CUR_EVENT(trace);

    return NULL ;
}

/*
 * Reads the next event either from an archive or a regular trace
 */
litl_read_t* litl_next_event(litl_trace_read_t* arch) {
    litl_med_size_t i;
    litl_read_t* event = NULL;

    for (i = 0; i < arch->nb_traces; i++) {
        event = litl_next_trace_event(arch, i);

        if (event != NULL )
            break;
    }

    return event;
}

/*
 * Closes the trace and frees the buffer
 */
void litl_close_trace(litl_trace_read_t* arch) {
    litl_med_size_t i, j;

    // close the file
    close(arch->f_handle);
    arch->f_handle = -1;

    // free traces
    for (i = 0; i < arch->nb_traces; i++) {
        free(arch->traces[i].header_buffer_ptr);

        for (j = 0; j < arch->traces[i].nb_buffers; j++) {
            free(arch->traces[i].buffers[j].buffer_ptr);
            free(arch->traces[i].buffers[j].tids);
        }

        free(arch->traces[i].buffers);
    }

    // free an archive
    free(arch->traces);
//    free(arch->header_buffer);
    free(arch);

    // set pointers to NULL
    arch->traces = NULL;
    arch->header_buffer = NULL;
    arch = NULL;
}

int main(int argc, char **argv) {
    litl_med_size_t i;
    litl_read_t* event;
    litl_trace_read_t *arch;
    litl_header_t* header;

    // parse the arguments passed to this program
    __parse_args(argc, argv);

    arch = litl_open_trace(__input_filename);

    litl_init_traces(arch);

    header = litl_get_trace_header(arch);

    // print the header
    printf(" LiTL v.%s\n", header->litl_ver);
    printf(" %s\n", header->sysinfo);
    if (arch->is_trace_archive)
        printf(" nb_traces \t %d\n", arch->nb_traces);
    else
        printf(" nb_threads \t %d\n", header->nb_threads);
    printf(" buffer_size \t %d\n", header->buffer_size);

    printf(
            "[Timestamp]\t[ThreadID]\t[EventType]\t[EventCode]\t[NbParam]\t[Parameters]\n");
    while (1) {
        event = litl_next_event(arch);

        if (event == NULL )
            break;

        switch (LITL_GET_TYPE(event)) {
        case LITL_TYPE_REGULAR: { // regular event
            printf("%"PRTIu64" \t%"PRTIu64" \t  Reg   %"PRTIx32" \t %"PRTIu32,
                    LITL_GET_TIME(event), LITL_GET_TID(event),
                    LITL_GET_CODE(event), LITL_REGULAR(event)->nb_params);

            for (i = 0; i < LITL_REGULAR(event)->nb_params; i++)
                printf("\t %"PRTIx64, LITL_REGULAR(event)->param[i]);
            break;
        }
        case LITL_TYPE_RAW: { // raw event
            LITL_GET_CODE(event) = clear_bit(LITL_GET_CODE(event));
            printf("%"PRTIu64"\t%"PRTIu64" \t  Raw   %"PRTIx32" \t %"PRTIu32,
                    LITL_GET_TIME(event), LITL_GET_TID(event),
                    LITL_GET_CODE(event), LITL_RAW(event)->size);
            printf("\t %s", (litl_data_t *) LITL_RAW(event)->data);
            break;
        }
        case LITL_TYPE_PACKED: { // packed event
            printf(
                    "%"PRTIu64" \t%"PRTIu64" \t  Packed   %"PRTIx32" \t %"PRTIu32"\t",
                    LITL_GET_TIME(event), LITL_GET_TID(event),
                    LITL_GET_CODE(event), LITL_PACKED(event)->size);
            for (i = 0; i < LITL_PACKED(event)->size; i++) {
                printf(" %x", LITL_PACKED(event)->param[i]);
            }
            break;
        }
        case LITL_TYPE_OFFSET: { // offset event
            continue;
        }
        default: {
            fprintf(stderr, "Unknown event type %d\n", LITL_GET_TYPE(event));
            abort();
        }
        }

        printf("\n");
    }

    litl_close_trace(arch);

    return EXIT_SUCCESS;
}
