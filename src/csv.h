#ifndef CSV_H
#define CSV_H
#include "bar.h"
#include <stddef.h>

// helper function to get the bars from the csv file into an array
// mutates out to hold the array of bars
int read_bars(const char *path, struct Bar **out, size_t *count);

#endif