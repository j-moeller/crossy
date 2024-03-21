import argparse
import json
import os
from collections import defaultdict
from itertools import combinations

from common.util import ErrorClasses, StatAggregation, get_diffs


def get_path(output_dir, diff):
    return os.path.join(output_dir, diff.job_id, diff.hashname)


def plot_burnout(rundir, ignore_missing, tuple_size):
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

    # Rank differences according to the number of disagreements:
    # 1 parser says its a difference vs all parsers say its a difference


def main():
    parser = argparse.ArgumentParser(description="Foo")
    parser.add_argument("rundir", help="Rundir", type=str)
    parser.add_argument(
        "--only-available",
        help="Parse only files that are available in the filesystem",
        action="store_true",
    )
    parser.add_argument(
        "--tuple-size",
        help="Number of parsers per candidate",
        type=int,
        required=True,
    )
    args = parser.parse_args()

    plot_burnout(
        args.rundir, ignore_missing=args.ignore_missing, tuple_size=args.tuple_size
    )


if __name__ == "__main__":
    main()
