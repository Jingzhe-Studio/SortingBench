#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// Framework headers -- use the same include paths as the static library
#include "../algorithms/BubbleSort.h"
#include "../algorithms/InsertionSort.h"
#include "../algorithms/MergeSort.h"
#include "../algorithms/QuickSort.h"
#include "../algorithms/SelectionSort.h"
#include "../algorithms/ShellSort.h"
#include "../algorithms/SlowSort.h"
#include "../benchmark/BenchmarkConfig.h"
#include "../benchmark/BenchmarkDatasetResult.h"
#include "../benchmark/BenchmarkResult.h"
#include "../benchmark/BenchmarkRunner.h"
#include "../benchmark/SortStats.h"
#include "../contract/SortCancelledException.h"
#include "../contract/Sorter.h"
#include "../data/DataGenerator.h"
#include "../data/DataReader.h"
#include "../data/DataWriter.h"
#include "../data/Dataset.h"
#include "../postprocess/ReportWriter.h"
#include "../postprocess/ResultRanker.h"
#include "../postprocess/TrainingExport.h"
#include "../trace/TraceArray.h"

namespace py = pybind11;

// ============================================================
// Trampoline class -- allows Python subclasses of Sorter
// ============================================================
class PySorter : public Sorter {
public:
    using Sorter::Sorter;

    std::string name() const override {
        PYBIND11_OVERRIDE_PURE(std::string, Sorter, name);
    }

    void sort(TraceArray& data) override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function override = pybind11::get_override(this, "sort");
        if (override) {
            // Cast pointer (not reference) so pybind11 wraps the original
            // object rather than making a copy.
            override(pybind11::cast(&data));
        } else {
            PYBIND11_OVERRIDE_PURE(void, Sorter, sort, data);
        }
    }
};

// ============================================================
// Module definition
// ============================================================
PYBIND11_MODULE(sorting_bench, m) {
    m.doc() = "Sorting algorithm benchmarking framework - Python bindings";

    // --------------------------------------------------------
    // Exception
    // --------------------------------------------------------
    py::register_exception<SortCancelledException>(m, "SortCancelledError");

    // --------------------------------------------------------
    // Enum: DataType
    // --------------------------------------------------------
    py::enum_<DataType>(m, "DataType")
        .value("RANDOM", DataType::Random)
        .value("SORTED", DataType::Sorted)
        .value("REVERSED", DataType::Reversed)
        .value("NEARLY_SORTED", DataType::NearlySorted)
        .value("DUPLICATES", DataType::Duplicates)
        .value("CONSTANT", DataType::Constant)
        .export_values();

    // --------------------------------------------------------
    // Free utility functions
    // --------------------------------------------------------
    m.def("data_type_name", &dataTypeName,
          py::arg("type"), "Return the string name of a DataType");
    m.def("default_dataset_id", &defaultDatasetId,
          py::arg("type"), py::arg("input_size"),
          "Return a default dataset ID like 'random_1000'");

    // --------------------------------------------------------
    // Struct: SortStats
    // --------------------------------------------------------
    py::class_<SortStats>(m, "SortStats")
        .def(py::init<>())
        .def_readwrite("compare_count", &SortStats::compareCount)
        .def_readwrite("move_count", &SortStats::moveCount)
        .def_readwrite("swap_count", &SortStats::swapCount)
        .def("key_op_count", &SortStats::keyOpCount,
             "Total key operations = compareCount + moveCount");

    // --------------------------------------------------------
    // Struct: BenchmarkConfig
    // --------------------------------------------------------
    py::class_<BenchmarkConfig>(m, "BenchmarkConfig")
        .def(py::init<>())
        .def_readwrite("dataset_id", &BenchmarkConfig::datasetId)
        .def_readwrite("input_id", &BenchmarkConfig::inputId)
        .def_readwrite("data_type", &BenchmarkConfig::dataType)
        .def_readwrite("repeat_times", &BenchmarkConfig::repeatTimes)
        .def_readwrite("timeout_ms", &BenchmarkConfig::timeoutMs);

    // --------------------------------------------------------
    // Struct: BenchmarkResult (read-only from Python side)
    // --------------------------------------------------------
    py::class_<BenchmarkResult>(m, "BenchmarkResult")
        .def(py::init<>())
        .def_readwrite("dataset_id", &BenchmarkResult::datasetId)
        .def_readwrite("input_id", &BenchmarkResult::inputId)
        .def_readwrite("data_type", &BenchmarkResult::dataType)
        .def_readwrite("data_size", &BenchmarkResult::dataSize)
        .def_readwrite("algorithm_name", &BenchmarkResult::algorithmName)
        .def_readwrite("elapsed_ms", &BenchmarkResult::elapsedMs)
        .def_readwrite("median_elapsed_ms", &BenchmarkResult::medianElapsedMs)
        .def_readwrite("stddev_elapsed_ms", &BenchmarkResult::stddevElapsedMs)
        .def_readwrite("compare_count", &BenchmarkResult::compareCount)
        .def_readwrite("move_count", &BenchmarkResult::moveCount)
        .def_readwrite("swap_count", &BenchmarkResult::swapCount)
        .def_readwrite("key_op_count", &BenchmarkResult::keyOpCount)
        .def_readwrite("sorted_correctly", &BenchmarkResult::sortedCorrectly)
        .def_readwrite("timed_out", &BenchmarkResult::timedOut)
        .def_readwrite("error_message", &BenchmarkResult::errorMessage)
        .def_readwrite("time_rank", &BenchmarkResult::timeRank)
        .def_readwrite("operation_rank", &BenchmarkResult::operationRank)
        .def_readwrite("overall_score", &BenchmarkResult::overallScore)
        .def_readwrite("overall_rank", &BenchmarkResult::overallRank)
        .def("__repr__",
             [](const BenchmarkResult& r) {
                 return "<BenchmarkResult algo='" + r.algorithmName +
                        "' time=" + std::to_string(r.elapsedMs) + "ms>";
             });

    // --------------------------------------------------------
    // Struct: DatasetInput
    // --------------------------------------------------------
    py::class_<DatasetInput>(m, "DatasetInput")
        .def(py::init<>())
        .def_readwrite("id", &DatasetInput::id)
        .def_readwrite("values", &DatasetInput::values);

    // --------------------------------------------------------
    // Struct: DatasetSpec
    // --------------------------------------------------------
    py::class_<DatasetSpec>(m, "DatasetSpec")
        .def(py::init<>())
        .def_readwrite("id", &DatasetSpec::id)
        .def_readwrite("data_type", &DatasetSpec::dataType)
        .def_readwrite("input_size", &DatasetSpec::inputSize)
        .def_readwrite("input_count", &DatasetSpec::inputCount)
        .def_readwrite("min_value", &DatasetSpec::minValue)
        .def_readwrite("max_value", &DatasetSpec::maxValue)
        .def("effective_id", &DatasetSpec::effectiveId,
             "Return id if set, otherwise a default like 'random_1000'");

    // --------------------------------------------------------
    // Struct: Dataset
    // --------------------------------------------------------
    py::class_<Dataset>(m, "Dataset")
        .def(py::init<>())
        .def_readwrite("spec", &Dataset::spec)
        .def_readwrite("inputs", &Dataset::inputs)
        .def("id", &Dataset::id)
        .def("data_type_name", &Dataset::dataTypeName)
        .def("input_count", &Dataset::inputCount);

    // --------------------------------------------------------
    // Struct: DatasetGridSpec
    // --------------------------------------------------------
    py::class_<DatasetGridSpec>(m, "DatasetGridSpec")
        .def(py::init<>())
        .def_readwrite("data_types", &DatasetGridSpec::dataTypes)
        .def_readwrite("input_sizes", &DatasetGridSpec::inputSizes)
        .def_readwrite("input_count_per_dataset", &DatasetGridSpec::inputCountPerDataset)
        .def_readwrite("min_value", &DatasetGridSpec::minValue)
        .def_readwrite("max_value", &DatasetGridSpec::maxValue)
        .def_readwrite("id_prefix", &DatasetGridSpec::idPrefix);

    // --------------------------------------------------------
    // Struct: DatasetMixPart
    // --------------------------------------------------------
    py::class_<DatasetMixPart>(m, "DatasetMixPart")
        .def(py::init<>())
        .def_readwrite("data_type", &DatasetMixPart::dataType)
        .def_readwrite("ratio", &DatasetMixPart::ratio);

    // --------------------------------------------------------
    // Struct: DatasetMixSpec
    // --------------------------------------------------------
    py::class_<DatasetMixSpec>(m, "DatasetMixSpec")
        .def(py::init<>())
        .def_readwrite("input_size", &DatasetMixSpec::inputSize)
        .def_readwrite("total_input_count", &DatasetMixSpec::totalInputCount)
        .def_readwrite("parts", &DatasetMixSpec::parts)
        .def_readwrite("min_value", &DatasetMixSpec::minValue)
        .def_readwrite("max_value", &DatasetMixSpec::maxValue)
        .def_readwrite("id_prefix", &DatasetMixSpec::idPrefix);

    // --------------------------------------------------------
    // Struct: DatasetSuite
    // --------------------------------------------------------
    py::class_<DatasetSuite>(m, "DatasetSuite")
        .def(py::init<>())
        .def_readwrite("datasets", &DatasetSuite::datasets)
        .def("dataset_count", &DatasetSuite::datasetCount)
        .def("input_row_count", &DatasetSuite::inputRowCount);

    // --------------------------------------------------------
    // Struct: BenchmarkDatasetResult
    // --------------------------------------------------------
    py::class_<BenchmarkDatasetResult>(m, "BenchmarkDatasetResult")
        .def(py::init<>())
        .def_readwrite("dataset", &BenchmarkDatasetResult::dataset)
        .def_readwrite("ranked_results_by_input", &BenchmarkDatasetResult::rankedResultsByInput)
        .def("input_row_count", &BenchmarkDatasetResult::inputRowCount)
        .def("flatten_results", &BenchmarkDatasetResult::flattenResults,
             "Flatten all per-input results into a single list");

    // --------------------------------------------------------
    // Struct: BenchmarkSuiteResult
    // --------------------------------------------------------
    py::class_<BenchmarkSuiteResult>(m, "BenchmarkSuiteResult")
        .def(py::init<>())
        .def_readwrite("dataset_results", &BenchmarkSuiteResult::datasetResults)
        .def("dataset_count", &BenchmarkSuiteResult::datasetCount)
        .def("input_row_count", &BenchmarkSuiteResult::inputRowCount)
        .def("flatten_results", &BenchmarkSuiteResult::flattenResults,
             "Flatten all dataset results into a single list");

    // --------------------------------------------------------
    // Class: TraceArray (instrumented array for sorting)
    // --------------------------------------------------------
    py::class_<TraceArray>(m, "TraceArray")
        .def(py::init<>())
        .def("size", &TraceArray::size)
        .def("__len__", &TraceArray::size)
        .def("empty", &TraceArray::empty)
        .def("get", &TraceArray::get, py::arg("index"))
        .def("__getitem__", &TraceArray::get, py::arg("index"))
        .def("set", &TraceArray::set, py::arg("index"), py::arg("value"))
        .def("__setitem__",
             [](TraceArray& self, size_t index, int value) { self.set(index, value); },
             py::arg("index"), py::arg("value"))
        .def("less", &TraceArray::less, py::arg("left"), py::arg("right"))
        .def("less_value", &TraceArray::lessValue, py::arg("index"), py::arg("value"))
        .def("greater", &TraceArray::greater, py::arg("left"), py::arg("right"))
        .def("greater_value", &TraceArray::greaterValue, py::arg("index"), py::arg("value"))
        .def("swap", &TraceArray::swap, py::arg("left"), py::arg("right"))
        .def("values", &TraceArray::values,
             "Return a read-only view of the underlying data vector");

    // --------------------------------------------------------
    // Class: Sorter (abstract base, with trampoline)
    // --------------------------------------------------------
    py::class_<Sorter, PySorter, std::shared_ptr<Sorter>>(m, "Sorter")
        .def(py::init<>())
        .def("name", &Sorter::name)
        .def("sort", &Sorter::sort, py::arg("data"));

    // --------------------------------------------------------
    // Class: BenchmarkRunner
    // --------------------------------------------------------
    py::class_<BenchmarkRunner>(m, "BenchmarkRunner")
        .def(py::init<>())
        .def("add_sorter", &BenchmarkRunner::addSorter,
             py::arg("sorter"), py::keep_alive<1, 2>(),
             "Register a sorter (C++ or Python subclass)")
        .def("clear_sorters", &BenchmarkRunner::clearSorters,
             "Remove all registered sorters")
        .def("run", &BenchmarkRunner::run,
             py::arg("raw_data"), py::arg("config"),
             "Run all sorters on a single input vector, returns list of BenchmarkResult")
        .def("run_dataset", &BenchmarkRunner::runDataset,
             py::arg("dataset"),
             py::arg("config") = BenchmarkConfig(),
             "Run all sorters on a Dataset, returns BenchmarkDatasetResult")
        .def("run_suite", &BenchmarkRunner::runSuite,
             py::arg("suite"),
             py::arg("config") = BenchmarkConfig(),
             "Run all sorters on a DatasetSuite, returns BenchmarkSuiteResult");

    // --------------------------------------------------------
    // Built-in algorithm: QuickSort
    // --------------------------------------------------------
    py::class_<QuickSort, Sorter, std::shared_ptr<QuickSort>>(m, "QuickSort")
        .def(py::init<>());

    // --------------------------------------------------------
    // Built-in algorithm: BubbleSort
    // --------------------------------------------------------
    py::class_<BubbleSort, Sorter, std::shared_ptr<BubbleSort>>(m, "BubbleSort")
        .def(py::init<>());

    // --------------------------------------------------------
    // Built-in algorithm: SelectionSort
    // --------------------------------------------------------
    py::class_<SelectionSort, Sorter, std::shared_ptr<SelectionSort>>(m, "SelectionSort")
        .def(py::init<>());

    // --------------------------------------------------------
    // Built-in algorithm: InsertionSort
    // --------------------------------------------------------
    py::class_<InsertionSort, Sorter, std::shared_ptr<InsertionSort>>(m, "InsertionSort")
        .def(py::init<>());

    // --------------------------------------------------------
    // Built-in algorithm: ShellSort
    // --------------------------------------------------------
    py::class_<ShellSort, Sorter, std::shared_ptr<ShellSort>>(m, "ShellSort")
        .def(py::init<>());

    // --------------------------------------------------------
    // Built-in algorithm: MergeSort
    // --------------------------------------------------------
    py::class_<MergeSort, Sorter, std::shared_ptr<MergeSort>>(m, "MergeSort")
        .def(py::init<>());

    // --------------------------------------------------------
    // Built-in algorithm: SlowSort
    // --------------------------------------------------------
    py::class_<SlowSort, Sorter, std::shared_ptr<SlowSort>>(m, "SlowSort")
        .def(py::init<>());

    // --------------------------------------------------------
    // Static class: DataGenerator
    // --------------------------------------------------------
    py::class_<DataGenerator>(m, "DataGenerator")
        .def_static("random_data", &DataGenerator::randomData,
                    py::arg("size"), py::arg("min_val"), py::arg("max_val"))
        .def_static("sorted_data", &DataGenerator::sortedData,
                    py::arg("size"), py::arg("start_val") = 0,
                    py::arg("step") = 1)
        .def_static("reversed_data", &DataGenerator::reversedData,
                    py::arg("size"), py::arg("start_val") = -1)
        .def_static("nearly_sorted_data", &DataGenerator::nearlySortedData,
                    py::arg("size"), py::arg("start_val") = 0,
                    py::arg("step") = 1, py::arg("swap_ratio") = 0.05)
        .def_static("duplicate_data", &DataGenerator::duplicateData,
                    py::arg("size"), py::arg("min_val"), py::arg("max_val"),
                    py::arg("distinct_count") = 5)
        .def_static("constant_data", &DataGenerator::constantData,
                    py::arg("size"), py::arg("value") = 42)
        // generate(string, ...)
        .def_static("generate",
                    py::overload_cast<const std::string&, size_t, int, int>(
                        &DataGenerator::generate),
                    py::arg("data_type"), py::arg("size"),
                    py::arg("min_val") = 0, py::arg("max_val") = 10000)
        // generate(DataType, ...)
        .def_static("generate_by_type",
                    py::overload_cast<DataType, size_t, int, int>(
                        &DataGenerator::generate),
                    py::arg("data_type"), py::arg("size"),
                    py::arg("min_val") = 0, py::arg("max_val") = 10000)
        // generateDataset(string, ...)
        .def_static("generate_dataset",
                    py::overload_cast<
                        const std::string&, size_t, size_t, int, int>(
                        &DataGenerator::generateDataset),
                    py::arg("data_type"), py::arg("input_count"),
                    py::arg("input_size"),
                    py::arg("min_val") = 0, py::arg("max_val") = 10000)
        // generateDataset(DatasetSpec)
        .def_static("generate_dataset_from_spec",
                    py::overload_cast<const DatasetSpec&>(
                        &DataGenerator::generateDataset),
                    py::arg("spec"))
        // generateSuite(DatasetGridSpec)
        .def_static("generate_suite",
                    py::overload_cast<const DatasetGridSpec&>(
                        &DataGenerator::generateSuite),
                    py::arg("grid_spec"))
        // generateSuite(DatasetMixSpec)
        .def_static("generate_suite_from_mix",
                    py::overload_cast<const DatasetMixSpec&>(
                        &DataGenerator::generateSuite),
                    py::arg("mix_spec"));

    // --------------------------------------------------------
    // Static class: DataReader
    // --------------------------------------------------------
    py::class_<DataReader>(m, "DataReader")
        .def_static("read_binary", &DataReader::readBinary,
                    py::arg("file_path"))
        .def_static("read_text", &DataReader::readText,
                    py::arg("file_path"));

    // --------------------------------------------------------
    // Static class: DataWriter
    // --------------------------------------------------------
    py::class_<DataWriter>(m, "DataWriter")
        .def_static("write_binary", &DataWriter::writeBinary,
                    py::arg("data"), py::arg("file_path"))
        .def_static("write_text", &DataWriter::writeText,
                    py::arg("data"), py::arg("file_path"));

    // --------------------------------------------------------
    // Static class: ResultRanker
    // --------------------------------------------------------
    py::class_<ResultRanker>(m, "ResultRanker")
        .def_static("rank", &ResultRanker::rank, py::arg("results"),
                    "Fill timeRank, operationRank, overallScore, overallRank in-place");

    // --------------------------------------------------------
    // Static class: ReportWriter
    // --------------------------------------------------------
    py::class_<ReportWriter>(m, "ReportWriter")
        .def_static("write_csv",
                    py::overload_cast<
                        const std::vector<BenchmarkResult>&,
                        const std::string&>(&ReportWriter::writeCsv),
                    py::arg("results"), py::arg("file_path"))
        .def_static("write_suite_csv",
                    py::overload_cast<
                        const BenchmarkSuiteResult&,
                        const std::string&>(&ReportWriter::writeCsv),
                    py::arg("suite_result"), py::arg("file_path"))
        .def_static("write_console",
                    py::overload_cast<
                        const std::vector<BenchmarkResult>&>(
                        &ReportWriter::writeConsole),
                    py::arg("results"))
        .def_static("write_suite_console",
                    py::overload_cast<const BenchmarkSuiteResult&>(
                        &ReportWriter::writeConsole),
                    py::arg("suite_result"));

    // --------------------------------------------------------
    // Static class: TrainingExport
    // --------------------------------------------------------
    py::class_<TrainingExport>(m, "TrainingExport")
        .def_static("serialize_data", &TrainingExport::serializeData,
                    py::arg("raw_data"))
        .def_static("write_training_csv", &TrainingExport::writeTrainingCsv,
                    py::arg("raw_data"), py::arg("config"),
                    py::arg("ranked_results"), py::arg("file_path"))
        .def_static("write_training_dataset_csv",
                    py::overload_cast<
                        const std::vector<std::vector<int>>&,
                        const BenchmarkConfig&,
                        const std::vector<std::vector<BenchmarkResult>>&,
                        const std::string&>(
                        &TrainingExport::writeTrainingDatasetCsv),
                    py::arg("inputs"), py::arg("dataset_config"),
                    py::arg("ranked_results_by_input"), py::arg("file_path"))
        .def_static("write_training_dataset_csv_from_result",
                    py::overload_cast<
                        const BenchmarkDatasetResult&,
                        const std::string&>(
                        &TrainingExport::writeTrainingDatasetCsv),
                    py::arg("result"), py::arg("file_path"))
        .def_static("write_training_suite_csv",
                    &TrainingExport::writeTrainingSuiteCsv,
                    py::arg("result"), py::arg("file_path"));
}
