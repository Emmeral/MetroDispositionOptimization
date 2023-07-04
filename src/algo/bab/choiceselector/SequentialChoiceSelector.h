
#pragma once

#include "ChoiceSeletor.h"
#include "algo/bab/StateUpdater.h"
#include "algo/bab/states/SmallestUndecidedChoiceState.h"
namespace fb {


    template<typename State>
        requires std::derived_from<State, SmallestUndecidedChoiceState>
    class SequentialChoiceSelector : public ChoiceSelector<State> {

    public:
        graph::choice_index_t getBranchChoice(const State &state) override { return state.smallest_undecided_choice; }
    };

}// namespace fb
