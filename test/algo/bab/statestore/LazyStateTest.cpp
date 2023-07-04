#include "algo/bab/statestore/LazyState.h"
#include "algo/bab/StateUpdater.h"
#include "algo/bab/states/State.h"
#include "catch2/catch_test_macros.hpp"

namespace fb {


    struct MockState {
        int some_value{0};
    };

    typedef State<MockState> ComplexStateMock;

    struct MockInitializer : StateUpdater<ComplexStateMock> {
        void update(ComplexStateMock &state, graph::Selection::DecisionResult decision_result) const override {}
        void initialize(ComplexStateMock &state) const override { state.some_value = 7; }
    };


    TEST_CASE("LazyState") {

        graph::AlternativeGraph graph;
        graph.createChoice(graph::ChoiceType::STOP_PASS, graph::stopAlternative());
        graph::Selection sel{graph};
        sel.makeDecision({0, graph::stopAlternative()});

        auto initializer = MockInitializer{};

        ComplexStateMock state{std::move(sel)};
        state.lower_bound = 20;
        state.last_decision_made = graph::FullAlternativeIndex{0, graph::stopAlternative()};
        state.some_value = 13;


        LazyState<ComplexStateMock> lazy_state{std::move(state)};

        REQUIRE(lazy_state.getCompleteState(initializer).some_value == 13);
        REQUIRE(lazy_state.getCompleteState(initializer).lower_bound == 20);
        REQUIRE(lazy_state.getCommonState().lower_bound == 20);

        REQUIRE(lazy_state.getCommonState().selection.madeDecisions() == 1);
        REQUIRE(lazy_state.getCommonState().selection.chosenAlternative(0) == graph::stopAlternative());

        REQUIRE(lazy_state.isLoaded());
        lazy_state.free();
        REQUIRE(!lazy_state.isLoaded());

        REQUIRE(lazy_state.getCommonState().lower_bound == 20);
        REQUIRE(lazy_state.getCommonState().selection.madeDecisions() == 1);
        REQUIRE(lazy_state.getCommonState().selection.chosenAlternative(0) == graph::stopAlternative());

        REQUIRE(lazy_state.getCompleteState(initializer).some_value == 7);
        REQUIRE(lazy_state.isLoaded());
        REQUIRE(lazy_state.getCompleteState(initializer).lower_bound == 20);
        REQUIRE(lazy_state.getCompleteState(initializer).selection.madeDecisions() == 1);
        REQUIRE(lazy_state.getCompleteState(initializer).selection.chosenAlternative(0) == graph::stopAlternative());
    }

}// namespace fb
