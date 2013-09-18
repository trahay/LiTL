/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdlib.h>
#include <inttypes.h>
#include <math.h>

#include "litl_tools.h"
#include "litl_write.h"

/*
 * Returns the size in bytes of a REGULAR event depending on
 *   the number of arguments
 */
litl_med_size_t __litl_get_reg_event_size(litl_data_t nb_params) {
  return nb_params * sizeof(litl_param_t)
      + (litl_med_size_t) ceil(LITL_BASE_SIZE / (double) sizeof(litl_param_t))
          * sizeof(litl_param_t);
}

/*
 * Returns the size in bytes of an event of any type
 */
litl_med_size_t __litl_get_gen_event_size(litl_t *p_evt) {
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
