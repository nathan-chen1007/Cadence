#include "dca.h"
#include <math.h>

// Checks shared by simulateDCA and buyAndHoldBenchmark. Zeroes *result once it is
// known to be non-NULL. Returns 0 if the inputs are valid, 1 otherwise.
static int validateRunInputs(const struct Bar* bars, size_t start, size_t end,
                             double amount, struct RunResult* result) {
    // Nowhere to read from or write to
    if (bars == NULL || result == NULL) {
        return 1;
    }
    // Start from a clean result so no field is left with garbage, even on error
    *result = (struct RunResult){0};
    // Window [start, end) must contain at least one bar
    if (start >= end) {
        return 1;
    }
    // Must invest a positive amount (also rejects NaN, since NaN > 0 is false)
    if (!(amount > 0)) {
        return 1;
    }
    // Final close must be a finite positive price to value the shares
    if (!isfinite(bars[end - 1].close) || !(bars[end - 1].close > 0)) {
        return 1;
    }
    return 0;
}

int simulateDCA(const struct Bar* bars, size_t start, size_t end, size_t interval,
                 double dollarAmount, struct RunResult* simulationResult) {
    if (validateRunInputs(bars, start, end, dollarAmount, simulationResult) != 0) {
        return 1;
    }
    // interval 0 would never advance to the next buy
    if (interval == 0) {
        return 1;
    }
    // Every buy bar's open must be a finite positive price; reject the run
    // rather than skip the bar, which would change the buy schedule
    for (size_t i = start; i < end; i += interval) {
        if (!isfinite(bars[i].open) || !(bars[i].open > 0)) {
            return 1;
        }
    }

    simulationResult->totalInvested = 0;
    simulationResult->numBuys = 0;
    simulationResult->totalShares = 0;
    for (size_t i = start; i < end; i += interval) {
        simulationResult->totalInvested += dollarAmount;
        ++(simulationResult->numBuys);
        simulationResult->totalShares += dollarAmount / bars[i].open;
    }
    simulationResult->avgCostPerShare = simulationResult->totalInvested / 
                                        simulationResult->totalShares;
    simulationResult->finalValue = simulationResult->totalShares * 
                                   bars[end - 1].close;
    simulationResult->rateOfReturn = (simulationResult->finalValue - 
                                      simulationResult->totalInvested) /
                                     simulationResult->totalInvested;

    return 0;
}

int buyAndHoldBenchmark(const struct Bar* bars, size_t start, size_t end, 
                        double buyAmount, struct RunResult* buyAndHoldResult) {
    if (validateRunInputs(bars, start, end, buyAmount, buyAndHoldResult) != 0) {
        return 1;
    }
    // The single buy bar's open must be a finite positive price
    if (!isfinite(bars[start].open) || !(bars[start].open > 0)) {
        return 1;
    }
    buyAndHoldResult->avgCostPerShare = bars[start].open;
    buyAndHoldResult->numBuys = 1;
    buyAndHoldResult->totalInvested = buyAmount;
    buyAndHoldResult->totalShares = buyAmount / bars[start].open;
    buyAndHoldResult->finalValue = bars[end - 1].close * buyAndHoldResult->totalShares;
    buyAndHoldResult->rateOfReturn = (buyAndHoldResult->finalValue - buyAmount) / buyAmount;
    return 0;
}