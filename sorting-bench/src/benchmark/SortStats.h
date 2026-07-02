#pragma once

struct SortStats {
    long long compareCount = 0;
    long long moveCount = 0;
    long long swapCount = 0;

    long long keyOpCount() const {
        return compareCount + moveCount;
    }
};
