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

    if not rows:
        ap.error("benchmark CSV has no data rows")

    points = sorted((int(row["threads"]), float(row["speedup"])) for row in rows)
    threads, speedups = zip(*points)

    import matplotlib.pyplot as plt

    fig, ax = plt.subplots()
    ax.plot(threads, speedups, "o-", label="Measured")
    ax.plot(threads, threads, "--", label="Ideal")
    ax.set(xlabel="Workers", ylabel="Speedup", title="Sweep parallel speedup")
    ax.set_xticks(threads)
    ax.grid(True)
    ax.legend()
    fig.tight_layout()
    fig.savefig(args.out)
    plt.close(fig)


if __name__ == "__main__":
    main()
