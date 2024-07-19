#ifndef INPUT_READER_H
#define INPUT_READER_H

#include <fstream>
#include <string>

#include "FuzzerInternal.h"

class InputReader
{
    struct Item {
        Item();
        Item(std::string filename, fuzzer::Unit value);

        std::string filename;
        fuzzer::Unit value;
        bool valid;
    };

  public:
    InputReader(std::string config);
    InputReader::Item next();

    int getPosition() const;
    int getSize() const;

  private:
    int position;
    std::vector<std::string> filenames;
};

#endif