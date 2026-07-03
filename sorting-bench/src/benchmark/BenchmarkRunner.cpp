#include "BenchmarkRunner.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include "../contract/SortCancelledException.h"
#include "../contract/Sorter.h"
#include "../postprocess/ResultRanker.h"
#include "../trace/TraceArray.h"
#include "SortStats.h"

namespace {

double median(std::vector<double> samples) {
    if (samples.empty()) {
        return 0.0;
    }

    std::sort(samples.begin(), samples.end());

    const size_t mid = samples.size() / 2;
    if (samples.size() % 2 == 1) {
        return samples[mid];
    }

    return (samples[mid - 1] + samples[mid]) / 2.0;
}

double sampleStddev(const std::vector<double>& samples, double mean) {
    if (samples.size() < 2) {
        return 0.0;
    }

    double sumSquaredDiff = 0.0;
    for (double sample : samples) {
        const double diff = sample - mean;
        sumSquaredDiff += diff * diff;
    }

    return std::sqrt(sumSquaredDiff / static_cast<double>(samples.size() - 1));
}

}  // namespace

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

BenchmarkDatasetResult BenchmarkRunner::runDataset(
    const Dataset& dataset,
    const BenchmarkConfig& config
) const {
    BenchmarkDatasetResult datasetResult;
    datasetResult.dataset = dataset;
    datasetResult.rankedResultsByInput.reserve(dataset.inputs.size());

    for (const auto& input : dataset.inputs) {
        BenchmarkConfig inputConfig = config;
        inputConfig.datasetId = dataset.spec.id;
        inputConfig.inputId = input.id;
        inputConfig.dataType = dataset.dataTypeName();

        auto results = run(input.values, inputConfig);
        ResultRanker::rank(results);
        datasetResult.rankedResultsByInput.push_back(std::move(results));
    }

    return datasetResult;
}

BenchmarkSuiteResult BenchmarkRunner::runSuite(
    const DatasetSuite& suite,
    const BenchmarkConfig& config
) const {
    BenchmarkSuiteResult suiteResult;
    suiteResult.datasetResults.reserve(suite.datasets.size());

    for (const auto& dataset : suite.datasets) {
        suiteResult.datasetResults.push_back(runDataset(dataset, config));
    }

    return suiteResult;
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
    const int n = config.repeatTimes > 0 ? config.repeatTimes : 1;
    std::vector<double> elapsedSamples;
    elapsedSamples.reserve(static_cast<size_t>(n));

    bool anyCancelled = false;

    for (int r = 0; r < n; ++r) {
        SortStats stats;

        // Shared cancellation flag — the timer thread captures a copy of
        // this shared_ptr so the atomic lives on even after we leave scope.
        auto cancelled = std::make_shared<std::atomic<bool>>(false);
        TraceArray dataCopy(rawData, stats, cancelled.get());

        if (config.timeoutMs > 0) {
            std::thread([cancelled, ms = config.timeoutMs] {
                std::this_thread::sleep_for(std::chrono::milliseconds(ms));
                cancelled->store(true, std::memory_order_relaxed);
            }).detach();
        }

        auto start = std::chrono::high_resolution_clock::now();

        bool thisRepeatCancelled = false;
        try {
            sorter->sort(dataCopy);
        } catch (const SortCancelledException&) {
            thisRepeatCancelled = true;
        }

        auto end = std::chrono::high_resolution_clock::now();

        if (thisRepeatCancelled) {
            anyCancelled = true;
            allSortedCorrectly = false;
            continue;  // skip accumulation — data is incomplete
        }

        double elapsedMs =
            std::chrono::duration<double, std::milli>(end - start).count();

        totalElapsedMs += elapsedMs;
        elapsedSamples.push_back(elapsedMs);
        totalCompareCount += stats.compareCount;
        totalMoveCount += stats.moveCount;
        totalSwapCount += stats.swapCount;

        if (!isSorted(dataCopy.values())) {
            allSortedCorrectly = false;
        }
    }

    BenchmarkResult result;
    result.datasetId = config.datasetId;
    result.inputId = config.inputId;
    result.dataType = config.dataType;
    result.dataSize = static_cast<int>(rawData.size());
    result.algorithmName = sorter->name();

    result.elapsedMs = totalElapsedMs / n;
    result.medianElapsedMs = median(elapsedSamples);
    result.stddevElapsedMs = sampleStddev(elapsedSamples, result.elapsedMs);
    result.compareCount = totalCompareCount / n;
    result.moveCount = totalMoveCount / n;
    result.swapCount = totalSwapCount / n;
    result.keyOpCount = result.compareCount + result.moveCount;

    result.sortedCorrectly = allSortedCorrectly;

    result.timedOut = anyCancelled;
    if (anyCancelled) {
        result.errorMessage = "timeout after " + std::to_string(config.timeoutMs) + " ms";
    }

    return result;
}
