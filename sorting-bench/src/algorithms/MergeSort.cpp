#include "MergeSort.h"

std::string MergeSort::name() const {
    return "MergeSort";
}

void MergeSort::sort(TraceArray& data) {
    const size_t n = data.size();
    if (n < 2) {
        return;
    }

    std::vector<int> buffer(n);
    mergeSort(data, buffer, 0, n);
}

void MergeSort::mergeSort(TraceArray& data, std::vector<int>& buffer, size_t left, size_t right) {
    if (right - left < 2) {
        return;
    }

    const size_t mid = left + (right - left) / 2;
    mergeSort(data, buffer, left, mid);
    mergeSort(data, buffer, mid, right);
    merge(data, buffer, left, mid, right);
}

void MergeSort::merge(TraceArray& data, std::vector<int>& buffer, size_t left, size_t mid, size_t right) {
    size_t i = left;
    size_t j = mid;
    size_t k = left;

    while (i < mid && j < right) {
        if (!data.greater(i, j)) {
            buffer[k++] = data.get(i++);
        } else {
            buffer[k++] = data.get(j++);
        }
    }

    while (i < mid) {
        buffer[k++] = data.get(i++);
    }

    while (j < right) {
        buffer[k++] = data.get(j++);
    }

    for (size_t index = left; index < right; ++index) {
        data.set(index, buffer[index]);
    }
}
