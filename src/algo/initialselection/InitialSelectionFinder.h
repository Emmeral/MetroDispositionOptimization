#pragma once
#include "algo/path/LongestPathCalculator.h"
#include "model/graph/GraphWithMeta.h"
#include "model/graph/Selection.h"

namespace fb {

    class InitialSelectionFinder {
    public:
        struct Result {
            graph::Selection selection;
            std::optional<LongestPathCalculator::State> pc_state{std::nullopt};
            std::optional<OnlineTopologicalSorter::State> order_state{std::nullopt};

            explicit Result(graph::Selection &&selection) : selection(std::move(selection)){};
            Result(graph::Selection &&selection, LongestPathCalculator::State &&pc_state,
                   OnlineTopologicalSorter::State &&order_state)
                : selection(std::move(selection)), pc_state(std::move(pc_state)), order_state(std::move(order_state)) {}
        };

        virtual Result calculateInitialSelectionWithMeta(graph::GraphWithMeta const &gwm) = 0;
        virtual graph::Selection calculateInitialSelection(graph::GraphWithMeta const &gwm) {
            return std::move(calculateInitialSelectionWithMeta(gwm).selection);
        };
    };


}// namespace fb
