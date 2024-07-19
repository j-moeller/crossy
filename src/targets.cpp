#include "targets.h"

#include <fcntl.h>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>

#include "runner.h"

extern "C" {
pid_t spawnChild(struct ExecveParams execveParams,
                 struct StandardStreams* streams);
}

void initExecveParams(ExecveParams* execveParams, std::string cmd,
                      std::vector<std::string> args,
                      std::vector<std::string> env)
{
    const int len = cmd.size();
    char* buffer = new char[len + 1];
    std::memcpy(buffer, cmd.c_str(), len);
    buffer[len] = 0;
    execveParams->cmd = buffer;

    {
        execveParams->args = new char*[args.size() + 1];
        for (int i = 0; i < args.size(); i++) {
            const int len = args[i].size();
            char* buffer = new char[len + 1];
            std::memcpy(buffer, args[i].c_str(), len);
            buffer[len] = 0;
            execveParams->args[i] = buffer;
        }
        execveParams->args[args.size()] = NULL;
    }

    {
        execveParams->env = new char*[env.size() + 1];
        for (int i = 0; i < env.size(); i++) {
            const int len = env[i].size();
            char* buffer = new char[len + 1];
            std::memcpy(buffer, env[i].c_str(), len);
            buffer[len] = 0;
            execveParams->env[i] = buffer;
        }
        execveParams->env[env.size()] = NULL;
    }
}

void freeExecveParams(ExecveParams* execveParams)
{
    delete[] execveParams->cmd;

    for (int i = 0; execveParams->args[i] != NULL; i++) {
        delete[] execveParams->args[i];
    }
    delete[] execveParams->args;

    for (int i = 0; execveParams->env[i] != NULL; i++) {
        delete[] execveParams->env[i];
    }
    delete[] execveParams->env;
}

SharedMemory createSharedMemory(size_t max_size, std::string name)
{
    const char* shm_name = name.c_str();

    // TODO: Remove this?
    shm_unlink(shm_name);
    int fd = shm_open(shm_name, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, max_size) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    return {fd, shm_name};
}

void Target::addSectionHandler(int section)
{
    this->sectionHandlers.push_back(section);
}

const std::vector<int>& Target::getSectionHandlers() const
{
    return this->sectionHandlers;
}

SharedLibraryTarget::SharedLibraryTarget(std::string id,
                                         SharedLibrary<UserCallback> lib)
    : id(id), lib(lib)
{
}

std::string SharedLibraryTarget::getId() const { return this->id; }

void SharedLibraryTarget::run(const Package& package)
{
    uint8_t* out_data = NULL;
    size_t out_size = 0;

    this->output.exit_code = this->lib.fn(package.payload, package.payloadSize,
                                          &out_data, &out_size);

    // TODO: Use out_size (which includes null-terminator)?
    if (out_data != NULL) {
        this->output.data = std::vector<uint8_t>(out_data, out_data + out_size);
        free(out_data);
    }
}

Target::Output SharedLibraryTarget::waitForTarget() { return this->output; }

void SharedLibraryTarget::reset()
{
    this->output.data = std::vector<uint8_t>();
}

void SharedLibraryTarget::checkForNewCoverage(Runner& runner) {}

ChildProcess::ChildProcess(SharedMemory shm_output,
                           SharedMemoryRegion<uint32_t> shm_output_region,
                           SharedMemory shm_edges,
                           SharedMemoryRegion<uint8_t> shm_edges_region,
                           StandardStreams streams, pid_t pid)
    : shm_edges(shm_edges), shm_edges_region(shm_edges_region),
      size_marker((uint32_t*)(shm_edges_region.start)), n_edges_child(0),
      shm_output(shm_output), shm_output_region(shm_output_region),
      streams(streams), pid(pid)
{
}

void ChildProcess::reset()
{
    // We register the edges with libFuzzer, which should reset the memory for
    // us. Just to be sure we do it ourselves.
    std::memset(this->shm_edges_region.start + sizeof(*this->size_marker), 0,
                *this->size_marker);

    // The returnvalues have to be reset by us
    std::memset(this->shm_output_region.start, 0,
                this->shm_output_region.end - this->shm_output_region.start);
}

void ChildProcess::checkForNewCoverage(Runner& runner)
{
    runner.registerNewEdges(*this);
}

PersistentProcess::PersistentProcess(
    std::string id, SharedMemory shm_output,
    SharedMemoryRegion<uint32_t> shm_output_region, SharedMemory shm_edges,
    SharedMemoryRegion<uint8_t> shm_edges_region, StandardStreams streams,
    pid_t pid)
    : ChildProcess(shm_output, shm_output_region, shm_edges, shm_edges_region,
                   streams, pid),
      id(id)
{
}

std::string PersistentProcess::getId() const { return this->id; }

void PersistentProcess::run(const Package& package)
{
    write(this->streams.fdin[PIPE_WRITE], package.package, package.packageSize);
}

Target::Output PersistentProcess::waitForTarget()
{
    uint64_t buf;
    read(this->streams.fdout[PIPE_READ], &buf, sizeof(buf));

    SharedMemoryLayout* layout = static_cast<SharedMemoryLayout*>(
        static_cast<void*>(this->shm_output_region.start));

    Target::Output output;
    output.exit_code = layout->exit_code;
    output.data =
        std::vector<uint8_t>(layout->data, layout->data + strlen(layout->data));

    return output;
}

CompositionTarget::CompositionTarget(
    std::vector<std::shared_ptr<Target>> targets)
    : targets(targets)
{
}

std::string CompositionTarget::getId() const
{
    std::stringstream ss;
    for (int i = 0; i < this->targets.size(); i++) {
        ss << this->targets[i]->getId();
        if (i + 1 < this->targets.size()) {
            ss << "++";
        }
    }
    return ss.str();
}

Target::Output CompositionTarget::waitForTarget() { return this->output; }

void CompositionTarget::run(const Package& init)
{
    Target::Output output;
    Package package(init);

    for (auto& target : this->targets) {
        target->run(package);
        output = target->waitForTarget();

        if (output.exit_code != 0) {
            output.data = std::vector<uint8_t>();
            break;
        }

        output.data.erase(std::find(output.data.begin(), output.data.end(), 0),
                          output.data.end());

        package = Package(&output.data[0], output.data.size());
    }

    this->output = output;
}

void CompositionTarget::reset()
{
    for (auto& target : this->targets) {
        target->reset();
    }
}

void CompositionTarget::checkForNewCoverage(Runner& runner)
{
    for (auto& target : this->targets) {
        (void)target;
        // TODO:
        // We currently register coverage for all children of the composite, not
        // for the composite itself

        // target->checkForNewCoverage(runner);
    }
}
