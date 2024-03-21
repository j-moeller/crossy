import argparse
import json
import sqlite3
from collections import defaultdict
from pathlib import Path

query = """
SELECT reason_names.reason, target_1.target, target_2.target FROM data
INNER JOIN reasons ON data.reason_id = reasons.id
INNER JOIN reason_names ON reason_names.id = reasons.reason_name_id
INNER JOIN targets AS target_1 ON target_1.id = data.target_1_id
INNER JOIN targets AS target_2 ON target_2.id = data.target_2_id
GROUP BY data.target_1_id, data.target_2_id, reason_names.reason;
"""


def normalize_target(target):
    if target == "configs/json/cpp-json.cfg":
        return "nlohmann"

    if target == "configs/json/py-json.cfg":
        return "py-json"

    return "".join(target[len("configs/json/") : -len(".cfg")].split("-")[1:])


def to_color(size):
    return f"\\cellcolor{{myblue!{size * 15}}} {size}"


def run_query(database_path):
    rundir = Path(database_path).parent

    with open(rundir / "runconfig.json") as f:
        configs = json.load(f)["configs"]

    with sqlite3.connect(database_path) as con:
        c = con.cursor()
        c.execute(query)

        original_reasons = set()
        reason_set = set()

        M = defaultdict(lambda: defaultdict(lambda: set()))
        for reason, target_1, target_2 in c.fetchall():
            original_reasons.add(reason)

            idx = reason.find("(")
            idx2 = reason.find(")")
            if idx >= 0 and idx2 >= 0:
                reason = reason[:idx] + reason[idx2 + 1 :]

            M[target_1][target_2].add(reason)
            M[target_2][target_1].add(reason)

            reason_set.add(reason)

        print("\\toprule")
        print(
            "",
            *["\\rot{" + normalize_target(t) + "}" for t in configs],
            sep=" & ",
            end=" \\\\\n",
        )

        for row_config in configs:
            print(normalize_target(row_config), end=" & ")
            print(
                *[
                    to_color(len(M[row_config][column_config]))
                    for column_config in configs
                ],
                sep=" & ",
                end=" \\\\\n",
            )
        print("\\bottomrule")

        for reason in sorted(reason_set):
            print(reason)

        print("=======")

        for reason in sorted(original_reasons):
            print(reason)


def main():
    parser = argparse.ArgumentParser(description="Foo")
    parser.add_argument("database", help="Rundir", type=str)
    args = parser.parse_args()

    run_query(args.database)


if __name__ == "__main__":
    main()
