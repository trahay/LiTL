/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_SPLIT_H_
#define LITL_SPLIT_H_

#include "litl_types.h"

/*
 * Writes each trace from an archive into a separate trace file
 */
void litl_split_archive(const char *, const char *);

#endif /* LITL_SPLIT_H_ */
