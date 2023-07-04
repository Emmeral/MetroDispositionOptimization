#include "algo/bab/statestore/BestOfLastInsertedStateStore.h"
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


    TEST_CASE("BOL State Store") {

        auto initializer = MockInitializer{};
        BestOfLastInsertedStateStore<ComplexStateMock, MockInitializer> store{3, 3000, initializer};

        graph::AlternativeGraph graph;

        ComplexStateMock s1{graph::Selection(graph)};
        s1.lower_bound = 1;
        ComplexStateMock s2{graph::Selection(graph)};
        s2.lower_bound = 2;
        ComplexStateMock s3{graph::Selection(graph)};
        s3.lower_bound = 3;
        ComplexStateMock s4{graph::Selection(graph)};
        s4.lower_bound = 4;


        SECTION("Value out of last") {
            store.registerState(ComplexStateMock{s1});
            store.registerState(ComplexStateMock{s2});

            REQUIRE(store.peekState().lower_bound == 1);

            store.registerState(ComplexStateMock{s3});
            store.registerState(ComplexStateMock{s4});

            REQUIRE(store.peekState().lower_bound == 2);// 1 is no longer part of the last 3
            REQUIRE(store.peekState().lower_bound == 2);

            store.popState();

            REQUIRE(store.peekState().lower_bound == 1);

            store.popState();

            REQUIRE(store.peekState().lower_bound == 3);
        }

        SECTION("Re register Head") {
            store.registerState(ComplexStateMock{s1});
            store.registerState(ComplexStateMock{s2});
            store.registerState(ComplexStateMock{s3});

            REQUIRE(store.peekState().lower_bound == 1);
            store.reRegisterHead();

            store.registerState(ComplexStateMock{s3});
            store.registerState(ComplexStateMock{s3});
            REQUIRE(store.peekState().lower_bound == 1);// head was moved to newest position -> so still in last 3
        }

        SECTION("Prunes") {
            store.registerState(ComplexStateMock{s1});
            store.registerState(ComplexStateMock{s2});
            store.registerState(ComplexStateMock{s3});
            store.registerState(ComplexStateMock{s3});
            store.registerState(ComplexStateMock{s3});

            REQUIRE(store.size() == 5);

            store.registerNewUpperBound(s3);

            REQUIRE(store.size() == 2);
            REQUIRE(store.peekState().lower_bound == 1);
        }
    }


}// namespace fb
