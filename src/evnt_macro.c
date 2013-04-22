/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdlib.h>
#include <inttypes.h>
#include <math.h>

#include "evnt_macro.h"
#include "evnt_write.h"

#define BITS_IN_BITE 8

/*
 * This function returns the number of components in a particular event depending on the number of arguments
 */
evnt_size_t get_event_components(evnt_size_t nb_params) {
    // ceil(...) corresponds to the memory slot occupied by tid (evnt_tid_t), time (evnt_time_t), code (evnt_code_t),
    // type (evnt_type_t) and nb_params (evnt_size_t). And, it equals to n * sizeof(evnt_param_t)
    return nb_params + (evnt_size_t) ceil(EVNT_BASE_SIZE / (double) sizeof(evnt_param_t));
}

/*
 * This function returns the size of a particular event in bytes depending on the number of arguments
 */
evnt_size_t get_event_size(evnt_size_t nb_params) {
    return nb_params * sizeof(evnt_param_t)
            + (evnt_size_t) ceil(EVNT_BASE_SIZE / (double) sizeof(evnt_param_t)) * sizeof(evnt_param_t);
}

/*
 * This function returns the size of a particular event in bytes depending on the number of arguments
 */
evnt_size_t get_event_size_type(evnt_t *p_evt) {
    switch (p_evt->type) {
    case EVNT_TYPE_REGULAR:
        return p_evt->parameters.regular.nb_params * sizeof(evnt_param_t)
                + (evnt_size_t) ceil(EVNT_BASE_SIZE / (double) sizeof(evnt_param_t)) * sizeof(evnt_param_t);
    case EVNT_TYPE_RAW:
        return p_evt->parameters.raw.size + EVNT_BASE_SIZE ;
    case EVNT_TYPE_PACKED:
        return p_evt->parameters.packed.size + EVNT_BASE_SIZE ;
    default:
        fprintf(stderr, "Unknown event type %d!\n", p_evt->type);
        abort();
    }

    return 0;
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

    for (i = 0; i < ev->parameters.regular.nb_params; i++)
        if (i == 0)
            len += sprintf(str + len, "%"PRIu64, ev->parameters.regular.param[i]);
        else
            len += sprintf(str + len, "%"PRIu64, ev->parameters.regular.param[i]);

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
