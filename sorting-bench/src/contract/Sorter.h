#pragma once

#include <string>

class TraceArray;

class Sorter {
public:
    virtual std::string name() const = 0;

    virtual void sort(TraceArray& data) = 0;

    virtual ~Sorter() = default;
};
