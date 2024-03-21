import argparse
import json
import sys

from common.util import parse_outfile


def display_summary(outfile_content):
    gold_parsers = {}
    for config, output in outfile_content:
        if len(config) == 1:
            continue

        gold_parser = config[-1]

        if gold_parser not in gold_parsers:
            gold_parsers[gold_parser] = [(config, output)]
        else:
            gold_parsers[gold_parser].append((config, output))

    n_diffs = 0

    for gold_parser, configs_and_outputs in gold_parsers.items():
        configs, outputs = zip(*configs_and_outputs)
        unique_outputs = set(outputs)
        if len(unique_outputs) > 1:
            print("*****", end="")
        print(gold_parser, len(unique_outputs))
        for output in unique_outputs:
            print("-", output)
        print()

        if len(unique_outputs) > 1:
            n_diffs += 1

    print()
    print("=" * 30)
    print()
    print("Summary: ", len(gold_parsers), n_diffs)


def display_outputs(outfile_content, escaped):
    print("[SINGLE PARSERS]")
    print("WARNING: Stripping whitespace")
    for parser, output in outfile_content:
        if len(parser) > 1:
            continue

        output = output.replace(b" ", b"")
        output = output.replace(b"\n", b"")

        parser = parser[0]

        if escaped:
            print(output, flush=True)
        else:
            sys.stdout.buffer.write(output)
            sys.stdout.buffer.flush()
        try:
            pass
            # print(json.loads(output))
        except json.decoder.JSONDecodeError as e:
            print("ERROR: ", e.msg)

        print(" " * (50 - len(output)), "(", parser, ")", flush=True, sep="")

    print()
    print("=" * 30)
    print()

    print("[GOLD PARSERS]")
    for parser, output in outfile_content:
        if len(parser) == 1:
            continue

        print(parser, end=": ", flush=True)
        if escaped:
            print(output)
        else:
            sys.stdout.buffer.write(output)
            sys.stdout.buffer.flush()
            print()


def main(filename, summary, escaped):
    if filename.endswith(".out"):
        # out_file = filename
        in_file = filename[: -len(".out")]
    else:
        in_file = filename
        # out_file = filename + ".out"

    print("[INPUT]", in_file)

    with open(in_file) as f:
        print(f.read())

    print()
    print("=" * 30)
    print()

    outfile_content = parse_outfile(filename)

    if summary:
        display_summary(outfile_content)
    else:
        display_outputs(outfile_content, escaped)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Foo")
    parser.add_argument("output", help="Output file", type=str)
    parser.add_argument("--summary", help="Summarize outputs", action="store_true")
    parser.add_argument("--escaped", help="Use python's 'print'", action="store_true")
    args = parser.parse_args()

    main(args.output, summary=args.summary, escaped=args.escaped)
