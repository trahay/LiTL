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
litl_size_t __litl_get_reg_event_size(litl_data_t nb_params) {
  return LITL_BASE_SIZE + (nb_params * sizeof(litl_param_t)) + sizeof(litl_data_t);
}

litl_size_t __litl_get_event_size(litl_type_t type, int param_size) {
  switch (type) {
  case LITL_TYPE_REGULAR:
    return LITL_BASE_SIZE + (param_size * sizeof(litl_param_t)) + sizeof(litl_data_t);
  case LITL_TYPE_RAW:
    return LITL_BASE_SIZE + param_size + sizeof(((litl_t*)0)->parameters.raw.size);
  case LITL_TYPE_PACKED:
    return LITL_BASE_SIZE + param_size + sizeof(((litl_t*)0)->parameters.packed.size);
  case LITL_TYPE_OFFSET:
    return LITL_BASE_SIZE + param_size + sizeof(((litl_t*)0)->parameters.offset.nb_params);
  default:
    fprintf(stderr, "Unknown event type %d!\n", type);
    abort();
  }
  return 0;
}

/*
 * Returns the size in bytes of an event of any type
 */
litl_size_t __litl_get_gen_event_size(litl_t *p_evt) {
  switch (p_evt->type) {
  case LITL_TYPE_REGULAR:
    return __litl_get_event_size(p_evt->type, p_evt->parameters.regular.nb_params);
  case LITL_TYPE_RAW:
    return __litl_get_event_size(p_evt->type, p_evt->parameters.raw.size);
  case LITL_TYPE_PACKED:
    return __litl_get_event_size(p_evt->type, p_evt->parameters.packed.size);
  case LITL_TYPE_OFFSET:
    return __litl_get_event_size(p_evt->type, p_evt->parameters.offset.nb_params);
  default:
    fprintf(stderr, "Unknown event type %d!\n", p_evt->type);
    abort();
  }

  return 0;
}
