/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdlib.h>

#include "evnt_macro.h"

#define BITS_IN_BITE 8

/*
 * This function returns the number of components in a particular event depending on the number of arguments
 */
evnt_size_t get_event_components(evnt_size_t nb_params) {
    // 4 is the size of tid, time, code, and nb_params
    return nb_params + 4;
}

/*
 * This function returns the size of a particular event in bytes depending on the number of arguments
 */
evnt_size_t get_event_size(evnt_size_t nb_params) {
    return sizeof(evnt_tid_t) + sizeof(evnt_time_t) + sizeof(evnt_code_t) + sizeof(evnt_size_t)
            + nb_params * sizeof(evnt_param_t);
}

/*
 * This function converts evnt's parameters to string. As a separator, space is used
 */
char* params_to_string(evnt_t* ev) {
    evnt_size_t i;
    int len;
    char* str;
    // sizeof(evnt_param_t) * EVNT_MAX_PARAMS is the maximum length of all parameters together
    str = malloc(sizeof(evnt_param_t) * EVNT_MAX_PARAMS);
    len = 0;

    for (i = 0; i < ev->nb_params; i++)
        if (i == 0)
            len += sprintf(str + len, "%lu", ev->param[i]);
        else
            len += sprintf(str + len, " %lu", ev->param[i]);

    return str;
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
uint8_t get_bit(evnt_code_t val) {
    return (uint8_t) (val >> (BITS_IN_BITE * sizeof(evnt_code_t) - 1));
}
