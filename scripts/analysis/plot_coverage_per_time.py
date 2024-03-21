import argparse
import json
import os.path
import sqlite3
from functools import reduce

import matplotlib.pyplot as plt
import pandas as pd

MAX_COVERAGE = {
    "configs/json/c-ccan.cfg": 622,
    "configs/json/c-cjson.cfg": 958,
    "configs/json/c-frozen.cfg": 695,
    "configs/json/c-jansson.cfg": 1527,
    "configs/json/c-jsmn.cfg": 221,
    "configs/json/c-json-c.cfg": 1560,
    "configs/json/c-json-parser.cfg": 621,
    "configs/json/c-jsonh.cfg": 849,
    "configs/json/c-libjson.cfg": 256,
    "configs/json/c-poco.cfg": 7405,
    "configs/json/c-yajl.cfg": 960,
    "configs/json/cpp-boost.cfg": 6142,
    "configs/json/cpp-jsoncpp.cfg": 4667,
    "configs/json/cpp-json.cfg": 2800,
    "configs/json/cpp-spidermonkey.cfg": 7324,
    "configs/json/cpp-rapidjson.cfg": 815,
    "configs/json/cpp-v8.cfg": 3764,
    "configs/json/java-gson.cfg": 4096,
    "configs/json/java-jackson.cfg": 65536,
    "configs/json/py-json.cfg": 440,
    "configs/json/py-simplejson.cfg": 788,
    "configs/json/rust-serde.cfg": 24254,
}


def parse_coverage_log(coverage_log_path):
    with open(coverage_log_path) as f:
        return pd.read_csv(f)


def get_first_occurrences(db_name):
    with sqlite3.connect(db_name) as con:
        cur = con.cursor()
        sql_query = """
            SELECT MIN(inputs.time), reason_names.reason
            FROM data
            INNER JOIN inputs ON data.input_id=inputs.id
            INNER JOIN reasons ON data.reason_id=reasons.id
            INNER JOIN reason_names ON reasons.reason_name_id=reason_names.id
            GROUP BY (reason_id, data.target_id_1, data.target_id_2)
            ORDER BY inputs.time
            """
        return cur.execute(sql_query).fetchall()


def read_parser_names(runconfig):
    try:
        with open(runconfig) as f:
            config = json.load(f)
            return config["configs"]
    except FileNotFoundError:
        print("WARNING: Working without a runconfig.json")
        return None


def get_language(config):
    return config.split("/")[-1].split("-")[0]


def main(rundir, outfile=None, sql_db=None):
    outdir = os.path.join(rundir, "output")

    parser_names = read_parser_names(os.path.join(rundir, "runconfig.json"))
    languages = sorted(set(get_language(c) for c in parser_names))

    coverage_logs = []
    for job_id in os.listdir(outdir):
        job_dir = os.path.join(outdir, job_id)
        coverage_log_path = os.path.join(job_dir, "coverage-log")

        coverage_logs.append(parse_coverage_log(coverage_log_path))
        break

    coverage_log = coverage_logs[0]

    group_by = {}
    for lang in languages:
        group_by[lang] = []
        for config in parser_names:
            if "configs/json/c-tiny-json.cfg" == config:
                continue

            if get_language(config) == lang:
                group_by[lang].append(coverage_log[config].divide(MAX_COVERAGE[config]))

    aggregate = {}
    for language, group in group_by.items():
        aggregate[language] = reduce(
            lambda a, b: a.add(b, fill_value=0).divide(len(group)), group
        )

    plt.stackplot(
        coverage_log["Time"],
        [aggregate[lang] for lang in languages],
        labels=languages,
        alpha=0.8,
    )

    t0 = coverage_log["Time"][0]

    if sql_db:
        for time, name in get_first_occurrences(sql_db):
            print(time, name)
            plt.plot([t0 + time, t0 + time], [0, 1])

    plt.xlim(t0, t0 + 1000)
    plt.legend()
    if outfile is None:
        plt.show()
    else:
        plt.savefig(outfile)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Foo")
    parser.add_argument("rundir", help="List of parsers", type=str)
    parser.add_argument("--outfile", help="List of parsers", type=str)
    parser.add_argument("--sql-db", help="SQL Database", type=str)
    args = parser.parse_args()

    print("WARNING: Only plotting coverage for job-0")

    main(args.rundir, outfile=args.outfile, sql_db=args.sql_db)
