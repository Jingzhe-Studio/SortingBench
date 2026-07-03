#pragma once

#include <exception>

/// Thrown by TraceArray operations when a timeout has fired.
struct SortCancelledException : std::exception {
    const char* what() const noexcept override {
        return "Sort cancelled due to timeout";
    }
};
