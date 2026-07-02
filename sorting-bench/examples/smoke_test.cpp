#include "../src/data/DataGenerator.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <set>

int main() {
    constexpr size_t N = 1000;

    // 1. randomData
    {
        auto v = DataGenerator::randomData(N, 10, 200);
        assert(v.size() == N);
        for (int x : v) assert(x >= 10 && x <= 200);
        std::cout << "randomData: PASS\n";
    }

    // 2. sortedData
    {
        auto v = DataGenerator::sortedData(N, 5, 3);
        assert(v.size() == N);
        assert(v[0] == 5);
        assert(std::is_sorted(v.begin(), v.end()));
        std::cout << "sortedData: PASS\n";
    }

    // 3. reversedData
    {
        auto v = DataGenerator::reversedData(N);
        assert(v.size() == N);
        assert(v[0] == static_cast<int>(N - 1));
        for (size_t i = 1; i < N; ++i) assert(v[i] <= v[i - 1]);
        std::cout << "reversedData: PASS\n";
    }

    // 4. nearlySortedData: preserves multiset
    {
        auto v = DataGenerator::nearlySortedData(N, 0, 1, 0.10);
        auto baseline = DataGenerator::sortedData(N, 0, 1);
        std::sort(baseline.begin(), baseline.end());
        std::sort(v.begin(), v.end());
        assert(v == baseline);
        std::cout << "nearlySortedData: PASS\n";
    }

    // 5. duplicateData
    {
        auto v = DataGenerator::duplicateData(N, 0, 9, 4);
        std::set<int> distinct(v.begin(), v.end());
        assert(distinct.size() <= 4);
        std::cout << "duplicateData: PASS\n";
    }

    // 6. constantData
    {
        auto v = DataGenerator::constantData(N, 7);
        assert(v.size() == N);
        for (int x : v) assert(x == 7);
        std::cout << "constantData: PASS\n";
    }

    // 7. generate dispatch
    {
        assert(DataGenerator::generate("sorted",       10, 0, 0)[0] == 0);
        assert(DataGenerator::generate("constant",     10, 99, 0)[0] == 99);
        assert(DataGenerator::generate("nearly_sorted",10, 0, 0).size() == 10);
        // unknown string → fallback to random
        auto v = DataGenerator::generate("nonexistent",10, 0, 100);
        assert(v.size() == 10);
        for (int x : v) assert(x >= 0 && x <= 100);
        std::cout << "generate dispatch: PASS\n";
    }

    // 8. edge: empty size
    {
        assert(DataGenerator::randomData(0, 0, 100).empty());
        assert(DataGenerator::sortedData(0).empty());
        assert(DataGenerator::reversedData(0).empty());
        assert(DataGenerator::nearlySortedData(0).empty());
        assert(DataGenerator::duplicateData(0, 0, 100).empty());
        assert(DataGenerator::constantData(0).empty());
        std::cout << "empty size: PASS\n";
    }

    // 9. dataset: a group of inputs
    {
        auto dataset = DataGenerator::generateDataset("random", 4, 25, 0, 9);
        assert(dataset.size() == 4);
        for (const auto& input : dataset) {
            assert(input.size() == 25);
            for (int x : input) assert(x >= 0 && x <= 9);
        }
        std::cout << "generateDataset: PASS\n";
    }

    // 10. typed dataset and suite specs
    {
        DatasetSpec spec;
        spec.id = "random_25";
        spec.dataType = DataType::Random;
        spec.inputSize = 25;
        spec.inputCount = 3;
        spec.minValue = 0;
        spec.maxValue = 9;

        auto dataset = DataGenerator::generateDataset(spec);
        assert(dataset.spec.id == "random_25");
        assert(dataset.inputs.size() == 3);
        assert(dataset.inputs[0].id == "input_0");
        assert(dataset.inputs[0].values.size() == 25);

        DatasetGridSpec grid;
        grid.dataTypes = { DataType::Random, DataType::Sorted };
        grid.inputSizes = { 10, 20 };
        grid.inputCountPerDataset = 2;
        auto suite = DataGenerator::generateSuite(grid);
        assert(suite.datasetCount() == 4);
        assert(suite.inputRowCount() == 8);

        DatasetMixSpec mix;
        mix.inputSize = 10;
        mix.totalInputCount = 10;
        mix.parts = {
            { DataType::Random, 0.7 },
            { DataType::Reversed, 0.3 }
        };
        auto mixedSuite = DataGenerator::generateSuite(mix);
        assert(mixedSuite.datasetCount() == 2);
        assert(mixedSuite.inputRowCount() == 10);

        std::cout << "dataset suite specs: PASS\n";
    }

    std::cout << "\nAll smoke tests passed.\n";
    return 0;
}
