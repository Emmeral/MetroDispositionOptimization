
#pragma once
#include "InitialSelectionFinder.h"
#include "algo/OnlineTopologicalSorter.h"
#include "algo/path/LongestPathCalculator.h"

namespace fb {
    class OriginalInitialSelectionFinder : public InitialSelectionFinder {

    public:
        Result calculateInitialSelectionWithMeta(const graph::GraphWithMeta &gwm) override;
    };
}// namespace fb
