/*
 * evnt_macro.c
 *
 *  Created on: Nov 7, 2012
 *      Author: Roman Iakymchuk
 */

#include "evnt_macro.h"

#define BITS_IN_BITE 8

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

/*
 * This functions sets the bit of the higher order to one
 */
evnt_code_t set_bit(evnt_code_t val) {
    return val = val | 1UL << (BITS_IN_BITE * sizeof(evnt_code_t) - 1);
}

/*
 * This functions sets the bit of the higher order to zero
 */
evnt_code_t clear_bit(evnt_code_t val) {
    return val = val & ~(1UL << (BITS_IN_BITE * sizeof(evnt_code_t) - 1));
}

/*
 * This function returns the bit of the higher order
 */
uint8_t get_bit(evnt_code_t val){
    return (uint8_t) (val >> (BITS_IN_BITE * sizeof(evnt_code_t) - 1));
}
