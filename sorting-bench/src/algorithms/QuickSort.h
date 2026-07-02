#pragma once

#include "../contract/Sorter.h"
#include "../trace/TraceInt.h"

class QuickSort : public Sorter {
public:
    std::string name() const override;

    void sort(std::vector<TraceInt>& data) override;

private:
    void quickSort(std::vector<TraceInt>& data, int low, int high);

    int partition(std::vector<TraceInt>& data, int low, int high);
};
