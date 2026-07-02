#pragma once

struct SortStats {
    long long compareCount = 0;
    long long writeCount = 0;

    long long keyOpCount() const {
        return compareCount + writeCount;
    }
};
