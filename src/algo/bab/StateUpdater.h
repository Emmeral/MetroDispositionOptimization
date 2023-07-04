#pragma once


#include "algo/bab/states/State.h"
#include "model/graph/Selection.h"
namespace fb {

    template<typename State>
    class StateUpdater {
    public:
        typedef State state_type;

        virtual void update(State &state, graph::Selection::DecisionResult decision_result) const = 0;
        virtual void initialize(State &state) const = 0;
    };

    template<typename State, typename... Updaters>
    class CompositeUpdater;


    template<typename State, typename FirstUpdater, typename... RestUpdaters>
        requires std::derived_from<FirstUpdater, StateUpdater<State>>
    class CompositeUpdater<State, FirstUpdater, RestUpdaters...> : public CompositeUpdater<State, RestUpdaters...> {
    public:
        explicit CompositeUpdater(FirstUpdater updater, RestUpdaters... rest)
            : CompositeUpdater<State, RestUpdaters...>{rest...}, updater_(updater) {}
        void update(State &state, graph::Selection::DecisionResult decision_result) const override {
            updater_.update(state, decision_result);
            CompositeUpdater<State, RestUpdaters...>::update(state, decision_result);
        }
        void initialize(State &state) const override {
            updater_.initialize(state);
            CompositeUpdater<State, RestUpdaters...>::initialize(state);
        }

    private:
        FirstUpdater updater_;
    };


    template<typename State>
    class CompositeUpdater<State> : public StateUpdater<State> {
    public:
        void update(State &state, graph::Selection::DecisionResult decision_result) const override {}
        void initialize(State &state) const override {}
    };


}// namespace fb
