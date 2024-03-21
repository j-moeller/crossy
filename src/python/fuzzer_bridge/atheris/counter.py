import mmap
import struct

UINT32_T_SIZE = 4

shm_edges_ptr = mmap.mmap(-1, UINT32_T_SIZE, flags=mmap.MAP_PRIVATE)
n_edges = 0
pristine = True

covered = set()


def reserve_counter():
    global n_edges
    n_edges += 1

    pos = shm_edges_ptr.tell()
    shm_edges_ptr.seek(0)
    shm_edges_ptr.write(struct.pack("i", n_edges))
    shm_edges_ptr.seek(pos)

    return n_edges


def get_number_of_edges():
    global n_edges
    return n_edges


def init_edge_counters(mmap_fd):
    global shm_edges_ptr
    mmap_fd.write(shm_edges_ptr.read(UINT32_T_SIZE))

    if pristine:
        shm_edges_ptr.close()

    shm_edges_ptr = mmap_fd


def inc_counter(edge_id):
    global shm_edges_ptr

    # The first four bytes are reserved for the number of entries
    idx = edge_id + UINT32_T_SIZE

    v = shm_edges_ptr[idx] + 1
    shm_edges_ptr[idx] = v if v <= 255 else 255
