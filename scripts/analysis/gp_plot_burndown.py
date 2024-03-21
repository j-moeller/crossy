import argparse
import csv
import json
import os
import sys
from collections import defaultdict
from itertools import combinations
from math import comb
from pathlib import Path

import matplotlib.pyplot as plt
from common.style import apply_style, color_schema
from common.util import ErrorClasses, StatAggregation, get_diffs, process_outfile


def get_path(output_dir, diff):
    return os.path.join(output_dir, diff.job_id, diff.hashname)


def create_csv(rundir, ignore_missing, tuple_size):
    with open(os.path.join(rundir, "runconfig.json")) as f:
        runconfig = json.load(f)

    gold_parsers = runconfig["gold_parsers"]

    output_dir = os.path.join(rundir, "output")
    diffs = get_diffs(output_dir)

    if ignore_missing:
        diffs = filter(
            lambda x: os.path.exists(get_path(output_dir, x))
            and os.path.exists(get_path(output_dir, x) + ".out"),
            diffs,
        )

    analyzed_classes = ErrorClasses(tp=True, fp=True, tn=True, fn=True)
    candidates = list(combinations(gold_parsers, tuple_size))

    candidate_errors = defaultdict(lambda: StatAggregation())

    csvwriter = csv.writer(sys.stdout)
    csvwriter.writerow(["set_size", "wall_time"])

    last_line = None
    item_remaining = False

    for diff in diffs:
        outfile = get_path(output_dir, diff) + ".out"
        stats = process_outfile(outfile, analyzed_classes, candidates)

        for k, stat in stats.items():
            candidate_errors[k].tp += len(stat.tp)
            candidate_errors[k].tn += len(stat.tn)
            candidate_errors[k].fp += len(stat.fp)
            candidate_errors[k].fn += len(stat.fn)

        sort_by_errors = sorted(
            candidate_errors.items(), key=lambda x: x[1].fp + x[1].fn
        )

        lowest = sort_by_errors[0][1].fp + sort_by_errors[0][1].fn
        n_lowest = 0

        for candidate, stat in sort_by_errors:
            if stat.fn + stat.fp > lowest:
                break

            n_lowest += 1

        if last_line is None:
            csvwriter.writerow((n_lowest, diff.wall))
            item_remaining = False
        elif last_line[0] == n_lowest:
            item_remaining = True
        else:
            csvwriter.writerow(last_line)
            csvwriter.writerow((n_lowest, diff.wall))
            item_remaining = False

        last_line = (n_lowest, diff.wall)

    if item_remaining:
        csvwriter.writerow(last_line)


def plot_csv(csv_file, outfile, tuple_size):
    def seconds_to_time(seconds):
        hours = seconds // 3600
        return f"{hours:02d}:00"

    HEADER = ["set_size", "wall_time"]

    try:
        with open(Path(csv_file).parent / "runconfig.json") as f:
            config = json.load(f)
            n_parsers = len(config["configs"])
    except (FileNotFoundError, json.decoder.JSONDecodeError, KeyError):
        print("WARNING: Using n_parsers=22 as default")
        n_parsers = 22

    n_candidates = comb(n_parsers, tuple_size)
    time_values, count_values = [], []

    with open(csv_file, "r") as csvfile:
        csvreader = csv.reader(csvfile)
        assert (line := next(csvreader)) == HEADER, f"{line} vs {HEADER}"
        for i, row in enumerate(csvreader):
            if i == 0:
                time_values.append(int(row[1]))
                count_values.append(1)

            count_values.append(int(row[0]) / n_candidates)
            time_values.append(int(row[1]))

    time_values = [t_n - time_values[0] for t_n in time_values]

    # Plotting
    apply_style()

    colors_idxes = ["myblue"]

    fig, ax = plt.subplots(figsize=(4.2, 3))
    ax.grid(alpha=0.7)

    # ax.set_ylim([0, 10])

    ax.spines["right"].set_visible(False)
    ax.spines["top"].set_visible(False)
    ax.set_axisbelow(True)
    ax.set_xlabel("Time", fontsize="large")
    ax.set_ylabel("Candidates (\\%)", fontsize="large")

    ax.plot()

    ax.plot(
        time_values,
        count_values,
        #         label=label[i],
        color=color_schema[colors_idxes[0]],
        markersize=2.5,
        markevery=1,
        linewidth=1,
        linestyle="solid",
    )

    x_min = 0
    x_offset = 5 * 60
    x_max = 8 * 60 * 60

    # ax.set_xlim(x_min - x_offset, x_max + x_offset)

    # Format x-axis ticks as time strings
    time_ticks = range(0, x_max + 1, 2 * 60 * 60)  # 1 hour intervals
    ax.set_xticks(time_ticks)
    ax.set_xticklabels([seconds_to_time(t) for t in time_ticks])

    # Set axis labels and title
    # fig.legend(loc="center", fontsize="small", bbox_to_anchor=(0.57, 0.93), ncol=3)
    fig.tight_layout()
    plt.subplots_adjust(top=0.87)

    # Show the plot
    if outfile:
        plt.savefig(outfile)
    else:
        plt.show()


def main():
    parser = argparse.ArgumentParser(description="Foo")
    subparser = parser.add_subparsers(dest="mode")

    parser.add_argument(
        "--tuple-size",
        help="Number of parsers per candidate",
        type=int,
        default=3,
        required=True,
    )

    csv_parser = subparser.add_parser("csv")
    csv_parser.add_argument("rundir", help="Rundir", type=str)
    csv_parser.add_argument(
        "--ignore-missing",
        help="Parse only files that are available in the filesystem",
        action="store_true",
    )

    plot_parser = subparser.add_parser("plot")
    plot_parser.add_argument("csv_file", help="csv file to plot", type=str)
    plot_parser.add_argument("--outfile", help="where to save the image", type=str)

    args = parser.parse_args()

    if args.mode == "csv":
        create_csv(
            args.rundir, ignore_missing=args.ignore_missing, tuple_size=args.tuple_size
        )
    elif args.mode == "plot":
        plot_csv(args.csv_file, args.outfile, args.tuple_size)


if __name__ == "__main__":
    main()
