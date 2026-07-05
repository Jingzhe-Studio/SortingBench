#include "BubbleSort.h"

std::string BubbleSort::name() const {
    return "BubbleSort";
}

void BubbleSort::sort(TraceArray& data) {
    const size_t n = data.size();
    if (n < 2) {
        return;
    }

    for (size_t i = 0; i + 1 < n; ++i) {
        bool swapped = false;
        for (size_t j = 0; j + 1 < n - i; ++j) {
            if (data.greater(j, j + 1)) {
                data.swap(j, j + 1);
                swapped = true;
            }
        }
        if (!swapped) {
            break;
        }
    }
}
