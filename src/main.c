/*
 * Created by Roman Iakymchuk
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "evnt_types.h"
//#include "evnt_write.h"
//#include "evnt_read.h"

int main(int argc, const char * argv[]) {

    printf("Evnt Project\n");
    printf("int = %d\n", (int) sizeof(int));
    printf("long int = %d\n", (int) sizeof(long int));
    printf("uint64_t = %d\n", (int) sizeof(uint64_t));
    printf("long long int = %d\n", (int) sizeof(long long int));

    return EXIT_SUCCESS;
}
