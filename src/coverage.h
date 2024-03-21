#if 0
#ifndef COVERAGE_H
#define COVERAGE_H

#include <map>
#include <vector>

#include "FuzzerDifferential.h"
#include "FuzzerSHA1.h"

class EdgeTracker
{
    /**
     * This class is used to track edge coverage information. After a batch has
     * been processes (i.e. one input has been processed by each target
     * program), the batchResult can be appended.
     *
     * Since each batchResult contains the entire edge coverage, this should
     * only be done, if only a few inputs are processed. Otherwise, the memory
     * consumption explodes quickly.
     */

  public:
    explicit EdgeTracker(int);
    std::vector<fuzzer::EdgeCoverage>
    symbolize(const std::vector<fuzzer::EdgeCoverage>& edges, int targetIndex);

    void dumpEdges(std::string prefix);

    std::vector<fuzzer::BatchResult> batchResults;
    std::vector<std::map<uintptr_t, int>> edgeNames;
};
#endif // COVERAGE_H
#endif