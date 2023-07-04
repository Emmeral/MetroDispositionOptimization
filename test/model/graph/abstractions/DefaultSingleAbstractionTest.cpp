#include "TrainAtStationAbstractionTestHelper.h"
#include "catch2/catch_test_macros.hpp"
#include "model/graph/abstractions/SingleAbstractions/MultiTrackSingleAbstraction.h"

#include "output/dot/graph/GraphDotWriter.h"
#include "output/dot/graph/NameHintHelper.h"

namespace fb::graph {


    TEST_CASE("Default Single Abstraction Test") {

        AlternativeGraph graph{};


        ScheduleItem si{};

        seconds_t stop_time = 80;
        si.original_track = 0;
        si.original_activity = Activity::STOP;
        si.arrival = 20;
        si.departure = si.arrival + stop_time;
        si.node = 0;

        Course course;

        course.direction = Direction::WB;
        course.start_node = 0;
        course.index = 0;
        course.schedule = {si};

        fb::Node node;
        node.index = 0;

        SECTION("Single Track") {
            node.tracks = {Direction::WB};

            graph::GraphWithMeta graph_with_meta{1, 1, 1};
            graph_with_meta.graph = graph;
            auto &abstraction = graph_with_meta.abstractions.emplaceAbstraction<TrackDecidedSingleAbstraction>(
                    node, course, si, TaggedWeight::fixed(si.departure - si.arrival));

            abstraction.materialize(graph);

            checkStopConsidered(graph, abstraction, stop_time);
            checkLineHeadwayInConsidered(graph, abstraction, stop_time);
            checkTrackHeadwayInConsidered(graph, abstraction, stop_time);


            // currently only a manuel test by checking the output
            std::vector<std::string> name_hints{graph.nodes.size()};
            addNameHints(name_hints, abstraction);
            GraphDotWriter::writeToDot(graph_with_meta, name_hints, "DefaultSingleAbstraction_SingleTrack.dot", false);
        }

        SECTION("Multi Track") {
            node.tracks = {Direction::WB, Direction::WB};

            graph::GraphWithMeta graph_with_meta{1, 1, 1};
            graph_with_meta.graph = graph;
            auto &abstraction =
                    graph_with_meta.abstractions.emplaceAbstraction<MultiTrackSingleAbstraction>(node, course, 0);

            abstraction.materialize(graph);

            checkStopConsidered(graph, abstraction, stop_time);
            checkLineHeadwayInConsidered(graph, abstraction, stop_time);
            checkTrackHeadwayInConsidered(graph, abstraction, stop_time);

            // currently only a manuel test by checking the output
            std::vector<std::string> name_hints{graph.nodes.size()};
            addNameHints(name_hints, abstraction);
            GraphDotWriter::writeToDot(graph_with_meta, name_hints, "DefaultSingleAbstraction_MultiTrack.dot", false);
        }
    }
}// namespace fb::graph
