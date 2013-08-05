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
 * Returns the size of a particular event in bytes depending on
 * the number of arguments
 */
litl_size_t get_event_size(litl_size_t nb_params) {
    return nb_params * sizeof(litl_param_t)
            + (litl_size_t) ceil(LITL_BASE_SIZE / (double) sizeof(litl_param_t))
                    * sizeof(litl_param_t);
}

/*
 * Returns the size of a particular event in bytes depending on
 * the number of arguments
 */
litl_size_t get_event_size_type(litl_t *p_evt) {
    switch (p_evt->type) {
    case LITL_TYPE_REGULAR:
        return p_evt->parameters.regular.nb_params * sizeof(litl_param_t)
                + LITL_BASE_SIZE ;
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
 * Sets the bit of the higher order to one
 */
litl_code_t set_bit(litl_code_t val) {
    return val = val | 1UL << (BITS_IN_BITE * sizeof(litl_code_t) - 1);
}

/*
 * Sets the bit of the higher order to zero
 */
litl_code_t clear_bit(litl_code_t val) {
    return val = val & ~(1UL << (BITS_IN_BITE * sizeof(litl_code_t) - 1));
}

/*
 * Returns the bit of the higher order
 */
uint8_t get_bit(litl_code_t val) {
    return (uint8_t) (val >> (BITS_IN_BITE * sizeof(litl_code_t) - 1));
}
