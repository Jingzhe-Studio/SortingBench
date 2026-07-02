#include "TrainingExport.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#include "../benchmark/BenchmarkConfig.h"
#include "../benchmark/BenchmarkResult.h"

namespace {

bool writeHeaderIfNeeded(std::ofstream& out, bool fileExists)
{
    if (!fileExists) {
        out << "dataset_id,input_id,data_type,size,data,"
            << "best_algorithm,best_time_ms,best_key_ops,best_moves,"
            << "best_swaps,best_correct,"
            << "all_algorithms,all_times_ms,all_key_ops,all_moves,"
            << "all_swaps\n";
    }

    return static_cast<bool>(out);
}

bool writeTrainingRow(
    std::ofstream& out,
    const std::vector<int>& rawData,
    const BenchmarkConfig& config,
    const std::vector<BenchmarkResult>& rankedResults)
{
    // Find the winner (overallRank == 1)
    const BenchmarkResult* winner = nullptr;
    for (const auto& r : rankedResults) {
        if (r.overallRank == 1 && r.sortedCorrectly) {
            winner = &r;
            break;
        }
    }

    // ---- identity columns ----
    out << config.datasetId << ","
        << config.inputId << ","
        << config.dataType << ","
        << rawData.size() << ",";

    // ---- data blob (features) ----
    out << TrainingExport::serializeData(rawData) << ",";

    // ---- best algorithm (label) ----
    if (winner) {
        out << winner->algorithmName << ","
            << std::setprecision(6) << winner->elapsedMs << ","
            << winner->keyOpCount << ","
            << winner->moveCount << ","
            << winner->swapCount << ","
            << (winner->sortedCorrectly ? "true" : "false") << ",";
    } else {
        out << "NONE," << "0.0," << "0," << "0," << "0," << "false,";
    }

    // ---- all algorithms (semicolon-delimited) ----
    for (size_t i = 0; i < rankedResults.size(); ++i) {
        if (i > 0) out << ";";
        out << rankedResults[i].algorithmName;
    }
    out << ",";
    for (size_t i = 0; i < rankedResults.size(); ++i) {
        if (i > 0) out << ";";
        out << std::setprecision(6) << rankedResults[i].elapsedMs;
    }
    out << ",";
    for (size_t i = 0; i < rankedResults.size(); ++i) {
        if (i > 0) out << ";";
        out << rankedResults[i].keyOpCount;
    }
    out << ",";
    for (size_t i = 0; i < rankedResults.size(); ++i) {
        if (i > 0) out << ";";
        out << rankedResults[i].moveCount;
    }
    out << ",";
    for (size_t i = 0; i < rankedResults.size(); ++i) {
        if (i > 0) out << ";";
        out << rankedResults[i].swapCount;
    }
    out << "\n";

    return static_cast<bool>(out);
}

}  // namespace

/* ========================================================================= */
/*  serializeData                                                            */
/* ========================================================================= */

std::string TrainingExport::serializeData(const std::vector<int>& rawData)
{
    if (rawData.empty()) return "";

    std::ostringstream oss;

    for (size_t i = 0; i < rawData.size(); ++i) {
        if (i > 0) oss << ' ';
        oss << rawData[i];
    }

    return oss.str();
}

/* ========================================================================= */
/*  Training CSV export                                                      */
/* ========================================================================= */

bool TrainingExport::writeTrainingCsv(
    const std::vector<int>& rawData,
    const BenchmarkConfig& config,
    const std::vector<BenchmarkResult>& rankedResults,
    const std::string& filePath)
{
    // Check whether the file already exists to decide on header.
    bool fileExists = static_cast<bool>(std::ifstream(filePath));

    std::ofstream out(filePath, std::ios::app);
    if (!out) return false;

    writeHeaderIfNeeded(out, fileExists);

    return writeTrainingRow(out, rawData, config, rankedResults);
}

bool TrainingExport::writeTrainingDatasetCsv(
    const std::vector<std::vector<int>>& inputs,
    const BenchmarkConfig& datasetConfig,
    const std::vector<std::vector<BenchmarkResult>>& rankedResultsByInput,
    const std::string& filePath)
{
    if (inputs.size() != rankedResultsByInput.size()) {
        return false;
    }

    bool fileExists = static_cast<bool>(std::ifstream(filePath));

    std::ofstream out(filePath, std::ios::app);
    if (!out) return false;

    writeHeaderIfNeeded(out, fileExists);

    for (size_t i = 0; i < inputs.size(); ++i) {
        BenchmarkConfig inputConfig = datasetConfig;
        inputConfig.inputId = "input_" + std::to_string(i);

        if (!writeTrainingRow(
                out, inputs[i], inputConfig, rankedResultsByInput[i])) {
            return false;
        }
    }

    return true;
}
