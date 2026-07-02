#pragma once

#include <string>

struct BenchmarkConfig {
    std::string datasetId = "default_dataset";
    std::string dataType = "unknown";

    int repeatTimes = 1;
};
