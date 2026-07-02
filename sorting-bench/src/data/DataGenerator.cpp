#include "DataGenerator.h"

#include <algorithm>
#include <array>
#include <functional>
#include <random>
#include <string>
#include <utility>
#include <vector>

/* -------------------------------------------------------------------------- */
/*  Random engine (file-static, anonymous namespace)                          */
/* -------------------------------------------------------------------------- */

namespace {

std::mt19937& engine() {
    static std::mt19937 eng = []() {
        std::random_device rd;
        std::array<int, std::mt19937::state_size> seedData;
        std::generate(seedData.begin(), seedData.end(), std::ref(rd));
        std::seed_seq seq(seedData.begin(), seedData.end());
        return std::mt19937(seq);
    }();
    return eng;
}

std::string prefixedDatasetId(
    const std::string& prefix,
    DataType type,
    size_t inputSize
) {
    std::string id = defaultDatasetId(type, inputSize);
    if (!prefix.empty()) {
        id = prefix + "_" + id;
    }
    return id;
}

}  // anonymous namespace

/* -------------------------------------------------------------------------- */
/*  randomData                                                                */
/* -------------------------------------------------------------------------- */

std::vector<int> DataGenerator::randomData(
    size_t size, int minVal, int maxVal
) {
    if (minVal > maxVal) {
        std::swap(minVal, maxVal);
    }

    std::vector<int> data(size);
    std::uniform_int_distribution<int> dist(minVal, maxVal);

    for (size_t i = 0; i < size; ++i) {
        data[i] = dist(engine());
    }

    return data;
}

/* -------------------------------------------------------------------------- */
/*  sortedData                                                                */
/* -------------------------------------------------------------------------- */

std::vector<int> DataGenerator::sortedData(
    size_t size, int startVal, int step
) {
    std::vector<int> data(size);

    for (size_t i = 0; i < size; ++i) {
        data[i] = startVal + static_cast<int>(i) * step;
    }

    return data;
}

/* -------------------------------------------------------------------------- */
/*  reversedData                                                              */
/* -------------------------------------------------------------------------- */

std::vector<int> DataGenerator::reversedData(
    size_t size, int startVal
) {
    if (startVal == -1) {
        startVal = static_cast<int>(size) - 1;
    }

    std::vector<int> data(size);

    for (size_t i = 0; i < size; ++i) {
        data[i] = startVal - static_cast<int>(i);
    }

    return data;
}

/* -------------------------------------------------------------------------- */
/*  nearlySortedData                                                          */
/* -------------------------------------------------------------------------- */

std::vector<int> DataGenerator::nearlySortedData(
    size_t size, int startVal, int step, double swapRatio
) {
    auto data = sortedData(size, startVal, step);

    if (size < 2) {
        return data;
    }

    // Clamp swapRatio to [0.0, 1.0]
    if (swapRatio < 0.0) swapRatio = 0.0;
    if (swapRatio > 1.0) swapRatio = 1.0;

    size_t swapCount = static_cast<size_t>(static_cast<double>(size) * swapRatio);
    if (swapCount == 0 && swapRatio > 0.0) {
        swapCount = 1;  // at least one swap when ratio > 0
    }

    std::uniform_int_distribution<size_t> indexDist(0, size - 1);

    for (size_t k = 0; k < swapCount; ++k) {
        size_t a = indexDist(engine());
        size_t b = indexDist(engine());
        std::swap(data[a], data[b]);
    }

    return data;
}

/* -------------------------------------------------------------------------- */
/*  duplicateData                                                             */
/* -------------------------------------------------------------------------- */

std::vector<int> DataGenerator::duplicateData(
    size_t size, int minVal, int maxVal, int distinctCount
) {
    if (minVal > maxVal) {
        std::swap(minVal, maxVal);
    }

    if (distinctCount < 1) {
        distinctCount = 1;
    }

    // Build a pool of distinct values.
    int rangeWidth = maxVal - minVal + 1;
    if (distinctCount > rangeWidth) {
        distinctCount = rangeWidth;
    }

    std::vector<int> pool(static_cast<size_t>(distinctCount));
    {
        // Draw `distinctCount` values without replacement from the range
        // using a simple rejection-sampling loop (adequate for small pools).
        std::uniform_int_distribution<int> valDist(minVal, maxVal);
        size_t filled = 0;
        while (filled < static_cast<size_t>(distinctCount)) {
            int candidate = valDist(engine());
            bool exists = false;
            for (size_t j = 0; j < filled; ++j) {
                if (pool[j] == candidate) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                pool[filled++] = candidate;
            }
        }
    }

    // Fill the result by randomly indexing into the pool.
    std::vector<int> data(size);
    std::uniform_int_distribution<size_t> poolDist(0, pool.size() - 1);

    for (size_t i = 0; i < size; ++i) {
        data[i] = pool[poolDist(engine())];
    }

    return data;
}

/* -------------------------------------------------------------------------- */
/*  constantData                                                              */
/* -------------------------------------------------------------------------- */

std::vector<int> DataGenerator::constantData(
    size_t size, int value
) {
    return std::vector<int>(size, value);
}

/* -------------------------------------------------------------------------- */
/*  generate  (dispatch helper)                                               */
/* -------------------------------------------------------------------------- */

std::vector<int> DataGenerator::generate(
    const std::string& dataType, size_t size,
    int minVal, int maxVal
) {
    if (dataType == "sorted") {
        return sortedData(size, minVal);
    }
    if (dataType == "reversed") {
        return reversedData(size, minVal);
    }
    if (dataType == "nearly_sorted") {
        return nearlySortedData(size, minVal);
    }
    if (dataType == "duplicates") {
        return duplicateData(size, minVal, maxVal);
    }
    if (dataType == "constant") {
        return constantData(size, minVal);
    }
    // default fallback
    return randomData(size, minVal, maxVal);
}

std::vector<int> DataGenerator::generate(
    DataType dataType, size_t size,
    int minVal, int maxVal
) {
    return generate(dataTypeName(dataType), size, minVal, maxVal);
}

/* -------------------------------------------------------------------------- */
/*  generateDataset                                                           */
/* -------------------------------------------------------------------------- */

std::vector<std::vector<int>> DataGenerator::generateDataset(
    const std::string& dataType, size_t inputCount, size_t inputSize,
    int minVal, int maxVal
) {
    std::vector<std::vector<int>> dataset;
    dataset.reserve(inputCount);

    for (size_t i = 0; i < inputCount; ++i) {
        dataset.push_back(generate(dataType, inputSize, minVal, maxVal));
    }

    return dataset;
}

Dataset DataGenerator::generateDataset(const DatasetSpec& spec)
{
    Dataset dataset;
    dataset.spec = spec;
    dataset.spec.id = spec.effectiveId();
    dataset.inputs.reserve(spec.inputCount);

    for (size_t i = 0; i < spec.inputCount; ++i) {
        DatasetInput input;
        input.id = "input_" + std::to_string(i);
        input.values = generate(
            spec.dataType, spec.inputSize, spec.minValue, spec.maxValue);
        dataset.inputs.push_back(std::move(input));
    }

    return dataset;
}

DatasetSuite DataGenerator::generateSuite(const DatasetGridSpec& spec)
{
    DatasetSuite suite;
    suite.datasets.reserve(spec.dataTypes.size() * spec.inputSizes.size());

    for (auto inputSize : spec.inputSizes) {
        for (auto dataType : spec.dataTypes) {
            DatasetSpec datasetSpec;
            datasetSpec.id = prefixedDatasetId(
                spec.idPrefix, dataType, inputSize);
            datasetSpec.dataType = dataType;
            datasetSpec.inputSize = inputSize;
            datasetSpec.inputCount = spec.inputCountPerDataset;
            datasetSpec.minValue = spec.minValue;
            datasetSpec.maxValue = spec.maxValue;

            suite.datasets.push_back(generateDataset(datasetSpec));
        }
    }

    return suite;
}

DatasetSuite DataGenerator::generateSuite(const DatasetMixSpec& spec)
{
    DatasetSuite suite;
    if (spec.parts.empty() || spec.totalInputCount == 0) {
        return suite;
    }

    struct Allocation {
        DataType dataType = DataType::Random;
        size_t count = 0;
        double remainder = 0.0;
    };

    double ratioTotal = 0.0;
    for (const auto& part : spec.parts) {
        if (part.ratio > 0.0) {
            ratioTotal += part.ratio;
        }
    }
    if (ratioTotal <= 0.0) {
        return suite;
    }

    std::vector<Allocation> allocations;
    allocations.reserve(spec.parts.size());

    size_t assigned = 0;
    for (const auto& part : spec.parts) {
        if (part.ratio <= 0.0) {
            continue;
        }

        double exact = static_cast<double>(spec.totalInputCount)
                     * part.ratio / ratioTotal;
        auto count = static_cast<size_t>(exact);
        allocations.push_back({part.dataType, count, exact - count});
        assigned += count;
    }

    std::sort(allocations.begin(), allocations.end(),
              [](const Allocation& a, const Allocation& b) {
                  return a.remainder > b.remainder;
              });

    for (size_t i = assigned; i < spec.totalInputCount; ++i) {
        allocations[(i - assigned) % allocations.size()].count++;
    }

    for (const auto& allocation : allocations) {
        if (allocation.count == 0) {
            continue;
        }

        DatasetSpec datasetSpec;
        datasetSpec.id = prefixedDatasetId(
            spec.idPrefix, allocation.dataType, spec.inputSize);
        datasetSpec.dataType = allocation.dataType;
        datasetSpec.inputSize = spec.inputSize;
        datasetSpec.inputCount = allocation.count;
        datasetSpec.minValue = spec.minValue;
        datasetSpec.maxValue = spec.maxValue;

        suite.datasets.push_back(generateDataset(datasetSpec));
    }

    return suite;
}
