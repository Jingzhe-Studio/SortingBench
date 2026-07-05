#pragma once

#include <vector>

#include "../contract/Sorter.h"
#include "../trace/TraceArray.h"

class MergeSort : public Sorter {
public:
    std::string name() const override;

    void sort(TraceArray& data) override;

private:
    void mergeSort(TraceArray& data, std::vector<int>& buffer, size_t left, size_t right);

    void merge(TraceArray& data, std::vector<int>& buffer, size_t left, size_t mid, size_t right);
};
