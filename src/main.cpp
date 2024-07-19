#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

#include "cli.h"
#include "input-reader.h"
#include "process.h"
#include "runner.h"
#include "shared-library.h"
#include "targets.h"
#include "util.h"

#include "FuzzerInternal.h"

extern "C" {
#include "FuzzerDifferential.h"
}

enum { US_PER_SECOND = 1000000 };
struct timeval add(struct timeval t1, struct timeval t2);
struct timeval sub(struct timeval t1, struct timeval t2);

static Runner runner;
static FuzzerCallbacks callbacks;
static uint64_t iteration = 0;

static CLIArguments cli_args;

static const std::chrono::time_point<std::chrono::steady_clock> startTime =
    std::chrono::steady_clock::now();
static std::ofstream timeLog;

static std::ofstream coverageLog;

static std::unique_ptr<InputReader> inputReader;

extern "C" {
size_t LLVMFuzzerMutate(uint8_t* Data, size_t Size, size_t MaxSize);

pid_t spawnChild(struct ExecveParams execveParams,
                 struct StandardStreams* streams);
}

namespace fuzzer
{
extern bool DisableCoverage;
} // namespace fuzzer

void writeConfigToFile(int argc, char** argv, int* libfuzzer_argc,
                       char*** libfuzzer_argv)
{
    const auto p1 = std::chrono::steady_clock::now();
    const auto timeSinceEpoch =
        std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch())
            .count();

    std::ofstream meta;
    meta.open(cli_args.o_args.out_prefix + "meta.txt", std::ios_base::out);
    meta << "- Start: " << timeSinceEpoch << "\n"
         << "- Arguments (fuzzer_bridge): \n";
    for (int i = 0; i < argc; i++) {
        meta << "\t- " << i << ": " << argv[i] << "\n";
    }
    meta << "- Arguments (libfuzzer): \n";
    for (int i = 0; i < *libfuzzer_argc; i++) {
        meta << "\t- " << i << ": " << (*libfuzzer_argv)[i] << "\n";
    }
}

extern "C" int LLVMFuzzerInitialize(int* libfuzzer_argc, char*** libfuzzer_argv)
{
    std::cout << "**********************************************************\n";
    std::cout << "**********************************************************\n";
    std::cout << "**********************************************************\n";
    std::cout << "*******************Starting fuzzing run*******************\n";
    std::cout << "**********************************************************\n";
    std::cout << "**********************************************************\n";
    std::cout << "**********************************************************\n";

    int argc = *libfuzzer_argc;
    char** argv = *libfuzzer_argv;

    {
        /**
         * All arguments before '--' are passed to our framework and all
         * arguments after are passed to libFuzzer. If no '--' is found, all
         * arguments are passed to our framework.
         */
        bool found = false;

        for (int i = 0; i < argc; i++) {
            if (strncmp(argv[i], "--", strlen(argv[i])) == 0) {
                found = true;
                *libfuzzer_argv = &argv[i];
                *libfuzzer_argc = argc - i;
                argc = i;
                argv[i] = argv[0];
                break;
            }
        }

        if (!found) {
            *libfuzzer_argc = 1;
        }
    }

    cli_args = parseArgs(argc, argv);

    callbacks.startBatch = LLVMFuzzerStartBatch;
    callbacks.endBatch = LLVMFuzzerEndBatch;
    callbacks.startRun = LLVMFuzzerStartRun;
    callbacks.endRun = LLVMFuzzerEndRun;

    {
        auto& args = cli_args.i_args;

        if (args.inputList.size() > 0) {
            // Process-Inputs-Mode
            runner.setIsFuzzingRun(false);
            inputReader = std::make_unique<InputReader>(args.inputList);
        } else {
            // Fuzzing-Mode
            runner.setIsFuzzingRun(true);
            writeConfigToFile(argc, argv, libfuzzer_argc, libfuzzer_argv);
        }
    }

    {
        auto& args = cli_args.t_args;

        if (args.timeLogPath.size() > 0) {
            timeLog.open(args.timeLogPath, std::ios_base::out);
            timeLog << "User(s),User(us),System(s),System(us),Wall-Time(ms),"
                       "Iteration\n";
        }
    }

    if (cli_args.has_no_coverage_flag) {
        fuzzer::DisableCoverage = true;
    }

    runner.setOutputPrefix(cli_args.o_args.out_prefix);
    runner.setAnalysisCriterium(cli_args.ac_args.analysisCriterium);
    runner.setGoldParserStrategy(cli_args.gx_args.strategy);
    runner.setCrossLanguageCoverage(!cli_args.has_no_cross_language_cov_flag);

    for (auto& config : cli_args.config_args.configs) {
        registerConfig(runner, config);
    }

    for (auto& config : cli_args.g_args.gold_parsers) {
        registerConfig(runner, config, false, true);
    }

    {
        auto& args = cli_args.c_args;

        if (args.coverageLogPath.size() > 0) {
            coverageLog.open(args.coverageLogPath, std::ios_base::out);
            runner.setCoverageLogs(&coverageLog);

            coverageLog << "Time,Counter,";
            auto targets = runner.getTargets();
            for (int i = 0; i < targets.size(); i++) {
                coverageLog << targets[i]->getId() << ",";
            }
            coverageLog << "Nezha-Coarse,Nezha-Fine\n";
        }
    }

    return 0;
}

void useCustomInput()
{
    auto item = inputReader->next();
    if (!item.valid) {
        fuzzer::Fuzzer::StaticGracefulExitCallback();
        return;
    }

    fuzzer::Unit& input = item.value;
    runner.runAll(&input[0], input.size(), callbacks);

    const auto& targets = runner.getTargets();
    const auto& outputs = runner.getOutputs();
    const auto& goldParsers = runner.getGoldParsers();
    const auto& goldOutputs = runner.getGoldOutputs();

    std::ofstream f(item.filename + ".out", std::ios::out);

    /*
     * First line consists of all targets
     */
    for (int i = 0; i < targets.size(); i++) {
        f << targets[i]->getId();
        if (i + 1 < targets.size()) {
            f << ":";
        }
    }
    f << "\n";

    /*
     * Second line consists of all gold parsers
     */
    for (int i = 0; i < goldParsers.size(); i++) {
        f << goldParsers[i]->getId();
        if (i + 1 < goldParsers.size()) {
            f << ":";
        }
    }
    f << "\n";

    /*
     * After that we save all outputs in "Package-format", i.e. the first 64 bit
     * determine the number of 8 bit characters of the following 'payload'.
     * There should be {targets.size()+goldParsers.size()} many Packages.
     */
    for (int i = 0; i < outputs.size(); i++) {
        const auto& output = outputs[i].data;
        Package package(&output[0], output.size());
        f << package.toString();
    }

    for (int i = 0; i < goldOutputs.size(); i++) {
        const auto& output = goldOutputs[i].data;
        Package package(&output[0], output.size());
        f << package.toString();
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Raw, size_t Size)
{
    if (cli_args.i_args.inputList.size() > 0) {
        useCustomInput();
        return -1;
    }

    if (cli_args.has_ascii_only_flag) {
        for (size_t i = 0; i < Size; i++) {
            if (Raw[i] >> 7 == 1) {
                return -1;
            }
        }
    }

    if (cli_args.has_no_numbers_flag) {
        for (size_t i = 0; i < Size; i++) {
            if ('0' <= Raw[i] && Raw[i] <= '9') {
                return -1;
            }
        }
    }

    if (cli_args.has_no_backslash_u_flag) {
        for (size_t i = 0; i + 1 < Size; i++) {
            if (i + 1 < Size) {
                if (Raw[i] == '\\' && Raw[i + 1] == 'u') {
                    return -1;
                }
            }
        }
    }

    // Run with time measurement
    if (cli_args.t_args.timeLogPath.size() > 0) {
        static std::chrono::time_point<std::chrono::steady_clock> last =
            std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();

        static struct timeval total_utime = {0};
        static struct timeval total_stime = {0};

        struct rusage pre_usage;
        getrusage(RUSAGE_SELF, &pre_usage);

        runner.runAll(Raw, Size, callbacks);

        struct rusage post_usage;
        getrusage(RUSAGE_SELF, &post_usage);

        total_utime =
            add(total_utime, sub(post_usage.ru_utime, pre_usage.ru_utime));
        total_stime =
            add(total_stime, sub(post_usage.ru_stime, pre_usage.ru_stime));

        if (now - last > cli_args.t_args.logFrequency) {
            last = now;
            auto diff = last - startTime;
            auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(diff);

            timeLog << total_utime.tv_sec << "," << total_utime.tv_usec << ","
                    << total_stime.tv_sec << "," << total_stime.tv_usec << ","
                    << elapsed.count() << "," << iteration << std::endl;
        }
    } else {
        runner.runAll(Raw, Size, callbacks);
    }

    iteration++;

    return 0;
}

struct timeval add(struct timeval t1, struct timeval t2)
{
    struct timeval td;
    td.tv_usec = t2.tv_usec + t1.tv_usec;
    td.tv_sec = t2.tv_sec + t1.tv_sec;
    if (td.tv_usec >= US_PER_SECOND) {
        td.tv_usec -= US_PER_SECOND;
        td.tv_sec++;
    } else if (td.tv_usec <= -US_PER_SECOND) {
        td.tv_usec += US_PER_SECOND;
        td.tv_sec--;
    }
    return td;
}

struct timeval sub(struct timeval t2, struct timeval t1)
{
    struct timeval td;
    td.tv_usec = t2.tv_usec - t1.tv_usec;
    td.tv_sec = t2.tv_sec - t1.tv_sec;
    if (td.tv_sec > 0 && td.tv_usec < 0) {
        td.tv_usec += US_PER_SECOND;
        td.tv_sec--;
    } else if (td.tv_sec < 0 && td.tv_usec > 0) {
        td.tv_usec -= US_PER_SECOND;
        td.tv_sec++;
    }
    return td;
}