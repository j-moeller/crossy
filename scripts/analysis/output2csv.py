import argparse
import csv
import functools
import json
import multiprocessing
import os
import os.path
import sys
from dataclasses import dataclass
from pathlib import Path

import tqdm
from common.diff import (
    ParserErrorBoth,
    ParserErrorOne,
    are_equal,
)
from common.util import Diff, parse_difflog, parse_outfile

"""
This file prints a csv file (to stdout) which contains difference information:
Input,Target1,Target2,Reason,Type1,Type2

where
 - Input is the filepath to the input
 - TargetX is the filepath to the config for target X
 - Reason is reason for the difference (see are_equal)
 - Type1 / Type2 are the types of the object where the difference occurred

usage: python scripts/gen-csv-overview.py <dir> > output/diffs.csv
"""

EXCLUDE_LESS_IMPORTANT = True


@dataclass
class Item:
    in_file: str
    time: int


@dataclass
class DiffMeta:
    in_file: str
    out_file: str
    diff: Diff


def parse_outputs(outputs, in_file):
    for i in range(len(outputs)):
        target1, json_obj1, is_parsable_1, exc_1, orig_1 = outputs[i]
        for j in range(i, len(outputs)):
            target2, json_obj2, is_parsable_2, exc_2, orig_2 = outputs[j]

            if orig_1 == orig_2 and len(orig_1) > 0:
                continue

            # both reject to parse
            if not is_parsable_1 and not is_parsable_2:
                if exc_1 is not None and exc_2 is not None:
                    exp = ParserErrorBoth(exc_1, exc_2)
                    if exp.msg1 == "Expecting value" and exp.msg2 == "Expecting value":
                        pass
                    else:
                        yield target1, target2, exp

                elif exc_1 is not None:
                    yield target1, target2, ParserErrorOne(exc_1)
                elif exc_2 is not None:
                    yield target1, target2, ParserErrorOne(exc_2)
                else:
                    yield target1, target2, ParserErrorBoth(None, None)
                continue

            # only one rejects to parse
            if not is_parsable_1:
                yield target1, target2, ParserErrorOne(exc_1)
                continue

            if not is_parsable_2:
                yield target1, target2, ParserErrorOne(exc_2)
                continue

            ok, error = are_equal(json_obj1, json_obj2, in_file)

            # TODO: What happens if ok=True
            # That means we found an error before, but not anymore.
            # better have a look at these inputs

            if not ok:
                yield target1, target2, error


class QuickFixError(RuntimeError):
    def __init__(self, msg):
        self.msg = msg


def process_filename(diff_meta, exclude_parsers=None):
    outputs = []
    for parser, output in parse_outfile(diff_meta.out_file):
        if len(parser) != 1:
            # throw away gold parser output, we'll normalize with python's
            # json parser
            continue

        parser = parser[0]

        if exclude_parsers is not None:
            if parser in exclude_parsers:
                continue

        if False and parser == "configs/json/c-jsonh.cfg":
            output_start = output

            output = output.replace(b"\x01", b"\\u0001")
            output = output.replace(b"\x02", b"\\u0002")
            output = output.replace(b"\x03", b"\\u0003")
            output = output.replace(b"\x04", b"\\u0004")
            output = output.replace(b"\x05", b"\\u0005")
            output = output.replace(b"\x06", b"\\u0006")
            output = output.replace(b"\x07", b"\\u0007")

            output = output.replace(b"\x0b", b"\\u000b")

            output = output.replace(b"\x0e", b"\\u000e")
            output = output.replace(b"\x0f", b"\\u000f")
            output = output.replace(b"\x10", b"\\u0010")
            output = output.replace(b"\x11", b"\\u0011")
            output = output.replace(b"\x12", b"\\u0012")
            output = output.replace(b"\x13", b"\\u0013")
            output = output.replace(b"\x14", b"\\u0014")
            output = output.replace(b"\x15", b"\\u0015")
            output = output.replace(b"\x16", b"\\u0016")
            output = output.replace(b"\x17", b"\\u0017")
            output = output.replace(b"\x18", b"\\u0018")
            output = output.replace(b"\x19", b"\\u0019")
            output = output.replace(b"\x1a", b"\\u001a")
            output = output.replace(b"\x1b", b"\\u001b")
            output = output.replace(b"\x1c", b"\\u001c")
            output = output.replace(b"\x1d", b"\\u001d")
            output = output.replace(b"\x1e", b"\\u001e")
            output = output.replace(b"\x1f", b"\\u001f")

        if False and parser == "configs/json/c-json-parser.cfg":
            output = output.replace(b"\x01", b"\\u0001")
            output = output.replace(b"\x02", b"\\u0002")
            output = output.replace(b"\x03", b"\\u0003")
            output = output.replace(b"\x04", b"\\u0004")
            output = output.replace(b"\x05", b"\\u0005")
            output = output.replace(b"\x06", b"\\u0006")
            output = output.replace(b"\x07", b"\\u0007")

            output = output.replace(b"\x0b", b"\\u000b")

            output = output.replace(b"\x0e", b"\\u000e")
            output = output.replace(b"\x0f", b"\\u000f")
            output = output.replace(b"\x10", b"\\u0010")
            output = output.replace(b"\x11", b"\\u0011")
            output = output.replace(b"\x12", b"\\u0012")
            output = output.replace(b"\x13", b"\\u0013")
            output = output.replace(b"\x14", b"\\u0014")
            output = output.replace(b"\x15", b"\\u0015")
            output = output.replace(b"\x16", b"\\u0016")
            output = output.replace(b"\x17", b"\\u0017")
            output = output.replace(b"\x18", b"\\u0018")
            output = output.replace(b"\x19", b"\\u0019")
            output = output.replace(b"\x1a", b"\\u001a")
            output = output.replace(b"\x1b", b"\\u001b")
            output = output.replace(b"\x1c", b"\\u001c")
            output = output.replace(b"\x1d", b"\\u001d")
            output = output.replace(b"\x1e", b"\\u001e")
            output = output.replace(b"\x1f", b"\\u001f")

        try:
            json_output = json.loads(output)
        except json.decoder.JSONDecodeError as e:
            outputs.append((parser, None, False, e, output))
        except UnicodeDecodeError as e:
            outputs.append((parser, None, False, QuickFixError(msg=str(e)), output))
        else:
            outputs.append((parser, json_output, True, None, output))

    return diff_meta, list(parse_outputs(outputs, diff_meta.in_file))


def collect_diff_metas(output_dir, ignore_missing=False):
    items = []
    diff_logs = []

    files = set()

    for job_name in sorted(os.listdir(output_dir)):
        job_dir = os.path.join(output_dir, job_name)

        if not os.path.isdir(job_dir):
            continue

        diff_log_file = os.path.join(job_dir, "diff-log.txt")
        diff_logs.append(diff_log_file)

        # sort to only use the difference with the lowest time
        diff_log = sorted(parse_difflog(diff_log_file), key=lambda x: x.wall)
        for diff in diff_log:
            if diff.hashname in files:
                continue

            in_file = os.path.join(job_dir, diff.hashname)
            out_file = in_file + ".out"

            if ignore_missing:
                if not (os.path.exists(in_file) and os.path.isfile(in_file)):
                    continue

                if not (os.path.exists(out_file) and os.path.isfile(out_file)):
                    continue
            else:
                assert os.path.exists(in_file) and os.path.isfile(in_file)
                assert os.path.exists(out_file) and os.path.isfile(out_file)

            files.add(diff.hashname)
            items.append(DiffMeta(diff=diff, in_file=in_file, out_file=out_file))

    return items, diff_logs


def c_time_to_seconds(time):
    return int(time[0]) + int(time[1]) / 1_000_000


def print_info(diff_meta, output_results):
    if len(output_results) == 0:
        return

    print(diff_meta.in_file)
    with open(diff_meta.in_file) as f:
        print(f.read())

    print("====")

    for target1, target2, reason in output_results:
        print(
            target1,
            "vs",
            target2,
            type(reason).__name__,
            reason.obj1_typename,
            reason.obj2_typename,
        )

    print()


def use_file(filename, exclude_parsers=None):
    if filename.endswith(".out"):
        in_file = filename
        out_file = in_file[: -len(".out")]
    else:
        in_file = filename
        out_file = in_file + ".out"

    job_dir = Path(in_file).parent
    while True:
        diff_log_file = job_dir / "diff-log.txt"

        if diff_log_file.exists():
            diff_log = parse_difflog(diff_log_file)
            break

        if job_dir == job_dir.parent:
            break

        job_dir = job_dir.parent

    diff_log_files = {job_dir / diff.hashname: diff for diff in diff_log}

    assert Path(in_file) in diff_log_files
    assert os.path.exists(in_file) and os.path.isfile(in_file)
    assert os.path.exists(out_file) and os.path.isfile(out_file)

    diff = diff_log_files[Path(filename)]

    diff_meta = DiffMeta(diff=diff, in_file=in_file, out_file=out_file)
    diff_meta, unfiltered_output_results = process_filename(diff_meta, exclude_parsers)
    print_info(diff_meta, unfiltered_output_results)


def use_dir(
    rundir,
    n_jobs,
    batch_size,
    exclude_parsers=None,
    ignore_missing=False,
    relevant_files_log=None,
):
    output_dir = os.path.join(rundir, "output")
    diff_metas, diff_logs = collect_diff_metas(
        output_dir, ignore_missing=ignore_missing
    )

    csvwriter = csv.writer(sys.stdout)

    process_fn = functools.partial(
        process_filename,
        exclude_parsers=exclude_parsers,
    )

    relevant_files = set()

    with multiprocessing.Pool(n_jobs) as p:
        for diff_meta, unfiltered_output_results in tqdm.tqdm(
            p.imap_unordered(
                process_fn,
                diff_metas,
                chunksize=batch_size,
            ),
            total=len(diff_metas),
        ):
            for target1, target2, reason in unfiltered_output_results:
                row = [
                    diff_meta.in_file,
                    c_time_to_seconds(diff_meta.diff.user),
                    target1,
                    target2,
                    reason.get_name(),
                    reason.obj1_typename,
                    reason.obj2_typename,
                ]

                csvwriter.writerow(row)

                if relevant_files_log is not None:
                    relevant_files.add(diff_meta.in_file)

    if relevant_files_log is not None:
        with open(relevant_files_log, "w") as f:
            for relevant_file in relevant_files:
                print(relevant_file, file=f)

            for diff_log in diff_logs:
                print(diff_log, file=f)


def main(
    rundir_or_filename,
    n_jobs,
    batch_size,
    exclude_parsers=None,
    ignore_missing=False,
    relevant_files_log=None,
):
    print("Input", "Time", "Target1", "Target2", "Reason", "Type1", "Type2", sep=",")

    if os.path.isfile(rundir_or_filename):
        use_file(rundir_or_filename, exclude_parsers=exclude_parsers)
    else:
        use_dir(
            rundir_or_filename,
            n_jobs,
            batch_size,
            exclude_parsers=exclude_parsers,
            ignore_missing=ignore_missing,
            relevant_files_log=relevant_files_log,
        )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Foo")
    parser.add_argument("rundir_or_filename", help="List of parsers", type=str)
    parser.add_argument(
        "--n_jobs",
        help="Number of parallel workers to start",
        type=int,
        default=1,
    )
    parser.add_argument(
        "--batch_size",
        help="Size of a batch for a worker",
        type=int,
        default=256,
    )
    parser.add_argument("--ignore-missing", action="store_true")
    parser.add_argument("--relevant-files-log", type=str)
    parser.add_argument("--exclude-parsers", type=str, nargs="*")
    args = parser.parse_args()

    main(
        args.rundir_or_filename,
        args.n_jobs,
        args.batch_size,
        exclude_parsers=args.exclude_parsers,
        ignore_missing=args.ignore_missing,
        relevant_files_log=args.relevant_files_log,
    )
