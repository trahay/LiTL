/*
 * evnt_macro.h
 *
 *  Created on: Oct 30, 2012
 *      Author: Roman Iakymchuk
 */

#ifndef EVNT_MACRO_H_
#define EVNT_MACRO_H_

#include<stdint.h>

void set_filename(char* filename);

/*
 * This function returns the size of event depending on the number of arguments
 */
uint64_t get_event_size(uint64_t);

#endif /* EVNT_MACRO_H_ */
