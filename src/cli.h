#ifndef CLI_H
#define CLI_H

#include <chrono>
#include <fstream>
#include <memory>

#include "runner.h"
#include "util.h"

#include "FuzzerInternal.h"

struct CLIArguments {
    struct {
        std::vector<std::string> configs;
    } config_args;

    struct {
        std::string inputList;
    } i_args;

    struct {
        std::string timeLogPath;
        std::chrono::seconds logFrequency = std::chrono::seconds(5);
    } t_args;

    struct {
        std::string coverageLogPath;
    } c_args;

    struct {
        enum AnalysisCriterium analysisCriterium =
            AnalysisCriterium::ALL_ACCEPT;
    } ac_args;

    struct {
        std::vector<std::string> gold_parsers;
    } g_args;

    struct {
        enum GoldParserStrategy strategy = GoldParserStrategy::ROUND_ROBIN;
    } gx_args;

    struct {
        std::string out_prefix = "/tmp/local/";
    } o_args;

    bool has_log_rejected = false;
    bool has_ascii_only_flag = false;
    bool has_no_coverage_flag = false;
    bool has_no_numbers_flag = false;
    bool has_no_backslash_u_flag = false;
    bool has_no_cross_language_cov_flag = false;
};

CLIArguments parseArgs(int argc, char** argv);

#endif