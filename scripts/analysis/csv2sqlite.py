import csv
import sqlite3
import sys

from common.util import UniqueId

INPUT_ID = 0
TIME_ID = 1
TARGET1_ID = 2
TARGET2_ID = 3
REASON_ID = 4
TYPE1_ID = 5
TYPE2_ID = 6


def create_db(db_name):
    with sqlite3.connect(db_name) as con:
        cur = con.cursor()

        try:
            cur.execute(
                """CREATE TABLE inputs (
                id INTEGER PRIMARY KEY,
                filename TEXT,
                time INTEGER,
                UNIQUE(filename)
            );"""
            )
        except sqlite3.OperationalError as e:
            if "already exists" in str(e):
                print("WARNING:", e, file=sys.stderr)
            else:
                raise e

        try:
            cur.execute("CREATE TABLE targets (id INTEGER PRIMARY KEY, target TEXT);")
        except sqlite3.OperationalError as e:
            if "already exists" in str(e):
                print("WARNING:", e, file=sys.stderr)
            else:
                raise e

        try:
            cur.execute(
                "CREATE TABLE reason_names (id INTEGER PRIMARY KEY, reason TEXT);"
            )
        except sqlite3.OperationalError as e:
            if "already exists" in str(e):
                print("WARNING:", e, file=sys.stderr)
            else:
                raise e

        try:
            cur.execute(
                """CREATE TABLE reasons (
                id INTEGER PRIMARY KEY,
                reason_name_id INTEGER,
                type_1 TEXT,
                type_2 TEXT,
                UNIQUE(reason_name_id, type_1, type_2),
                FOREIGN KEY(reason_name_id) REFERENCES reason_names(id)
            );"""
            )
        except sqlite3.OperationalError as e:
            if "already exists" in str(e):
                print("WARNING:", e, file=sys.stderr)
            else:
                raise e

        try:
            cur.execute(
                """CREATE TABLE data (
                id INTEGER PRIMARY KEY,
                input_id INTEGER,
                target_1_id INTEGER,
                target_2_id INTEGER,
                reason_id INTEGER,
                FOREIGN KEY(input_id) REFERENCES inputs(id),
                FOREIGN KEY(target_1_id) REFERENCES targets(id),
                FOREIGN KEY(target_2_id) REFERENCES targets(id),
                FOREIGN KEY(reason_id) REFERENCES reasons(id),
                UNIQUE(input_id, target_1_id, target_2_id, reason_id)
            );"""
            )
        except sqlite3.OperationalError as e:
            if "already exists" in str(e):
                print("WARNING:", e, file=sys.stderr)
            else:
                raise e

        cur.execute("PRAGMA foreign_keys = ON;")
        con.commit()


def fill_foreign_tables(db_name, inputs, targets, reason_names, reasons):
    with sqlite3.connect(db_name) as con:
        cur = con.cursor()

        try:
            cur.executemany(
                "INSERT INTO inputs (id, filename, time) VALUES (?, ?, ?)",
                [(i, *k) for i, k in inputs.getIdKeyEntries()],
            )
        except sqlite3.IntegrityError as e:
            print("WARNING:", e, file=sys.stderr)
        try:
            cur.executemany(
                "INSERT INTO targets (id, target) VALUES (?, ?)",
                list(targets.getIdKeyEntries()),
            )
        except sqlite3.IntegrityError as e:
            print("WARNING:", e, file=sys.stderr)
        try:
            cur.executemany(
                "INSERT INTO reason_names (id, reason) VALUES (?, ?)",
                list(reason_names.getIdKeyEntries()),
            )
        except sqlite3.IntegrityError as e:
            print("WARNING:", e, file=sys.stderr)
        try:
            cur.executemany(
                "INSERT INTO reasons (id, reason_name_id, type_1, type_2) VALUES (?, ?, ?, ?)",
                [(i, *k) for i, k in reasons.getIdKeyEntries()],
            )
        except sqlite3.IntegrityError as e:
            print("WARNING:", e, file=sys.stderr)

        con.commit()


class Spliterator:
    def __init__(self, it, n_split):
        self.it = it
        self.n_split = n_split

    def __iter__(self):
        return self

    def __next__(self):
        batch = []
        try:
            for i in range(self.n_split):
                batch.append(next(self.it))
        except StopIteration:
            pass

        if len(batch) == 0:
            raise StopIteration
        else:
            return batch


def read_csv(csv_filename, inputs, targets, reason_names, reasons):
    with open(csv_filename) as f:
        reader = csv.reader(f)
        _ = next(reader)
        for i, row in enumerate(reader):
            yield (
                inputs[(row[INPUT_ID], row[TIME_ID])],
                targets[row[TARGET1_ID]],
                targets[row[TARGET2_ID]],
                reasons[(reason_names[row[REASON_ID]], row[TYPE1_ID], row[TYPE2_ID])],
            )


def fill_data_table(db_name, csv_filename, inputs, targets, reason_names, reasons):
    csv_line = read_csv(csv_filename, inputs, targets, reason_names, reasons)
    spliterator = Spliterator(csv_line, n_split=2048)

    with sqlite3.connect(db_name) as con:
        cur = con.cursor()
        for data in spliterator:
            cur.executemany(
                "INSERT INTO data (input_id, target_1_id, target_2_id, reason_id) VALUES (?, ?, ?, ?)",
                data,
            )
            con.commit()


def main():
    if len(sys.argv) != 3:
        print("ERROR: usage python3 csv2sqlite.py <csvfile> <dbname>")
        exit(1)

    csvfile, db_name = sys.argv[1:]

    create_db(db_name)

    inputs = UniqueId()
    targets = UniqueId()
    reason_names = UniqueId()
    reasons = UniqueId()

    with open(csvfile) as f:
        reader = csv.reader(f)
        columns = tuple(next(reader))

        for i, row in enumerate(reader):
            inputs.add(tuple([row[INPUT_ID], row[TIME_ID]]))
            targets.add(row[TARGET1_ID])
            targets.add(row[TARGET2_ID])
            reason_name_id = reason_names.add(row[REASON_ID])
            reasons.add(tuple([reason_name_id, row[TYPE1_ID], row[TYPE2_ID]]))

    fill_foreign_tables(db_name, inputs, targets, reason_names, reasons)
    fill_data_table(db_name, csvfile, inputs, targets, reason_names, reasons)


if __name__ == "__main__":
    main()
