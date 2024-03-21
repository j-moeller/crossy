#!/usr/bin/env python3

import argparse
import json
import os
import subprocess
import sys
import tempfile


def start_container(command, output_dir, host_corpus_dir, container_name):
    assert str(output_dir).startswith("/"), f"{output_dir} does not start with '/'"
    assert str(host_corpus_dir).startswith(
        "/"
    ), f"{host_corpus_dir} does not start with '/'"

    args = [
        "/bin/sh",
        "scripts/dev/start_run_container.sh",
        "--host_output_dir",
        str(output_dir),
        "--host_corpus_dir",
        str(host_corpus_dir),
        "--name",
        container_name,
        command,
    ]
    p = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    try:
        p.check_returncode()
    except subprocess.CalledProcessError:
        print(p.stdout, file=sys.stderr)
        print(p.stderr, file=sys.stderr)
        raise

    container_hash = p.stdout.decode().strip()
    return container_hash


def get_input_files(rundir, run_type):
    p = subprocess.run(
        ["python3", "scripts/util/stats.py", rundir, run_type],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    p.check_returncode()

    stdout = p.stdout.decode("utf-8")
    input_files = stdout.split("\n")

    for input_file in input_files:
        if len(input_file) > 0:
            yield input_file


def main(rundir, run_type):
    tag = rundir.split("_")[-1].replace("/", "")

    with open(os.path.join(rundir, "runconfig.json")) as f:
        runconfig = json.load(f)

    configs = runconfig["configs"]
    gold_parsers = runconfig["gold_parsers"]
    analysis_criterium = runconfig["analysis_criterium"]

    outdir = os.path.join(rundir, "output")

    with tempfile.TemporaryDirectory() as tmpdir:
        input_list_path = os.path.join(tmpdir, "input_list.txt")

        n_files = 0
        with open(input_list_path, "w") as f:
            for input_file in get_input_files(rundir, run_type):
                assert os.path.exists(input_file), f"No file: {input_file}"
                n_files += 1
                print(os.path.join("output", input_file[len(outdir) + 1 :]), file=f)

        if n_files == 0:
            print("No files to process")
            exit(0)

        print(f"Processing {n_files} missing files")

        command = [
            "./build/poc",
            *configs,
            *[f"-g {g}" for g in gold_parsers],
            "-i /app/corpus/input_list.txt",
            "--analysis-criterium",
            analysis_criterium,
            "--",
            "-detect_leaks=0",
        ]

        start_container(
            " ".join(command), os.path.abspath(outdir), tmpdir, "postprocessing_" + tag
        )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Foo")
    parser.add_argument("rundir", help="List of parsers", type=str)

    parser.add_argument(
        "--input",
        action="store_true",
        help="Use all inputs found in diff-log.txt",
    )
    parser.add_argument(
        "--missing",
        action="store_true",
        help="Use all inputs found in diff-log.txt where no corresponding .out file is available",
    )
    parser.add_argument(
        "--input-available",
        action="store_true",
        help="Use all inputs found in diff-log.txt that actually exist in the filesystem",
    )
    parser.add_argument(
        "--missing-available",
        action="store_true",
        help="Use all inputs found in diff-log.txt that actually exist in the filesystem where no corresponding .out file is available",
    )

    args = parser.parse_args()

    n = sum([args.input, args.missing, args.input_available, args.missing_available])
    if n > 1:
        print(
            "error: --input, --missing, and --input-available must not be used in conjunction"
        )
        exit(1)

    if args.input:
        run_type = "--input"

    elif args.missing:
        run_type = "--missing"

    elif args.input_available:
        run_type = "--input-available"

    elif args.missing_available:
        run_type = "--missing-available"

    else:
        run_type = "--missing"

    main(args.rundir, run_type)
