#pragma once

#include <string>
#include <vector>

struct BenchmarkResult;

class ReportWriter {
public:
    /// Write results as a CSV file.  Returns true on success.
    static bool writeCsv(
        const std::vector<BenchmarkResult>& results,
        const std::string& filePath);

    /// Pretty-print results to stdout as a fixed-width table.
    static void writeConsole(
        const std::vector<BenchmarkResult>& results);
};
