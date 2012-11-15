/*
 * Created by Roman Iakymchuk
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "evnt_types.h"
#include "evnt_write.h"
#include "evnt_read.h"

int main(int argc, const char * argv[]) {

    open_trace("test_evnt_write.trace");

    return EXIT_SUCCESS;
}
