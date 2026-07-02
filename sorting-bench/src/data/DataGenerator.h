#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Dataset.h"

class DataGenerator {
public:
    static std::vector<int> randomData(
        size_t size, int minVal, int maxVal);

    static std::vector<int> sortedData(
        size_t size, int startVal = 0, int step = 1);

    static std::vector<int> reversedData(
        size_t size, int startVal = -1);  // -1 means "size - 1"

    static std::vector<int> nearlySortedData(
        size_t size, int startVal = 0, int step = 1,
        double swapRatio = 0.05);

    static std::vector<int> duplicateData(
        size_t size, int minVal, int maxVal,
        int distinctCount = 5);

    static std::vector<int> constantData(
        size_t size, int value = 42);

    static std::vector<int> generate(
        const std::string& dataType, size_t size,
        int minVal = 0, int maxVal = 10000);

    static std::vector<int> generate(
        DataType dataType, size_t size,
        int minVal = 0, int maxVal = 10000);

    static std::vector<std::vector<int>> generateDataset(
        const std::string& dataType, size_t inputCount, size_t inputSize,
        int minVal = 0, int maxVal = 10000);

    static Dataset generateDataset(const DatasetSpec& spec);

    static DatasetSuite generateSuite(const DatasetGridSpec& spec);

    static DatasetSuite generateSuite(const DatasetMixSpec& spec);
};
