

#include "algo/CycleFinder.h"
#include "catch2/catch_test_macros.hpp"
#include "model/graph/AlternativeGraph.h"


namespace fb {
    TEST_CASE("CycleFinder") {


        graph::AlternativeGraph graph{};

        auto n1 = graph.createNode();
        auto n2 = graph.createNode();
        auto n3 = graph.createNode();
        auto n4 = graph.createNode();
        auto n5 = graph.createNode();


        SECTION("Simple cycle fixed") {
            graph.createFixedEdge(n1, n2);
            graph.createFixedEdge(n2, n3);
            graph.createFixedEdge(n3, n4);
            graph.createFixedEdge(n4, n5);
            graph.createFixedEdge(n3, n1);

            CycleFinder finder{};
            auto cycle = finder.findCycle(graph, graph::Selection{graph});

            REQUIRE(!cycle.empty());
            REQUIRE(cycle.nodes.size() == 3);
            REQUIRE(cycle.nodes[0]->index == n1);
            REQUIRE(cycle.nodes[1]->index == n2);
            REQUIRE(cycle.nodes[2]->index == n3);

            REQUIRE(cycle.edges.size() == 3);
            REQUIRE(cycle.edges[0]->from == n1);
            REQUIRE(cycle.edges[0]->to == n2);
            REQUIRE(cycle.edges[1]->from == n2);
            REQUIRE(cycle.edges[1]->to == n3);
            REQUIRE(cycle.edges[2]->from == n3);
            REQUIRE(cycle.edges[2]->to == n1);
        }

        SECTION("Simple cycle alternative") {


            graph.createFixedEdge(n1, n2);
            graph.createFixedEdge(n2, n3);
            graph.createFixedEdge(n3, n4);
            graph.createFixedEdge(n4, n5);

            auto choice = graph.createChoice(graph::ChoiceType::STOP_PASS, graph::AlternativeIndex::FIRST);

            graph.createAlternativeEdge(n3, n1, {choice, graph::AlternativeIndex::FIRST});

            CycleFinder finder{};
            auto selection = graph::Selection{graph};

            auto cycle = finder.findCycle(graph, selection);
            REQUIRE(cycle.empty());

            selection.makeDecision({choice, graph::AlternativeIndex::FIRST});

            cycle = finder.findCycle(graph, selection);
            REQUIRE(!cycle.empty());
            REQUIRE(cycle.nodes.size() == 3);
            REQUIRE(cycle.nodes[0]->index == n1);
            REQUIRE(cycle.nodes[1]->index == n2);
            REQUIRE(cycle.nodes[2]->index == n3);

            REQUIRE(cycle.edges.size() == 3);
            REQUIRE(cycle.edges[0]->from == n1);
            REQUIRE(cycle.edges[0]->to == n2);
            REQUIRE(cycle.edges[1]->from == n2);
            REQUIRE(cycle.edges[1]->to == n3);
            REQUIRE(cycle.edges[2]->from == n3);
            REQUIRE(cycle.edges[2]->to == n1);
        }

        SECTION("Cycle not in beginning") {

            graph.createFixedEdge(n1, n2);

            graph.createFixedEdge(n3, n4);
            graph.createFixedEdge(n4, n5);
            graph.createFixedEdge(n5, n3);

            CycleFinder finder{};
            auto cycle = finder.findCycle(graph, graph::Selection{graph});

            REQUIRE(!cycle.empty());
            REQUIRE(cycle.nodes.size() == 3);
            REQUIRE(cycle.nodes[0]->index == n3);
            REQUIRE(cycle.nodes[1]->index == n4);
            REQUIRE(cycle.nodes[2]->index == n5);
        }
    }
}// namespace fb
