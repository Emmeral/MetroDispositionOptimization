#pragma once

#include "ChoiceSeletor.h"
#include "algo/bab/states/ChoiceImpactState.h"
#include <numeric>

namespace fb {


    template<typename State>
        requires std::derived_from<State, ChoiceImpactState>
    class BiggestDistanceToEndChoiceSelector : public ChoiceSelector<State> {

    public:
        graph::choice_index_t getBranchChoice(const State &state) override { return state.choice_queue.top().choice; }
    };
}// namespace fb
