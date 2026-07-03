#pragma once

#include <string>

struct BenchmarkConfig {
    std::string datasetId = "default_dataset";
    std::string inputId = "input_0";
    std::string dataType = "unknown";

    int repeatTimes = 1;

    /// Per-run wall-clock timeout in milliseconds.  0 = disabled.
    /// When exceeded the sorter is cancelled cooperatively and the
    /// result is marked timedOut.
    int timeoutMs = 0;
};
