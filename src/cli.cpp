#include "cli.h"

#include <iostream>

CLIArguments parseArgs(int argc, char** argv)
{
    CLIArguments cli_args;
    int arg = 1;

    /**
     * A set of config files are given as arguments to our program. Each file
     * corresponds to an fuzzing target and describes how the target is
     * registered in the runner.
     */
    if (argc < 2) {
        std::cerr << "Usage: ./crossy config ... [-i inputs_path] [-g command]\n";
        exit(1);
    }

    /**
     * Read config files
     */
    for (; arg < argc; arg++) {
        auto& args = cli_args.config_args;
        std::string configOrFlag(argv[arg]);

        if (configOrFlag.rfind("-", 0) == 0) {
            break;
        }

        args.configs.push_back(configOrFlag);
    }

    /**
     * Read flags
     */
    for (; arg < argc; arg++) {
        std::string configOrFlag(argv[arg]);

        if (configOrFlag == "-i") {
            auto& args = cli_args.i_args;

            if (++arg >= argc) {
                std::cerr << "Error: Missing input after -i flag\n";
                exit(1);
            }

            args.inputList = argv[arg];
        }

        else if (configOrFlag == "-t") {
            auto& args = cli_args.t_args;

            if (++arg >= argc) {
                std::cerr << "Error: Missing input after -t flag\n";
                exit(1);
            }

            args.timeLogPath = argv[arg];
        }

        else if (configOrFlag == "-c") {
            auto& args = cli_args.c_args;

            if (++arg >= argc) {
                std::cerr << "Error: Missing input after -c flag\n";
                exit(1);
            }

            args.coverageLogPath = argv[arg];
        }

        else if (configOrFlag == "-o") {
            auto& args = cli_args.o_args;

            if (++arg >= argc) {
                std::cerr << "Error: Missing input after -o flag\n";
                exit(1);
            }

            args.out_prefix = argv[arg];
        }

        else if (configOrFlag == "-g") {
            auto& args = cli_args.g_args;

            if (++arg >= argc) {
                std::cerr << "Error: Missing input after -g flag\n";
                exit(1);
            }

            std::string config = argv[arg];
            args.gold_parsers.push_back(config);
        }

        else if (configOrFlag == "-gx") {
            auto& args = cli_args.gx_args;

            if (++arg >= argc) {
                std::cerr << "Error: Missing input after -gx flag\n";
                exit(1);
            }

            std::string strategy = argv[arg];
            if (strategy == "round-robin") {
                args.strategy = GoldParserStrategy::ROUND_ROBIN;
            } else if (strategy == "majority") {
                args.strategy = GoldParserStrategy::MAJORITY;
            } else {
                std::cerr << "Error: Unknown gold strategy '" << strategy
                          << "'\n";
                exit(1);
            }

        }

        else if (configOrFlag == "--log-rejected") {
            cli_args.has_log_rejected = true;
        }

        else if (configOrFlag == "--ascii-only") {
            cli_args.has_ascii_only_flag = true;
        }

        else if (configOrFlag == "--no-coverage") {
            cli_args.has_no_coverage_flag = true;
        }

        else if (configOrFlag == "--no-numbers") {
            cli_args.has_no_numbers_flag = true;
        }

        else if (configOrFlag == "--no-backslash-u") {
            cli_args.has_no_backslash_u_flag = true;
        }

        else if (configOrFlag == "--no-cross-language-coverage") {
            cli_args.has_no_cross_language_cov_flag = true;
        }

        else if (configOrFlag == "--analysis-criterium") {
            auto& args = cli_args.ac_args;

            if (++arg >= argc) {
                std::cerr
                    << "Error: Missing input after --analysis-criterium flag\n";
                exit(1);
            }

            std::string c = argv[arg];
            if (c == "all") {
                args.analysisCriterium = AnalysisCriterium::ALL_ACCEPT;
            } else if (c == "at_least_one") {
                args.analysisCriterium =
                    AnalysisCriterium::AT_LEAST_ONE_ACCEPTS;
            } else if (c == "discard") {
                args.analysisCriterium = AnalysisCriterium::DISCARD;
            } else {
                std::cerr << "Error: Unknown exit criterium '" << c << "'\n";
                exit(1);
            }
        }

        else {
            std::cerr << "Error: Unknown flag: " << configOrFlag << "\n";
            exit(EXIT_FAILURE);
        }
    }

    if (cli_args.gx_args.strategy == GoldParserStrategy::MAJORITY) {
        if (cli_args.g_args.gold_parsers.size() < 3 ||
            cli_args.g_args.gold_parsers.size() % 2 == 0) {
            std::cerr << "Majority voting requires at least 3 parsers and an "
                         "uneven number of parsers"
                      << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    return cli_args;
}