
#include "TrainAtStationAbstractionTestHelper.h"
#include "catch2/catch_test_macros.hpp"
#include "output/dot/graph/GraphDotWriter.h"
#include "output/dot/graph/NameHintHelper.h"

namespace fb::graph {


    TEST_CASE("Predefined Activity Single Abstraction") {

        AlternativeGraph graph{};

        ScheduleItem si{};
        si.original_track = 0;
        si.original_activity = Activity::STOP;
        si.node = 0;
        // only have a defined arrival
        si.arrival = 80;
        // expect train to not have stop time there
        seconds_t stop_time = 0;

        Course course;

        course.index = 0;
        course.direction = Direction::WB;
        course.start_node = 0;
        course.schedule = {si};

        fb::Node node;
        node.index = 0;

        SECTION("Single Track") {
            node.tracks = {Direction::WB};


            graph::GraphWithMeta graph_with_meta{1, 1, 1};
            graph_with_meta.graph = graph;
            auto &abstraction = graph_with_meta.abstractions.emplaceAbstraction<FullyDecidedSingleAbstraction>(
                    node, course, si, TaggedWeight::fixed(0));

            abstraction.materialize(graph);

            std::vector<std::string> name_hints{graph.nodes.size()};
            addNameHints(name_hints, abstraction);


            // currently only a manuel test by checking the output
            GraphDotWriter::writeToDot(graph_with_meta, name_hints, "EndAbstraction_SingleTrack.dot", false);

            checkStopConsidered(graph, abstraction, stop_time);
            checkLineHeadwayInConsidered(graph, abstraction, stop_time);
            checkTrackHeadwayInConsidered(graph, abstraction, stop_time);
        }

        SECTION("Multi Track") {
            node.tracks = {Direction::WB, Direction::WB};

            graph::GraphWithMeta graph_with_meta{1, 1, 1};
            graph_with_meta.graph = graph;

            auto &abstraction =
                    graph_with_meta.abstractions.emplaceAbstraction<PredefinedActivityMultiTrackAbstraction>(
                            node, course, si, TaggedWeight::fixed(0));
            abstraction.materialize(graph);

            // currently only a manuel test by checking the output
            std::vector<std::string> name_hints{graph.nodes.size()};
            addNameHints(name_hints, abstraction);
            GraphDotWriter::writeToDot(graph_with_meta, name_hints, "EndAbstraction_MultiTrack.dot", false);

            checkStopConsidered(graph, abstraction, stop_time);
            checkLineHeadwayInConsidered(graph, abstraction, stop_time);
            checkTrackHeadwayInConsidered(graph, abstraction, stop_time);
        }
    }


}// namespace fb::graph
