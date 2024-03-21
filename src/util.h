#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>

class Runner;

struct ProgramConfig {
    std::string type;
    std::string cmd;
    std::vector<std::string> args;
};

struct Package {
    Package(const uint8_t* raw, uint64_t rawSize);
    Package(const Package&);
    Package(Package&&) noexcept;
    Package& operator=(const Package&);
    Package& operator=(Package&&) noexcept;

    ~Package();

    std::string toString() const;
    static Package fromString(std::string s);

    uint64_t packageSize;
    uint8_t* package;
    uint64_t payloadSize;
    uint8_t* payload;
};

bool registerConfig(Runner& runner, std::string filename,
                    bool withCoverage = true, bool asGoldParser = false);

std::vector<std::vector<uint8_t>> readInputs(std::string from_file);

#endif