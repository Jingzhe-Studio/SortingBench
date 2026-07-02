#pragma once

#include <vector>

struct BenchmarkResult;

class ResultRanker {
public:
    /// Fill timeRank, operationRank, overallScore, overallRank on every
    /// result in-place.  Results whose sortedCorrectly == false are
    /// automatically pushed to the bottom.
    static void rank(std::vector<BenchmarkResult>& results);
};
