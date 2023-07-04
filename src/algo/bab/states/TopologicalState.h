#pragma once


#include "State.h"
#include "algo/OnlineTopologicalSorter.h"
#include "algo/bab/StateUpdater.h"
namespace fb {

    typedef OnlineTopologicalSorter::State TopologicalState;


    template<typename State>
        requires std::derived_from<State, TopologicalState>
    class TopologicalStateUpdater : public StateUpdater<State> {
    public:
        explicit TopologicalStateUpdater(OnlineTopologicalSorter &sorter) : sorter_(sorter) {}
        void update(State &state, graph::Selection::DecisionResult decision_result) const override {
            sorter_.updateTopologicalOrder(state, state.selection, decision_result.new_edges);
        }
        void initialize(State &state) const override {

            auto topo_state = sorter_.calculateInitialTopologicalOrder(state.selection);

            state.cycle = topo_state.cycle;
            state.topological_order = std::move(topo_state.topological_order);
            state.sorted_nodes = std::move(topo_state.sorted_nodes);
        }

    private:
        OnlineTopologicalSorter &sorter_;
    };

}// namespace fb
