#include "QuickSort.h"

std::string QuickSort::name() const {
    return "QuickSort";
}

void QuickSort::sort(TraceArray& data) {
    if (data.empty()) {
        return;
    }
    quickSort(data, 0, static_cast<int>(data.size()) - 1);
}

int QuickSort::partition(TraceArray& data, int low, int high) {
    int pivot = data.get(static_cast<size_t>(high));
    int i = low - 1;

    for (int j = low; j < high; ++j) {
        if (data.lessValue(static_cast<size_t>(j), pivot)) {
            ++i;
            data.swap(static_cast<size_t>(i), static_cast<size_t>(j));
        }
    }
    data.swap(static_cast<size_t>(i + 1), static_cast<size_t>(high));
    return i + 1;
}

void QuickSort::quickSort(TraceArray& data, int low, int high) {
    while (low < high) {
        int pi = partition(data, low, high);

        if (pi - low < high - pi) {
            quickSort(data, low, pi - 1);
            low = pi + 1;
        } else {
            quickSort(data, pi + 1, high);
            high = pi - 1;
        }
    }
}
