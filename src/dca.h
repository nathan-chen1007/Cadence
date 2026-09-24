#ifndef DCA_H
#define DCA_H
#include <stddef.h>
#include "bar.h"
#include <stdio.h>

struct RunResult {
    size_t numBuys;
    double totalInvested;
    double totalShares;
    double avgCostPerShare;
    double finalValue;
    double rateOfReturn;
};

// Simulates buying dollarAmount at the open of bars start, start+interval, ... (< end),
// valued at the close of bar end-1. Caller must ensure end <= number of bars.
// Returns 0 on success, 1 on invalid input, including any buy bar's open or bar end-1's
// close that is not a finite number > 0. On failure, *simulationResult is all zeros.
int simulateDCA(const struct Bar* bars, size_t start, size_t end, size_t interval, 
                double dollarAmount, struct RunResult* simulationResult);


#endif