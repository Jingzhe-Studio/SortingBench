#include "QuickSort.h"

#include <algorithm>  // std::swap

std::string QuickSort::name() const {
    return "QuickSort";
}

void QuickSort::sort(std::vector<TraceInt>& data) {
    if (data.empty()) {
        return;
    }
    quickSort(data, 0, static_cast<int>(data.size()) - 1);
}

int QuickSort::partition(std::vector<TraceInt>& data, int low, int high) {
    TraceInt pivot = data[high];
    int i = low - 1;

    for (int j = low; j < high; ++j) {
        if (data[j] < pivot) {
            ++i;
            std::swap(data[i], data[j]);
        }
    }
    std::swap(data[i + 1], data[high]);
    return i + 1;
}

void QuickSort::quickSort(std::vector<TraceInt>& data, int low, int high) {
    if (low < high) {
        int pi = partition(data, low, high);
        quickSort(data, low, pi - 1);
        quickSort(data, pi + 1, high);
    }
}
