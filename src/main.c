#include "bar.h"
#include "csv.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
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

// Parses text as a positive integer. Returns 0 on success, 1 if text is not entirely
// a whole number > 0 or is out of range.
static int parsePositiveSize(const char* text, size_t* out) {
    char* endptr;
    errno = 0;
    long value = strtol(text, &endptr, 10);
    // Must consume the whole (non-empty) string, e.g. reject "", "3x"
    if (endptr == text || *endptr != '\0') {
        return 1;
    }
    if (errno == ERANGE || value <= 0) {
        return 1;
    }
    *out = (size_t)value;
    return 0;
}

// Parses text as a positive dollar amount. Returns 0 on success, 1 if text is not
// entirely a finite number > 0.
static int parsePositiveAmount(const char* text, double* out) {
    char* endptr;
    errno = 0;
    double value = strtod(text, &endptr);
    // Must consume the whole (non-empty) string, e.g. reject "", "abc", "5x"
    if (endptr == text || *endptr != '\0') {
        return 1;
    }
    // Rejects overflow, inf, and NaN (NaN > 0 is false)
    if (errno == ERANGE || !isfinite(value) || !(value > 0)) {
        return 1;
    }
    *out = value;
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <csv-path> <window> <interval> <amount>\n", argv[0]);
        return 1;
    }
    char* csvData = argv[1];
    size_t window;
    size_t interval;
    double amount;
    if (parsePositiveSize(argv[2], &window) != 0) {
        fprintf(stderr, "Invalid window '%s': must be a positive integer\n", argv[2]);
        return 1;
    }
    if (parsePositiveSize(argv[3], &interval) != 0) {
        fprintf(stderr, "Invalid interval '%s': must be a positive integer\n", argv[3]);
        return 1;
    }
    if (parsePositiveAmount(argv[4], &amount) != 0) {
        fprintf(stderr, "Invalid amount '%s': must be a positive number\n", argv[4]);
        return 1;
    }
    struct Bar* bars = NULL; 
    size_t count;
    if (read_bars(csvData, &bars, &count) != 0) {
        fprintf(stderr, "CSV read failed\n");
        return 1;
    }
    if (count < window) {
        fprintf(stderr, "%s has only %zu bars, fewer than window %zu\n", csvData, count, window);
        free(bars);
        return 1;
    }
    // Simulate over the most recent window bars
    size_t start = count - window;
    size_t end = count;
    struct RunResult DCAResult;
    struct RunResult buyAndHoldResult;
    simulateDCA(bars, start, end, interval, amount, &DCAResult);
    if (buyAndHoldBenchmark(bars, start, end, DCAResult.totalInvested, &buyAndHoldResult) != 0) {
        fprintf(stderr, "Buy & hold benchmark failed\n");
        free(bars);
        return 1;
    }
    printf("%s: %s to %s (%zu bars), $%.2f every %zu bars\n", csvData, bars[start].timestamp,
           bars[end - 1].timestamp, window, amount, interval);
    printComparison(&DCAResult, &buyAndHoldResult);
    free(bars);
}