// Expected results for data/sample.csv, $100 per buy, interval 2, window = all 5 bars
// DCA:  buys at idx 0,2,4 (opens 100, 104, 107)
//       invested 300, shares 2.896117, avg cost 103.5870,
//       final value 315.6768, return 5.2256%
// Lump: 3 shares at 100, final value 327, return 9%

#include <math.h>
#include <stdio.h>
#include "bar.h"
#include "dca.h"

#define NUM_BARS 5

static int numChecks = 0;
static int numFailures = 0;

// Compares doubles with tolerance 1e-6, relative to expected once |expected| > 1
static void checkDouble(const char* name, double expected, double actual) {
    double scale = fabs(expected) > 1 ? fabs(expected) : 1;
    ++numChecks;
    if (fabs(expected - actual) <= 1e-6 * scale) {
        printf("PASS  %s\n", name);
    } else {
        ++numFailures;
        printf("FAIL  %s: expected %.10f, actual %.10f\n", name, expected, actual);
    }
}

// Compares counts and return codes exactly
static void checkSize(const char* name, size_t expected, size_t actual) {
    ++numChecks;
    if (expected == actual) {
        printf("PASS  %s\n", name);
    } else {
        ++numFailures;
        printf("FAIL  %s: expected %zu, actual %zu\n", name, expected, actual);
    }
}

// Checks every field of a successful run; prefix names the case in the output
static void checkResult(const char* prefix, int returnCode, const struct RunResult* expected,
                        const struct RunResult* actual) {
    char name[128];
    snprintf(name, sizeof name, "%s: returns 0", prefix);
    checkSize(name, 0, (size_t)returnCode);
    snprintf(name, sizeof name, "%s: numBuys", prefix);
    checkSize(name, expected->numBuys, actual->numBuys);
    snprintf(name, sizeof name, "%s: totalInvested", prefix);
    checkDouble(name, expected->totalInvested, actual->totalInvested);
    snprintf(name, sizeof name, "%s: totalShares", prefix);
    checkDouble(name, expected->totalShares, actual->totalShares);
    snprintf(name, sizeof name, "%s: avgCostPerShare", prefix);
    checkDouble(name, expected->avgCostPerShare, actual->avgCostPerShare);
    snprintf(name, sizeof name, "%s: finalValue", prefix);
    checkDouble(name, expected->finalValue, actual->finalValue);
    snprintf(name, sizeof name, "%s: rateOfReturn", prefix);
    checkDouble(name, expected->rateOfReturn, actual->rateOfReturn);
}

// Fills in the derived fields of an expected result from buys, invested, shares, final close
static struct RunResult expectedRun(size_t numBuys, double totalInvested, double totalShares,
                                    double finalClose) {
    struct RunResult expected;
    expected.numBuys = numBuys;
    expected.totalInvested = totalInvested;
    expected.totalShares = totalShares;
    expected.avgCostPerShare = totalInvested / totalShares;
    expected.finalValue = totalShares * finalClose;
    expected.rateOfReturn = expected.finalValue / totalInvested - 1;
    return expected;
}

// Same data as data/sample.csv
static void makeSampleBars(struct Bar* bars) {
    const double opens[NUM_BARS] = {100, 102, 104, 105, 107};
    const double highs[NUM_BARS] = {104, 106, 107, 108, 110};
    const double lows[NUM_BARS] = {98, 101, 103, 104, 105};
    const double closes[NUM_BARS] = {101, 105, 105, 106, 109};
    const double volumes[NUM_BARS] = {1000, 2000, 1500, 3000, 2500};
    for (size_t i = 0; i < NUM_BARS; ++i) {
        snprintf(bars[i].timestamp, sizeof bars[i].timestamp, "2026-09-%02zu", 14 + i);
        bars[i].open = opens[i];
        bars[i].high = highs[i];
        bars[i].low = lows[i];
        bars[i].close = closes[i];
        bars[i].volume = volumes[i];
    }
}

int main(void) {
    struct Bar bars[NUM_BARS];
    makeSampleBars(bars);
    struct RunResult result;
    struct RunResult expected;
    int returnCode;

    // a. Paper case: DCA $100 every 2 bars over all 5 bars (buys at bars 0, 2, 4)
    returnCode = simulateDCA(bars, 0, 5, 2, 100, &result);
    expected = expectedRun(3, 300, 100.0 / 100 + 100.0 / 104 + 100.0 / 107, 109);
    checkResult("a. DCA paper case", returnCode, &expected, &result);

    // b. Paper case: buy & hold $300 at bar 0's open
    returnCode = buyAndHoldBenchmark(bars, 0, 5, 300, &result);
    expected = expectedRun(1, 300, 3, 109);
    checkResult("b. buy & hold paper case", returnCode, &expected, &result);
    checkDouble("b. buy & hold paper case: avgCostPerShare is 100", 100, result.avgCostPerShare);
    checkDouble("b. buy & hold paper case: finalValue is 327", 327, result.finalValue);
    checkDouble("b. buy & hold paper case: rateOfReturn is 0.09", 0.09, result.rateOfReturn);

    // c. interval 1 buys at every bar
    returnCode = simulateDCA(bars, 0, 5, 1, 100, &result);
    expected = expectedRun(5, 500, 100.0 / 100 + 100.0 / 102 + 100.0 / 104 + 100.0 / 105 +
                                   100.0 / 107, 109);
    checkResult("c. DCA interval 1", returnCode, &expected, &result);

    // d. interval larger than the window buys only at bar 0
    returnCode = simulateDCA(bars, 0, 5, 10, 100, &result);
    expected = expectedRun(1, 100, 100.0 / 100, 109);
    checkResult("d. DCA interval > window", returnCode, &expected, &result);

    // e. one-bar window [2, 3): buy at open 104, valued at close 105
    returnCode = simulateDCA(bars, 2, 3, 1, 100, &result);
    expected = expectedRun(1, 100, 100.0 / 104, 105);
    checkResult("e. DCA one-bar window", returnCode, &expected, &result);

    // f. sub-window [1, 4) with interval 2: buys at bars 1 and 3, valued at bar 3's close
    returnCode = simulateDCA(bars, 1, 4, 2, 100, &result);
    expected = expectedRun(2, 200, 100.0 / 102 + 100.0 / 105, 106);
    checkResult("f. DCA sub-window", returnCode, &expected, &result);

    // g. Invalid inputs return 1
    checkSize("g. DCA NULL bars", 1, (size_t)simulateDCA(NULL, 0, 5, 2, 100, &result));
    checkSize("g. DCA NULL result", 1, (size_t)simulateDCA(bars, 0, 5, 2, 100, NULL));
    checkSize("g. DCA start == end", 1, (size_t)simulateDCA(bars, 3, 3, 2, 100, &result));
    checkSize("g. DCA start > end", 1, (size_t)simulateDCA(bars, 4, 2, 2, 100, &result));
    checkSize("g. DCA interval 0", 1, (size_t)simulateDCA(bars, 0, 5, 0, 100, &result));
    checkSize("g. DCA amount 0", 1, (size_t)simulateDCA(bars, 0, 5, 2, 0, &result));
    checkSize("g. DCA amount negative", 1, (size_t)simulateDCA(bars, 0, 5, 2, -100, &result));
    checkSize("g. DCA amount NaN", 1, (size_t)simulateDCA(bars, 0, 5, 2, NAN, &result));
    checkSize("g. buy & hold NULL bars", 1, (size_t)buyAndHoldBenchmark(NULL, 0, 5, 300, &result));
    checkSize("g. buy & hold NULL result", 1, (size_t)buyAndHoldBenchmark(bars, 0, 5, 300, NULL));
    checkSize("g. buy & hold start == end", 1,
              (size_t)buyAndHoldBenchmark(bars, 3, 3, 300, &result));
    checkSize("g. buy & hold start > end", 1,
              (size_t)buyAndHoldBenchmark(bars, 4, 2, 300, &result));
    checkSize("g. buy & hold amount 0", 1, (size_t)buyAndHoldBenchmark(bars, 0, 5, 0, &result));
    checkSize("g. buy & hold amount negative", 1,
              (size_t)buyAndHoldBenchmark(bars, 0, 5, -300, &result));
    checkSize("g. buy & hold amount NaN", 1,
              (size_t)buyAndHoldBenchmark(bars, 0, 5, NAN, &result));

    // h. Bad prices return 1
    struct Bar badBars[NUM_BARS];
    // Bar 2 is a DCA buy bar (interval 2) but not the buy & hold buy bar
    makeSampleBars(badBars);
    badBars[2].open = 0;
    checkSize("h. DCA buy bar 2 open 0", 1, (size_t)simulateDCA(badBars, 0, 5, 2, 100, &result));
    // Bar 0 is the first buy bar for both
    makeSampleBars(badBars);
    badBars[0].open = 0;
    checkSize("h. DCA buy bar 0 open 0", 1, (size_t)simulateDCA(badBars, 0, 5, 2, 100, &result));
    checkSize("h. buy & hold buy bar open 0", 1,
              (size_t)buyAndHoldBenchmark(badBars, 0, 5, 300, &result));
    makeSampleBars(badBars);
    badBars[NUM_BARS - 1].close = 0;
    checkSize("h. DCA final close 0", 1, (size_t)simulateDCA(badBars, 0, 5, 2, 100, &result));
    checkSize("h. buy & hold final close 0", 1,
              (size_t)buyAndHoldBenchmark(badBars, 0, 5, 300, &result));

    printf("\n%d checks, %d failed\n", numChecks, numFailures);
    return numFailures != 0;
}
