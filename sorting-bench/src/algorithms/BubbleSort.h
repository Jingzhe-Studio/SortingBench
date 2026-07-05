#pragma once

#include "../contract/Sorter.h"
#include "../trace/TraceArray.h"

class BubbleSort : public Sorter {
public:
    std::string name() const override;

    void sort(TraceArray& data) override;
};
