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
    ///   dataset_id, data_type, size, data,
    ///   best_algorithm, best_time_ms, best_key_ops, best_correct,
    ///   all_algorithms, all_times_ms, all_key_ops
    static bool writeTrainingCsv(
        const std::vector<int>& rawData,
        const BenchmarkConfig& config,
        const std::vector<BenchmarkResult>& rankedResults,
        const std::string& filePath);
};
