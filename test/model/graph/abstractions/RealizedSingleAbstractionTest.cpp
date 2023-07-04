#include "TrainAtStationAbstractionTestHelper.h"
#include "catch2/catch_test_macros.hpp"
#include "output/dot/graph/GraphDotWriter.h"
#include "output/dot/graph/NameHintHelper.h"

namespace fb::graph {


    TEST_CASE("Realized Single Abstraction Test") {

        AlternativeGraph graph{};


        ScheduleItem original_si{};
        original_si.original_track = 0;
        original_si.original_activity = Activity::STOP;
        original_si.node = 0;
        original_si.arrival = 30;
        original_si.departure = 80;
        original_si.index = 0;


        Course course;

        course.index = 0;
        course.direction = Direction::WB;
        course.start_node = 0;
        course.schedule = {original_si};


        ScheduleItem realized_si{};
        realized_si.index = 0;

        fb::Node node;
        node.index = 0;
        node.tracks = {Direction::WB, Direction::WB};

        SECTION("Realized Slower than original") {
            realized_si.original_activity = Activity::STOP;
            realized_si.arrival = original_si.arrival;
            realized_si.departure = original_si.departure + 10;

            graph::GraphWithMeta graph_with_meta{1, 1, 1};
            graph_with_meta.graph = graph;
            auto &abstraction = graph_with_meta.abstractions.emplaceAbstraction<RealizedSingleAbstraction>(node, course,
                                                                                                           realized_si);
            abstraction.materialize(graph);

            checkStopConsidered(graph, abstraction, realized_si.departure - realized_si.arrival);

            // currently only a manuel test by checking the output
            std::vector<std::string> name_hints{graph.nodes.size()};
            addNameHints(name_hints, abstraction);
            GraphDotWriter::writeToDot(graph_with_meta, name_hints, "FullyDecidedSingleAbstraction_RealizedSlower.dot",
                                       false);
        }

        SECTION("Realized Faster than original") {
            realized_si.original_activity = Activity::STOP;
            realized_si.arrival = original_si.arrival;
            realized_si.departure = original_si.departure - 10;

            graph::GraphWithMeta graph_with_meta{1, 1, 1};
            graph_with_meta.graph = graph;
            auto &abstraction = graph_with_meta.abstractions.emplaceAbstraction<RealizedSingleAbstraction>(node, course,
                                                                                                           realized_si);
            abstraction.materialize(graph);

            checkStopConsidered(graph, abstraction, realized_si.departure - realized_si.arrival);


            // currently only a manuel test by checking the output
            std::vector<std::string> name_hints{graph.nodes.size()};
            addNameHints(name_hints, abstraction);
            GraphDotWriter::writeToDot(graph_with_meta, name_hints, "FullyDecidedSingleAbstraction_RealizedFaster.dot",
                                       false);
        }

        SECTION("Realized Slower than original") {
            realized_si.original_activity = Activity::STOP;
            realized_si.arrival = original_si.arrival;
            realized_si.departure = original_si.departure - 10;


            graph::GraphWithMeta graph_with_meta{1, 1, 1};
            graph_with_meta.graph = graph;
            auto &abstraction = graph_with_meta.abstractions.emplaceAbstraction<RealizedSingleAbstraction>(node, course,
                                                                                                           realized_si);
            abstraction.materialize(graph);

            checkStopConsidered(graph, abstraction, realized_si.departure - realized_si.arrival);

            // currently only a manuel test by checking the output
            std::vector<std::string> name_hints{graph.nodes.size()};
            addNameHints(name_hints, abstraction);
            GraphDotWriter::writeToDot(graph_with_meta, name_hints, "FullyDecidedSingleAbstraction_RealizedFaster.dot",
                                       false);
        }

        SECTION("Realized not departed") {
            realized_si.original_activity = Activity::STOP;
            realized_si.arrival = original_si.arrival + 5;
            realized_si.departure = invalid<seconds_t>();

            graph::GraphWithMeta graph_with_meta{1, 1, 1};
            graph_with_meta.graph = graph;
            auto &abstraction = graph_with_meta.abstractions.emplaceAbstraction<RealizedSingleAbstraction>(node, course,
                                                                                                           realized_si);
            abstraction.materialize(graph);

            checkStopConsidered(graph, abstraction, original_si.departure - original_si.arrival);

            // currently only a manuel test by checking the output
            std::vector<std::string> name_hints{graph.nodes.size()};
            addNameHints(name_hints, abstraction);
            GraphDotWriter::writeToDot(graph_with_meta, name_hints,
                                       "FullyDecidedSingleAbstraction_RealizedNotDeparted.dot", false);
        }

        SECTION("Original is start") {
            course.schedule[0].arrival = invalid<seconds_t>();

            realized_si.original_activity = Activity::STOP;
            realized_si.arrival = 90;
            realized_si.departure = invalid<seconds_t>();

            graph::GraphWithMeta graph_with_meta{1, 1, 1};
            graph_with_meta.graph = graph;
            auto &abstraction = graph_with_meta.abstractions.emplaceAbstraction<RealizedSingleAbstraction>(node, course,
                                                                                                           realized_si);

            abstraction.materialize(graph);
            checkStopConsidered(graph, abstraction, 0);

            std::vector<std::string> name_hints{graph.nodes.size()};
            addNameHints(name_hints, abstraction);
        }
    }


}// namespace fb::graph
