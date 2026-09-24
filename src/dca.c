#include "dca.h"
#include <math.h>

int simulateDCA(const struct Bar* bars, size_t start, size_t end, size_t interval,
                 double dollarAmount, struct RunResult* simulationResult) {
    // Nowhere to read from or write to
    if (bars == NULL || simulationResult == NULL) {
        return 1;
    }
    // Start from a clean result so no field is left with garbage, even on error
    *simulationResult = (struct RunResult){0};
    // Window [start, end) must contain at least one bar
    if (start >= end) {
        return 1;
    }
    // interval 0 would never advance to the next buy
    if (interval == 0) {
        return 1;
    }
    // Must invest a positive amount (also rejects NaN, since NaN > 0 is false)
    if (!(dollarAmount > 0)) {
        return 1;
    }
    // Final close must be a finite positive price to value the shares
    if (!isfinite(bars[end - 1].close) || !(bars[end - 1].close > 0)) {
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