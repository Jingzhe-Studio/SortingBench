#pragma once

#include "../benchmark/SortStats.h"

class TraceSession;

class TraceInt {
private:
    int value = 0;

    static SortStats* currentStats;

    static void recordCompare() {
        if (currentStats != nullptr) {
            currentStats->compareCount++;
        }
    }

    static void recordWrite() {
        if (currentStats != nullptr) {
            currentStats->writeCount++;
        }
    }

public:
    TraceInt() = default;

    explicit TraceInt(int v) : value(v) {}

    // Copy / move constructors do NOT count as writes.
    // Only assignment operators record writes, so that:
    //   std::swap(a, b) → 2 writes (one per move-assign)
    //   TraceInt tmp = x  → 0 writes (construction, not array mutation)
    TraceInt(const TraceInt& other) = default;

    TraceInt(TraceInt&& other) noexcept = default;

    TraceInt& operator=(const TraceInt& other) {
        if (this != &other) {
            value = other.value;
            recordWrite();
        }
        return *this;
    }

    TraceInt& operator=(TraceInt&& other) noexcept {
        if (this != &other) {
            value = other.value;
            recordWrite();
        }
        return *this;
    }

    int getValue() const {
        return value;
    }

    bool operator<(const TraceInt& other) const {
        recordCompare();
        return value < other.value;
    }

    bool operator>(const TraceInt& other) const {
        recordCompare();
        return value > other.value;
    }

    bool operator<=(const TraceInt& other) const {
        recordCompare();
        return value <= other.value;
    }

    bool operator>=(const TraceInt& other) const {
        recordCompare();
        return value >= other.value;
    }

    bool operator==(const TraceInt& other) const {
        recordCompare();
        return value == other.value;
    }

    bool operator!=(const TraceInt& other) const {
        recordCompare();
        return value != other.value;
    }

    friend class TraceSession;
};
