#include "builder/graph/headways/track/TrackHeadwayNodeBuilder.h"
#include "algo/path/LongestPathCalculator.h"
#include "builder/graph/headways/track/util.h"
#include "catch2/catch_test_macros.hpp"
#include "model/graph/AlternativeGraph.h"

namespace fb {


    TEST_CASE("Reverse Bits") {

        REQUIRE(reverseLowestNBits(0b010, 1) == 0b010);
        REQUIRE(reverseLowestNBits(0b010, 2) == 0b001);
        REQUIRE(reverseLowestNBits(0b110, 2) == 0b101);


        REQUIRE(reverseLowestNBits(0b010, 3) == 0b010);
        REQUIRE(reverseLowestNBits(0b100, 3) == 0b001);
    }

    TEST_CASE("TrackHeadwayNodeBuilder") {

        graph::AlternativeGraph graph;
        TrackHeadwayNodeBuilder builder{graph};


        auto base = graph.createNode();
        auto end = graph.createNode();

        SECTION("Two tracks") {

            auto choice = graph.createChoice<graph::ChoiceType::TRACK>(graph::AlternativeIndex::FIRST);
            auto nodes = builder.createNodesForMultiTrack<TrackHeadwayNodeBuilder::Type::OUT>(base, {0, 2}, {choice});


            for (auto const &[track, node]: nodes) {
                graph.createFixedEdge(node, end, track * 100);
            }

            graph::Selection sel{graph};
            LongestPathCalculator calc{graph, base, {end}};

            auto state = calc.calculateInitialPaths(sel);
            REQUIRE(state.distance_from_start[end] == MIN_DISTANCE);


            auto copy_sel = sel;

            sel.makeDecision({choice, graph::AlternativeIndex::FIRST});
            copy_sel.makeDecision({choice, graph::AlternativeIndex::SECOND});

            state = calc.calculateInitialPaths(sel);
            CHECK(state.distance_from_start[end] == 0);

            state = calc.calculateInitialPaths(copy_sel);
            CHECK(state.distance_from_start[end] == 2 * 100);
        }


        SECTION("Three tracks") {

            auto low_bit = graph.createChoice<graph::ChoiceType::TRACK>(graph::AlternativeIndex::FIRST);
            auto high_bit = graph.createChoice<graph::ChoiceType::TRACK>(graph::AlternativeIndex::FIRST);
            auto nodes = builder.createNodesForMultiTrack<TrackHeadwayNodeBuilder::Type::OUT>(base, {0, 1, 2},
                                                                                              {low_bit, high_bit});


            for (auto const &[track, node]: nodes) {
                graph.createFixedEdge(node, end, track * 100);
            }

            graph::Selection sel_xx{graph};
            LongestPathCalculator calc{graph, base, {end}};

            auto state = calc.calculateInitialPaths(sel_xx);
            REQUIRE(state.distance_from_start[end] == MIN_DISTANCE);


            auto sel_x0 = sel_xx;
            sel_x0.makeDecision({low_bit, graph::AlternativeIndex::FIRST});

            auto sel_x1 = sel_xx;
            sel_x1.makeDecision({low_bit, graph::AlternativeIndex::SECOND});


            state = calc.calculateInitialPaths(sel_x0);
            REQUIRE(state.distance_from_start[end] == MIN_DISTANCE);// could be 0 or 2
            state = calc.calculateInitialPaths(sel_x1);
            REQUIRE(state.distance_from_start[end] == 100);// can only be 1

            auto sel_00 = sel_x0;
            sel_00.makeDecision({high_bit, graph::AlternativeIndex::FIRST});

            auto sel_10 = sel_x0;
            sel_10.makeDecision({high_bit, graph::AlternativeIndex::SECOND});

            auto sel_01 = sel_x1;
            sel_01.makeDecision({high_bit, graph::AlternativeIndex::FIRST});

            auto sel_11 = sel_x1;
            sel_11.makeDecision({high_bit, graph::AlternativeIndex::SECOND});


            state = calc.calculateInitialPaths(sel_00);
            REQUIRE(state.distance_from_start[end] == 0);// is 0
            state = calc.calculateInitialPaths(sel_10);
            REQUIRE(state.distance_from_start[end] == 200);

            state = calc.calculateInitialPaths(sel_01);
            REQUIRE(state.distance_from_start[end] == 100);
            state = calc.calculateInitialPaths(sel_11);
            REQUIRE(state.distance_from_start[end] == 100);// no track 3 -> must be 1
        }

        SECTION("Four tracks") {

            auto low_bit = graph.createChoice<graph::ChoiceType::TRACK>(graph::AlternativeIndex::FIRST);
            auto high_bit = graph.createChoice<graph::ChoiceType::TRACK>(graph::AlternativeIndex::FIRST);
            auto nodes = builder.createNodesForMultiTrack<TrackHeadwayNodeBuilder::Type::OUT>(base, {0, 1, 2, 3},
                                                                                              {low_bit, high_bit});


            for (auto const &[track, node]: nodes) {
                graph.createFixedEdge(node, end, track * 100);
            }

            graph::Selection sel_xx{graph};
            LongestPathCalculator calc{graph, base, {end}};

            auto state = calc.calculateInitialPaths(sel_xx);
            REQUIRE(state.distance_from_start[end] == MIN_DISTANCE);


            auto sel_x0 = sel_xx;
            sel_x0.makeDecision({low_bit, graph::AlternativeIndex::FIRST});

            auto sel_x1 = sel_xx;
            sel_x1.makeDecision({low_bit, graph::AlternativeIndex::SECOND});


            state = calc.calculateInitialPaths(sel_x0);
            REQUIRE(state.distance_from_start[end] == MIN_DISTANCE);// could be 0 or 2
            state = calc.calculateInitialPaths(sel_x1);
            REQUIRE(state.distance_from_start[end] == MIN_DISTANCE);// could be 1 or 3

            auto sel_00 = sel_x0;
            sel_00.makeDecision({high_bit, graph::AlternativeIndex::FIRST});

            auto sel_10 = sel_x0;
            sel_10.makeDecision({high_bit, graph::AlternativeIndex::SECOND});

            auto sel_01 = sel_x1;
            sel_01.makeDecision({high_bit, graph::AlternativeIndex::FIRST});

            auto sel_11 = sel_x1;
            sel_11.makeDecision({high_bit, graph::AlternativeIndex::SECOND});


            state = calc.calculateInitialPaths(sel_00);
            REQUIRE(state.distance_from_start[end] == 0);// is 0
            state = calc.calculateInitialPaths(sel_10);
            REQUIRE(state.distance_from_start[end] == 200);

            state = calc.calculateInitialPaths(sel_01);
            REQUIRE(state.distance_from_start[end] == 100);
            state = calc.calculateInitialPaths(sel_11);
            REQUIRE(state.distance_from_start[end] == 300);
        }

        SECTION("Six tracks") {

            auto lowest_bit = graph.createChoice<graph::ChoiceType::TRACK>(graph::AlternativeIndex::FIRST);
            auto middle_bit = graph.createChoice<graph::ChoiceType::TRACK>(graph::AlternativeIndex::FIRST);
            auto highest_bit = graph.createChoice<graph::ChoiceType::TRACK>(graph::AlternativeIndex::FIRST);
            auto nodes = builder.createNodesForMultiTrack<TrackHeadwayNodeBuilder::Type::OUT>(
                    base, {0, 1, 2, 3, 4, 5}, {lowest_bit, middle_bit, highest_bit});


            for (auto const &[track, node]: nodes) {
                graph.createFixedEdge(node, end, track * 100);
            }

            graph::Selection sel_xxx{graph};
            LongestPathCalculator calc{graph, base, {end}};

            auto state = calc.calculateInitialPaths(sel_xxx);
            REQUIRE(state.distance_from_start[end] == MIN_DISTANCE);


            auto sel_xx0 = sel_xxx;
            sel_xx0.makeDecision({lowest_bit, graph::AlternativeIndex::FIRST});

            auto sel_xx1 = sel_xxx;
            sel_xx1.makeDecision({lowest_bit, graph::AlternativeIndex::SECOND});


            state = calc.calculateInitialPaths(sel_xx0);
            REQUIRE(state.distance_from_start[end] == MIN_DISTANCE);
            state = calc.calculateInitialPaths(sel_xx1);
            REQUIRE(state.distance_from_start[end] == MIN_DISTANCE);

            auto sel_x00 = sel_xx0;
            sel_x00.makeDecision({middle_bit, graph::AlternativeIndex::FIRST});

            auto sel_x10 = sel_xx0;
            sel_x10.makeDecision({middle_bit, graph::AlternativeIndex::SECOND});

            auto sel_x01 = sel_xx1;
            sel_x01.makeDecision({middle_bit, graph::AlternativeIndex::FIRST});

            auto sel_x11 = sel_xx1;
            sel_x11.makeDecision({middle_bit, graph::AlternativeIndex::SECOND});


            state = calc.calculateInitialPaths(sel_x00);
            REQUIRE(state.distance_from_start[end] == MIN_DISTANCE);// could be 4 or 0
            state = calc.calculateInitialPaths(sel_x10);
            REQUIRE(state.distance_from_start[end] == 200);// the highest bit is irrelevant with 5 total tracks

            state = calc.calculateInitialPaths(sel_x01);
            REQUIRE(state.distance_from_start[end] == MIN_DISTANCE);// could be 5 or 1
            state = calc.calculateInitialPaths(sel_x11);
            REQUIRE(state.distance_from_start[end] == 300);// the highest bit is irrelevant with 5 total tracks


            auto sel_000 = sel_x00;
            sel_000.makeDecision({highest_bit, graph::AlternativeIndex::FIRST});
            auto sel_100 = sel_x00;
            sel_100.makeDecision({highest_bit, graph::AlternativeIndex::SECOND});
            auto sel_001 = sel_x01;
            sel_001.makeDecision({highest_bit, graph::AlternativeIndex::FIRST});
            auto sel_101 = sel_x01;
            sel_101.makeDecision({highest_bit, graph::AlternativeIndex::SECOND});

            auto sel_010 = sel_x10;
            sel_010.makeDecision({highest_bit, graph::AlternativeIndex::FIRST});
            auto sel_110 = sel_x10;
            sel_110.makeDecision({highest_bit, graph::AlternativeIndex::SECOND});
            auto sel_011 = sel_x11;
            sel_011.makeDecision({highest_bit, graph::AlternativeIndex::FIRST});
            auto sel_111 = sel_x11;
            sel_111.makeDecision({highest_bit, graph::AlternativeIndex::SECOND});

            state = calc.calculateInitialPaths(sel_000);
            REQUIRE(state.distance_from_start[end] == 0);
            state = calc.calculateInitialPaths(sel_100);
            REQUIRE(state.distance_from_start[end] == 400);

            state = calc.calculateInitialPaths(sel_001);
            REQUIRE(state.distance_from_start[end] == 100);
            state = calc.calculateInitialPaths(sel_101);
            REQUIRE(state.distance_from_start[end] == 500);


            state = calc.calculateInitialPaths(sel_010);
            REQUIRE(state.distance_from_start[end] == 200);
            state = calc.calculateInitialPaths(sel_110);
            REQUIRE(state.distance_from_start[end] == 200);

            state = calc.calculateInitialPaths(sel_011);
            REQUIRE(state.distance_from_start[end] == 300);
            state = calc.calculateInitialPaths(sel_111);
            REQUIRE(state.distance_from_start[end] == 300);
        }
    }


}// namespace fb
