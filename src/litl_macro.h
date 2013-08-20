/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_MACRO_H_
#define LITL_MACRO_H_

#include<stdint.h>
#include "litl_types.h"

/*
 * Returns the size of a particular event in bytes depending on
 * the number of arguments
 */
litl_med_size_t litl_get_reg_event_size(litl_tiny_size_t nb_params);

/*
 * Returns the size of a particular event in bytes depending on
 * the number of arguments
 */
litl_med_size_t litl_get_gen_event_size(litl_t *p_evt);

#endif /* LITL_MACRO_H_ */
