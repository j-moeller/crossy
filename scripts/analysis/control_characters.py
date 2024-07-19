import sys
from collections import defaultdict

from common.util import parse_outfile

TOKEN = b"TOKEN"


def is_correct_cc(value, index):
    return len(value) == 0


def is_correct_xx(value, index):
    c = chr(index).encode()

    return TOKEN in value and c not in value


def get_output(parser, value, index):
    if len(value) == 0:
        return " "

    c = chr(index).encode()
    print(
        "get_output: ",
        parser,
        value,
        index,
        TOKEN in value,
        c in value,
        file=sys.stderr,
    )

    if TOKEN in value:
        if c in value:
            return "x"
        else:
            return "u"
    else:
        if c in value:
            return "X"
        else:
            return "U"


def filter_mapping(mapping_cc, mapping_xx):
    invalid_keys = set()

    for (k_cc, V_cc), (k_xx, V_xx) in zip(mapping_cc.items(), mapping_xx.items()):
        assert k_cc == k_xx

        for (i_cc, v_cc), (i_xx, v_xx) in zip(V_cc.items(), V_xx.items()):
            assert i_cc == i_xx

            correct_cc = is_correct_cc(v_cc, i_cc)
            correct_xx = is_correct_xx(v_xx, i_xx)

            if not (correct_cc and correct_xx):
                invalid_keys.add(k_cc)

    new_mapping_cc = {}
    new_mapping_xx = {}

    for key in invalid_keys:
        new_mapping_cc[key] = mapping_cc[key]
        new_mapping_xx[key] = mapping_xx[key]

    return new_mapping_cc, new_mapping_xx


def print_mapping(mapping, parsers, prefix):
    for i in range(0x20):
        hexvalue = f"{i:02x}"
        print(prefix + hexvalue, end=" & ")
        print(
            *[get_output(parser, mapping[parser][i], i) for parser in parsers],
            sep=" & ",
            end=" \\\\\n",
        )


def load_file(filename):
    mapping = defaultdict(lambda: {})

    for i in range(0x20):
        hexvalue = f"{i:02x}"

        outputs = parse_outfile(filename.format(hexvalue))
        for parser, payload in outputs:
            if len(parser) == 1:
                mapping[parser[0][len("configs/json/") : -len(".cfg")]][i] = payload

    return mapping


def main():
    mapping_cc = load_file("output/control-characters/cc-{}.out")
    mapping_xx = load_file("output/control-characters/u00{}.out")

    mapping_cc, mapping_xx = filter_mapping(mapping_cc, mapping_xx)

    parsers_cc = sorted(list(mapping_cc.keys()))
    parsers_xx = sorted(list(mapping_xx.keys()))

    assert parsers_xx == parsers_cc

    print("\\toprule")
    print("ASCII Character & ", end="")
    print(
        *["\\rot{" + k + "}" for k in parsers_cc],
        sep=" & ",
        end=" \\\\\n",
    )
    print("\\midrule")
    print_mapping(mapping_cc, parsers_cc, "0x")
    print("\\midrule")
    print_mapping(mapping_xx, parsers_xx, "\\textbackslash{}u00")
    print("\\bottomrule")


def create_files():
    files = []

    for i in range(0x20):
        hexvalue = f"{i:02x}"

        filename = f"output/control-characters/cc-{hexvalue}"
        files.append(filename)

        with open(filename, "wb") as f:
            f.write(b'["')
            f.write(chr(i).encode())
            f.write(TOKEN)
            f.write(b'"]')

        filename = f"output/control-characters/u00{hexvalue}"
        files.append(filename)
        with open(filename, "wb") as f:
            f.write(b'["\\u00')
            f.write(hexvalue.encode())
            f.write(TOKEN)
            f.write(b'"]')

    with open("output/control-characters/filelist.txt", "w") as f:
        for file in files:
            print(file, file=f)


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--create":
        create_files()
    else:
        main()
