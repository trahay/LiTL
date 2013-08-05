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
litl_size_t get_event_size(litl_size_t);

/*
 * Returns the size of a particular event in bytes depending on
 * the number of arguments
 */
litl_size_t get_event_size_type(litl_t *p_evt);

/*
 * Sets the bit of the higher order to one
 */
litl_code_t set_bit(litl_code_t);

/*
 * Sets the bit of the higher order to zero
 */
litl_code_t clear_bit(litl_code_t);

/*
 * Returns the bit of the higher order
 */
uint8_t get_bit(litl_code_t);

#endif /* LITL_MACRO_H_ */
