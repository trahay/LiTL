/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdlib.h>
#include <inttypes.h>
#include <math.h>

#include "litl_macro.h"
#include "litl_write.h"

#define BITS_IN_BITE 8

/*
 * This function returns the number of components in a particular event depending on the number of arguments
 */
litl_size_t get_event_components(litl_size_t nb_params) {
    // ceil(...) corresponds to the memory slot occupied by tid (litl_tid_t), time (litl_time_t), code (litl_code_t),
    // type (litl_type_t) and nb_params (litl_size_t). And, it equals to n * sizeof(litl_param_t)
    return nb_params + (litl_size_t) ceil(LITL_BASE_SIZE / (double) sizeof(litl_param_t));
}

/*
 * This function returns the size of a particular event in bytes depending on the number of arguments
 */
litl_size_t get_event_size(litl_size_t nb_params) {
    return nb_params * sizeof(litl_param_t)
            + (litl_size_t) ceil(LITL_BASE_SIZE / (double) sizeof(litl_param_t)) * sizeof(litl_param_t);
}

/*
 * This function returns the size of a particular event in bytes depending on the number of arguments
 */
litl_size_t get_event_size_type(litl_t *p_evt) {
    switch (p_evt->type) {
    case LITL_TYPE_REGULAR:
        return p_evt->parameters.regular.nb_params * sizeof(litl_param_t) + LITL_BASE_SIZE ;
    case LITL_TYPE_RAW:
        return p_evt->parameters.raw.size + LITL_BASE_SIZE + 7;
    case LITL_TYPE_PACKED:
        return p_evt->parameters.packed.size + LITL_BASE_SIZE ;
    default:
        fprintf(stderr, "Unknown event type %d!\n", p_evt->type);
        abort();
    }

    return 0;
}

/*
 * This function converts evnt's parameters to string. As a separator, space is used
 */
char* params_to_string(litl_t* ev) {
    litl_size_t i;
    int len;
    char* str;
    // sizeof(litl_param_t) * LITL_MAX_PARAMS is the maximum length of all parameters together
    str = malloc(sizeof(litl_param_t) * LITL_MAX_PARAMS);
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
litl_code_t set_bit(litl_code_t val) {
    return val = val | 1UL << (BITS_IN_BITE * sizeof(litl_code_t) - 1);
}

/*
 * This functions sets the bit of the higher order to zero
 */
litl_code_t clear_bit(litl_code_t val) {
    return val = val & ~(1UL << (BITS_IN_BITE * sizeof(litl_code_t) - 1));
}

/*
 * This function returns the bit of the higher order
 */
uint8_t get_bit(litl_code_t val) {
    return (uint8_t) (val >> (BITS_IN_BITE * sizeof(litl_code_t) - 1));
}
