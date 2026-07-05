#include "SelectionSort.h"

std::string SelectionSort::name() const {
    return "SelectionSort";
}

void SelectionSort::sort(TraceArray& data) {
    const size_t n = data.size();
    if (n < 2) {
        return;
    }

    for (size_t i = 0; i + 1 < n; ++i) {
        size_t minIndex = i;
        for (size_t j = i + 1; j < n; ++j) {
            if (data.less(j, minIndex)) {
                minIndex = j;
            }
        }
        if (minIndex != i) {
            data.swap(i, minIndex);
        }
    }
}
