import argparse
import os.path

import matplotlib.pyplot as plt
from common.util import merge_sorted, parse_difflog, unique_sorted


def main(rundir, outfile=None):
    outdir = os.path.join(rundir, "output")

    diff_logs = []
    for job_id in os.listdir(outdir):
        job_dir = os.path.join(outdir, job_id)
        diff_log_path = os.path.join(job_dir, "diff-log.txt")

        diff_log = parse_difflog(diff_log_path)
        diff_logs.append(diff_log)

    diffs_sorted = merge_sorted(*diff_logs, key=lambda x: x.user[0])
    diffs_unique = unique_sorted(diffs_sorted, key=lambda x: x.hashname)
    # diffs_deduplicate = filter_cracks(enumerate(diffs_unique), key=lambda x: x[0])

    x = []
    y = []

    for i, diff in enumerate(diffs_unique):
        x.append(diff.user[0])
        y.append(i)

    plt.plot(x, y)

    if outfile is None:
        plt.show()
    else:
        plt.savefig(outfile)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Foo")
    parser.add_argument("rundir", help="List of parsers", type=str)
    parser.add_argument("--outfile", help="List of parsers", type=str)
    args = parser.parse_args()

    main(args.rundir, outfile=args.outfile)
