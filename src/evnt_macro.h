/*
 * evnt_macro.h
 *
 *  Created on: Oct 30, 2012
 *      Author: Roman Iakymchuk
 */

#ifndef EVNT_MACRO_H_
#define EVNT_MACRO_H_

#include<stdint.h>
#include "evnt_types.h"

/*
 * This function returns the number of components in a particular event depending on the number of arguments
 */
evnt_size_t get_event_components(evnt_size_t);

/*
 * This function returns the size of a particular event in bytes depending on the number of arguments
 */
evnt_size_t get_event_size(evnt_size_t);

/*
 * This function converts evnt's parameters to string. As a separator, a space is used
 */
char* params_to_string(evnt_t *);

/*
 * This functions sets the bit of the higher order to one
 */
evnt_code_t set_bit(evnt_code_t);

/*
 * This functions sets the bit of the higher order to zero
 */
evnt_code_t clear_bit(evnt_code_t);

/*
 * This function returns the bit of the higher order
 */
uint8_t get_bit(evnt_code_t);


#endif /* EVNT_MACRO_H_ */
