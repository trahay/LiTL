/*
 * evnt_macro.c
 *
 *  Created on: Nov 7, 2012
 *      Author: Roman Iakymchuk
 */

#include "evnt_macro.h"

/*
 * This function returns the number of components in a particular event depending on the number of arguments
 */
evnt_size_t get_event_components(evnt_size_t nb_args) {
    // 4 is the size of tid, time, code, and nb_args
    return nb_args + 4;
}

/*
 * This function returns the size of a particular event in bytes depending on the number of arguments
 */
evnt_size_t get_event_size(evnt_size_t nb_args) {
    return sizeof(evnt_tid_t) + sizeof(evnt_time_t) + sizeof(evnt_code_t) + sizeof(evnt_size_t)
            + nb_args * sizeof(evnt_args_t);
}
