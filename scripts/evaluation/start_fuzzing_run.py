import argparse
import datetime
import json
import os
import random
import shutil
import subprocess
import sys
from pathlib import Path

IS_LOCAL = False


def read_wordlist(filename):
    with open(filename) as f:
        return [line.strip() for line in f.readlines() if len(line.strip()) > 0]


def find_all_files(directory):
    for root, subdirs, filenames in os.walk(directory):
        for filename in filenames:
            yield os.path.join(root, filename)


def get_git_commit():
    p = subprocess.run(
        ["git", "rev-parse", "HEAD"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    try:
        p.check_returncode()
    except subprocess.CalledProcessError:
        print(p.stderr, file=sys.stderr)
        raise

    return p.stdout.decode().strip()


def get_git_status():
    return ""
    p = subprocess.run(
        ["git", "status", "--ignored", "--porcelain"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    try:
        p.check_returncode()
    except subprocess.CalledProcessError:
        print(p.stderr, file=sys.stderr)
        raise

    return p.stdout.decode().strip()


def start_container(
    command, output_dir, host_corpus_dir, container_name, writable_corpus=True
):
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
        *(["--writable_corpus"] if writable_corpus else []),
        *(["--detach"] if not IS_LOCAL else []),
        command,
    ]

    if IS_LOCAL:
        p = subprocess.run(args)
        return None
    else:
        p = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            p.check_returncode()
        except subprocess.CalledProcessError:
            print(p.stdout, file=sys.stderr)
            print(p.stderr, file=sys.stderr)
            raise

        container_hash = p.stdout.decode().strip()
        return container_hash


def generate_name():
    adjective = random.choice(
        read_wordlist(Path("scripts") / "wordlist-adjectives.txt")
    )
    noun = random.choice(read_wordlist(Path("scripts") / "wordlist-nouns.txt"))

    return adjective + "-" + noun


def main(
    configs,
    gold_parsers,
    writeable_corpus,
    initial_corpus,
    analysis_criterium,
    ascii_only,
    no_numbers,
    no_backslash_u,
    no_coverage,
    log_iteration_time,
    max_total_time,
    seed,
    n_jobs,
    max_len,
    gold_type,
    no_cross_language_coverage,
):
    assert not IS_LOCAL or n_jobs == 1

    now = datetime.datetime.now()
    commit = get_git_commit()

    run_name = generate_name()
    dir_name = now.isoformat().replace(":", "_") + "_" + run_name

    parent_dir = Path("output") / "fuzzing" / dir_name
    parent_dir.mkdir(exist_ok=True, parents=True)

    host_corpus_dir = parent_dir / "corpus"

    with open(parent_dir / "runconfig.json", "w") as f:
        meta = {
            "commit": commit,
            "seed": seed,
            "max_total_time": max_total_time,
            "initial_corpus": initial_corpus,
            "writeable_corpus": writeable_corpus,
            "host_corpus_dir": str(host_corpus_dir),
            "log_iteration_time": log_iteration_time,
            "ascii_only": ascii_only,
            "no_backslash_u": no_backslash_u,
            "no_numbers": no_numbers,
            "no_coverage": no_coverage,
            "no_cross_language_coverage": no_cross_language_coverage,
            "configs": configs,
            "gold_parsers": gold_parsers,
            "analysis_criterium": analysis_criterium,
            "n_jobs": n_jobs,
            "max_len": max_len,
        }
        json.dump(meta, f, indent=4)

    command_args = [
        "./build/crossy",
        *configs,
        *[f"-g {g}" for g in gold_parsers],
        *([f"-gx {gold_type}"] if len(gold_parsers) > 0 else []),
        "--analysis-criterium",
        analysis_criterium,
        *(["-t", "./output/iteration-log.txt"] if log_iteration_time else []),
        *(["-c", "./output/coverage-log"] if not no_coverage else []),
        *(["--ascii-only"] if ascii_only else []),
        *(["--no-backslash-u"] if no_backslash_u else []),
        *(["--no-numbers"] if no_numbers else []),
        *(["--no-coverage"] if no_coverage else []),
        *(["--no-cross-language-coverage"] if no_cross_language_coverage else []),
        "-o ./output/",
        "--",
        "corpus/",
        "<SEED PLACEHOLDER>",
        "-detect_leaks=0",
        "-artifact_prefix=./output/",
        f"-max_total_time={max_total_time}",
        f"-max_len={max_len}",
        *([">./output/stdout"] if not IS_LOCAL else []),
        *(["2>./output/stderr"] if not IS_LOCAL else []),
    ]
    seed_index = command_args.index("<SEED PLACEHOLDER>")

    container_base_name = "crossy-fuzzing-" + now.isoformat().replace(":", "_")

    with open(parent_dir / "runmeta.json", "w") as f:
        meta = {
            "start_time_iso": now.isoformat(),
            "start_time": now.timestamp(),
            "container_base_name": container_base_name,
            "command_args": command_args,
        }
        json.dump(meta, f, indent=4)

    with open(parent_dir / "git-status", "w") as f:
        f.write(get_git_status())

    if initial_corpus:
        if not os.path.isdir(initial_corpus):
            raise RuntimeError(f"{initial_corpus} is not a directory")

        destination = shutil.copytree(initial_corpus, host_corpus_dir)
        assert destination == host_corpus_dir, f"{destination} vs {host_corpus_dir}"
    else:
        host_corpus_dir.mkdir(exist_ok=True, parents=True)

    with open(parent_dir / "initial_corpus", "w") as f:
        for filepath in find_all_files(host_corpus_dir):
            print(filepath, file=f)

    container_hashes = []
    for i in range(n_jobs):
        job_command_args = command_args[:]
        job_command_args[seed_index] = f"-seed={seed + i}"
        command = " ".join(job_command_args)

        output_dir = parent_dir / "output" / f"job-{i}"
        output_dir.mkdir(exist_ok=True, parents=True)

        with open(output_dir / "command.json", "w") as f:
            meta = {
                "command_args": job_command_args,
                "command": command,
            }
            json.dump(meta, f, indent=4)

        container_hash = start_container(
            command,
            output_dir.absolute(),
            host_corpus_dir.absolute(),
            container_base_name + "_job" + str(i),
            writable_corpus=writeable_corpus,
        )
        print(f"[Job {i}]: Started container {container_hash}")
        container_hashes.append(container_hash)

    if not IS_LOCAL:
        print("docker kill", *container_hashes)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Foo")
    parser.add_argument("configs", help="List of parsers", type=str, nargs="+")
    parser.add_argument(
        "-g",
        "--gold_parsers",
        help="List of gold parsers",
        type=str,
        nargs="*",
        default=[],
    )

    parser.add_argument(
        "--n_jobs",
        help="Number of parallel fuzzers to start",
        type=int,
        default=1,
        required=True,
    )
    parser.add_argument(
        "--max_total_time",
        help="Time to run the fuzzer",
        type=int,
        default=20 * 60,
        required=True,
    )

    parser.add_argument(
        "--max_length", help="Maximum length of an input", type=int, default=512
    )
    parser.add_argument(
        "--initial_corpus",
        help="If present, a copy of the files in the directory will be used as the initial corpus",
        type=str,
        default=None,
    )
    parser.add_argument(
        "--writable_corpus",
        help="If present, the corpus is writable and thus shared between fuzzers in a fuzzing run",
        action="store_true",
    )
    parser.add_argument(
        "--analysis_criterium",
        help="How many parsers need to exit successfully to analyze an input for a difference",
        type=str,
        choices=["all", "at_least_one", "discard"],
        default="all",
    )
    parser.add_argument(
        "--no_coverage",
        help="If present, the fuzzer will not register or track coverage information",
        action="store_true",
    )
    parser.add_argument(
        "--ascii_only",
        help="If present, inputs will only contain ascii characters",
        action="store_true",
    )
    parser.add_argument(
        "--no_numbers",
        help="If present, inputs will not contain characters between '0' and '9'",
        action="store_true",
    )
    parser.add_argument(
        "--no_backslash_u",
        help="If present, inputs will not contain the string '\\u'",
        action="store_true",
    )
    parser.add_argument(
        "--no_cross_language_coverage",
        help="If present, cross language coverage is disabled",
        action="store_true",
    )
    parser.add_argument(
        "--seed",
        help="Seed from which the fuzzing seeds are generated",
        type=int,
        default=42,
    )
    parser.add_argument(
        "-gx",
        "--gold_type",
        help="What execution mode the gold parsers should follow",
        choices=["round-robin", "majority"],
        default="majority",
    )

    args = parser.parse_args()

    main(
        configs=args.configs,
        gold_parsers=args.gold_parsers,
        writeable_corpus=args.writable_corpus,
        initial_corpus=args.initial_corpus,
        analysis_criterium=args.analysis_criterium,
        ascii_only=args.ascii_only,
        seed=args.seed,
        max_total_time=args.max_total_time,
        n_jobs=args.n_jobs,
        max_len=args.max_length,
        no_numbers=args.no_numbers,
        no_backslash_u=args.no_backslash_u,
        no_coverage=args.no_coverage,
        log_iteration_time=True,
        gold_type=args.gold_type,
        no_cross_language_coverage=args.no_cross_language_coverage,
    )
