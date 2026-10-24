#!/usr/bin/env python3
"""Plot one metric from a sweep CSV against one parameter.

usage: plot_sweep.py results/matmul_l2.csv --x l2.size_bytes --y total_cycles --out results/matmul_l2.png
"""
import argparse
import csv


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("csv")
    ap.add_argument("--x", required=True, help="parameter name inside the label column")
    ap.add_argument("--y", required=True, help="metric column")
    ap.add_argument("--out", required=True)
    args = ap.parse_args()

    with open(args.csv, newline="") as f:
        rows = list(csv.DictReader(f))

    # TODO: parse args.x from each row's "label" ("key=value,key=value"),
    # group or sort by it, and plot args.y with matplotlib.
    import matplotlib.pyplot as plt  # noqa: F401
    print(f"read {len(rows)} rows; plotting not implemented yet")


if __name__ == "__main__":
    main()
