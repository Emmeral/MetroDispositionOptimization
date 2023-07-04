#pragma once

#include "model/graph/Selection.h"
#include "penalty/definitions.h"
namespace fb {


    struct CommonState {

        explicit CommonState(graph::Selection &&sel) : selection(sel) {}
        CommonState(CommonState &other) = default;
        CommonState(CommonState &&other) noexcept = default;
        CommonState &operator=(CommonState const &other) = default;
        // base properties
        graph::Selection selection;
        penalty_t lower_bound{0};

        graph::FullAlternativeIndex last_decision_made{invalid<graph::choice_index_t>(),
                                                       graph::AlternativeIndex::FIRST};
    };

    template<typename... DataClass>
    struct State : CommonState, DataClass... {

        explicit State(graph::Selection &&selection) : CommonState(std::move(selection)) {}
        explicit State(CommonState &&state) : CommonState(state) {}

        State(State<DataClass...> &other) = default;
        State(State<DataClass...> &&other) noexcept = default;

        State &operator=(State<DataClass...> const &other) = default;
    };


}// namespace fb
