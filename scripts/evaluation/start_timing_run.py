import datetime
import subprocess
import sys
from pathlib import Path

CORPUS = "corpus-final"
MAX_TOTAL_TIME = 60 * 2
SEED = 12345678

PARSERS = [
    "configs/json/c-ccan.cfg",
    "configs/json/c-cjson.cfg",
    "configs/json/c-frozen.cfg",
    "configs/json/c-jansson.cfg",
    "configs/json/c-jsmn.cfg",
    "configs/json/c-json-c.cfg",
    "configs/json/c-json-parser.cfg",
    "configs/json/c-jsonh.cfg",
    "configs/json/c-libjson.cfg",
    "configs/json/c-poco.cfg",
    "configs/json/c-yajl.cfg",
    "configs/json/cpp-boost.cfg",
    "configs/json/cpp-json.cfg",
    "configs/json/cpp-jsoncpp.cfg",
    "configs/json/cpp-rapidjson.cfg",
    "configs/json/cpp-spidermonkey.cfg",
    "configs/json/cpp-v8.cfg",
    "configs/json/java-jackson.cfg",
    "configs/json/java-gson.cfg",
    "configs/json/py-json.cfg",
    "configs/json/py-simplejson.cfg",
    "configs/json/rust-serde.cfg",
]


def start_container(
    command, output_dir, corpus_dir, container_name, writable_corpus=True
):
    args = [
        "/bin/sh",
        "scripts/dev/start_run_container.sh",
        "--host_output_dir",
        str(output_dir),
        "--host_corpus_dir",
        str(corpus_dir),
        "--name",
        container_name,
        *(["--writable_corpus"] if writable_corpus else []),
        "--detach",
        command,
    ]
    p = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    try:
        p.check_returncode()
    except subprocess.CalledProcessError:
        print("ERROR: ", p.stderr.decode(), file=sys.stderr)

    container_hash = p.stdout.decode().strip()
    return container_hash


def main(parsers=PARSERS, corpus=CORPUS, max_total_time=MAX_TOTAL_TIME, seed=SEED):
    now = datetime.datetime.now()

    for parser in parsers:
        target_name = Path(parser).name[: -len(".cfg")]
        container_name = "crossy-timing-" + target_name.replace("-", "_")

        output_dir = (
            Path("output") / "timing" / now.isoformat().replace(":", "_") / target_name
        )
        output_dir.mkdir(exist_ok=True, parents=True)

        command_args = [
            "./build/crossy",
            parser,
            "-t",
            "output/timelog.txt",
            "--",
            f"-seed={seed}",
            "-detect_leaks=0",
            f"-max_total_time={max_total_time}",
            ">./output/stdout 2>./output/stderr",
        ]

        command = " ".join(command_args)
        container_hash = start_container(
            command,
            output_dir.absolute(),
            corpus,
            container_name,
            writable_corpus=False,
        )


if __name__ == "__main__":
    main()
