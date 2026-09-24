#include "bar.h"
#include "csv.h"
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    struct Bar* bars = NULL; 
    size_t count;
    if (read_bars("data/sample.csv", &bars, &count) != 0) {
        fprintf(stderr, "CSV read failed\n");
        return 1;
    }
    printf("%zu\n", count);
    free(bars);
}