#include "ReportWriter.h"

#include <fstream>
#include <iomanip>
#include <iostream>

#include "../benchmark/BenchmarkResult.h"

/* ========================================================================= */
/*  writeCsv                                                                 */
/* ========================================================================= */

bool ReportWriter::writeCsv(
    const std::vector<BenchmarkResult>& results,
    const std::string& filePath)
{
    std::ofstream out(filePath);
    if (!out) {
        return false;
    }

    // ---- header ----
    out << "dataset_id,data_type,data_size,"
        << "algorithm,"
        << "elapsed_ms,compare_count,write_count,key_op_count,"
        << "sorted_correctly,"
        << "time_rank,operation_rank,overall_score,overall_rank\n";

    // ---- rows ----
    for (const auto& r : results) {
        out << r.datasetId << ","
            << r.dataType << ","
            << r.dataSize << ","
            << r.algorithmName << ","
            << std::fixed << std::setprecision(6) << r.elapsedMs << ","
            << r.compareCount << ","
            << r.writeCount << ","
            << r.keyOpCount << ","
            << (r.sortedCorrectly ? "true" : "false") << ","
            << r.timeRank << ","
            << r.operationRank << ","
            << std::setprecision(2) << r.overallScore << ","
            << r.overallRank << "\n";
    }

    return true;
}

/* ========================================================================= */
/*  writeConsole                                                             */
/* ========================================================================= */

void ReportWriter::writeConsole(
    const std::vector<BenchmarkResult>& results)
{
    if (results.empty()) {
        std::cout << "(no results)\n";
        return;
    }

    // Column widths
    constexpr int W_ALGO   = 16;
    constexpr int W_SIZE   =  8;
    constexpr int W_TYPE   = 14;
    constexpr int W_TIME   = 10;
    constexpr int W_CMP    = 12;
    constexpr int W_WRITE  = 10;
    constexpr int W_KEY    = 12;
    constexpr int W_OK     =  8;
    constexpr int W_TRANK  =  8;
    constexpr int W_ORANK  =  8;
    constexpr int W_SCORE  =  8;
    constexpr int W_OVR    =  8;

    auto printHeader = [&] {
        std::cout << std::left
                  << std::setw(W_ALGO) << "algorithm"
                  << std::setw(W_SIZE) << "size"
                  << std::setw(W_TYPE) << "type"
                  << std::setw(W_TIME) << "time_ms"
                  << std::setw(W_CMP)  << "compares"
                  << std::setw(W_WRITE)<< "writes"
                  << std::setw(W_KEY)  << "keyOps"
                  << std::setw(W_OK)   << "correct"
                  << std::setw(W_TRANK)<< "tRank"
                  << std::setw(W_ORANK)<< "opRank"
                  << std::setw(W_SCORE)<< "score"
                  << std::setw(W_OVR)  << "overall"
                  << "\n";
    };

    auto printSep = [&] {
        std::cout << std::string(W_ALGO+W_SIZE+W_TYPE+W_TIME+W_CMP+W_WRITE
                                +W_KEY+W_OK+W_TRANK+W_ORANK+W_SCORE+W_OVR, '-')
                  << "\n";
    };

    printHeader();
    printSep();

    for (const auto& r : results) {
        std::cout << std::left
                  << std::setw(W_ALGO) << r.algorithmName
                  << std::setw(W_SIZE) << r.dataSize
                  << std::setw(W_TYPE) << r.dataType
                  << std::fixed << std::setprecision(3)
                  << std::setw(W_TIME) << r.elapsedMs
                  << std::setw(W_CMP)  << r.compareCount
                  << std::setw(W_WRITE)<< r.writeCount
                  << std::setw(W_KEY)  << r.keyOpCount
                  << std::setw(W_OK)   << (r.sortedCorrectly ? "YES" : "NO")
                  << std::setw(W_TRANK)<< r.timeRank
                  << std::setw(W_ORANK)<< r.operationRank
                  << std::setprecision(2)
                  << std::setw(W_SCORE)<< r.overallScore
                  << std::setw(W_OVR)  << r.overallRank
                  << "\n";
    }

    if (results.size() > 1) {
        // Find the overall winner.
        auto best = &results[0];
        for (const auto& r : results) {
            if (r.overallRank < best->overallRank) best = &r;
        }
        std::cout << "\n"
                  << (best->sortedCorrectly
                        ? "* Best: " + best->algorithmName
                        : "* All algorithms failed correctness check")
                  << "\n";
    }
}
