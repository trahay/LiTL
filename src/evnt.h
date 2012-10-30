/*
 * evnt.h
 *
 *  Created on: Oct 29, 2012
 *      Author: Roman Iakymchuk
 */

#ifndef EVNT_H_
#define EVNT_H_

#include <stdint.h>

#define EVNT_MAX_PARAMS 6

struct evnt_64 {
    uint64_t time; // time of the measurement
    uint64_t code; // code of the event
    uint64_t nb_args; // number of arguments
    uint64_t args[EVNT_MAX_PARAMS]; // array of arguments; the array is of lengths from 0 to 6
};

enum evnt_flags {
    EVNT_FLUSH, EVNT_NOFLUSH
};

#endif /* EVNT_H_ */
