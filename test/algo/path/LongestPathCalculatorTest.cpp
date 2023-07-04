
#include "algo/path/LongestPathCalculator.h"
#include "catch2/catch_test_macros.hpp"
#include "model/graph/AlternativeGraph.h"
namespace fb {


    TEST_CASE("LongestPathCalculator Initial Path") {

        graph::AlternativeGraph graph;
        graph::Selection selection{graph};

        auto start = graph.createNode();
        auto end = graph.createNode();
        std::vector<graph::node_index_t> ends = {end};


        SECTION("Calculates simple path") {
            auto inter1 = graph.createNode();
            auto inter2 = graph.createNode();

            graph.createFixedEdge(start, inter1, 10);
            graph.createFixedEdge(inter1, inter2, 10);
            graph.createFixedEdge(inter2, end, 10);

            LongestPathCalculator pathCalculator{graph, start, ends};
            OnlineTopologicalSorter sorter{graph};
            auto state =
                    pathCalculator.calculateInitialPaths(selection, sorter.calculateInitialTopologicalOrder(selection));

            REQUIRE(state.distance_from_start[end] == 30);
        }

        SECTION("Calculates split path") {
            auto inter1 = graph.createNode();
            auto inter2 = graph.createNode();

            graph.createFixedEdge(start, inter1, 20);
            graph.createFixedEdge(start, inter2, 10);
            graph.createFixedEdge(inter1, end, 20);
            graph.createFixedEdge(inter2, end, 40);

            LongestPathCalculator pathCalculator{graph, start, ends};
            OnlineTopologicalSorter sorter{graph};
            auto state =
                    pathCalculator.calculateInitialPaths(selection, sorter.calculateInitialTopologicalOrder(selection));

            REQUIRE(state.distance_from_start[end] == 50);
        }

        SECTION("Does not use invalid path") {

            auto inter1 = graph.createNode();
            auto wrong1 = graph.createNode();
            auto wrong2 = graph.createNode();

            graph.createFixedEdge(start, inter1, 20);
            graph.createFixedEdge(inter1, end, 20);


            graph.createFixedEdge(wrong1, wrong2, 50);
            graph.createFixedEdge(wrong2, end, 50);

            LongestPathCalculator pathCalculator{graph, start, ends};
            OnlineTopologicalSorter sorter{graph};
            auto state =
                    pathCalculator.calculateInitialPaths(selection, sorter.calculateInitialTopologicalOrder(selection));

            REQUIRE(state.distance_from_start[end] == 40);
        }

        SECTION("With no path") {

            auto inter1 = graph.createNode();
            auto inter2 = graph.createNode();

            graph.createFixedEdge(start, inter1, 20);
            graph.createFixedEdge(inter2, end, 40);

            LongestPathCalculator pathCalculator{graph, start, ends};
            OnlineTopologicalSorter sorter{graph};
            auto state =
                    pathCalculator.calculateInitialPaths(selection, sorter.calculateInitialTopologicalOrder(selection));

            REQUIRE(state.distance_from_start[end] == MIN_DISTANCE);
        }
    }


    TEST_CASE("LongestPathCalculator Path Update") {

        graph::AlternativeGraph graph;

        for (auto i = 0; i < 10; i++) {
            graph.createNode();
        }
        auto start = 0u;
        auto end = 9u;
        std::vector<graph::node_index_t> ends = {end};

        auto choice = graph.createChoice<graph::ChoiceType::LINE_HEADWAY>(graph::AlternativeIndex::FIRST);
        graph::FullAlternativeIndex fai = {choice, graph::AlternativeIndex::FIRST};
        std::vector<graph::a_edge_index_t> created;

        SECTION("No Update Necessary") {

            for (auto i = 0; i < 9; i++) {
                graph.createFixedEdge(i, i + 1, 10);
            }

            for (auto i = 1; i < 5; i++) {
                created.push_back(graph.createAlternativeEdge(i, 2 * i, fai, 5));
            }

            LongestPathCalculator calculator{graph, start, ends};
            OnlineTopologicalSorter sorter{graph};
            graph::Selection sel{graph};

            auto sorter_state = sorter.calculateInitialTopologicalOrder(sel);
            auto state = calculator.calculateInitialPaths(sel, sorter_state);

            for (auto i = 0; i < 10; i++) {
                REQUIRE(state.distance_from_start[i] == 10 * i);
            }
            sel.makeDecision(fai);

            sorter.updateTopologicalOrder(sorter_state, sel, created);
            calculator.updateBasedOnNewEdges(state, sel, sorter_state, created);

            // nothing changed
            for (auto i = 0; i < 10; i++) {
                REQUIRE(state.distance_from_start[i] == 10 * i);
            }
        }


        SECTION("Simple Updated Path") {

            for (auto i = 0; i < 9; i++) {
                graph.createFixedEdge(i, i + 1, 10);
            }

            // edges not improving
            for (auto i = 1; i < 5; i++) {
                created.push_back(graph.createAlternativeEdge(i, 2 * i, fai, 5));
            }
            // edges improving path
            created.push_back(graph.createAlternativeEdge(3, 8, fai, 60));
            created.push_back(graph.createAlternativeEdge(1, 4, fai, 35));

            LongestPathCalculator calculator{graph, start, ends};
            OnlineTopologicalSorter sorter{graph};
            graph::Selection sel{graph};

            auto sorter_state = sorter.calculateInitialTopologicalOrder(sel);
            auto state = calculator.calculateInitialPaths(sel, sorter_state);

            for (auto i = 0; i < 10; i++) {
                REQUIRE(state.distance_from_start[i] == 10 * i);
            }
            sel.makeDecision(fai);

            sorter.updateTopologicalOrder(sorter_state, sel, created);
            calculator.updateBasedOnNewEdges(state, sel, sorter_state, created);

            // adapt for changed better paths
            for (auto i = 0; i < 4; i++) {
                REQUIRE(state.distance_from_start[i] == 10 * i);
            }
            for (auto i = 4; i < 8; i++) {
                REQUIRE(state.distance_from_start[i] == 10 * i + 5);
            }
            for (auto i = 8; i < 10; i++) {
                REQUIRE(state.distance_from_start[i] == 10 * i + 10);
            }
        }

        SECTION("New Nodes Reachable") {

            for (auto i = 0; i < 5; i++) {
                graph.createFixedEdge(i, i + 1, 10);
            }
            graph.createFixedEdge(5, 9, 10);

            for (auto i = 6; i < 9; i++) {
                graph.createFixedEdge(i, i + 1, 10);
            }

            // edges improving path
            created.push_back(graph.createAlternativeEdge(5, 6, fai, 10));

            LongestPathCalculator calculator{graph, start, ends};
            OnlineTopologicalSorter sorter{graph};
            graph::Selection sel{graph};

            auto sorter_state = sorter.calculateInitialTopologicalOrder(sel);
            auto state = calculator.calculateInitialPaths(sel, sorter_state);


            for (auto i = 0; i < 6; i++) {
                REQUIRE(state.distance_from_start[i] == 10 * i);
            }
            for (auto i = 6; i < 9; i++) {
                REQUIRE(state.distance_from_start[i] == MIN_DISTANCE);
            }
            REQUIRE(state.distance_from_start[end] == 60);


            sel.makeDecision(fai);

            sorter.updateTopologicalOrder(sorter_state, sel, created);
            calculator.updateBasedOnNewEdges(state, sel, sorter_state, created);


            for (auto i = 0; i < 10; i++) {
                REQUIRE(state.distance_from_start[i] == 10 * i);
            }
        }


        SECTION("Double new Edge") {

            for (auto i = 0; i < 9; i++) {
                graph.createFixedEdge(i, i + 1, 10);
            }

            created.push_back(graph.createAlternativeEdge(5, 7, fai, 100));
            created.push_back(graph.createAlternativeEdge(2, 5, fai, 100));

            LongestPathCalculator calculator{graph, start, ends};
            OnlineTopologicalSorter sorter{graph};
            graph::Selection sel{graph};

            auto sorter_state = sorter.calculateInitialTopologicalOrder(sel);
            auto state = calculator.calculateInitialPaths(sel, sorter_state);

            sel.makeDecision(fai);

            sorter.updateTopologicalOrder(sorter_state, sel, created);
            calculator.updateBasedOnNewEdges(state, sel, sorter_state, created);

            // updated double edges
            REQUIRE(state.distance_from_start[0] == 0);
            REQUIRE(state.distance_from_start[1] == 10);
            REQUIRE(state.distance_from_start[2] == 20);
            REQUIRE(state.distance_from_start[3] == 30);
            REQUIRE(state.distance_from_start[4] == 40);
            REQUIRE(state.distance_from_start[5] == 120);
            REQUIRE(state.distance_from_start[6] == 130);
            REQUIRE(state.distance_from_start[7] == 220);
            REQUIRE(state.distance_from_start[8] == 230);
            REQUIRE(state.distance_from_start[9] == 240);
        }
    }

}// namespace fb
