#pragma once

#include <string>
#include <vector>

struct BenchmarkConfig;
struct BenchmarkResult;

class TrainingExport {
public:
    /// Serialize data to a compact space-separated string.
    /// Example: "42 17 99 3 71 ..."
    static std::string serializeData(const std::vector<int>& rawData);

    /// Append one training row to a CSV file.
    /// Creates the file with header if it does not exist.
    /// Returns true on success.
    ///
    /// CSV columns:
    ///   dataset_id, input_id, data_type, size, data,
    ///   best_algorithm, best_time_ms, best_key_ops, best_moves, best_swaps,
    ///   best_correct, all_algorithms, all_times_ms, all_key_ops,
    ///   all_moves, all_swaps
    static bool writeTrainingCsv(
        const std::vector<int>& rawData,
        const BenchmarkConfig& config,
        const std::vector<BenchmarkResult>& rankedResults,
        const std::string& filePath);

    /// Append one dataset consisting of multiple inputs to a CSV file.
    /// Each input is written as a row with the same dataset_id and a distinct
    /// input_id, so downstream consumers can group rows back into a dataset.
    static bool writeTrainingDatasetCsv(
        const std::vector<std::vector<int>>& inputs,
        const BenchmarkConfig& datasetConfig,
        const std::vector<std::vector<BenchmarkResult>>& rankedResultsByInput,
        const std::string& filePath);
};
