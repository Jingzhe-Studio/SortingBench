#include "SlowSort.h"

#include <chrono>
#include <thread>

std::string SlowSort::name() const { return "SlowSort"; }

void SlowSort::sort(TraceArray& data) {
    const auto delay = std::chrono::milliseconds(10);
    const size_t n = data.size();

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j + 1 < n - i; ++j) {
            if (data.greater(j, j + 1)) {
                data.swap(j, j + 1);
            }
            std::this_thread::sleep_for(delay);
        }
    }
}
