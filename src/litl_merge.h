/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_MERGE_H_
#define LITL_MERGE_H_

#include "litl_types.h"
#include "litl_queue.h"

/*
 * Sets a new name for the archive
 */
void litl_set_archive_name(litl_trace_merge_t *, char *);

/*
 * Creates and opens an archive for traces.
 * Allocates memory for the buffer
 */
litl_trace_merge_t litl_init_archive(char *, litl_size_t, litl_queue_t*);

/*
 * Merges trace files. This is a modified version of the cat implementation
 *   from the Kernighan & Ritchie book
 */
void litl_merge_file(litl_trace_merge_t *, int, const char *);

/*
 * Frees the allocated memory
 */
void litl_finalize_archive(litl_trace_merge_t *);

#endif /* LITL_MERGE_H_ */
