#include "util.h"

#include "runner.h"

#include <fstream>
#include <iostream>
#include <sstream>

extern "C" {
#include "FuzzerDifferential.h"
};

bool readConfig(std::string filename, ProgramConfig* config)
{
    std::ifstream infile(filename);
    std::string line;

    int read = 0;

    while (std::getline(infile, line)) {
        if (line[0] == '#') {
            continue;
        }

        if (read == 0) {
            config->type = line;
        } else if (read == 1) {
            config->cmd = line;
        } else {
            config->args.push_back(line);
        }

        read++;
    }

    return true;
}

bool registerConfig(Runner& runner, std::string filename, bool withCoverage,
                    bool asGoldParser)
{
    if (filename.at(filename.size() - 1) == '.') {
        std::cerr << "Skipping config file: " << filename << std::endl;
        return false;
    }

    ProgramConfig config;
    if (!readConfig(filename, &config)) {
        std::cerr << "Could not parse config file: " << filename << std::endl;
        return false;
    }

    if (withCoverage) {
        LLVMFuzzerStartRegistration();
    }

    if (config.type == "python") {
        runner.registerPythonProgram(filename, config.cmd, config.args,
                                     std::vector<std::string>(), asGoldParser);
    } else if (config.type == "javascript") {
        runner.registerJSProgram(filename, config.cmd, config.args,
                                 std::vector<std::string>(), asGoldParser);
    } else if (config.type == ".so") {
        runner.registerSharedLib(filename, config.cmd, config.args[0],
                                 asGoldParser);
    } else if (config.type == "java") {
        runner.registerJavaProgram(filename, config.args[0], config.args[1],
                                   config.args[2], asGoldParser);
    } else if (config.type == "composition") {
        std::vector<std::shared_ptr<Target>> targets;
        for (auto& arg : config.args) {
            if (!registerConfig(runner, arg, false)) {
                std::cerr << "Registration of composition element failed: "
                          << arg << std::endl;
                return false;
            }
            targets.push_back(runner.popLastTarget());
        }
        runner.addTarget(std::make_shared<CompositionTarget>(targets),
                         asGoldParser);
    } else {
        std::cerr << "Unknown runner type: '" << config.type << "'"
                  << " in " << filename << std::endl;
        exit(1);
    }

    if (withCoverage) {
        int handler = LLVMFuzzerEndRegistration();

        auto target = runner.popLastTarget();
        target->addSectionHandler(handler);
        runner.addTarget(target, asGoldParser);
    }

    return true;
}

std::vector<std::vector<uint8_t>> readInputs(std::string from_file)
{
    std::vector<std::vector<uint8_t>> inputs;
    std::ifstream list_file(from_file, std::ios::in);

    std::string line;
    while (std::getline(list_file, line)) {
        if (line.size() == 0 || line.rfind("#", 0) == 0) {
            continue;
        }

        std::ifstream input(line, std::ios::binary);
        std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(input), {});

        if (buffer.size() > 0) {
            inputs.push_back(buffer);
        }
    }

    return inputs;
}

Package::Package(const uint8_t* Data, uint64_t Size)
    : packageSize(Size + sizeof(Size)), package(new uint8_t[packageSize]),
      payloadSize(Size), payload(&package[sizeof(payloadSize)])
{
    std::memcpy(&package[0], &payloadSize, sizeof(payloadSize));
    std::memcpy(&package[sizeof(payloadSize)], Data, payloadSize);
}

Package::Package(const Package& o)
    : packageSize(o.packageSize), package(new uint8_t[packageSize]),
      payloadSize(o.payloadSize), payload(&package[sizeof(payloadSize)])
{
    std::memcpy(package, o.package, o.packageSize);
}

Package::Package(Package&& o) noexcept
    : packageSize(o.packageSize), package(o.package),
      payloadSize(o.payloadSize), payload(o.payload)
{
    o.package = nullptr;
    o.packageSize = 0;
    o.payload = nullptr;
    o.payloadSize = 0;
}

Package& Package::operator=(const Package& o)
{
    if (this == &o) {
        return *this;
    }

    if (this->package != nullptr) {
        delete[] this->package;
    }

    this->packageSize = o.packageSize;
    this->payloadSize = o.payloadSize;
    this->package = new uint8_t[this->packageSize];
    this->payload = &this->package[sizeof(this->payloadSize)];

    return *this;
}

Package& Package::operator=(Package&& o) noexcept
{
    if (this == &o) {
        return *this;
    }

    if (this->package != nullptr) {
        delete[] this->package;
    }

    this->packageSize = o.packageSize;
    this->payloadSize = o.payloadSize;
    this->package = o.package;
    this->payload = o.payload;

    o.package = nullptr;
    o.packageSize = 0;
    o.payload = nullptr;
    o.payloadSize = 0;

    return *this;
}

Package::~Package()
{
    if (this->package != nullptr) {
        delete[] this->package;
    }
}

std::string Package::toString() const
{
    return std::string(this->package, this->package + this->packageSize);
}

Package Package::fromString(std::string s)
{
    if (s.size() < sizeof(Package::packageSize)) {
        std::cerr << "Package::fromString (packageSize): " << s.size() << "<"
                  << sizeof(Package::packageSize) << std::endl;
        throw "up";
    }

    const char* characters = s.c_str();
    uint64_t payloadSize = *(uint64_t*)characters;

    if (s.size() - sizeof(Package::payloadSize) < payloadSize) {
        std::cerr << "Package::fromString (payloadSize): "
                  << (s.size() - sizeof(Package::payloadSize)) << "<"
                  << payloadSize << std::endl;
        throw "up";
    }

    const uint8_t* payload =
        (const uint8_t*)&characters[sizeof(Package::payloadSize)];

    return Package(payload, payloadSize);
}