#include "csv.h"
#include <stdio.h>
#include <stdlib.h>

int read_bars(const char *path, struct Bar **out, size_t *count) {
    // local variables to help keep out clean 
    struct Bar* bars = NULL; 
    size_t bars_count = 0;

    FILE *f = fopen(path, "r");
	size_t bars_max = 0; // how many fit before you must grow
	if (f == NULL) {
		perror("fopen");
		return 1;
	}
	// skip the header line (the column labels)
	// the read itself will skip the header line
	// if the header line is there, it will succeed and skip it
	// if it gets some bullshit (EOF for ex), it will early return
    char line[256];
	if (fgets(line, sizeof line, f) == NULL) {
		fclose(f);
		return 1;
	}
	while(fgets(line, sizeof line, f) != NULL) {
		// this loop prints the typical_price of each bar if it's a valid bar
		struct Bar b;
		int got = sscanf(line, "%31[^,], %lf, %lf, %lf, %lf, %lf,",
						 b.timestamp, &b.open, &b.high, &b.low, &b.close, &b.volume);
		if (got != 6) {
			printf("Input line is not valid bar\n");
			continue; 
		}
		// doubling method for dynamic array
		if (bars_count == bars_max) {
			if (bars_max == 0) {
				bars_max = 2;
			} else {
				bars_max *= 2;
			}
			// only reason I am doing is this is for if the realloc fails, very unlikely
			// safe coding practice
			struct Bar* tmp = realloc(bars, bars_max * sizeof(struct Bar));
			if (tmp == NULL) {
				fclose(f);
				free(bars);
				return 1;
			} else {
				bars = tmp;
			}
		}
		bars[bars_count] = b;
		bars_count++;
	}
	fclose(f);
    *out = bars; // everything worked, so set your actual out to the local
    *count = bars_count; // hand the count to the caller 
    return 0; 
}