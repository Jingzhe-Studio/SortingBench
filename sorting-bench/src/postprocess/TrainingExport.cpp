#include "TrainingExport.h"

#include <fstream>
#include <iomanip>
#include <sstream>

#include "../benchmark/BenchmarkConfig.h"
#include "../benchmark/BenchmarkResult.h"

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
    // Find the winner (overallRank == 1)
    const BenchmarkResult* winner = nullptr;
    for (const auto& r : rankedResults) {
        if (r.overallRank == 1 && r.sortedCorrectly) {
            winner = &r;
            break;
        }
    }

    // Check whether the file already exists to decide on header.
    bool fileExists = static_cast<bool>(std::ifstream(filePath));

    std::ofstream out(filePath, std::ios::app);
    if (!out) return false;

    // Write header only for a new file.
    if (!fileExists) {
        out << "dataset_id,data_type,size,data,"
            << "best_algorithm,best_time_ms,best_key_ops,best_correct,"
            << "all_algorithms,all_times_ms,all_key_ops\n";
    }

    // ---- identity columns ----
    out << config.datasetId << ","
        << config.dataType << ","
        << rawData.size() << ",";

    // ---- data blob (features) ----
    out << serializeData(rawData) << ",";

    // ---- best algorithm (label) ----
    if (winner) {
        out << winner->algorithmName << ","
            << std::setprecision(6) << winner->elapsedMs << ","
            << winner->keyOpCount << ","
            << (winner->sortedCorrectly ? "true" : "false") << ",";
    } else {
        out << "NONE," << "0.0," << "0," << "false,";
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
    out << "\n";

    return true;
}
