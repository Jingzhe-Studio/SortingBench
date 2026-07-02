#pragma once

#include <string>
#include <vector>

class TraceInt;

class Sorter {
public:
    virtual std::string name() const = 0;

    virtual void sort(std::vector<TraceInt>& data) = 0;

    virtual ~Sorter() = default;
};
