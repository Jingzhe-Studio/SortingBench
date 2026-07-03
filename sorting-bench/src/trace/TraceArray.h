#pragma once

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <utility>
#include <vector>

#include "../benchmark/SortStats.h"
#include "../contract/SortCancelledException.h"

class TraceArray {
private:
    std::vector<int> data;
    SortStats* stats = nullptr;
    std::atomic<bool>* cancelled = nullptr;

    void recordCompare() {
        if (stats != nullptr) {
            stats->compareCount++;
        }
    }

    void recordMove(long long count = 1) {
        if (stats != nullptr) {
            stats->moveCount += count;
        }
    }

    void recordSwap() {
        if (stats != nullptr) {
            stats->swapCount++;
        }
    }

    void checkCancelled() const {
        if (cancelled != nullptr && cancelled->load(std::memory_order_relaxed)) {
            throw SortCancelledException{};
        }
    }

public:
    TraceArray() = default;

    TraceArray(std::vector<int> rawData, SortStats& sortStats)
        : data(std::move(rawData)), stats(&sortStats) {}

    TraceArray(std::vector<int> rawData, SortStats& sortStats,
               std::atomic<bool>* cancel)
        : data(std::move(rawData)), stats(&sortStats), cancelled(cancel) {}

    size_t size() const {
        return data.size();
    }

    bool empty() const {
        return data.empty();
    }

    int get(size_t index) const {
        checkCancelled();
        return data[index];
    }

    void set(size_t index, int value) {
        checkCancelled();
        data[index] = value;
        recordMove();
    }

    bool less(size_t left, size_t right) {
        checkCancelled();
        recordCompare();
        return data[left] < data[right];
    }

    bool lessValue(size_t index, int value) {
        checkCancelled();
        recordCompare();
        return data[index] < value;
    }

    bool greater(size_t left, size_t right) {
        checkCancelled();
        recordCompare();
        return data[left] > data[right];
    }

    bool greaterValue(size_t index, int value) {
        checkCancelled();
        recordCompare();
        return data[index] > value;
    }

    void swap(size_t left, size_t right) {
        checkCancelled();
        if (left == right) {
            return;
        }

        std::swap(data[left], data[right]);
        recordSwap();
        recordMove(2);
    }

    const std::vector<int>& values() const {
        return data;
    }
};
