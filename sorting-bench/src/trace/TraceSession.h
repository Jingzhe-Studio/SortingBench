#pragma once

#include "TraceInt.h"
#include "../benchmark/SortStats.h"

class TraceSession {
private:
    SortStats* previousStats;

public:
    explicit TraceSession(SortStats& stats)
        : previousStats(TraceInt::currentStats) {
        TraceInt::currentStats = &stats;
    }

    ~TraceSession() {
        TraceInt::currentStats = previousStats;
    }

    TraceSession(const TraceSession&) = delete;
    TraceSession& operator=(const TraceSession&) = delete;
};
