

#include "TrainAtStationAbstractionTestHelper.h"
#include "algo/path/LongestPathCalculator.h"
#include "builder/graph/headways/track/TrackHeadwayNodeBuilder.h"
#include "catch2/catch_test_macros.hpp"

namespace fb::graph {

    void checkStopConsidered(AlternativeGraph graph, TrainAtStationAbstraction &tas, seconds_t stop_time) {
        // intentional copy of graph such that changes are not persisted;

        Selection sel{graph};

        auto start = graph.createNode();
        auto end = graph.createNode();
        graph.createFixedEdge(start, tas.getArrivalTimeInNode());
        graph.createFixedEdge(tas.getDepartureTimeOutNode(), end);

        LongestPathCalculator calc{graph, start, {end}};
        OnlineTopologicalSorter sorter{graph};
        auto sorter_state = sorter.calculateInitialTopologicalOrder(sel);
        auto state = calc.calculateInitialPaths(sel, sorter_state);


        if (tas.predefinedActivity() && tas.predefinedActivity().value() == Activity::STOP) {
            REQUIRE(state.distance_from_start[end] == stop_time);
        } else {
            REQUIRE(state.distance_from_start[end] == 0);
        }

        if (tas.hasStopPassChoice()) {
            auto decision_result =
                    sel.makeDecision(FullAlternativeIndex::stopAlternative(tas.getStopPassChoice().value()));

            sorter.updateTopologicalOrder(sorter_state, sel, decision_result.new_edges);
            calc.updateBasedOnNewEdges(state, sel, sorter_state, decision_result.new_edges);

            REQUIRE(state.distance_from_start[end] == stop_time);
        }
    }


    void checkLineHeadwayInConsidered(AlternativeGraph graph, TrainAtStationAbstraction &tas, seconds_t stop_time) {


        auto start = graph.createNode();
        auto end = graph.createNode();
        graph.createFixedEdge(start, tas.getArrivalTimeInNode());
        graph.createFixedEdge(tas.getDepartureTimeOutNode(), end);


        auto choice = graph.createChoice<ChoiceType::LINE_HEADWAY>(AlternativeIndex::FIRST, {});

        graph.createAlternativeEdge(start, tas.getDepartureTimeInNode(), {choice, AlternativeIndex ::FIRST},
                                    stop_time + 100);


        Selection sel{graph};

        LongestPathCalculator calc{graph, start, {end}};
        OnlineTopologicalSorter sorter{graph};
        auto sorter_state = sorter.calculateInitialTopologicalOrder(sel);
        auto state = calc.calculateInitialPaths(sel, sorter_state);

        if (tas.predefinedActivity() && tas.predefinedActivity().value() == Activity::STOP) {
            REQUIRE(state.distance_from_start[end] == stop_time);
        } else {
            REQUIRE(state.distance_from_start[end] == 0);
        }

        auto decision_result = sel.makeDecision({choice, AlternativeIndex ::FIRST});
        sorter.updateTopologicalOrder(sorter_state, sel, decision_result.new_edges);
        calc.updateBasedOnNewEdges(state, sel, sorter_state, decision_result.new_edges);

        REQUIRE(state.distance_from_start[end] == stop_time + 100);
    }
    void checkTrackHeadwayInConsidered(AlternativeGraph graph, TrainAtStationAbstraction &tas, seconds_t stop_time) {


        auto start = graph.createNode();
        auto end = graph.createNode();
        graph.createFixedEdge(start, tas.getArrivalTimeInNode());
        graph.createFixedEdge(tas.getDepartureTimeOutNode(), end);


        auto choice = graph.createChoice(ChoiceType::TRACK_HEADWAY, AlternativeIndex::FIRST);

        auto getTimeFromTrack = [stop_time](track_id_t track) { return stop_time + 100 * (track + 1); };

        TrackHeadwayNodeBuilder track_headway_node_builder{graph};

        auto hw_in = track_headway_node_builder.createHeadwayNodes<TrackHeadwayNodeBuilder::Type::IN>(tas);
        track_id_t last_considered_track;

        for (auto index = 0; index < tas.getNode().tracks.all_tracks.size(); ++index) {

            if (tas.considersTrack(index)) {
                graph.createAlternativeEdge(start, hw_in[index], {choice, AlternativeIndex ::FIRST},
                                            getTimeFromTrack(index));
                last_considered_track = index;
            }
        }

        Selection sel{graph};

        LongestPathCalculator calc{graph, start, {end}};
        OnlineTopologicalSorter sorter{graph};
        auto sorter_state = sorter.calculateInitialTopologicalOrder(sel);
        auto state = calc.calculateInitialPaths(sel, sorter_state);

        seconds_t base = 0;
        if (tas.predefinedActivity() && tas.predefinedActivity().value() == Activity::STOP) {
            base = stop_time;
        }
        REQUIRE(state.distance_from_start[end] == base);

        auto decision_result = sel.makeDecision({choice, AlternativeIndex ::FIRST});
        sorter.updateTopologicalOrder(sorter_state, sel, decision_result.new_edges);
        calc.updateBasedOnNewEdges(state, sel, sorter_state, decision_result.new_edges);

        if (tas.hasTrackChoice()) {
            // nothing changed as no track is selected
            REQUIRE(state.distance_from_start[end] == base);
        } else {
            // we already have a change as there is no track decision
            REQUIRE(state.distance_from_start[end] == getTimeFromTrack(last_considered_track) + base);
            return;
        }


        auto track_choices = tas.getTrackChoices();
        for (auto index = 0; index < tas.getNode().tracks.all_tracks.size(); ++index) {

            if (!tas.considersTrack(index)) {
                continue;
            }

            Selection copied_sel = sel;// copy
            auto copied_state = state;
            auto copied_sorter_state = sorter_state;

            graph::Selection::DecisionResult accumulated_result{};

            for (auto ci = 0; ci < track_choices->size(); ++ci) {

                bool select = ((0x1 << ci) & index) > 0;

                decision_result =
                        copied_sel.makeDecision({track_choices->at(ci), static_cast<AlternativeIndex>(select)});
                accumulated_result.append(decision_result);
            }
            sorter.updateTopologicalOrder(copied_sorter_state, copied_sel, decision_result.new_edges);
            calc.updateBasedOnNewEdges(copied_state, copied_sel, copied_sorter_state, decision_result.new_edges);
            REQUIRE(copied_state.distance_from_start[end] == getTimeFromTrack(index) + base);
        }
    }

}// namespace fb::graph
