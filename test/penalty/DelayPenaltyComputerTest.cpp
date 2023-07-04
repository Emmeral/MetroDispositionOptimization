#include "catch2/catch_test_macros.hpp"
#include "penalty/PenaltyComputer.h"

namespace fb {


    TEST_CASE("DelayPenaltyComputer") {


        SECTION("Computes Delay Penalty") {

            std::vector<distance_t> distances{120, 240, 300};

            graph::MeasureNodes measure_nodes{};
            measure_nodes.addMeasureNode(0, 0);
            measure_nodes.addMeasureNode(1, 1);
            DelayPenaltyComputer computer{measure_nodes};

            auto penalty = computer.computeDelayPenalty(distances);
            // 0 -> below threshold
            REQUIRE(penalty == 4 * DELAY_COST_PER_MINUTE);
        }

        SECTION("Computes Expected penalty change") {

            graph::AlternativeGraph graph{};

            for (auto i = 0; i < 6; ++i) {
                graph.createNode();
            }
            auto choice = graph.createChoice(graph::ChoiceType::STOP_PASS, graph::AlternativeIndex::FIRST);
            auto edge1 = graph.createAlternativeEdge(0, 2, graph::FullAlternativeIndex::stopAlternative(choice), 120);
            auto edge2 = graph.createAlternativeEdge(1, 3, graph::FullAlternativeIndex::stopAlternative(choice), 120);

            std::vector<distance_t> distances{60, 60, 120, 120, 180, 240};
            std::vector<std::array<DistDestPair, 1>> distances_to_end(graph.nodes.size());
            distances_to_end[2] = {{60, 4}};
            distances_to_end[3] = {{120, 5}};


            graph::MeasureNodes measure_nodes{};
            measure_nodes.addMeasureNode(4, 0);
            measure_nodes.addMeasureNode(5, 1);
            DelayPenaltyComputer computer{measure_nodes};
            // TODO: add test with bias
            auto change = computer.computeDelayPenaltyChange(graph, {edge1, edge2}, distances, distances_to_end);

            auto expected_change = DelayPenaltyComputer::getPenaltyForDelay(60 + 120 + 60) -
                                   DelayPenaltyComputer::getPenaltyForDelay(180) +
                                   DelayPenaltyComputer::getPenaltyForDelay(60 + 120 + 120) -
                                   DelayPenaltyComputer::getPenaltyForDelay(240);
            REQUIRE(change == expected_change);
        }

        SECTION("Computes Expected penalty change with bias") {

            graph::AlternativeGraph graph{};

            for (auto i = 0; i < 6; ++i) {
                graph.createNode();
            }
            auto choice = graph.createChoice(graph::ChoiceType::STOP_PASS, graph::AlternativeIndex::FIRST);
            auto edge1 = graph.createAlternativeEdge(0, 2, graph::FullAlternativeIndex::stopAlternative(choice), 120);
            auto edge2 = graph.createAlternativeEdge(1, 3, graph::FullAlternativeIndex::stopAlternative(choice), 120);

            std::vector<distance_t> distances{60, 60, 120, 120, 180, 240};
            std::vector<std::array<DistDestPair, 1>> distances_to_end(graph.nodes.size());

            auto bias4 = 200;
            auto bias5 = 100;
            distances_to_end[2] = {{60 + bias4, 4}};
            distances_to_end[3] = {{120 + bias5, 5}};


            graph::MeasureNodes measure_nodes{};
            measure_nodes.addMeasureNode(4, 0);
            measure_nodes.addMeasureNode(5, 1);
            DelayPenaltyComputer computer{measure_nodes};
            // TODO: add test with bias
            auto change = computer.computeDelayPenaltyChange(graph, {edge1, edge2}, distances, distances_to_end,
                                                             {bias4, bias5});

            auto expected_change = DelayPenaltyComputer::getPenaltyForDelay(60 + 120 + 60) -
                                   DelayPenaltyComputer::getPenaltyForDelay(180) +
                                   DelayPenaltyComputer::getPenaltyForDelay(60 + 120 + 120) -
                                   DelayPenaltyComputer::getPenaltyForDelay(240);
            REQUIRE(change == expected_change);
        }
    }

}// namespace fb
