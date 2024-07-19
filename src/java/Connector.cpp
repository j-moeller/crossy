#include "Connector.h"
#include "com_paco_runtime_CoverageMap.h"

#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <jni.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>
#include <vector>

static uint32_t* shm_out;
static uint8_t* shm_edges;
static int32_t shm_edges_size;

__attribute__((constructor)) extern "C" void init()
{
    // ## Load environment names
    std::string prefix = std::string(std::getenv("FUZZER_BRIDGE_PREFIX"));

    // # Init shared memory
    std::string shm_output_name = prefix + "-output";
    int shm_output_size =
        std::atoi(std::getenv("FUZZER_BRIDGE_SHM_OUTPUT_SIZE"));

    int shm_output_fd =
        shm_open(shm_output_name.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_output_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    shm_out = static_cast<uint32_t*>(
        mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, shm_output_fd, 0));

    if (shm_out == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    std::string shm_edges_name = prefix + "-edges";
    shm_edges_size = std::atoi(std::getenv("FUZZER_BRIDGE_MAX_SHM_EDGES_SIZE"));

    std::cout << "[Java] Max size of edges: " << shm_edges_size << std::endl;

    int shm_edges_fd =
        shm_open(shm_edges_name.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_edges_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    shm_edges =
        static_cast<uint8_t*>(mmap(NULL, shm_edges_size, PROT_READ | PROT_WRITE,
                                   MAP_SHARED, shm_edges_fd, 0));

    std::cout << "[Java] Initial size: " << *((uint32_t*)shm_edges)
              << std::endl;

    if (shm_edges == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
}

[[maybe_unused]] JNIEXPORT jobject JNICALL
Java_com_paco_runtime_CoverageMap_getCounterBuffer(JNIEnv* env, jclass a)
{
    std::cout << "[Java] getCounterBuffer(" << shm_edges_size << ")"
              << std::endl;
    jobject bb = (env)->NewDirectByteBuffer((void*)shm_edges, shm_edges_size);
    return bb;
}
