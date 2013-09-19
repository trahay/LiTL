/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_MERGE_H_
#define LITL_MERGE_H_

/**
 *  \file litl_merge.h
 *  \brief litl_merge Provides a set of functions for merging trace files into
 *  an archive of traces
 *
 *  \authors
 *    Developers are : \n
 *        Roman Iakymchuk   -- roman.iakymchuk@telecom-sudparis.eu \n
 *        Francois Trahay   -- francois.trahay@telecom-sudparis.eu \n
 */

#include "litl_types.h"

/**
 * \defgroup litl_merge LiTL Merging Functions
 */

/**
 * \ingroup litl_merge
 * \brief Merges trace files into an archive. This is a modified version of the
 * implementation of the cat function from the Kernighan & Ritchie book
 * \param arch_name A name of an archive
 * \param traces_names An array of traces names
 * \param nb_traces A number of trace files to be composed into an archive
 */
void litl_merge_traces(const char* arch_name, char** traces_names,
                       const int nb_traces);

#endif /* LITL_MERGE_H_ */
