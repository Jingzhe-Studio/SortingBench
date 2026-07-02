#include "BenchmarkRunner.h"

#include <chrono>

#include "../contract/Sorter.h"
#include "../trace/TraceArray.h"
#include "SortStats.h"

/* -------------------------------------------------------------------------- */
/*  Public API                                                                */
/* -------------------------------------------------------------------------- */

void BenchmarkRunner::addSorter(std::shared_ptr<Sorter> sorter) {
    if (sorter) {
        sorters.push_back(std::move(sorter));
    }
}

void BenchmarkRunner::clearSorters() {
    sorters.clear();
}

std::vector<BenchmarkResult> BenchmarkRunner::run(
    const std::vector<int>& rawData,
    const BenchmarkConfig& config
) const {
    std::vector<BenchmarkResult> results;

    for (const auto& sorter : sorters) {
        BenchmarkResult result = runOnce(rawData, sorter, config);
        results.push_back(result);
    }

    return results;
}

/* -------------------------------------------------------------------------- */
/*  Private helpers                                                           */
/* -------------------------------------------------------------------------- */

bool BenchmarkRunner::isSorted(
    const std::vector<int>& data
) const {
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i - 1] > data[i]) {
            return false;
        }
    }
    return true;
}

BenchmarkResult BenchmarkRunner::runOnce(
    const std::vector<int>& rawData,
    const std::shared_ptr<Sorter>& sorter,
    const BenchmarkConfig& config
) const {
    // ---- accumulate metrics across repeats ----
    double totalElapsedMs = 0.0;
    long long totalCompareCount = 0;
    long long totalMoveCount = 0;
    long long totalSwapCount = 0;
    bool allSortedCorrectly = true;

    for (int r = 0; r < config.repeatTimes; ++r) {
        SortStats stats;
        TraceArray dataCopy(rawData, stats);

        auto start = std::chrono::high_resolution_clock::now();

        sorter->sort(dataCopy);

        auto end = std::chrono::high_resolution_clock::now();

        double elapsedMs =
            std::chrono::duration<double, std::milli>(end - start).count();

        totalElapsedMs += elapsedMs;
        totalCompareCount += stats.compareCount;
        totalMoveCount += stats.moveCount;
        totalSwapCount += stats.swapCount;

        if (!isSorted(dataCopy.values())) {
            allSortedCorrectly = false;
        }
    }

    int n = config.repeatTimes;

    BenchmarkResult result;
    result.datasetId = config.datasetId;
    result.inputId = config.inputId;
    result.dataType = config.dataType;
    result.dataSize = static_cast<int>(rawData.size());
    result.algorithmName = sorter->name();

    result.elapsedMs = totalElapsedMs / n;
    result.compareCount = totalCompareCount / n;
    result.moveCount = totalMoveCount / n;
    result.swapCount = totalSwapCount / n;
    result.keyOpCount = result.compareCount + result.moveCount;

    result.sortedCorrectly = allSortedCorrectly;

    return result;
}
