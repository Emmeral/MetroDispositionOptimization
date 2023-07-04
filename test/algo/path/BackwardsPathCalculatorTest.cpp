#include "algo/path/BackwardsPathCalculator.h"
#include "algo/OnlineTopologicalSorter.h"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "model/graph/AlternativeGraph.h"
#include "model/graph/Selection.h"

namespace fb {


    TEST_CASE("Backwards Path Calculator Merge") {


        SECTION("Merges") {
            std::array<DistDestPair, 5> first = {{{30, 3}, {20, 2}, {10, 1}, DistDestPair(), DistDestPair()}};
            std::array<DistDestPair, 5> second = {{{25, 2}, {13, 3}, {1, 1}, DistDestPair(), DistDestPair()}};
            std::array<DistDestPair, 5> dest;

            dest.fill(DistDestPair());

            bool first_first = GENERATE(true, false);
            bool from_second;

            if (first_first) {
                from_second = BackwardsPathCalculator::merge(first, second, dest);
            } else {
                from_second = BackwardsPathCalculator::merge(second, first, dest);
            }

            REQUIRE(from_second);

            REQUIRE(dest[0] == DistDestPair{30, 3});
            REQUIRE(dest[1] == DistDestPair{25, 2});
            REQUIRE(dest[2] == DistDestPair{10, 1});
            REQUIRE(dest[3] == DistDestPair());
            REQUIRE(dest[4] == DistDestPair());
        }

        SECTION("Only uses first") {
            std::array<DistDestPair, 5> first = {{{30, 3}, {20, 2}, {10, 1}, DistDestPair(), DistDestPair()}};
            std::array<DistDestPair, 5> second = {{{18, 2}, {13, 3}, {1, 1}, DistDestPair(), DistDestPair()}};
            std::array<DistDestPair, 5> dest;

            dest.fill(DistDestPair());

            bool from_second = BackwardsPathCalculator::merge(first, second, dest);

            REQUIRE(!from_second);

            REQUIRE(dest[0] == DistDestPair{30, 3});
            REQUIRE(dest[1] == DistDestPair{20, 2});
            REQUIRE(dest[2] == DistDestPair{10, 1});
            REQUIRE(dest[3] == DistDestPair());
            REQUIRE(dest[4] == DistDestPair());
        }
    }

    TEST_CASE("Backwards Path Calculator Initial Paths") {

        graph::AlternativeGraph graph;


        auto start = graph.createNode();
        auto end1 = graph.createNode();
        auto end2 = graph.createNode();
        auto end3 = graph.createNode();
        std::vector<graph::node_index_t> ends = {end1, end2, end3};


        SECTION("Calculates simple path") {

            auto inter = graph.createNode();

            graph.createFixedEdge(start, inter, 10);
            graph.createFixedEdge(inter, end1, 10);
            graph.createFixedEdge(inter, end2, 20);
            graph.createFixedEdge(inter, end3, 30);

            graph::Selection selection{graph};
            OnlineTopologicalSorter sorter{graph};
            auto order_state = sorter.calculateInitialTopologicalOrder(selection);
            BackwardsPathCalculator calc{graph, start, ends};

            SECTION("Single entry") {

                auto state = calc.calculateInitialPaths<1>(selection, order_state);

                REQUIRE(state.distances_to_end[inter][0].dist == 30);
                REQUIRE(state.distances_to_end[inter][0].dest == end3);

                REQUIRE(state.distances_to_end[start][0].dist == 40);
                REQUIRE(state.distances_to_end[start][0].dest == end3);
            }
            SECTION("Two Entries") {
                auto state = calc.calculateInitialPaths<2>(selection, order_state);

                REQUIRE(state.distances_to_end[inter][0].dist == 30);
                REQUIRE(state.distances_to_end[inter][0].dest == end3);

                REQUIRE(state.distances_to_end[inter][1].dist == 20);
                REQUIRE(state.distances_to_end[inter][1].dest == end2);

                REQUIRE(state.distances_to_end[start][0].dist == 40);
                REQUIRE(state.distances_to_end[start][0].dest == end3);

                REQUIRE(state.distances_to_end[start][1].dist == 30);
                REQUIRE(state.distances_to_end[start][1].dest == end2);
            }

            SECTION("Three Entries") {
                auto state = calc.calculateInitialPaths<3>(selection, order_state);

                REQUIRE(state.distances_to_end[inter][0].dist == 30);
                REQUIRE(state.distances_to_end[inter][0].dest == end3);

                REQUIRE(state.distances_to_end[inter][1].dist == 20);
                REQUIRE(state.distances_to_end[inter][1].dest == end2);

                REQUIRE(state.distances_to_end[inter][2].dist == 10);
                REQUIRE(state.distances_to_end[inter][2].dest == end1);

                REQUIRE(state.distances_to_end[start][0].dist == 40);
                REQUIRE(state.distances_to_end[start][0].dest == end3);

                REQUIRE(state.distances_to_end[start][1].dist == 30);
                REQUIRE(state.distances_to_end[start][1].dest == end2);

                REQUIRE(state.distances_to_end[start][2].dist == 20);
                REQUIRE(state.distances_to_end[start][2].dest == end1);
            }
        }

        SECTION("Calculates complex path") {

            //     /--20--n1 --10--- n4 ---0--e1
            //    /       |20         |30
            //   s         >-- n3 -10-->------e2
            //    \       |20         |10
            //     \--10--n2 --10--- n5 --20--e3


            auto n1 = graph.createNode();
            auto n2 = graph.createNode();
            auto n3 = graph.createNode();
            auto n4 = graph.createNode();
            auto n5 = graph.createNode();

            graph.createFixedEdge(start, n1, 20);
            graph.createFixedEdge(start, n2, 10);

            graph.createFixedEdge(n1, n3, 20);
            graph.createFixedEdge(n1, n4, 10);

            graph.createFixedEdge(n2, n3, 20);
            graph.createFixedEdge(n2, n5, 10);

            graph.createFixedEdge(n3, end2, 10);

            graph.createFixedEdge(n4, end1, 0);
            graph.createFixedEdge(n4, end2, 30);

            graph.createFixedEdge(n5, end2, 10);
            graph.createFixedEdge(n5, end3, 20);


            graph::Selection selection{graph};
            OnlineTopologicalSorter sorter{graph};
            auto order_state = sorter.calculateInitialTopologicalOrder(selection);
            BackwardsPathCalculator calc{graph, start, ends};

            SECTION("Single entry") {
                auto state = calc.calculateInitialPaths<1>(selection, order_state);

                REQUIRE(state.distances_to_end[start][0].dist == 60);
                REQUIRE(state.distances_to_end[start][0].dest == end2);
            }

            SECTION("Two entries") {
                auto state = calc.calculateInitialPaths<2>(selection, order_state);

                REQUIRE(state.distances_to_end[start][0].dist == 60);
                REQUIRE(state.distances_to_end[start][0].dest == end2);

                REQUIRE(state.distances_to_end[start][1].dist == 40);
                REQUIRE(state.distances_to_end[start][1].dest == end3);
            }

            SECTION("Three entries") {
                auto state = calc.calculateInitialPaths<3>(selection, order_state);

                REQUIRE(state.distances_to_end[start][0].dist == 60);
                REQUIRE(state.distances_to_end[start][0].dest == end2);

                REQUIRE(state.distances_to_end[start][1].dist == 40);
                REQUIRE(state.distances_to_end[start][1].dest == end3);

                REQUIRE(state.distances_to_end[start][2].dist == 30);
                REQUIRE(state.distances_to_end[start][2].dest == end1);
            }
        }
    }

    TEST_CASE("Backwards Path Calculator update paths") {


        graph::AlternativeGraph graph{};

        auto start = graph.createNode();
        auto end1 = graph.createNode();
        auto end2 = graph.createNode();
        auto end3 = graph.createNode();
        std::vector<graph::node_index_t> ends = {end1, end2, end3};

        SECTION("Simple Path") {

            //   /-10-- n1 -------10--- e1
            //  /       \.10.\
            //  s            n3 --30--- e2
            //  \       /.10./
            //   \-10-- n2 -------20--- e3


            auto n1 = graph.createNode();
            auto n2 = graph.createNode();
            auto n3 = graph.createNode();

            graph.createFixedEdge(start, n1, 10);
            graph.createFixedEdge(start, n2, 10);

            graph.createFixedEdge(n1, end1, 10);
            graph.createFixedEdge(n2, end3, 20);

            graph.createFixedEdge(n3, end2, 30);

            auto choice = graph.createChoice(graph::ChoiceType::STOP_PASS, graph::AlternativeIndex::FIRST);
            graph.createAlternativeEdge(n1, n3, {choice, graph::AlternativeIndex::FIRST}, 10);
            graph.createAlternativeEdge(n2, n3, {choice, graph::AlternativeIndex::FIRST}, 10);

            graph::Selection selection{graph};
            OnlineTopologicalSorter sorter{graph};
            auto order_state = sorter.calculateInitialTopologicalOrder(selection);
            BackwardsPathCalculator calc{graph, start, ends};


            DYNAMIC_SECTION("Single Entry") {

                auto state = calc.calculateInitialPaths<1>(selection, order_state);

                REQUIRE(state.distances_to_end[start][0].dist == 30);
                REQUIRE(state.distances_to_end[start][0].dest == end3);

                auto result = selection.makeDecision({choice, graph::AlternativeIndex::FIRST});
                sorter.updateTopologicalOrder(order_state, selection, result.new_edges);
                calc.updateBasedOnNewEdges<1>(state, selection, order_state, result.new_edges);

                REQUIRE(state.distances_to_end[start][0].dist == 50);
                REQUIRE(state.distances_to_end[start][0].dest == end2);
            }

            DYNAMIC_SECTION("Multiple Entries") {

                auto state = calc.calculateInitialPaths<3>(selection, order_state);

                REQUIRE(state.distances_to_end[start][0] == DistDestPair{30, end3});
                REQUIRE(state.distances_to_end[start][1] == DistDestPair{20, end1});
                REQUIRE(state.distances_to_end[start][2] == DistDestPair{});

                auto result = selection.makeDecision({choice, graph::AlternativeIndex::FIRST});
                sorter.updateTopologicalOrder(order_state, selection, result.new_edges);
                calc.updateBasedOnNewEdges<3>(state, selection, order_state, result.new_edges);

                REQUIRE(state.distances_to_end[start][0] == DistDestPair{50, end2});
                REQUIRE(state.distances_to_end[start][1] == DistDestPair{30, end3});
                REQUIRE(state.distances_to_end[start][2] == DistDestPair{20, end1});
            }
        }
    }
}// namespace fb
