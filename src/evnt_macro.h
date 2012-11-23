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

#endif /* EVNT_MACRO_H_ */
