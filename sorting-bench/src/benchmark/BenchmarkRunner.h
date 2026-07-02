#pragma once

#include <memory>
#include <vector>

#include "BenchmarkConfig.h"
#include "BenchmarkResult.h"

class Sorter;
class TraceInt;

class BenchmarkRunner {
private:
    std::vector<std::shared_ptr<Sorter>> sorters;

    std::vector<TraceInt> toTraceData(const std::vector<int>& rawData) const;

    bool isSorted(const std::vector<TraceInt>& data) const;

    BenchmarkResult runOnce(
        const std::vector<int>& rawData,
        const std::shared_ptr<Sorter>& sorter,
        const BenchmarkConfig& config
    ) const;

public:
    void addSorter(std::shared_ptr<Sorter> sorter);

    void clearSorters();

    std::vector<BenchmarkResult> run(
        const std::vector<int>& rawData,
        const BenchmarkConfig& config
    ) const;
};
