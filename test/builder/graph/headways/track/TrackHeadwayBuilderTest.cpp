#include "builder/graph/headways/track/TrackHeadwayBuilder.h"
#include "algo/path/LongestPathCalculator.h"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "model/graph/AlternativeGraph.h"
#include "model/graph/Node.h"
#include "model/graph/abstractions/SingleAbstractions/MultiTrackSingleAbstraction.h"
#include "model/main/Model.h"
#include "output/dot/graph/GraphDotWriter.h"
#include "util.h"


namespace fb {

    TEST_CASE("Default Track Headways") {

        graph::AlternativeGraph graph{};


        Node node_1{};
        node_1.index = 0;
        node_1.tracks = Tracks{Direction::WB, Direction::WB};

        Course c1;
        c1.index = 0;
        c1.direction = Direction::WB;
        c1.planned_start = 100;
        c1.planned_end = 1000;
        c1.category = CourseCategory::PASSENGER;
        c1.schedule = std::vector<ScheduleItem>{3};

        ScheduleItem s1{};
        s1.arrival = 400;
        s1.departure = 400;
        s1.original_activity = Activity::STOP;
        s1.original_track = 0;
        s1.node = node_1.index;
        c1.schedule[1] = s1;


        Course c2;
        c2.index = 1;
        c2.direction = Direction::WB;
        c2.planned_start = 100;
        c2.planned_end = 1000;
        c2.category = CourseCategory::PASSENGER;
        c2.schedule = std::vector<ScheduleItem>{3};

        ScheduleItem s2{};
        s2.arrival = 400;
        s2.departure = 400;
        s2.original_activity = Activity::STOP;
        s2.original_track = 0;
        s2.node = node_1.index;
        c2.schedule[1] = s2;


        auto abstractions = graph::Abstractions{1, 2, 1};

        auto &tas1 = abstractions.emplaceAbstraction<graph::MultiTrackSingleAbstraction>(node_1, c1, 1);
        auto &tas2 = abstractions.emplaceAbstraction<graph::MultiTrackSingleAbstraction>(node_1, c2, 1);

        tas1.materialize(graph);
        tas2.materialize(graph);

        auto start_node = graph.createNode();
        graph.createFixedEdge(start_node, tas1.getArrivalTimeInNode(), tas1.getOriginalArrival());
        graph.createFixedEdge(start_node, tas2.getArrivalTimeInNode(), tas2.getOriginalArrival());
        LongestPathCalculator calc_bef_head{graph, start_node, {}};
        auto initial_distances = calc_bef_head.calculateInitialPaths(graph::Selection(graph)).distance_from_start;

        GraphBuilderOptions opts{};
        opts.track_relation_max_difference = 10000;
        TrackHeadwayBuilder thb{graph, abstractions, initial_distances, opts};
        auto headway_choice = thb.build(tas1, tas2);

        REQUIRE(headway_choice.valid);
        REQUIRE(isValid(headway_choice.choice_index));

        for (auto tc: *tas1.getTrackChoices()) {
            auto const &relations = graph.choices[headway_choice.choice_index].relations;
            auto found = std::find(relations.begin(), relations.end(), tc);
            REQUIRE(found != relations.end());
        }
        for (auto tc: *tas2.getTrackChoices()) {
            auto const &relations = graph.choices[headway_choice.choice_index].relations;
            auto found = std::find(relations.begin(), relations.end(), tc);
            REQUIRE(found != relations.end());
        }


        // setup test nodes
        auto start = graph.createNode();
        auto end1 = graph.createNode();
        auto end2 = graph.createNode();

        graph.createFixedEdge(start, tas1.getArrivalTimeInNode(), tas1.getOriginalArrival());
        graph.createFixedEdge(start, tas2.getArrivalTimeInNode(), tas2.getOriginalArrival());


        graph.createFixedEdge(tas1.getDepartureTimeOutNode(), end1, 0);
        graph.createFixedEdge(tas2.getDepartureTimeOutNode(), end2, 0);

        std::vector ends = {end1, end2};

        graph::Selection sel{graph};
        OnlineTopologicalSorter sorter{graph};
        LongestPathCalculator calc{graph, start, ends};

        auto sorter_state = sorter.calculateInitialTopologicalOrder(sel);
        auto state = calc.calculateInitialPaths(sel, sorter_state);

        // no change before decisions are made
        REQUIRE(state.distance_from_start[end1] == tas1.getOriginalArrival());
        REQUIRE(state.distance_from_start[end2] == tas2.getOriginalArrival());


        auto t1_choice = tas1.getTrackChoices()->at(0);
        auto t2_choice = tas2.getTrackChoices()->at(0);

        auto accumulated_result = sel.makeDecision({headway_choice.choice_index, graph::AlternativeIndex::FIRST});
        sorter.updateTopologicalOrder(sorter_state, sel, accumulated_result.new_edges);
        calc.updateBasedOnNewEdges(state, sel, sorter_state, accumulated_result.new_edges);

        // no change as no track is selected
        REQUIRE(state.distance_from_start[end1] == tas1.getOriginalArrival());
        REQUIRE(state.distance_from_start[end2] == tas2.getOriginalArrival());


        graph::AlternativeIndex first = GENERATE(graph::AlternativeIndex::FIRST, graph::AlternativeIndex::SECOND);
        graph::AlternativeIndex second = GENERATE(graph::AlternativeIndex::FIRST, graph::AlternativeIndex::SECOND);

        accumulated_result = {};
        auto decision_result = sel.makeDecision({t1_choice, first});
        accumulated_result.append(decision_result);
        decision_result = sel.makeDecision({t2_choice, second});
        accumulated_result.append(decision_result);

        // the following lines can be removed once a track min path is implemented
        decision_result =
                sel.makeDecision(graph::FullAlternativeIndex::passAlternative(tas2.getStopPassChoice().value()));
        accumulated_result.append(decision_result);
        decision_result =
                sel.makeDecision(graph::FullAlternativeIndex::passAlternative(tas1.getStopPassChoice().value()));
        accumulated_result.append(decision_result);

        sorter.updateTopologicalOrder(sorter_state, sel, accumulated_result.new_edges);
        calc.updateBasedOnNewEdges(state, sel, sorter_state, accumulated_result.new_edges);

        if (first == second) {
            REQUIRE(state.distance_from_start[end2] == tas1.getOriginalArrival() + 30);
        } else {
            REQUIRE(state.distance_from_start[end2] == tas1.getOriginalArrival());
        }
        REQUIRE(state.distance_from_start[end1] == tas1.getOriginalArrival());

        std::vector<std::string> name_hints = getTopologicalNameHints(graph, state, sorter_state);
    }
}// namespace fb
