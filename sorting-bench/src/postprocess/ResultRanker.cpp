#include "ResultRanker.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include "../benchmark/BenchmarkResult.h"

namespace {

/* ------------------------------------------------------------------------ */
/*  Competition ranking  ("1 2 2 4") — equal values share the same rank,    */
/*  then we skip.                                                           */
/* ------------------------------------------------------------------------ */
template <typename Proj>
void applyRank(
    std::vector<BenchmarkResult>& results,
    Proj proj,                         // double proj(const BenchmarkResult&)
    int BenchmarkResult::* rankField   // e.g. &BenchmarkResult::timeRank
) {
    // Sort indices by the projected value ascending.
    std::vector<size_t> idx(results.size());
    for (size_t i = 0; i < idx.size(); ++i) idx[i] = i;

    std::sort(idx.begin(), idx.end(), [&](size_t a, size_t b) {
        return proj(results[a]) < proj(results[b]);
    });

    // Walk in sorted order, assigning ranks.
    for (size_t pos = 0; pos < idx.size(); ) {
        size_t sameEnd = pos;
        while (sameEnd + 1 < idx.size() &&
               proj(results[idx[sameEnd + 1]]) == proj(results[idx[pos]])) {
            ++sameEnd;
        }
        int rank = static_cast<int>(pos + 1);       // 1-based
        for (size_t k = pos; k <= sameEnd; ++k) {
            results[idx[k]].*rankField = rank;
        }
        pos = sameEnd + 1;
    }
}

}  // anonymous namespace

/* ------------------------------------------------------------------------ */
/*  Public API                                                              */
/* ------------------------------------------------------------------------ */

void ResultRanker::rank(std::vector<BenchmarkResult>& results) {
    if (results.empty()) {
        return;
    }

    // 1. timeRank — by elapsedMs ascending
    applyRank(results,
              [](const BenchmarkResult& r) { return r.elapsedMs; },
              &BenchmarkResult::timeRank);

    // 2. operationRank — by keyOpCount ascending
    applyRank(results,
              [](const BenchmarkResult& r) {
                  return static_cast<double>(r.keyOpCount);
              },
              &BenchmarkResult::operationRank);

    // 3. overallScore — equal-weight combination of the two ranks.
    //    A failed sorter gets a sentinel so it always sorts last.
    for (auto& r : results) {
        if (r.sortedCorrectly) {
            r.overallScore = r.timeRank * 0.5 + r.operationRank * 0.5;
        } else {
            r.overallScore = std::numeric_limits<double>::max();
        }
    }

    // 4. overallRank — by overallScore ascending
    applyRank(results,
              [](const BenchmarkResult& r) { return r.overallScore; },
              &BenchmarkResult::overallRank);
}
