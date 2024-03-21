#include "runner.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <signal.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "FuzzerIO.h"
#include "FuzzerSHA1.h"

extern "C" {
#include "FuzzerDifferential.h"
}

const std::string SHARED_PREFIX = "/fuzzer-bridge-";

extern "C" {
pid_t spawnChild(struct ExecveParams execveParams,
                 struct StandardStreams* streams);
void __sanitizer_cov_8bit_counters_init(uint8_t* start, uint8_t* end);
void __sanitizer_cov_pcs_init(const uintptr_t* pcs_beg,
                              const uintptr_t* pcs_end);
}

int nop_startruncallback() { return -1; }
void nop_endruncallback(const int*, int, int) {}
void nop_startbatchcallback(int) {}
void nop_endbatchcallback() {}

void handleSignal(int signo, siginfo_t* info, void* context)
{
    if (signo == SIGCHLD) {
        if (info->si_status == CLD_EXITED || info->si_status == CLD_KILLED ||
            info->si_status == CLD_DUMPED || info->si_status == CLD_STOPPED) {
            // TODO: Only if a _target_ died, we need to exit.
            // => Create a mapping (pid => target) and query the list here.
            // exit(1);
        }
    }
}

Runner::Runner()
{
    struct sigaction act = {{}};

    sigset_t mask;
    sigfillset(&mask);

    act.sa_flags = SA_SIGINFO | SA_RESTART;
    act.sa_sigaction = &handleSignal;
    act.sa_mask = mask;

    sigaction(SIGCHLD, &act, NULL);
}

void Runner::setIsFuzzingRun(bool isFuzzingRun)
{
    this->isFuzzingRun = isFuzzingRun;
}

void Runner::setOutputPrefix(std::string out_prefix)
{
    this->out_prefix = out_prefix;
    this->differenceLog.open(out_prefix + "diff-log.txt", std::ios_base::out);
    this->differenceLog
        << "User(s),User(us),System(s),System(us),Wall-Time(ms),"
           "Hashname,Gold-Parser-Index\n";
}

void Runner::setGoldParserStrategy(GoldParserStrategy strategy)
{
    this->goldParserStrategy = strategy;
}

void Runner::setAnalysisCriterium(AnalysisCriterium ac)
{
    this->analysisCriterium = ac;
}

void Runner::setLogRejected(bool logRejected)
{
    this->logRejected = logRejected;
}

void Runner::setCoverageLogs(std::ofstream* coverageLog)
{
    this->coverageLog = coverageLog;
}

void Runner::setCrossLanguageCoverage(bool crossLanguageCoverage)
{
    this->crossLanguageCoverage = crossLanguageCoverage;
}

void Runner::registerPythonProgram(std::string id, std::string cmd,
                                   std::vector<std::string> args,
                                   std::vector<std::string> env,
                                   bool asGoldParser)
{
    this->registerExternalProgram(id, cmd, args, env, asGoldParser);
}

void Runner::registerJSProgram(std::string id, std::string cmd,
                               std::vector<std::string> args,
                               std::vector<std::string> env, bool asGoldParser)
{
    this->registerExternalProgram(id, cmd, args, env, asGoldParser);
}

void Runner::registerJavaProgram(std::string id, std::string classPath,
                                 std::string className, std::string methodName,
                                 bool asGoldParser)
{
    std::string cmd = "./build/java/jvm_launcher";

    std::vector<std::string> args;
    args.push_back(classPath);
    args.push_back(className);
    args.push_back(methodName);

    std::vector<std::string> env;

    this->registerExternalProgram(id, cmd, args, env, asGoldParser);
}

void Runner::registerExternalProgram(std::string id, std::string cmd,
                                     std::vector<std::string> args,
                                     std::vector<std::string> env,
                                     bool asGoldParser)
{
    StandardStreams streams;
    if (pipe(streams.fdin) < 0) {
        perror("pipe fdin");
        exit(EXIT_FAILURE);
    }

    if (pipe(streams.fdout) < 0) {
        perror("pipe fdout");
        exit(EXIT_FAILURE);
    }

    if (pipe(streams.fderr) < 0) {
        perror("pipe fderr");
        exit(EXIT_FAILURE);
    }

    // We pass the prefix of shared memory, semaphores, etc. as the
    // first argument to our child process.
    std::string prefix;
    {
        std::stringstream ss;
        ss << SHARED_PREFIX << this->targets.size();
        prefix = ss.str();
        env.push_back("FUZZER_BRIDGE_PREFIX=" + prefix);
    }

    {
        std::stringstream ss;
        ss << "FUZZER_BRIDGE_MAX_SHM_EDGES_SIZE=" << MAX_SHM_EDGES_SIZE;
        env.push_back(ss.str());
    }

    {
        std::stringstream ss;
        ss << "FUZZER_BRIDGE_SHM_OUTPUT_SIZE=" << SHM_OUTPUT_SIZE;
        env.push_back(ss.str());
    }

    {
        std::stringstream ss;
        ss << "FUZZER_BRIDGE_FD_IN=" << streams.fdin[PIPE_READ];
        env.push_back(ss.str());
    }

    {
        std::stringstream ss;
        ss << "FUZZER_BRIDGE_FD_OUT=" << streams.fdout[PIPE_WRITE];
        env.push_back(ss.str());
    }

    // Init shared memory
    SharedMemory shm_edges =
        createSharedMemory(MAX_SHM_EDGES_SIZE, prefix + "-edges");
    SharedMemory shm_output =
        createSharedMemory(SHM_OUTPUT_SIZE, prefix + "-output");

    SharedMemoryRegion<uint32_t> shm_output_region;
    {
        uint32_t* rv = static_cast<uint32_t*>(
            mmap(nullptr, SHM_OUTPUT_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                 shm_output.fd, 0));

        if (rv == MAP_FAILED) {
            perror("mmap (output)");
            exit(EXIT_FAILURE);
        }

        shm_output_region = {rv, rv + SHM_OUTPUT_SIZE / sizeof(*rv)};
    }

    SharedMemoryRegion<uint8_t> shm_edges_region;
    {
        uint8_t* rv = static_cast<uint8_t*>(mmap(nullptr, MAX_SHM_EDGES_SIZE,
                                                 PROT_READ | PROT_WRITE,
                                                 MAP_SHARED, shm_edges.fd, 0));

        if (rv == MAP_FAILED) {
            perror("mmap (edges)");
            exit(EXIT_FAILURE);
        }

        shm_edges_region = {rv, rv + MAX_SHM_EDGES_SIZE / sizeof(*rv)};

        uint32_t* size_marker = (uint32_t*)shm_edges_region.start;
        *size_marker = 0;
    }

    // Prepare parameters for execve()
    ExecveParams execveParams;
    initExecveParams(&execveParams, cmd, args, env);

    // Spawn child
    pid_t pid = spawnChild(execveParams, &streams);

    // Wait for child to be initialized
    auto child = std::make_shared<PersistentProcess>(
        id, shm_output, shm_output_region, shm_edges, shm_edges_region, streams,
        pid);

    // Get size of shared memory / number of edges from child
    child->waitForTarget();

    // TODO: GoldParsers should never register (even dynamically). This is
    // currently not handled
    if (!asGoldParser) {
        child->checkForNewCoverage(*this);
    }

    freeExecveParams(&execveParams);

    if (asGoldParser) {
        this->goldParsers.push_back(child);
    } else {
        this->coveredEdges.push_back(std::set<unsigned long>());
        this->targets.push_back(child);
    }
}

void Runner::registerSharedLib(std::string id, std::string path,
                               std::string fnname, bool asGoldParser)
{
    SharedLibrary<UserCallback> lib;
    if (get_interface_fn<UserCallback>(&lib, path.c_str(), fnname.c_str()) !=
        EXIT_SUCCESS) {
        perror("get_interface_fn");
        exit(EXIT_FAILURE);
    }

    auto target = std::make_shared<SharedLibraryTarget>(id, lib);

    if (asGoldParser) {
        this->goldParsers.push_back(target);
    } else {
        this->coveredEdges.push_back(std::set<unsigned long>());
        this->targets.push_back(target);
    }
}

void Runner::runAll(const uint8_t* Data, size_t Size)
{
    FuzzerCallbacks cb;
    cb.startRun = &nop_startruncallback;
    cb.endRun = &nop_endruncallback;
    cb.startBatch = &nop_startbatchcallback;
    cb.endBatch = &nop_endbatchcallback;

    this->runAll(Data, Size, cb);
}

Target::Output Runner::process(std::shared_ptr<Target> target,
                               const Target::Output& input)
{
    if (input.exit_code != 0) {
        Target::Output output;
        output.exit_code = input.exit_code;
        return output;
    }

    target->run({&input.data[0], input.data.size()});
    Target::Output output = target->waitForTarget();

    // TODO: Quick fix
    if (output.data.size() == 0) {
        output.exit_code = 1;
    }

    if (output.exit_code == 1) {
        output.data.clear();
    }

    // Remove everything after '\0'
    output.data.erase(std::find(output.data.begin(), output.data.end(), 0),
                      output.data.end());

    return output;
}

void Runner::runAll(const uint8_t* Data, size_t Size, FuzzerCallbacks cb)
{
    static uint64_t counter = 0;
    counter++;

    Target::Output input;
    input.exit_code = 0;
    input.data = std::vector<uint8_t>(Data, Data + Size);

    this->outputs.clear();
    for (auto target : this->targets) {
        target->reset();
    }

    for (int i = 0; i < this->targets.size(); i++) {
        this->targets[i]->checkForNewCoverage(*this);
    }

    bool coverageIncreased = false;
    int oldCoarse, oldFine;
    LLVMNezhaCoverage(&oldCoarse, &oldFine);

    cb.startBatch(this->targets.size());
    for (int i = 0; i < this->targets.size(); i++) {
        Target::Output output;
        auto& target = targets[i];
        auto& handlers = target->getSectionHandlers();

        int targetIndex = cb.startRun();

        {
            output = this->process(target, input);
            this->outputs.push_back(output);
        }

        cb.endRun(&handlers[0], handlers.size(), output.exit_code);

        if (targetIndex >= 0) {
            if (targetIndex != i) {
                std::cerr << "TARGET INDEX IS NOT THE SAME AS THE ITERATOR IN "
                             "THE CURRENT ITERATION"
                          << i << " vs " << targetIndex << std::endl;
                exit(1);
            }

            const unsigned long* edges;
            int edgeSize;

            LLVMTargetCoverage(targetIndex, &edges, &edgeSize);

            int prevSize = this->coveredEdges[i].size();
            this->coveredEdges[i].insert(edges, edges + edgeSize);
            int newSize = this->coveredEdges[i].size();

            if (newSize > prevSize) {
                coverageIncreased = true;
            }
        }
    }
    cb.endBatch();

    if (this->coverageLog) {
        int newCoarse, newFine;
        LLVMNezhaCoverage(&newCoarse, &newFine);

        if (coverageIncreased || newCoarse > oldCoarse || newFine > oldFine) {
            const auto p1 = std::chrono::system_clock::now();
            const auto timeSinceEpoch =
                std::chrono::duration_cast<std::chrono::seconds>(
                    p1.time_since_epoch())
                    .count();

            *(this->coverageLog) << timeSinceEpoch << "," << counter << ",";

            for (int i = 0; i < this->targets.size(); i++) {
                *(this->coverageLog) << this->coveredEdges[i].size() << ",";
            }

            *(this->coverageLog) << newCoarse << "," << newFine << std::endl;
        }
    }

    if (this->isFuzzingRun) {
        this->analyzeForDifference(Data, Size);
    } else {
        this->executeGoldParsers(Data, Size);
    }
}

bool criteriumMet(AnalysisCriterium criterium, std::vector<int> ExitCodes)
{
    if (criterium == AnalysisCriterium::DISCARD) {
        return false;
    }

    if (criterium == AnalysisCriterium::AT_LEAST_ONE_ACCEPTS) {
        for (auto& e : ExitCodes) {
            if (e == 0) {
                return true;
            }
        }
        return false;
    }

    if (criterium == AnalysisCriterium::ALL_ACCEPT) {
        bool allZero = true;

        for (auto& e : ExitCodes) {
            if (e != 0) {
                allZero = false;
                break;
            }
        }

        return allZero;
    }

    return false;
}

void Runner::saveDifference(const uint8_t* Input, size_t InputSize,
                            std::string category)
{
    if (this->out_prefix.size() == 0) {
        return;
    }

    fuzzer::Unit u({Input, Input + InputSize});
    std::string hashname = fuzzer::Hash(u);

    std::string noPrefixPath = category + "/" + hashname.substr(0, 2) + "/";
    std::string path = this->out_prefix + noPrefixPath;

    std::string filename = hashname.substr(2);

    fuzzer::MkDirRecursive(path);
    fuzzer::WriteToFile(u, path + filename);

    const auto p1 = std::chrono::system_clock::now();
    const auto timeSinceEpoch =
        std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch())
            .count();

    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    this->differenceLog << usage.ru_utime.tv_sec << ","
                        << usage.ru_utime.tv_usec << ","
                        << usage.ru_stime.tv_sec << ","
                        << usage.ru_stime.tv_usec << "," << timeSinceEpoch
                        << "," << (noPrefixPath + filename) << ","
                        << this->roundRobinIdx << std::endl;
}

bool Runner::analyzeForDifference(const uint8_t* Input, size_t InputSize)
{
    std::vector<int> exitCodes;
    for (auto& o : this->outputs) {
        exitCodes.push_back(o.exit_code);
    }

    if (!criteriumMet(analysisCriterium, exitCodes)) {
        return false;
    }

    /*
     * If no gold parser is given, we simply compare the outputs directly.
     */
    if (this->goldParsers.size() == 0) {
        if (this->hasDifference(this->outputs, 0, this->outputs.size())) {
            this->saveDifference(Input, InputSize, "bare_diff");
            return true;
        }

        return false;
    }

    /*
     * Update the current gold parser
     */
    this->roundRobinIdx = (this->roundRobinIdx + 1) % this->goldParsers.size();

    /*
     * Reserve space for the outputs
     *
     * Memory layout for parser i (p_i) and output j (x_j):
     *
     * | p_0(x_0) | p_0(x_1) | p_0(x_2) | ... | p_1(x_0) | p_1(x_1) | ... |
     *
     * The idea is that a parser 'normalizes' the output so we can compare each
     * output of the same parser p_i, i.e. we will compare every output of p_0
     * against each other, every output of p_1 against each other and so on.
     */
    this->goldOutputs = std::vector<Target::Output>(this->outputs.size() *
                                                    this->goldParsers.size());

    /*
     * Calculate the output for the current gold parser
     */
    const int parserOffset = this->roundRobinIdx * this->outputs.size();
    for (int outputId = 0; outputId < this->outputs.size(); outputId++) {
        const auto& output = this->outputs[outputId];
        this->goldOutputs[parserOffset + outputId] =
            this->process(this->goldParsers[this->roundRobinIdx], output);
    }

    /*
     * Compare the outputs of the current gold parser against each other.
     */
    const int begin = this->roundRobinIdx * this->outputs.size();
    const int end = (this->roundRobinIdx + 1) * this->outputs.size();

    bool hasDifference = this->hasDifference(this->goldOutputs, begin, end);
    if (!hasDifference) {
        if (this->logRejected) {
            this->saveDifference(Input, InputSize, "rr_no-diff");
        }
        return false;
    }

    if (this->goldParserStrategy == GoldParserStrategy::ROUND_ROBIN) {
        this->saveDifference(Input, InputSize, "rr_diff");
        return true;
    }

    /*
     * After processing the input with each parser, we use a list of 'gold
     * parsers' to normalize the outputs.
     *
     * If the outputs of the parsers differ, after we processed them with
     * one gold parser, we process them with each gold parser and do a
     * majority voting to find the decision.
     */

    /*
     * Determine for each gold parser whether they found a difference
     */
    std::vector<bool> differences(this->goldParsers.size());

    /*
     * We already evaluated it for the current gold parser and can just fill its
     * result in
     */
    differences[this->roundRobinIdx] = hasDifference;

    /*
     * Now we run all remaining gold parsers
     */
    for (int i = 0; i < this->goldParsers.size(); i++) {
        if (i == this->roundRobinIdx) {
            continue;
        }

        const int parserOffset = i * this->outputs.size();
        const int nextParserOffset = (i + 1) * this->outputs.size();

        for (int outputId = 0; outputId < this->outputs.size(); outputId++) {
            const auto& output = this->outputs[outputId];
            this->goldOutputs[parserOffset + outputId] =
                this->process(this->goldParsers[i], output);
        }

        differences[i] = this->hasDifference(this->goldOutputs, parserOffset,
                                             nextParserOffset);
    }

    /*
     * If we found a difference according to the majority voting, we save the
     * difference to a file
     */
    bool foundDifference = this->doMajorityVoting(differences);
    if (foundDifference) {
        this->saveDifference(Input, InputSize, "mv_diff");
        return true;
    } else {
        if (this->logRejected) {
            this->saveDifference(Input, InputSize, "mv_no-diff");
        }
        return false;
    }
}

void Runner::executeGoldParsers(const uint8_t* Input, size_t InputSize)
{
    if (this->goldParsers.size() == 0) {
        return;
    }

    this->goldOutputs = std::vector<Target::Output>(this->outputs.size() *
                                                    this->goldParsers.size());

    for (int parserId = 0; parserId < this->goldParsers.size(); parserId++) {
        const int parserOffset = parserId * this->outputs.size();

        for (int outputId = 0; outputId < this->outputs.size(); outputId++) {
            const auto& output = this->outputs[outputId];
            this->goldOutputs[parserOffset + outputId] =
                this->process(this->goldParsers[parserId], output);
        }
    }
}

bool Runner::hasDifference(std::vector<Target::Output> goldOutputs,
                           const int begin, const int end)
{
    /*
     * Because of the transitivity of '==', we can simply check everything
     * against the first item. If we found a difference, we can return
     * early.
     */
    const auto& ref = goldOutputs[begin];

    for (int i = begin + 1; i < end; i++) {
        if (ref.exit_code != goldOutputs[i].exit_code) {
            return true;
        }

        if (ref.data != goldOutputs[i].data) {
            return true;
        }
    }

    return false;
}

bool Runner::doMajorityVoting(std::vector<bool> v)
{
    return std::count(v.begin(), v.end(), true) > (v.size() / 2);
}

std::vector<std::shared_ptr<Target>> Runner::getTargets() const
{
    return this->targets;
}

std::vector<std::shared_ptr<Target>> Runner::getGoldParsers() const
{
    return this->goldParsers;
}

std::vector<Target::Output> Runner::getOutputs() const { return this->outputs; }

std::vector<Target::Output> Runner::getGoldOutputs() const
{
    return this->goldOutputs;
}

std::shared_ptr<Target> Runner::popLastTarget()
{
    assert(this->targets.size() > 0);
    std::shared_ptr<Target> target = this->targets[this->targets.size() - 1];
    this->targets.pop_back();
    this->coveredEdges.pop_back();
    return target;
}

void Runner::addTarget(std::shared_ptr<Target> target, bool asGoldParser)
{
    if (asGoldParser) {
        this->goldParsers.push_back(target);
    } else {
        this->coveredEdges.push_back(std::set<unsigned long>());
        this->targets.push_back(target);
    }
}

uint32_t Runner::registerNewEdges(ChildProcess& child)
{
    if (!this->crossLanguageCoverage) {
        return 0;
    }

    uint32_t current = *child.size_marker;
    uint32_t last = child.n_edges_child;

    if (current == last) {
        return 0;
    }

    if (current < last) {
        std::cout << "[" << child.getId() << "] Current: " << current << " vs "
                  << "Last: " << last << std::endl;
        perror("registerNewEdges (negative new edges)");
        exit(EXIT_FAILURE);
    }

    uint32_t n_new = current - last;
    std::cout << "[Runner] Register new edges: " << n_new << "(" << last << "->"
              << current << ") for " << child.getId() << std::endl;

    const int IGNORED_PARAM = -1;
    PCTableEntry* pctable = static_cast<PCTableEntry*>(
        mmap(nullptr, n_new * sizeof(PCTableEntry), PROT_READ | PROT_WRITE,
             MAP_ANONYMOUS | MAP_PRIVATE, IGNORED_PARAM, 0));

    if (pctable == MAP_FAILED) {
        perror("mmap (pctable)");
        exit(EXIT_FAILURE);
    }

    // COVERAGE
    uint8_t* start = child.shm_edges_region.start + sizeof(uint32_t) + last;
    uint8_t* end = child.shm_edges_region.start + sizeof(uint32_t) + current;

    LLVMFuzzerStartRegistration();
    __sanitizer_cov_8bit_counters_init(start, end);

    // PCS
    __sanitizer_cov_pcs_init(reinterpret_cast<uintptr_t*>(pctable),
                             reinterpret_cast<uintptr_t*>(pctable + n_new));
    int sectionHandler = LLVMFuzzerEndRegistration();

    child.addSectionHandler(sectionHandler);

    child.n_edges_child = current;
    return n_new;
}
