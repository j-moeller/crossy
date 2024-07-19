import argparse
import datetime
import os
import re
import sys


def parse_difflog(filename):
    HEADER = (
        "User(s),User(us),System(s),System(us),Wall-Time(ms),Hashname,Gold-Parser-Index"
    )

    with open(filename) as f:
        header_line = f.readline()
        assert header_line.strip() == HEADER

        for line in f.readlines():
            line = line.strip()
            (
                user_s,
                user_us,
                system_s,
                system_us,
                wall_ms,
                hashname,
                gp_index,
            ) = line.split(",")

            yield [user_s, user_us], [system_s, system_us], wall_ms, hashname, gp_index


def is_valid_outfile(out_filename):
    return os.path.exists(out_filename) and os.stat(out_filename).st_size > 0
    try:
        with open(out_filename) as f:
            hash_line = f.readline()
    except FileNotFoundError:
        return False

    return re.match("[0-9a-f]{38}", hash_line)


def is_deprecated(filename, timestamp):
    if timestamp is None:
        return False

    return os.stat(filename).st_mtime < timestamp


def main(rundir, mode, deprecation_timestamp):
    out_dir = os.path.join(rundir, "output")
    job_dirs = [
        os.path.join(out_dir, j) for j in os.listdir(out_dir) if j.startswith("job-")
    ]

    for job_dir in job_dirs:
        for utime, stime, wtime, hashname, gp_index in parse_difflog(
            os.path.join(job_dir, "diff-log.txt")
        ):
            filename = os.path.join(job_dir, hashname)
            out_filename = filename + ".out"

            if mode == "input":
                print(filename)

            elif mode == "input-available":
                if os.path.exists(filename):
                    print(filename)

            elif mode == "output":
                if is_valid_outfile(out_filename):
                    print(out_filename)

            elif mode == "missing":
                if not is_valid_outfile(out_filename) or is_deprecated(
                    out_filename, deprecation_timestamp
                ):
                    print(filename)

            elif mode == "missing-available":
                if os.path.exists(filename):
                    if not is_valid_outfile(out_filename) or is_deprecated(
                        out_filename, deprecation_timestamp
                    ):
                        print(filename)

            else:
                print(filename)

                if is_valid_outfile(out_filename):
                    print(out_filename)


def assert_mutually_exclusive(*args, error):
    if sum([1 for arg in args if arg is not None]) > len(args):
        print(error, file=sys.stderr)
        exit(1)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="stats.py",
        description="What the program does",
        epilog="Text at the bottom of help",
    )

    parser.add_argument("rundir", type=str)

    # Of the following five only one may be selected at a time
    parser.add_argument("--input", action="store_true")
    parser.add_argument("--output", action="store_true")
    parser.add_argument("--missing", action="store_true")
    parser.add_argument("--input-available", action="store_true")
    parser.add_argument("--missing-available", action="store_true")

    parser.add_argument(
        "--date-iso", type=lambda s: datetime.datetime.fromisoformat(s).timestamp()
    )
    parser.add_argument("--date-timestamp", type=float)

    args = parser.parse_args()

    assert_mutually_exclusive(
        args.input,
        args.output,
        args.missing,
        args.input_available,
        args.missing_available,
        error="error: --input, --output, --missing, --input-available, and --missing-available must not be used in conjunction",
    )

    assert_mutually_exclusive(
        args.date_iso,
        args.date_timestamp,
        error="error: --date-iso and --date-timestamp must not be used in conjunction",
    )

    mode = None
    if args.input:
        mode = "input"
    elif args.output:
        mode = "output"
    elif args.missing:
        mode = "missing"
    elif args.input_available:
        mode = "input-available"
    elif args.missing_available:
        mode = "missing-available"

    deprecation_timestamp = None
    if args.date_iso:
        deprecation_timestamp = args.date_iso
    elif args.date_timestamp:
        deprecation_timestamp = args.date_timestamp

    main(
        rundir=args.rundir,
        mode=mode,
        deprecation_timestamp=deprecation_timestamp,
    )
