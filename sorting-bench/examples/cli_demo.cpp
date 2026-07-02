#include "../src/data/DataGenerator.h"
#include "../src/data/DataReader.h"
#include "../src/data/DataWriter.h"
#include "../src/benchmark/BenchmarkRunner.h"
#include "../src/postprocess/ResultRanker.h"
#include "../src/postprocess/ReportWriter.h"
#include "../src/postprocess/TrainingExport.h"
#include "../src/algorithms/QuickSort.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

int main() {
    BenchmarkRunner runner;
    runner.addSorter(std::make_shared<QuickSort>());

    constexpr size_t N = 5000;

    // ---- single-algorithm smoke test (console only) ----
    std::cout << "=== QuickSort across data types (size=" << N
              << ", repeat=3) ===\n\n";

    std::vector<BenchmarkResult> allResults;

    const char* types[] = {
        "sorted", "reversed", "random", "nearly_sorted", "duplicates", "constant"
    };

    for (const char* t : types) {
        BenchmarkConfig config;
        config.datasetId   = std::string(t) + "_" + std::to_string(N);
        config.dataType    = t;
        config.repeatTimes = 3;

        auto rawData = DataGenerator::generate(t, N, 0, 100000);
        auto results = runner.run(rawData, config);
        ResultRanker::rank(results);

        ReportWriter::writeConsole(results);
        std::cout << "\n";

        allResults.insert(allResults.end(), results.begin(), results.end());
    }

    // ---- round-trip serialisation test ----
    std::cout << "\n=== DataWriter / DataReader round-trip ===\n";

    auto original = DataGenerator::randomData(100, 0, 999);

    DataWriter::writeBinary(original, "roundtrip_test.bin");
    auto fromBin = DataReader::readBinary("roundtrip_test.bin");
    assert(fromBin == original);
    std::cout << "  binary round-trip: " << fromBin.size()
              << " elements, OK\n";

    DataWriter::writeText(original, "roundtrip_test.txt");
    auto fromTxt = DataReader::readText("roundtrip_test.txt");
    assert(fromTxt == original);
    std::cout << "  text   round-trip: " << fromTxt.size()
              << " elements, OK\n";

    // ---- benchmark report CSV ----
    if (ReportWriter::writeCsv(allResults, "benchmark_report.csv")) {
        std::cout << "\n  benchmark_report.csv written\n";
    }

    // ---- training-data CSV ----
    std::cout << "\n=== Training Data Export ===\n";

    const char* trainTypes[] = {
        "random", "sorted", "reversed", "nearly_sorted",
        "duplicates", "constant"
    };
    constexpr size_t sizes[] = { 100, 500, 1000, 5000, 10000 };
    constexpr size_t inputsPerDataset = 5;

    size_t datasetCount = 0;
    size_t inputRowCount = 0;

    for (auto sz : sizes) {
        for (auto tt : trainTypes) {
            BenchmarkConfig config;
            config.datasetId   = "dataset_" + std::string(tt) + "_"
                                + std::to_string(sz);
            config.dataType    = tt;
            config.repeatTimes = 1;

            auto inputs = DataGenerator::generateDataset(
                tt, inputsPerDataset, sz, 0, 100000);

            std::vector<std::vector<BenchmarkResult>> rankedResultsByInput;
            rankedResultsByInput.reserve(inputs.size());

            for (size_t inputIndex = 0; inputIndex < inputs.size(); ++inputIndex) {
                BenchmarkConfig inputConfig = config;
                inputConfig.inputId = "input_" + std::to_string(inputIndex);

                auto results = runner.run(inputs[inputIndex], inputConfig);
                ResultRanker::rank(results);
                rankedResultsByInput.push_back(results);
            }

            TrainingExport::writeTrainingDatasetCsv(
                inputs, config, rankedResultsByInput, "training_data.csv");

            ++datasetCount;
            inputRowCount += inputs.size();
        }
    }

    std::cout << "  training_data.csv written ("
              << datasetCount << " datasets, "
              << inputRowCount << " input rows)\n";

    std::cout << "\nDone.\n";
    return 0;
}
