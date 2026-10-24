#!/usr/bin/env python3
"""Plot the bench CSV (threads, seconds, speedup) as a speedup graph.

usage: plot_speedup.py results/speedup.csv --out results/speedup.png
"""
import argparse
import csv


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("csv")
    ap.add_argument("--out", required=True)
    args = ap.parse_args()

    with open(args.csv, newline="") as f:
        rows = list(csv.DictReader(f))

    # TODO: plot measured speedup vs threads, with the ideal line y = x.
    import matplotlib.pyplot as plt  # noqa: F401
    print(f"read {len(rows)} rows; plotting not implemented yet")


if __name__ == "__main__":
    main()
