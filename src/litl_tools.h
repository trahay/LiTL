/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_TOOLS_H_
#define LITL_TOOLS_H_

/**
 *  \file litl_tools.h
 *  \brief litl_tools Provides a set of auxiliary functions
 *
 *  \authors
 *    Developers are : \n
 *        Roman Iakymchuk   -- roman.iakymchuk@telecom-sudparis.eu \n
 *        Francois Trahay   -- francois.trahay@telecom-sudparis.eu \n
 */

#include<stdint.h>
#include "litl_types.h"

/**
 * \defgroup litl_tools LiTL Auxiliary Functions
 */

/**
 * \ingroup litl_tools
 * \brief Returns the size of a regular event (in Bytes) depending on the number
 * of its parameters
 * \param nb_params A number of event's parameters
 * \return A size of a regular event
 */
litl_med_size_t __litl_get_reg_event_size(litl_data_t nb_params);

/**
 * \ingroup litl_tools
 * \brief Returns the size of a general event (in Bytes) depending on its type
 *  and the number of its parameters
 * \param p_evt A pointer to an event
 * \return A size of a given event
 */
litl_med_size_t __litl_get_gen_event_size(litl_t *p_evt);

#endif /* LITL_TOOLS_H_ */
