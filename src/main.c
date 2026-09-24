#include "bar.h"
#include "csv.h"
#include <stdlib.h>
#include <stdio.h>
#include "dca.h"

int main(void) {
    struct Bar* bars = NULL; 
    size_t count;
    if (read_bars("data/sample.csv", &bars, &count) != 0) {
        fprintf(stderr, "CSV read failed\n");
        return 1;
    }
    struct RunResult result;
    simulateDCA(bars, 0, 5, 2, 100, &result);
    printf("Total Invested: %f\n", result.totalInvested);
    printf("Number of buys: %zu\n", result.numBuys);
    printf("Total Shares: %f\n", result.totalShares);
    printf("Value of assets: %f\n", result.finalValue);
    printf("Average cost per share: %f\n", result.avgCostPerShare);
    printf("Rate of return: %f\n", result.rateOfReturn);
    free(bars);
}