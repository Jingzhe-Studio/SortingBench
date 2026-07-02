#pragma once

#include "../contract/Sorter.h"
#include "../trace/TraceArray.h"

class QuickSort : public Sorter {
public:
    std::string name() const override;

    void sort(TraceArray& data) override;

private:
    void quickSort(TraceArray& data, int low, int high);

    int partition(TraceArray& data, int low, int high);
};
