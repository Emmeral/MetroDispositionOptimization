#pragma once


#include "State.h"
#include "TopologicalState.h"
#include "algo/bab/StateUpdater.h"
#include "algo/path/LongestPathCalculator.h"
namespace fb {


    typedef LongestPathCalculator::State DistanceState;


    template<typename State>
        requires std::derived_from<State, DistanceState> && std::derived_from<State, CommonState> &&
                 std::derived_from<State, TopologicalState>
    class DistanceStateUpdater : public StateUpdater<State> {
    public:
        explicit DistanceStateUpdater(LongestPathCalculator &calculator) : calculator_(calculator) {}
        void update(State &state, graph::Selection::DecisionResult decision_result) const override {
            calculator_.updateBasedOnNewEdges(state, state.selection, state, decision_result.new_edges);
        }
        void initialize(State &state) const override {

            auto distance_state = calculator_.calculateInitialPaths(state.selection, state);
            state.distance_from_start = std::move(distance_state.distance_from_start);
            state.predecessor = std::move(distance_state.predecessor);
        }

    private:
        LongestPathCalculator &calculator_;
    };

}// namespace fb
