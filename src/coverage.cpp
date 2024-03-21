#if 0
#include "coverage.h"
#include <fstream>

EdgeTracker::EdgeTracker(int n) { this->edgeNames.resize(n); }

std::vector<fuzzer::EdgeCoverage>
EdgeTracker::symbolize(const std::vector<fuzzer::EdgeCoverage>& edges,
                       int targetIndex)
{
    std::vector<fuzzer::EdgeCoverage> mapping;
    for (auto& e : edges) {
        auto it = this->edgeNames[targetIndex].insert(
            std::make_pair(e.PC, this->edgeNames[targetIndex].size()));
        mapping.push_back(
            {static_cast<std::uintptr_t>(it.first->second), e.ptr, e.hits});
    }
    return mapping;
}

void EdgeTracker::dumpEdges(std::string prefix)
{
    for (int targetIndex = 0; targetIndex < this->edgeNames.size();
         targetIndex++) {
        for (auto& br : this->batchResults) {
            auto& data = br.inputData;
            std::string inputString(data.begin(), data.end());
            std::string filename = prefix + std::to_string(targetIndex) + "-" +
                                   fuzzer::Hash(data) + ".txt";
            std::ofstream f(filename, std::ios::out);

            f << inputString << "\n";

            auto normalized_edges =
                this->symbolize(br.edges[targetIndex], targetIndex);
            auto& edges = br.edges[targetIndex];

            for (int i = 0; i < edges.size(); i++) {
                f << "PC: 0x" << std::hex << edges[i].PC << std::dec << " ("
                  << normalized_edges[i].PC
                  << ") | Value: " << std::to_string(edges[i].hits) << " | "
                  << fuzzer::DescribePC("%F %L", edges[i].PC) << "\n";
            }
        }
    }
}

#endif