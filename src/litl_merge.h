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
 * Merges trace files. This is a modified version of the cat implementation
 *   from the Kernighan & Ritchie book
 */
void litl_merge_traces(const char *, litl_queue_t *, int);

#endif /* LITL_MERGE_H_ */
