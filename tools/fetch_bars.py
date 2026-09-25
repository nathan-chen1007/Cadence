#!/usr/bin/env python3
"""Download daily OHLCV bars from Yahoo Finance (via yfinance) into Cadence's CSV format.

Output columns: timestamp,open,high,low,close,volume  (timestamp = YYYY-MM-DD)

Usage:
    python3 tools/fetch_bars.py SPY 5y data/SPY_5y.csv
    python3 tools/fetch_bars.py AAPL 10y data/AAPL_10y.csv --adjusted

period: 1mo 3mo 6mo 1y 2y 5y 10y ytd max
--adjusted: use dividend- and split-adjusted prices (closer to a real investor's total return).
"""
import argparse
import csv
import math
import sys

import yfinance as yf


def main():
    parser = argparse.ArgumentParser(description="Fetch daily bars into Cadence CSV format")
    parser.add_argument("ticker")
    parser.add_argument("period", help="e.g. 1y, 5y, 10y, max")
    parser.add_argument("out", help="output CSV path, e.g. data/SPY_5y.csv")
    parser.add_argument("--adjusted", action="store_true",
                        help="adjust prices for dividends and splits")
    args = parser.parse_args()

    df = yf.Ticker(args.ticker).history(period=args.period, interval="1d",
                                        auto_adjust=args.adjusted)
    if df.empty:
        print(f"No data returned for {args.ticker}", file=sys.stderr)
        return 1

    written = 0
    skipped = 0
    with open(args.out, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["timestamp", "open", "high", "low", "close", "volume"])
        for date, row in df.iterrows():
            values = [row["Open"], row["High"], row["Low"], row["Close"], row["Volume"]]
            if any(v is None or (isinstance(v, float) and math.isnan(v)) for v in values):
                skipped += 1
                continue
            w.writerow([
                date.strftime("%Y-%m-%d"),
                f"{row['Open']:.4f}",
                f"{row['High']:.4f}",
                f"{row['Low']:.4f}",
                f"{row['Close']:.4f}",
                int(row["Volume"]),
            ])
            written += 1

    print(f"Wrote {written} bars to {args.out}" + (f" (skipped {skipped} incomplete rows)" if skipped else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())