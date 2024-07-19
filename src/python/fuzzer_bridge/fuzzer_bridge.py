import sys
import struct
import os
from typing import Callable

from .atheris.counter import init_edge_counters
from .atheris.trace_callbacks import set_user_callback_running
from .shm import SharedMemoryManager

PARSER_ERROR = 1

# write at least the exit code (32 bit = 4 byte) to the output
UINT32_T_SIZE = 4
UINT64_T_SIZE = 8


class ToolchainException(RuntimeError):
    pass


def get_env(name):
    value = os.environ.get(name)
    if value is None:
        raise ToolchainException(f"{name} is None")
    return value


def get_int_env(name):
    value = get_env(name)
    try:
        return int(value)
    except ValueError:
        raise ToolchainException(f"{name} is not an integer")


def get_envs():
    prefix = get_env("FUZZER_BRIDGE_PREFIX")
    if len(prefix) == 0:
        raise ToolchainException("prefix is empty")

    max_shm_edges_size = get_int_env("FUZZER_BRIDGE_MAX_SHM_EDGES_SIZE")
    if max_shm_edges_size == 0:
        raise ToolchainException("max_shm_edges_size is 0")

    shm_output_size = get_int_env("FUZZER_BRIDGE_SHM_OUTPUT_SIZE")
    if shm_output_size < UINT32_T_SIZE:
        raise ToolchainException(f"shm_output_size < {UINT32_T_SIZE}")

    fd_in = get_int_env("FUZZER_BRIDGE_FD_IN")
    if fd_in <= 0:
        raise ToolchainException(f"fd_in <= 0")

    fd_out = get_int_env("FUZZER_BRIDGE_FD_OUT")
    if fd_out <= 0:
        raise ToolchainException(f"fd_out <= 0")

    return prefix, max_shm_edges_size, shm_output_size, fd_in, fd_out


def fuzz(user_callback: Callable[[bytes, int], bytes]):
    prefix, max_shm_edges_size, shm_output_size, fd_in, fd_out = get_envs()

    smm_edges = SharedMemoryManager(prefix + "-edges", max_shm_edges_size)
    smm_output = SharedMemoryManager(prefix + "-output", shm_output_size)

    with smm_edges as shm_edges, smm_output as shm_output:
        init_edge_counters(shm_edges.mmap)

        os.write(fd_out, b"\0\0\0\0\0\0\0\0")

        while True:
            ret = os.read(fd_in, UINT64_T_SIZE)
            if len(ret) == 0:
                break

            n_bytes_read: int = struct.unpack("q", ret)[0]
            data = os.read(fd_in, n_bytes_read)

            output = b""
            try:
                set_user_callback_running(True)
                output = user_callback(data, n_bytes_read)
                set_user_callback_running(False)
            except SystemExit as e:
                exit_code = e.code
            except Exception as e:
                exit_code = PARSER_ERROR
            else:
                exit_code = 0

            shm_output.seek(0)
            shm_output.write(struct.pack("i", exit_code))
            shm_output.write(output[: shm_output_size - UINT32_T_SIZE - 1])
            shm_output.write(b"\0")

            os.write(fd_out, b"\0\0\0\0\0\0\0\0")
