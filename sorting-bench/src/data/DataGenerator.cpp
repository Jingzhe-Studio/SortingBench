#include "DataGenerator.h"

#include <algorithm>
#include <array>
#include <functional>
#include <random>

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
