#pragma once

namespace fb {

    struct SmallestUndecidedChoiceState {
        size_t smallest_undecided_choice{0};
    };

    template<typename State>
        requires std::derived_from<State, SmallestUndecidedChoiceState> && std::derived_from<State, CommonState>
    struct SmallestUndecidedChoiceUpdater : StateUpdater<State> {
        void update(State &state, graph::Selection::DecisionResult decision_result) const override {

            graph::Selection const &selection = state.selection;

            auto choice_count = selection.getGraph().choices.size();
            if (selection.isFull()) {
                state.smallest_undecided_choice = choice_count;
                return;
            }

            while (state.smallest_undecided_choice < choice_count &&
                   selection.decisionMade(state.smallest_undecided_choice)) {
                ++state.smallest_undecided_choice;
            }

            // we should not reach this step
            assert(state.smallest_undecided_choice < choice_count);
        }
        void initialize(State &state) const override {

            graph::Selection const &selection = state.selection;
            auto choice_count = selection.getGraph().choices.size();
            if (selection.isFull()) {
                state.smallest_undecided_choice = choice_count;
                return;
            }

            state.smallest_undecided_choice = 0;
            while (state.smallest_undecided_choice < choice_count &&
                   selection.decisionMade(state.smallest_undecided_choice)) {
                ++state.smallest_undecided_choice;
            }

            // we should not reach this step
            assert(state.smallest_undecided_choice < choice_count);
        }
    };
}// namespace fb
