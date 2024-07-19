import sys
import csv
import numpy as np
import itertools
from collections import defaultdict
from common.util import UniqueId

import matplotlib.pyplot as plt

ONLY_COMPOSITE = False

"""
Generate a confusion matrix from output/diffs.csv (see gen-csv-overview.py)
"""


def normalizeTarget(target):
    is_composite = False

    if target == "configs/json/gold-parser/so-v8.cfg":
        print("v8")
        return "v8"

    if target.endswith("-configs/json/so-v8.cfg"):
        target = target[: -len("-configs/json/so-v8.cfg")]
        is_composite = True

    if target.startswith("configs/json/"):
        target = target[len("configs/json/") :]

    if target.endswith(".cfg"):
        target = target[: -len(".cfg")]

    # _, *rest = target.split("-")
    # target = "".join(rest)

    if is_composite:
        target += "_v8"

    return target, is_composite


def main():
    filename = sys.argv[1]

    values = defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: 0)))
    target_ids = UniqueId()
    reason_ids = UniqueId()

    targets = set()

    with open(filename) as f:
        reader = csv.reader(f)
        next(reader)  # skip header

        for in_file, target_0, target_1, reason, type_0, type_1 in reader:
            target_0, is_composite_0 = normalizeTarget(target_0)
            target_1, is_composite_1 = normalizeTarget(target_1)

            include = False

            if ONLY_COMPOSITE:
                if is_composite_0 and is_composite_1:
                    include = True
            else:
                if not is_composite_0 and not is_composite_1:
                    include = True

            if include:
                targets.add(target_0)
                targets.add(target_1)

                reason_ids.add(reason)

    for target in sorted(targets):
        target_ids.add(target)

    with open(filename) as f:
        reader = csv.reader(f)
        next(reader)  # skip header

        for in_file, target_0, target_1, reason, type_0, type_1 in reader:
            target_0 = normalizeTarget(target_0)[0]
            target_1 = normalizeTarget(target_1)[0]

            values[target_0][target_1][reason] += 1

    M = np.zeros((len(target_ids), len(target_ids), len(reason_ids)), dtype=int)

    for i in range(len(target_ids)):
        for j in range(len(target_ids)):
            vs = values[target_ids.getKey(i)][target_ids.getKey(j)]

            for reason, v in vs.items():
                M[i][j][reason_ids[reason]] = v
                M[j][i][reason_ids[reason]] = v

    for reason_id in range(len(reason_ids)):
        plt.matshow(M[:, :, reason_id])
        plt.xticks(range(len(target_ids)), target_ids.getKeys(), rotation=90)
        plt.yticks(range(len(target_ids)), target_ids.getKeys())
        for i, j in itertools.product(range(len(target_ids)), range(len(target_ids))):
            plt.text(
                j,
                i,
                str(M[i, j, reason_id]),
                ha="center",
                va="center",
                fontsize=8,
                color="black"
                if M[i, j, reason_id] > 0.5 * np.max(M[:, :, reason_id])
                else "white",
            )
        plt.title(reason_ids.getKey(reason_id))
        # plt.savefig(f"output/confusion-matrix-{reason_id}.png", dpi=300, bbox_inches="tight", pad_inches=1)
        plt.show()


if __name__ == "__main__":
    main()
