#pragma once

#include <cstddef>
#include <string>
#include <vector>

enum class DataType {
    Random,
    Sorted,
    Reversed,
    NearlySorted,
    Duplicates,
    Constant
};

inline const char* dataTypeName(DataType type) {
    switch (type) {
        case DataType::Sorted:
            return "sorted";
        case DataType::Reversed:
            return "reversed";
        case DataType::NearlySorted:
            return "nearly_sorted";
        case DataType::Duplicates:
            return "duplicates";
        case DataType::Constant:
            return "constant";
        case DataType::Random:
        default:
            return "random";
    }
}

inline std::string defaultDatasetId(DataType type, size_t inputSize) {
    return std::string(dataTypeName(type)) + "_" + std::to_string(inputSize);
}

struct DatasetInput {
    std::string id;
    std::vector<int> values;
};

struct DatasetSpec {
    std::string id;
    DataType dataType = DataType::Random;
    size_t inputSize = 0;
    size_t inputCount = 1;
    int minValue = 0;
    int maxValue = 10000;

    std::string effectiveId() const {
        return id.empty() ? defaultDatasetId(dataType, inputSize) : id;
    }
};

struct Dataset {
    DatasetSpec spec;
    std::vector<DatasetInput> inputs;

    const std::string& id() const {
        return spec.id;
    }

    std::string dataTypeName() const {
        return ::dataTypeName(spec.dataType);
    }

    size_t inputCount() const {
        return inputs.size();
    }
};

struct DatasetGridSpec {
    std::vector<DataType> dataTypes;
    std::vector<size_t> inputSizes;
    size_t inputCountPerDataset = 1;
    int minValue = 0;
    int maxValue = 10000;
    std::string idPrefix;
};

struct DatasetMixPart {
    DataType dataType = DataType::Random;
    double ratio = 0.0;
};

struct DatasetMixSpec {
    size_t inputSize = 0;
    size_t totalInputCount = 0;
    std::vector<DatasetMixPart> parts;
    int minValue = 0;
    int maxValue = 10000;
    std::string idPrefix;
};

struct DatasetSuite {
    std::vector<Dataset> datasets;

    size_t datasetCount() const {
        return datasets.size();
    }

    size_t inputRowCount() const {
        size_t total = 0;
        for (const auto& dataset : datasets) {
            total += dataset.inputCount();
        }
        return total;
    }
};
