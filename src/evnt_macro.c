/*
 * evnt_macro.c
 *
 *  Created on: Nov 7, 2012
 *      Author: Roman Iakymchuk
 */

#include "evnt_macro.h"

/*
 * This function returns the size of event depending on the number of arguments
 */
uint64_t get_event_size(uint64_t nb_args) {
    // 4 is the size of tid, time, code, and nb_args
    return nb_args + 4;
}
