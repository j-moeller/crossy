import argparse
import csv
import os
import sys

HEADER = "User(s),User(us),System(s),System(us),Wall-Time(ms),Iteration"


def parse_timelog(path):
    with open(path) as f:
        csvreader = csv.reader(f)

        header = next(csvreader)
        assert ",".join(header) == HEADER, f"{header} vs {HEADER}"

        for line in csvreader:
            yield [int(l) for l in line]


def main(rundir):
    targets = []

    for target in os.listdir(rundir):
        target_path = os.path.join(rundir, target)

        if not os.path.isdir(target_path):
            continue

        timelog_path = os.path.join(target_path, "timelog.txt")

        last_line = list(parse_timelog(timelog_path))[-1]
        user_s, user_us, system_s, system_us, wall, iteration = last_line

        total_time_in_seconds = (
            user_s + user_us / 1_000_000 + system_s + system_us / 1_000_000
        )

        targets.append((target, iteration // total_time_in_seconds))

    csvwriter = csv.writer(sys.stdout)
    csvwriter.writerow(["Parser", "Exec/s"])

    for target, performance in sorted(targets, key=lambda x: x[1], reverse=True):
        csvwriter.writerow([target, performance])


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Foo")
    parser.add_argument("rundir", help="Rundir", type=str)

    args = parser.parse_args()

    main(args.rundir)
