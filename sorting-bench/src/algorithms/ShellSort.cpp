#include "ShellSort.h"

std::string ShellSort::name() const {
    return "ShellSort";
}

void ShellSort::sort(TraceArray& data) {
    const size_t n = data.size();

    for (size_t gap = n / 2; gap > 0; gap /= 2) {
        for (size_t i = gap; i < n; ++i) {
            const int value = data.get(i);
            size_t j = i;

            while (j >= gap && data.greaterValue(j - gap, value)) {
                data.set(j, data.get(j - gap));
                j -= gap;
            }
            data.set(j, value);
        }
    }
}
