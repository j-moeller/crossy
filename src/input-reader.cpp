#include "input-reader.h"

#include <sys/stat.h>

bool has_suffix(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

InputReader::Item::Item() : valid(false) {}

InputReader::Item::Item(std::string filename, fuzzer::Unit value)
    : filename(filename), value(value), valid(true)
{
}

InputReader::InputReader(std::string config) : position(0)
{
    std::ifstream list_file(config, std::ios::in);
    std::string filename;

    while (std::getline(list_file, filename)) {
        if (filename.size() == 0 || filename.rfind("#", 0) == 0) {
            continue;
        }

        if (has_suffix(filename, ".out")) {
            continue;
        }

        this->filenames.push_back(filename);
    }
}

InputReader::Item InputReader::next()
{
    if (this->position < this->filenames.size()) {
        std::string filename = this->filenames[this->position++];
        std::ifstream input(filename, std::ios::binary);
        std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(input), {});
        return InputReader::Item(filename, buffer);
    }

    return InputReader::Item();
}

int InputReader::getSize() const { return this->filenames.size(); }

int InputReader::getPosition() const { return this->position; }
