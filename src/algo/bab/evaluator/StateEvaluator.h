#pragma once


#include "penalty/PenaltyComputer.h"
namespace fb {

    template<typename State>
    class StateEvaluator {
    public:
        typedef State state_type;

        virtual penalty_t boundBeforeDecision(State const &state,
                                              graph::FullAlternativeIndex potential_decision) const = 0;
        virtual penalty_t boundBeforeUpdate(State const &state, graph::FullAlternativeIndex taken_decision,
                                            const graph::Selection::DecisionResult &decision_result) const = 0;
        virtual penalty_t bound(State const &state) const = 0;
    };
}// namespace fb
