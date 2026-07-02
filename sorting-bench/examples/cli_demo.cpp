#include "../include/sorting_benchmark.hpp"

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

    DatasetGridSpec trainingSpec;
    trainingSpec.dataTypes = {
        DataType::Random,
        DataType::Sorted,
        DataType::Reversed,
        DataType::NearlySorted,
        DataType::Duplicates,
        DataType::Constant
    };
    trainingSpec.inputSizes = { 100, 500, 1000, 5000, 10000 };
    trainingSpec.inputCountPerDataset = 5;
    trainingSpec.minValue = 0;
    trainingSpec.maxValue = 100000;
    trainingSpec.idPrefix = "dataset";

    BenchmarkConfig trainingConfig;
    trainingConfig.repeatTimes = 1;

    auto trainingSuite = DataGenerator::generateSuite(trainingSpec);
    auto trainingResults = runner.runSuite(trainingSuite, trainingConfig);
    TrainingExport::writeTrainingSuiteCsv(
        trainingResults, "training_data.csv");

    std::cout << "  training_data.csv written ("
              << trainingResults.datasetCount() << " datasets, "
              << trainingResults.inputRowCount() << " input rows)\n";

    std::cout << "\nDone.\n";
    return 0;
}
