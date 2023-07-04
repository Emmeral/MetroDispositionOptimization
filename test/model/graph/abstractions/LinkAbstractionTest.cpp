#include "TrainAtStationAbstractionTestHelper.h"
#include "catch2/catch_test_macros.hpp"

#include "model/graph/abstractions/LinkAbstractions/MultiTrackLinkAbstraction.h"
#include "output/dot/graph/GraphDotWriter.h"
#include "output/dot/graph/NameHintHelper.h"

namespace fb::graph {


    TEST_CASE("Default Link Abstraction Test") {

        AlternativeGraph graph{};
        seconds_t change_end_time = 80;


        ScheduleItem in_si{};
        in_si.original_track = 0;
        in_si.original_activity = Activity::STOP;
        in_si.arrival = 20;
        in_si.node = 0;

        Course in_course;

        in_course.direction = Direction::WB;
        in_course.start_node = 0;
        in_course.schedule = {in_si};
        in_course.index = 0;


        ScheduleItem out_si{};

        out_si.original_track = 0;
        out_si.original_activity = Activity::STOP;
        out_si.departure = 200;
        out_si.node = 0;

        Course out_course;
        out_course.direction = Direction::EB;
        out_course.start_node = 0;
        out_course.schedule = {out_si};
        out_course.index = 1;

        fb::Node node;
        node.index = 0;

        SECTION("Single Track") {
            node.tracks = {Direction::WB};


            graph::GraphWithMeta graph_with_meta{1, 2, 1};
            graph_with_meta.graph = graph;
            auto &abstraction = graph_with_meta.abstractions.emplaceAbstraction<FullyDecidedLinkAbstraction>(
                    node, in_course, out_course, change_end_time);
            abstraction.materialize(graph);

            // currently only a manuel test by checking the output
            std::vector<std::string> name_hints{graph.nodes.size()};
            addNameHints(name_hints, abstraction);
            GraphDotWriter::writeToDot(graph_with_meta, name_hints, "DefaultLinkAbstraction_SingleTrack.dot", false);

            checkStopConsidered(graph, abstraction, change_end_time);
            checkLineHeadwayInConsidered(graph, abstraction, change_end_time);
            checkTrackHeadwayInConsidered(graph, abstraction, change_end_time);
        }

        SECTION("Multi Track") {
            node.tracks = {Direction::BOTH, Direction::BOTH};


            graph::GraphWithMeta graph_with_meta{1, 1, 1};
            graph_with_meta.graph = graph;
            auto &abstraction = graph_with_meta.abstractions.emplaceAbstraction<MultiTrackLinkAbstraction>(
                    node, in_course, out_course, change_end_time);
            abstraction.materialize(graph);
            // currently only a manuel test by checking the output
            std::vector<std::string> name_hints{graph.nodes.size()};
            addNameHints(name_hints, abstraction);
            GraphDotWriter::writeToDot(graph_with_meta, name_hints, "DefaultLinkAbstraction_MultiTrack.dot", false);


            checkStopConsidered(graph, abstraction, change_end_time);
            checkLineHeadwayInConsidered(graph, abstraction, change_end_time);
            checkTrackHeadwayInConsidered(graph, abstraction, change_end_time);
        }
    }
}// namespace fb::graph
