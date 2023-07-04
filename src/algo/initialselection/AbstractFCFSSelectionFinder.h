
#pragma once

#include "InitialSelectionFinder.h"
#include "algo/path/LongestPathCalculator.h"
namespace fb {

    class AbstractFCFSSelectionFinder : public fb::InitialSelectionFinder {

    public:
        explicit AbstractFCFSSelectionFinder(const std::vector<RSDuty> &duties);

    protected:
        const std::vector<RSDuty> &duties_;

        struct AbsAndState {
            enum class State { ARRIVAL, DEPARTURE };

            graph::abs_index_t abs{invalid<graph::abs_index_t>()};
            State state{State::ARRIVAL};
        };

        void setTriviallyImpliedChoices(const graph::AlternativeGraph &graph, graph::Selection &sel) const;
        void setNonOrderingDecisionToOriginal(const graph::AlternativeGraph &graph, graph::Selection &sel) const;
        graph::Selection::DecisionResult makeDecisionsForChosen(const graph::GraphWithMeta &gwm, graph::Selection &sel,
                                                                const AbsAndState *chosen) const;
    };

}// namespace fb
