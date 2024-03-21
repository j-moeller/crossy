#ifndef RUNNER_H
#define RUNNER_H

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <semaphore.h>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "process.h"
#include "shared-library.h"
#include "targets.h"
#include "util.h"

typedef void (*StartBatchCallback)(int);
typedef void (*EndBatchCallback)();

typedef int (*StartRunCallback)();
typedef void (*EndRunCallback)(const int*, int, int);

enum AnalysisCriterium { DISCARD, AT_LEAST_ONE_ACCEPTS, ALL_ACCEPT };
enum GoldParserStrategy { ROUND_ROBIN, MAJORITY };

struct FuzzerCallbacks {
    StartBatchCallback startBatch;
    EndBatchCallback endBatch;
    StartRunCallback startRun;
    EndRunCallback endRun;
};

struct PCTableEntry {
    void* pc;
    long flags;
};

class Runner
{
  public:
    Runner();

    void setIsFuzzingRun(bool isFuzzingRun);
    void setOutputPrefix(std::string out_prefix);
    void setGoldParserStrategy(GoldParserStrategy strategy);
    void setAnalysisCriterium(AnalysisCriterium ac);
    void setLogRejected(bool logRejected);
    void setCoverageLogs(std::ofstream* coverageLog);
    void setCrossLanguageCoverage(bool);

    void registerPythonProgram(std::string id, std::string cmd,
                               std::vector<std::string> args,
                               std::vector<std::string> env, bool asGoldParser);

    void registerJSProgram(std::string id, std::string cmd,
                           std::vector<std::string> args,
                           std::vector<std::string> env, bool asGoldParser);

    void registerJavaProgram(std::string id, std::string classPath,
                             std::string className, std::string methodName,
                             bool asGoldParser);

    void registerSharedLib(std::string id, std::string path, std::string fnname,
                           bool asGoldParser);

    uint32_t registerNewEdges(ChildProcess& child);

    void runAll(const uint8_t* data, size_t size);
    void runAll(const uint8_t* data, size_t size, FuzzerCallbacks run);

    std::vector<std::shared_ptr<Target>> getTargets() const;
    std::vector<std::shared_ptr<Target>> getGoldParsers() const;
    std::vector<Target::Output> getOutputs() const;
    std::vector<Target::Output> getGoldOutputs() const;

    std::shared_ptr<Target> popLastTarget();
    void addTarget(std::shared_ptr<Target> target, bool asGoldParser);

  private:
    void registerExternalProgram(std::string id, std::string cmd,
                                 std::vector<std::string> args,
                                 std::vector<std::string> env,
                                 bool asGoldParser);

    bool analyzeForDifference(const uint8_t* Input, size_t InputSize);
    void executeGoldParsers(const uint8_t* Input, size_t InputSize);

    bool hasDifference(std::vector<Target::Output> goldOutputs, const int begin,
                       const int end);
    bool doMajorityVoting(std::vector<bool> v);

    Target::Output process(std::shared_ptr<Target> target,
                           const Target::Output& input);

    void saveDifference(const uint8_t* Input, size_t InputSize, std::string p);

    std::vector<Target::Output> outputs;
    std::vector<Target::Output> goldOutputs;
    std::vector<std::shared_ptr<Target>> targets;
    std::vector<std::shared_ptr<Target>> goldParsers;
    std::vector<std::set<unsigned long>> coveredEdges;

    int roundRobinIdx = 0;

    bool isFuzzingRun = true;
    std::string out_prefix = "/tmp/local/";
    GoldParserStrategy goldParserStrategy = GoldParserStrategy::ROUND_ROBIN;
    AnalysisCriterium analysisCriterium = AnalysisCriterium::ALL_ACCEPT;
    bool logRejected = false;
    bool crossLanguageCoverage = true;

    std::ofstream* coverageLog = nullptr;
    std::ofstream differenceLog;
};

#endif
