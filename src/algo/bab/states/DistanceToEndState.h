#pragma once


#include "TopologicalState.h"
#include "algo/path/BackwardsPathCalculator.h"
#include "algo/path/LongestPathCalculator.h"
namespace fb {


    template<size_t N>
    using DistanceToEndState = BackwardsPathState<N>;

    typedef DistanceToEndState<1> MaxDistanceToEndState;

    template<typename State>
        requires std::derived_from<State, DistanceToEndState<State::BACKWARDS_DISTANCES_COUNT>> &&
                 std::derived_from<State, CommonState> && std::derived_from<State, TopologicalState>
    class DistanceToEndStateUpdater : public StateUpdater<State> {
    public:
        explicit DistanceToEndStateUpdater(BackwardsPathCalculator &calculator) : calculator_(calculator) {}
        void update(State &state, graph::Selection::DecisionResult decision_result) const override {
            calculator_.updateBasedOnNewEdges(state, state.selection, state, decision_result.new_edges,
                                              state.distance_from_start);
        }
        void initialize(State &state) const override {

            auto distance_state = calculator_.calculateInitialPaths<State::BACKWARDS_DISTANCES_COUNT>(
                    state.selection, state, state.distance_from_start);
            state.distances_to_end = std::move(distance_state.distances_to_end);
            state.distance_to_end_bias = distance_state.distance_to_end_bias;
            state.unselected_edges_with_updated_target = std::move(distance_state.unselected_edges_with_updated_target);
        }

    private:
        BackwardsPathCalculator &calculator_;
    };


}// namespace fb
