


#include "algo/OnlineTopologicalSorter.h"
#include "catch2/catch_test_macros.hpp"

namespace fb {


    static void checkOrder(graph::AlternativeGraph &graph, graph::Selection &selection,
                           OnlineTopologicalSorter::State const &state) {

        auto const &order = state.topological_order;
        auto const &reverse = state.sorted_nodes;
        for (auto &edge: graph.fixed_edges) {
            REQUIRE(order[edge.from] < order[edge.to]);
        }
        for (auto &edge: graph.alternative_edges) {
            if (selection.isSelectedEdge(edge.index)) {
                REQUIRE(order[edge.from] < order[edge.to]);
            }
        }

        // check that the order matches the reverse order
        for (auto &node: graph.nodes) {
            REQUIRE(isValid(order[node.index]));
            REQUIRE(node.index == reverse[order[node.index]]);
        }
    }

    static void checkSorter(graph::AlternativeGraph &graph, graph::FullAlternativeIndex &fai,
                            const std::vector<graph::a_edge_index_t> &created, bool shouldBeValid) {
        OnlineTopologicalSorter sorter{graph};
        graph::Selection selection{graph};

        auto state = sorter.calculateInitialTopologicalOrder(selection);
        REQUIRE(!state.cycle);
        checkOrder(graph, selection, state);


        selection.makeDecision(fai);

        sorter.updateTopologicalOrder(state, selection, created);
        REQUIRE(!state.cycle == shouldBeValid);
        if (shouldBeValid) {
            checkOrder(graph, selection, state);
        }
    }


    TEST_CASE("OnlineTopologicalSorter") {

        graph::AlternativeGraph graph;

        for (int i = 0; i < 10; ++i) {
            graph.createNode();
        }

        // two lines
        for (int i = 0; i < 5; ++i) {
            graph.createFixedEdge(i, i + 1);
        }
        for (int i = 6; i < 9; ++i) {
            graph.createFixedEdge(i, i + 1);
        }

        auto choice = graph.createChoice<graph::ChoiceType::LINE_HEADWAY>(graph::AlternativeIndex::FIRST);
        graph::FullAlternativeIndex fai = {choice, graph::AlternativeIndex::FIRST};
        std::vector<graph::a_edge_index_t> created;


        // calculates initial order correct


        SECTION("Processes single additional edge") {
            created.push_back(graph.createAlternativeEdge(7, 2, fai));
            checkSorter(graph, fai, created, true);
        }

        SECTION("Processes single additional edge (other way)") {
            created.push_back(graph.createAlternativeEdge(2, 7, fai));
            checkSorter(graph, fai, created, true);
        }

        SECTION("Process Multi edges in order") {
            for (auto i = 1; i < 5; ++i) {
                created.push_back(graph.createAlternativeEdge(i, 2 * i, fai));
            }
            checkSorter(graph, fai, created, true);
        }


        SECTION("Processes multiple edges") {

            created.push_back(graph.createAlternativeEdge(6, 1, fai));
            created.push_back(graph.createAlternativeEdge(3, 8, fai));
            created.push_back(graph.createAlternativeEdge(9, 4, fai));
            checkSorter(graph, fai, created, true);
        }

        SECTION("Aborts on cycle") {

            // another edge not relevant for cycle
            created.push_back(graph.createAlternativeEdge(1, 4, fai));
            // cycle
            created.push_back(graph.createAlternativeEdge(9, 2, fai));
            created.push_back(graph.createAlternativeEdge(5, 7, fai));
            checkSorter(graph, fai, created, false);
        }

        SECTION("Another Case") {

            graph.createNode();

            created.push_back(graph.createAlternativeEdge(2, 10, fai));
            created.push_back(graph.createAlternativeEdge(10, 8, fai));
            created.push_back(graph.createAlternativeEdge(10, 4, fai));

            checkSorter(graph, fai, created, true);
        }

        SECTION("Processes multiple calls to same method") {
            fb::OnlineTopologicalSorter sorter{graph};

            graph::FullAlternativeIndex other = {choice, graph::AlternativeIndex::SECOND};
            std::vector<graph::a_edge_index_t> other_created;
            created.push_back(graph.createAlternativeEdge(2, 7, fai));
            other_created.push_back(graph.createAlternativeEdge(4, 6, other));

            graph::Selection selection{graph};

            auto state = sorter.calculateInitialTopologicalOrder(selection);
            REQUIRE(!state.cycle);
            checkOrder(graph, selection, state);


            state = sorter.calculateInitialTopologicalOrder(selection);
            REQUIRE(!state.cycle);
            checkOrder(graph, selection, state);


            auto sel_copy = selection;
            auto state_copy = state;
            selection.makeDecision(fai);
            sel_copy.makeDecision(other);

            sorter.updateTopologicalOrder(state, selection, created);
            sorter.updateTopologicalOrder(state_copy, sel_copy, other_created);


            checkOrder(graph, selection, state);
            checkOrder(graph, sel_copy, state_copy);
        }
    }


}// namespace fb
