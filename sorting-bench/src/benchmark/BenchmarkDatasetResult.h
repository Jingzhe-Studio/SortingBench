#pragma once

#include <vector>

#include "../data/Dataset.h"
#include "BenchmarkResult.h"

struct BenchmarkDatasetResult {
    Dataset dataset;
    std::vector<std::vector<BenchmarkResult>> rankedResultsByInput;

    size_t inputRowCount() const {
        return dataset.inputCount();
    }

    std::vector<BenchmarkResult> flattenResults() const {
        std::vector<BenchmarkResult> flat;
        for (const auto& inputResults : rankedResultsByInput) {
            flat.insert(flat.end(), inputResults.begin(), inputResults.end());
        }
        return flat;
    }
};

struct BenchmarkSuiteResult {
    std::vector<BenchmarkDatasetResult> datasetResults;

    size_t datasetCount() const {
        return datasetResults.size();
    }

    size_t inputRowCount() const {
        size_t total = 0;
        for (const auto& result : datasetResults) {
            total += result.inputRowCount();
        }
        return total;
    }

    std::vector<BenchmarkResult> flattenResults() const {
        std::vector<BenchmarkResult> flat;
        for (const auto& datasetResult : datasetResults) {
            auto datasetFlat = datasetResult.flattenResults();
            flat.insert(flat.end(), datasetFlat.begin(), datasetFlat.end());
        }
        return flat;
    }
};
