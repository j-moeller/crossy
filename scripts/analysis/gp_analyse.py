import csv
import sys

HEADER = ["run", "config", "tp", "fp", "tn", "fn"]
RUN_ID = 0
CONFIG_ID = 1
TP_ID = 2
FP_ID = 3
TN_ID = 4
FN_ID = 5


def main(csvfile):
    with open(csvfile) as f:
        csvreader = csv.reader(f)
        assert next(csvreader) == HEADER

        if csvfile.endswith(".csv"):
            out_file = csvfile[: -len(".csv")] + "_metrics.csv"
        else:
            out_file = csvfile + "_metrics.csv"

        with open(out_file, "w") as g:
            csvwriter = csv.writer(g)
            csvwriter.writerow(["run", "config", "accuracy", "fp", "fn"])

            rows = []
            for row in csvreader:
                row[RUN_ID] = row[RUN_ID].replace(
                    "output/fuzzing/2023-10-20T15_22_06.884244_cool-eagle/", "no-corpus"
                )
                row[RUN_ID] = row[RUN_ID].replace(
                    "output/fuzzing/2023-10-20T15_22_14.852741_grey-sloth/",
                    "blog-corpus",
                )
                row[RUN_ID] = row[RUN_ID].replace(
                    "output/fuzzing/2023-10-20T15_22_24.557479_wet-anaconda/",
                    "generated-corpus",
                )

                row[CONFIG_ID] = row[CONFIG_ID].replace("configs/json/", "")

                tp = int(row[TP_ID])
                fp = int(row[FP_ID])
                tn = int(row[TN_ID])
                fn = int(row[FN_ID])

                # f1_score = (2 * tp) / (2 * tp + fp + fn)
                # precision = tp / (tp + fp)
                # recall = tp / (tp + fn)
                # tnr = tn / (tn + fp)
                # fnr = fn / (fn + tp)
                # fpr = fp / (fp + tn)

                accuracy = (tp + tn) / (tp + tn + fp + fn)

                rows.append([row[RUN_ID], row[CONFIG_ID], accuracy, fp, fn])

            for row in sorted(
                sorted(rows, key=lambda x: x[2], reverse=True), key=lambda x: x[0]
            ):
                csvwriter.writerow(row)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: python3 scripts/analysis/analyse_gold_parser.py <csvfile>")
        exit(1)
    main(sys.argv[1])
