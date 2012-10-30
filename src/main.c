/*
 * Created by Roman Iakymchuk
 *
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char * argv[]) {
    FILE *fp;
    int num;

    fp = fopen("/sys/devices/system/cpu/cpu0/cache/index2/size", "r");

    fscanf(fp, "%d", &num);
    printf("L2 cache size is %d KB\n", num);

    fclose(fp);

    return EXIT_SUCCESS;
}
