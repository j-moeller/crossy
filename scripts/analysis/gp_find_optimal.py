import argparse
import csv
import functools
import json
import os
import sys
from collections import defaultdict
from itertools import combinations
from multiprocessing import Pool

import tqdm
from common.util import (
    ErrorClasses,
    Stat,
    StatAggregation,
    get_diffs,
    process_outfile,
)


def process_rundir(
    rundir,
    analyzed_classes,
    n_jobs,
    tuple_size,
    aggregate,
    candidates=None,
    ignore_missing=False,
):
    with open(os.path.join(rundir, "runconfig.json")) as f:
        runconfig = json.load(f)

    gold_parsers = runconfig["gold_parsers"]

    outdir = os.path.join(rundir, "output")
    output_files = [
        os.path.join(outdir, d.job_id, d.hashname + ".out") for d in get_diffs(outdir)
    ]
    if ignore_missing:
        output_files = [o for o in output_files if os.path.exists(o)]

    n_output_files = sum(1 for _ in get_diffs(outdir))

    if aggregate:
        results = defaultdict(lambda: StatAggregation())
    else:
        results = defaultdict(lambda: Stat())

    if candidates is None:
        candidates = list(combinations(gold_parsers, tuple_size))
    else:
        all_candidates = set(combinations(gold_parsers, tuple_size))
        for c in candidates:
            assert c in all_candidates, f"{c} is not in {all_candidates}"

    with Pool(n_jobs) as p:
        fn = functools.partial(
            process_outfile, analyzed_classes=analyzed_classes, candidates=candidates
        )
        stats_list = p.imap(fn, list(output_files), chunksize=1024)

        if aggregate:
            for stats in tqdm.tqdm(stats_list, total=n_output_files):
                for k, v in stats.items():
                    results[k].tp += len(v.tp)
                    results[k].tn += len(v.tn)
                    results[k].fp += len(v.fp)
                    results[k].fn += len(v.fn)
        else:
            for stats in tqdm.tqdm(stats_list, total=n_output_files):
                for k, v in stats.items():
                    results[k].tp += v.tp
                    results[k].tn += v.tn
                    results[k].fp += v.fp
                    results[k].fn += v.fn

    for config, stat in results.items():
        yield config, stat


def list_stats(
    rundirs, analyzed_classes, n_jobs, tuple_size, candidates, ignore_missing
):
    csvwriter = csv.writer(sys.stdout)
    csvwriter.writerow(["run", "config", "tp", "fp", "tn", "fn"])
    for rundir in rundirs:
        for config, stat in process_rundir(
            rundir,
            analyzed_classes,
            n_jobs,
            tuple_size,
            aggregate=True,
            candidates=candidates,
            ignore_missing=ignore_missing,
        ):
            csvwriter.writerow([rundir, config, stat.tp, stat.fp, stat.tn, stat.fn])


def list_diffs(
    rundirs, analyzed_classes, n_jobs, tuple_size, candidates, ignore_missing
):
    csvwriter = csv.writer(sys.stdout)
    csvwriter.writerow(["run", "config", "input", "type"])
    for rundir in rundirs:
        for config, stat in process_rundir(
            rundir,
            analyzed_classes,
            n_jobs,
            tuple_size,
            aggregate=False,
            candidates=candidates,
            ignore_missing=ignore_missing,
        ):
            for tp in stat.tp:
                csvwriter.writerow([rundir, config, tp, "tp"])
            for fp in stat.fp:
                csvwriter.writerow([rundir, config, fp, "fp"])
            for fn in stat.fn:
                csvwriter.writerow([rundir, config, fn, "fn"])
            for tn in stat.tn:
                csvwriter.writerow([rundir, config, tn, "tn"])


def main():
    parser = argparse.ArgumentParser(
        description=(
            "This script is used to analyze a 'gold-selection-run' to find a tuple "
            "of parsers that create the best possible approximation of our entire "
            "set of parsers. This script has two modes: 'stats' and 'diffs'. The "
            "former is used to give a quick overview over the statistics for each "
            "configuration (i.e., tuple of parsers) and the latter is used to give "
            "a detailled list of all differences."
        )
    )
    parser.add_argument(
        "--no_tp",
        help="Do not consider true positives",
        action="store_false",
    )
    parser.add_argument(
        "--no_fp",
        help="Do not consider false positives",
        action="store_false",
    )
    parser.add_argument(
        "--no_tn",
        help="Do not consider true negatives",
        action="store_false",
    )
    parser.add_argument(
        "--no_fn",
        help="Do not consider false negatives",
        action="store_false",
    )
    parser.add_argument(
        "--ignore-missing",
        help="Parse only files that are available in the filesystem",
        action="store_true",
    )
    parser.add_argument("--candidate", help="Which candidate to evaluate", type=str)
    parser.add_argument(
        "--n_jobs",
        help="Number of parallel workers to start",
        type=int,
        required=True,
    )
    parser.add_argument(
        "--tuple-size",
        help="Number of gold parsers in a tuple",
        type=int,
        default=3,
    )

    subparsers = parser.add_subparsers(dest="mode")

    stat_parser = subparsers.add_parser("stats")
    stat_parser.add_argument("rundirs", help="List of parsers", type=str, nargs="+")

    diff_parser = subparsers.add_parser("diffs")
    diff_parser.add_argument("rundirs", help="List of parsers", type=str, nargs="+")

    args = parser.parse_args()

    analyzed_classes = ErrorClasses(
        tp=args.no_tp, fp=args.no_fp, fn=args.no_fn, tn=args.no_tn
    )

    if args.candidate is None:
        candidates = None
    else:
        candidates = [eval(args.candidate)]

    if args.mode == "stats":
        list_stats(
            rundirs=args.rundirs,
            n_jobs=args.n_jobs,
            tuple_size=args.tuple_size,
            candidates=candidates,
            analyzed_classes=analyzed_classes,
            ignore_missing=args.ignore_missing,
        )
    elif args.mode == "diffs":
        list_diffs(
            rundirs=args.rundirs,
            n_jobs=args.n_jobs,
            tuple_size=args.tuple_size,
            candidates=candidates,
            analyzed_classes=analyzed_classes,
            ignore_missing=args.ignore_missing,
        )


if __name__ == "__main__":
    main()
