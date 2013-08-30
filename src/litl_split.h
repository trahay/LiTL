/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_SPLIT_H_
#define LITL_SPLIT_H_

/**
 *  \file litl_split.h
 *  \brief litl_split Provides a set of functions for extracting trace files
 *  from an archive of traces
 *
 *  \authors
 *    Developers are : \n
 *        Roman Iakymchuk   -- roman.iakymchuk@telecom-sudparis.eu \n
 *        Francois Trahay   -- francois.trahay@telecom-sudparis.eu \n
 */

#include "litl_types.h"

/**
 * \defgroup litl_split LiTL Splitting Functions
 */

/**
 * \ingroup litl_split
 * \brief Extracts each trace from an archive into a separate trace file
 * \param arch_name A name of an archive
 * \param output_dir A path to the directory with extracted traces
 */
void litl_split_archive(const char *arch_name, const char *output_dir);

#endif /* LITL_SPLIT_H_ */
