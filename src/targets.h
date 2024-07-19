#ifndef TARGETS_H
#define TARGETS_H

#include <memory>
#include <string>
#include <vector>

#include <inttypes.h>
#include <spawn.h>

#include "process.h"
#include "shared-library.h"
#include "util.h"

#include <iostream>

class Runner;

typedef int (*UserCallback)(const uint8_t* data, size_t size,
                            uint8_t** out_data, size_t* out_size);

const int32_t KB = 1024;
const int32_t MB = 1024 * KB;

const int32_t MAX_SHM_EDGES_SIZE = 2 * MB;
const int32_t SHM_OUTPUT_SIZE = MB;

struct SharedMemory {
    int fd;
    std::string name;
};

template <typename T> struct SharedMemoryRegion {
    T* start = 0;
    T* end = 0;
};

struct __attribute__((packed)) SharedMemoryLayout {
    uint32_t exit_code;
    char data[SHM_OUTPUT_SIZE - sizeof(uint32_t)];
};

void initExecveParams(ExecveParams* execveParams, std::string cmd,
                      std::vector<std::string> args,
                      std::vector<std::string> env);
void freeExecveParams(ExecveParams* execveParams);

SharedMemory createSharedMemory(size_t max_size, std::string name);

/**
 * Target is a base class from which different types of targets can inherit. In
 * each iteration, the targets registered in the Runner are executed on the
 * fuzzing input (using the run method). Since some targets execute
 * asynchronously, we might need to wait until the target is finished
 * (waitForTarget).
 */
class Target
{
  public:
    struct Output {
        uint32_t exit_code;
        std::vector<uint8_t> data;
    };

    virtual ~Target(){};

    virtual std::string getId() const = 0;
    virtual void run(const Package& package) = 0;
    virtual Output waitForTarget() = 0;
    virtual void reset() = 0;

    void addSectionHandler(int section);
    const std::vector<int>& getSectionHandlers() const;

    virtual void checkForNewCoverage(Runner& runner) = 0;

  private:
    std::vector<int> sectionHandlers;
};

/**
 * A SharedLibraryTarget is executed in our main process (i.e. no other process
 * is spawned). Thus, we do not need to wait for the target. However, in this
 * setup, the target must pass coverage information directly to libFuzzer.
 */
class SharedLibraryTarget : public Target
{
  public:
    SharedLibraryTarget(std::string id, SharedLibrary<UserCallback> lib);

    std::string getId() const;
    void run(const Package& package);
    Output waitForTarget();
    void reset();
    void checkForNewCoverage(Runner& runner);

  private:
    std::string id;
    SharedLibrary<UserCallback> lib;
    Output output;
};

/**
 * A ChildProcess spawns a child process in which the target is executed. This
 * is used to enable cross-programming fuzzing. Since a child process runs
 * asynchronous, it needs to implement inter-process communication methods to:
 *  - Read input data from the main process
 *  - Write coverage information to shared memory
 *  - Synchronize execution with main process, i.e. wait for input, execute
 * target method, wait for new input
 */
class ChildProcess : public Target
{
  public:
    ChildProcess(SharedMemory shm_output,
                 SharedMemoryRegion<uint32_t> shm_output_region,
                 SharedMemory shm_edges,
                 SharedMemoryRegion<uint8_t> shm_edges_region,
                 StandardStreams streams, pid_t pid);

    virtual void run(const Package& package) = 0;
    virtual Output waitForTarget() = 0;
    void reset();
    void checkForNewCoverage(Runner& runner);

    // The shared memory where the edge coverage is communicated
    SharedMemory shm_edges;
    SharedMemoryRegion<uint8_t> shm_edges_region;

    // Pointer to the first 4 bytes in the shared memory (size of the currently
    // allocatetd edges); can be changed by child during runs
    volatile const uint32_t* const size_marker;

    // Last known value of the number of allocated edges
    uint32_t n_edges_child;

    // shared memory for return value of a target
    SharedMemory shm_output;
    SharedMemoryRegion<uint32_t> shm_output_region;

    StandardStreams streams;
    pid_t pid;
};

class PersistentProcess : public ChildProcess
{
  public:
    PersistentProcess(std::string id, SharedMemory shm_output,
                      SharedMemoryRegion<uint32_t> shm_output_region,
                      SharedMemory shm_edges,
                      SharedMemoryRegion<uint8_t> shm_edges_region,
                      StandardStreams streams, pid_t pid);

    std::string getId() const;
    virtual void run(const Package& package);
    virtual Output waitForTarget();

  protected:
    std::string id;
};

/**
 *
 */
class CompositionTarget : public Target
{
  public:
    CompositionTarget(std::vector<std::shared_ptr<Target>> targets);

    std::string getId() const;
    Target::Output waitForTarget();
    void run(const Package& package);
    void reset();
    void checkForNewCoverage(Runner& runner);

  private:
    std::vector<std::shared_ptr<Target>> targets;
    Target::Output output;
};

#endif // TARGETS_H