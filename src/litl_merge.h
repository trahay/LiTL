/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_MERGE_H_
#define LITL_MERGE_H_
#include "evnt_types.h"

/*
 * This function for merging trace files is a modified version of the cat implementation from the Kernighan & Ritchie book
 */
void litl_merge_file(const int, const char *, const int);

#endif /* LITL_MERGE_H_ */
