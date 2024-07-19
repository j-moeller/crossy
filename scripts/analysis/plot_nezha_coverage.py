import argparse
import csv
import os

import matplotlib.pyplot as plt
import numpy as np
from common.style import apply_style, color_schema


def interpolate(x, y, new_x):
    """
    Interpolate y values for given x values and new_x values.
    """
    return np.interp(new_x, x, y)


def average_graph(graphs):
    """
    Create an average graph and calculate standard deviation.
    """
    # Find the common x values for all graphs
    common_x = np.unique(np.concatenate([graph[:, 0] for graph in graphs]))

    # Interpolate y values for each graph at common x values
    interpolated_graphs = [
        interpolate(graph[:, 0], graph[:, 1], common_x) for graph in graphs
    ]

    # Calculate the average y values
    avg_y = np.mean(interpolated_graphs, axis=0)

    # Calculate standard deviation between original and interpolated graphs
    std_dev = np.std(avg_y - interpolated_graphs, axis=0)

    return common_x, avg_y, std_dev


def seconds_to_time(seconds):
    hours = seconds // 3600
    return f"{hours:02d}:00"


def main(rundir, outfile):
    outdir = os.path.join(rundir, "output")

    graphs_coverage, graphs_fine, graphs_coarse = [], [], []

    min_time = None

    for job_id in os.listdir(outdir):
        job_dir = os.path.join(outdir, job_id)
        coverage_log_file = os.path.join(job_dir, "coverage-log")

        graph_coverage, graph_coarse, graph_fine = [], [], []

        with open(coverage_log_file) as f:
            csvreader = csv.reader(f)
            next(csvreader)

            for i, line in enumerate(csvreader):
                time, counter, *parser_coverage, nezha_coarse, nezha_fine = line

                time = int(time)
                nezha_coarse = int(nezha_coarse)
                nezha_fine = int(nezha_fine)

                if min_time is None:
                    min_time = time
                else:
                    min_time = min(time, min_time)

                # graph_coverage.append([time, sum([int(cov) for cov in parser_coverage])])
                graph_coarse.append([time, nezha_coarse])
                graph_fine.append([time, nezha_fine])

        # graphs_coverage.append(np.array(graph_coverage))
        graphs_coarse.append(np.array(graph_coarse))
        graphs_fine.append(np.array(graph_fine))

    apply_style()

    colors_idxes = ["myred", "myblue"]

    fig, ax = plt.subplots(figsize=(4.2, 3))
    ax.grid(alpha=0.7)

    # ax.set_ylim([0, 10])

    ax.spines["right"].set_visible(False)
    ax.spines["top"].set_visible(False)
    ax.set_xlabel("Time", fontsize="large")
    ax.set_ylabel("Path $\\delta$-diversity", fontsize="large")
    ax.set_axisbelow(True)

    # ax.set_xticks([np.min(X), np.max(X)])
    # ax.set_yticks([np.min(Y), np.max(Y)])

    label = ["coarse", "fine"]

    for i, graphs in enumerate([graphs_coarse, graphs_fine]):
        X, Y, Y_std = average_graph(graphs)

        X = X - min_time

        ax.plot(
            X,
            Y,
            label=label[i],
            color=color_schema[colors_idxes[i]],
            markersize=2.5,
            markevery=1,
            linewidth=1,
            linestyle="solid",
        )
        ax.fill_between(
            X,
            np.array(Y) - np.array(Y_std),
            np.array(Y) + np.array(Y_std),
            color=color_schema[colors_idxes[i]],
            alpha=0.1,
        )

    x_max = 24 * 60 * 60

    time_ticks = range(0, x_max + 1, 4 * 60 * 60)  # 1 hour intervals
    ax.set_xticks(time_ticks)
    ax.set_xticklabels([seconds_to_time(t) for t in time_ticks])

    ax.set_yticks(range(0, 40001, 10000))
    # ax.set_yticklabels(["0", "10,000", "20,000", "30,000", "40,000"])
    ax.set_yticklabels(["0", "10k", "20k", "30k", "40k"])

    fig.legend(loc="center", fontsize="small", bbox_to_anchor=(0.57, 0.93), ncol=3)
    fig.tight_layout()
    plt.subplots_adjust(top=0.87)

    if outfile:
        plt.savefig(outfile)
    else:
        plt.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Foo")
    parser.add_argument("rundir", help="List of parsers", type=str)
    parser.add_argument("--outfile", help="List of parsers", type=str)
    args = parser.parse_args()

    main(args.rundir, outfile=args.outfile)
