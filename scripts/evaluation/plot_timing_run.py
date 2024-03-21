import matplotlib.pyplot as plt
import sys
import numpy as np

from pathlib import Path
from dataclasses import dataclass

# python3 scripts/evaluation/plot_timing_run.py $(find output -type f -name timelog.txt)

@dataclass
class Performance:
    timestamp: int
    n_iterations: int

def get_target_name(filepath):
    # "-".join(str(Path(filename).parent).split("-")[1:][:-1])
    return Path(filepath).parent

def analyze_line(line):
    timestamp, n_iterations = line.split(": ")
    assert timestamp[0] == "[" and timestamp[-1] == "]"
    return Performance(int(timestamp[1:-1]), int(n_iterations))


def should_skip(filename):
    with open(filename) as f:
        content = f.readlines()

    if len(content) == 0:
        return True

    last_line = analyze_line(content[-1])

    return False


def main():
    filenames = sys.argv[1:]

    if len(filenames) == 0:
        print("usage: python scripts/evaluation/plot_timing_run.py [file] ...")
        exit(1)

    perf_dict = {}
    info = []

    for filename in filenames:
        if should_skip(filename):
            print("Skip: ", filename)
            #continue

        perfs = []
        with open(filename) as f:
            for line in f.readlines():
                perfs.append(analyze_line(line))
        perf_dict[filename] = perfs

        mean = np.mean([p.n_iterations for p in perfs])
        std = np.std([p.n_iterations for p in perfs])
        median = np.median([p.n_iterations for p in perfs])

        info.append([mean, std, median, filename])

    for mean, std, median, filename in sorted(info, key=lambda x: x[2], reverse=True):
        perfs = perf_dict.get(filename, None)

        if perfs is None:
            continue

        X, Y = [], []
        window_size = 7

        for i in range(len(perfs) - window_size + 1):
            window = perfs[i : i + window_size]

            window_x = np.mean([w.timestamp / window_size for w in window])
            window_y = np.mean([
                window[i + 1].n_iterations - window[i].n_iterations
                for i in range(len(window) - 1)
            ])

            X.append(window_x)
            Y.append(window_y)

        target = get_target_name(filename)
        plt.plot(X, Y, label=target)

    plt.legend()
    plt.show()


if __name__ == "__main__":
    main()
