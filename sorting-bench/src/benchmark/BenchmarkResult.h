#pragma once

#include <string>

struct BenchmarkResult {
    std::string datasetId;
    std::string inputId;
    std::string dataType;
    int dataSize = 0;

    std::string algorithmName;

    double elapsedMs = 0.0;

    long long compareCount = 0;
    long long moveCount = 0;
    long long swapCount = 0;
    long long keyOpCount = 0;

    bool sortedCorrectly = false;

    int timeRank = 0;
    int operationRank = 0;
    double overallScore = 0.0;
    int overallRank = 0;
};
