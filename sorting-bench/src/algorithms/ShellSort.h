#pragma once

#include "../contract/Sorter.h"
#include "../trace/TraceArray.h"

class ShellSort : public Sorter {
public:
    std::string name() const override;

    void sort(TraceArray& data) override;
};
