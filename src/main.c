#include "bar.h"
#include "csv.h"
#include <stdlib.h>
#include <stdio.h>
#include "dca.h"

// Prints DCA and buy & hold results side by side, then the difference in return
static void printComparison(const struct RunResult* DCAResult,
                            const struct RunResult* buyAndHoldResult) {
    printf("%-16s%9s%13s\n", "", "DCA", "Buy & Hold");
    printf("%-16s%9zu%13zu\n", "Buys", DCAResult->numBuys, buyAndHoldResult->numBuys);
    printf("%-16s%9.2f%13.2f\n", "Invested ($)",
           DCAResult->totalInvested, buyAndHoldResult->totalInvested);
    printf("%-16s%9.6f%13.6f\n", "Shares",
           DCAResult->totalShares, buyAndHoldResult->totalShares);
    printf("%-16s%9.4f%13.4f\n", "Avg cost ($)",
           DCAResult->avgCostPerShare, buyAndHoldResult->avgCostPerShare);
    printf("%-16s%9.2f%13.2f\n", "Final value ($)",
           DCAResult->finalValue, buyAndHoldResult->finalValue);
    // rateOfReturn is a fraction; scale to percent only for display
    printf("%-16s%8.2f%%%12.2f%%\n", "Return",
           DCAResult->rateOfReturn * 100, buyAndHoldResult->rateOfReturn * 100);
    printf("DCA vs Buy & Hold: %.2f percentage points\n",
           (DCAResult->rateOfReturn - buyAndHoldResult->rateOfReturn) * 100);
}

int main(void) {
    struct Bar* bars = NULL; 
    size_t count;
    if (read_bars("data/sample.csv", &bars, &count) != 0) {
        fprintf(stderr, "CSV read failed\n");
        return 1;
    }
    struct RunResult DCAResult;
    struct RunResult buyAndHoldResult;
    simulateDCA(bars, 0, 5, 2, 100, &DCAResult);
    if (buyAndHoldBenchmark(bars, 0, 5, DCAResult.totalInvested, &buyAndHoldResult) != 0) {
        fprintf(stderr, "Buy & hold benchmark failed\n");
        free(bars);
        return 1;
    }
    printComparison(&DCAResult, &buyAndHoldResult);
    free(bars);
}