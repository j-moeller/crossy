import mmap

import posix_ipc


class SharedMemoryManager:
    class Connection:
        def __init__(self, shm, size):
            self.shm = shm
            self.mmap = mmap.mmap(self.shm.fd, size)

        def seek(self, pos):
            self.mmap.seek(pos)

        def write(self, msg: bytes):
            self.mmap.write(msg)

    def __init__(self, name, size):
        self.name = name
        self.size = size

    def __enter__(self):
        shm = posix_ipc.SharedMemory(self.name)
        self.con = SharedMemoryManager.Connection(shm, self.size)
        return self.con

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.con.shm.close_fd()
