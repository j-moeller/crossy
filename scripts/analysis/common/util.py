import os
import struct
import typing
from collections import defaultdict
from dataclasses import dataclass

UINT64_T_SIZE = 8


@dataclass
class Diff:
    user: typing.Tuple[int, int]
    system: typing.Tuple[int, int]
    wall: int
    hashname: str
    gp_index: int
    job_id: str


class Stat:
    def __init__(self):
        self.tp = []
        self.fp = []
        self.fn = []
        self.tn = []


class StatAggregation:
    def __init__(self):
        self.tp = 0
        self.fp = 0
        self.fn = 0
        self.tn = 0


@dataclass
class ErrorClasses:
    tp: bool
    fp: bool
    tn: bool
    fn: bool


class Batcher:
    def __init__(self, it, batch_size):
        self.it = it
        self.batch_size = batch_size

    def __next__(self):
        batch = []

        try:
            for i in range(self.batch_size):
                batch.append(next(self.it))
        except StopIteration:
            if len(batch) == 0:
                raise StopIteration

        return batch

    def __iter__(self):
        return self

    @staticmethod
    def number_of_batches(n_iterations, batch_size):
        if n_iterations % batch_size == 0:
            return n_iterations // batch_size
        else:
            return n_iterations // batch_size + 1


class UniqueId:
    def __init__(self):
        self.key_to_id = {}
        self.id_to_key = {}

    def __getitem__(self, key):
        return self.key_to_id[key]

    def __len__(self):
        return len(self.key_to_id)

    def add(self, key):
        entry = self.key_to_id.get(key)

        if entry is not None:
            return entry

        n_entries = len(self.key_to_id)
        self.key_to_id[key] = n_entries
        self.id_to_key[n_entries] = key

        return self.key_to_id[key]

    def getKey(self, id):
        return self.id_to_key[id]

    def getKeys(self):
        return [self.id_to_key[id] for id in range(len(self.id_to_key))]

    def getIdKeyEntries(self):
        return self.id_to_key.items()


def int_list_to_bytes(output):
    return bytes([int(o) for o in output.split(",") if len(o.strip()) > 0])


def parse_outfile(filename):
    with open(filename, "rb") as f:
        target_names = f.readline().decode("utf-8").strip().split(":")
        gold_names = f.readline().decode("utf-8").strip().split(":")

        if gold_names == [""]:
            gold_names = []

        gold_parsers = []
        for g in gold_names:
            for t in target_names:
                gold_parsers.append((t, g))

        all_parser = [(t,) for t in target_names] + gold_parsers

        outputs = []
        for parser in all_parser:
            payload_size = f.read(UINT64_T_SIZE)
            assert len(payload_size) > 0, f"{filename}: {parser}, {len(outputs)}"
            payload = f.read(struct.unpack("q", payload_size)[0])
            outputs.append((parser, payload))

        return outputs


def is_output_diff(outputs):
    return len(set(outputs)) > 1


def is_majority_diff(gold_parser_outputs):
    # TODO: We got an off-by-one error where mv_diff / mv_no-diff disagree with
    # our calculation

    n_diffs = sum(
        [1 for outputs in gold_parser_outputs.values() if is_output_diff(outputs)]
    )
    return n_diffs > len(gold_parser_outputs) // 2


def get_gold_parser_outputs(outfile_content):
    gold_parser_outputs = defaultdict(lambda: [])

    for target, output in outfile_content:
        if len(target) < 2:
            continue

        gold_parser = target[-1]
        gold_parser_outputs[gold_parser].append(output)

    return gold_parser_outputs


def process_outfile(
    outfile, analyzed_classes: ErrorClasses, candidates: typing.Sequence[str]
):
    def process(outfile, candidates):
        outfile_content = parse_outfile(outfile)
        gold_parser_outputs = get_gold_parser_outputs(outfile_content)

        # First: Does the majority of all(!) parsers think the input is a difference?

        exp_diff = is_majority_diff(gold_parser_outputs)

        # Second (for every candidate-tuple of gold parsers):
        # Does the majority of the candidate tuple think it is a difference?

        for candidate in candidates:
            candidate_outputs = {gp: gold_parser_outputs[gp] for gp in candidate}
            act_diff = is_majority_diff(candidate_outputs)

            yield candidate, exp_diff, act_diff

    stats = {}

    for candidate, exp_diff, act_diff in process(outfile, candidates):
        if candidate not in stats:
            stats[candidate] = Stat()

        if exp_diff is True:
            if act_diff is True:
                if analyzed_classes.tp:
                    stats[candidate].tp.append(outfile)
            else:
                if analyzed_classes.fn:
                    stats[candidate].fn.append(outfile)
        else:
            if act_diff is True:
                if analyzed_classes.fp:
                    stats[candidate].fp.append(outfile)
            else:
                if analyzed_classes.tn:
                    stats[candidate].tn.append(outfile)

    return stats


def parse_difflog(filename, *, job_id=None):
    HEADER = (
        "User(s),User(us),System(s),System(us),Wall-Time(ms),Hashname,Gold-Parser-Index"
    )

    with open(filename) as f:
        header_line = f.readline()
        assert header_line.strip() == HEADER, filename

        for line in f.readlines():
            line = line.strip()
            (
                user_s,
                user_us,
                system_s,
                system_us,
                wall_ms,
                hashname,
                gp_index,
            ) = line.split(",")

            yield Diff(
                user=[int(user_s), int(user_us)],
                system=[int(system_s), int(system_us)],
                wall=int(wall_ms),
                hashname=hashname,
                gp_index=int(gp_index),
                job_id=job_id,
            )


def merge_sorted(*iterators, key=None):
    assert len(iterators) > 0

    if key is None:

        def key(x):
            return x

    iterators = [iter(it) for it in iterators]
    candidates = [next(it, None) for it in iterators]

    while any(element is not None for element in candidates):
        smallest_element = candidates[0]
        smallest_index = 0

        for i, element in enumerate(candidates):
            if element is None:
                continue

            if smallest_element is None or key(element) < key(smallest_element):
                smallest_element = element
                smallest_index = i

        if smallest_element is not None:
            yield smallest_element
            candidates[smallest_index] = next(iterators[smallest_index], None)


def unique_sorted(iterator, *, key=None):
    if key is None:

        def key(x):
            return x

    values = set()

    for item in iterator:
        if key(item) in values:
            continue

        values.add(key(item))
        yield item


def filter_cracks(iterator, *, key=None):
    if key is None:

        def key(x):
            return x

    last_y = None
    last_item = None
    item_remaining = False

    for item in iterator:
        y = key(item)

        if last_item is None:
            yield item
            item_remaining = False
        elif y == last_y:
            item_remaining = True
        else:
            yield last_item
            yield item
            item_remaining = False

        last_item = item
        last_y = y

    if item_remaining:
        yield item


def get_diffs(output_dir, *, unique=True):
    difflog_iterators = []
    for job_id in os.listdir(output_dir):
        job_dir = os.path.join(output_dir, job_id)

        difflog_path = os.path.join(job_dir, "diff-log.txt")
        difflog_iterators.append(parse_difflog(difflog_path, job_id=job_id))

    x = merge_sorted(*difflog_iterators, key=lambda x: x.wall)
    if unique:
        return unique_sorted(x, key=lambda x: x.hashname)
    return x
