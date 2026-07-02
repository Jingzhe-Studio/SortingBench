#include "BenchmarkRunner.h"

#include <chrono>

#include "../contract/Sorter.h"
#include "../trace/TraceInt.h"
#include "../trace/TraceSession.h"
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

std::vector<TraceInt> BenchmarkRunner::toTraceData(
    const std::vector<int>& rawData
) const {
    std::vector<TraceInt> result;
    result.reserve(rawData.size());

    for (int value : rawData) {
        result.emplace_back(value);
    }

    return result;
}

bool BenchmarkRunner::isSorted(
    const std::vector<TraceInt>& data
) const {
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i - 1].getValue() > data[i].getValue()) {
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
    long long totalWriteCount = 0;
    bool allSortedCorrectly = true;

    for (int r = 0; r < config.repeatTimes; ++r) {
        std::vector<TraceInt> dataCopy = toTraceData(rawData);
        SortStats stats;

        auto start = std::chrono::high_resolution_clock::now();

        {
            TraceSession session(stats);
            sorter->sort(dataCopy);
        }

        auto end = std::chrono::high_resolution_clock::now();

        double elapsedMs =
            std::chrono::duration<double, std::milli>(end - start).count();

        totalElapsedMs += elapsedMs;
        totalCompareCount += stats.compareCount;
        totalWriteCount += stats.writeCount;

        if (!isSorted(dataCopy)) {
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
    result.writeCount = totalWriteCount / n;
    result.keyOpCount = result.compareCount + result.writeCount;

    result.sortedCorrectly = allSortedCorrectly;

    return result;
}
