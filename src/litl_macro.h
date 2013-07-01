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
 * This function returns the number of components in a particular event depending on the number of arguments
 */
litl_size_t get_event_components(litl_size_t);

/*
 * This function returns the size of a particular event in bytes depending on the number of arguments
 */
litl_size_t get_event_size(litl_size_t);

/*
 * This function returns the size of a particular event in bytes depending on the number of arguments
 */
litl_size_t get_event_size_type(litl_t *p_evt);

/*
 * This function converts evnt's parameters to string. As a separator, a space is used
 */
char* params_to_string(litl_t *);

/*
 * This functions sets the bit of the higher order to one
 */
litl_code_t set_bit(litl_code_t);

/*
 * This functions sets the bit of the higher order to zero
 */
litl_code_t clear_bit(litl_code_t);

/*
 * This function returns the bit of the higher order
 */
uint8_t get_bit(litl_code_t);


#endif /* LITL_MACRO_H_ */
