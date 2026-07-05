#include "InsertionSort.h"

std::string InsertionSort::name() const {
    return "InsertionSort";
}

void InsertionSort::sort(TraceArray& data) {
    const size_t n = data.size();

    for (size_t i = 1; i < n; ++i) {
        const int key = data.get(i);
        size_t j = i;

        while (j > 0 && data.greaterValue(j - 1, key)) {
            data.set(j, data.get(j - 1));
            --j;
        }
        data.set(j, key);
    }
}
