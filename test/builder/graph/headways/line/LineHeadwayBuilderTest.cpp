#include "builder/graph/headways/line/LineHeadwayBuilder.h"
#include "../../../../test_util.h"
#include "algo/path/LongestPathCalculator.h"
#include "builder/graph/AbstractionLinker.h"
#include "builder/graph/headways/line/MinHeadways.h"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "model/graph/abstractions/SingleAbstractions/MultiTrackSingleAbstraction.h"
#include "output/dot/graph/GraphDotWriter.h"
#include "output/dot/graph/NameHintHelper.h"
#include "util.h"

namespace fb {


    TEST_CASE("Default Line Headways") {

        graph::AlternativeGraph graph{};
        AbstractionLinker linker{graph};


        Node node_1{};
        node_1.index = 1;
        node_1.tracks = Tracks{Direction::WB, Direction::EB, Direction::BOTH};

        Node node_2{};
        node_2.index = 2;
        node_2.tracks = Tracks{Direction::WB, Direction::EB, Direction::BOTH};

        Link link{};
        link.link_start_node = node_1.index;
        link.link_end_node = node_2.index;
        link.id = 1;

        Course c1;
        c1.direction = Direction::WB;
        c1.planned_start = 100;
        c1.planned_end = 1000;
        c1.category = CourseCategory::PASSENGER;
        c1.schedule = std::vector<ScheduleItem>{4};
        c1.index = 0;

        ScheduleItem c1_first{};
        c1_first.arrival = 400;
        c1_first.departure = 400;
        c1_first.original_activity = Activity::STOP;
        c1_first.original_track = 0;
        c1_first.node = node_1.index;
        c1.schedule[1] = c1_first;

        ScheduleItem c1_next{};
        c1.schedule[2] = c1_next;


        Course c2;
        c2.direction = Direction::WB;
        c2.planned_start = 100;
        c2.planned_end = 1000;
        c2.category = CourseCategory::PASSENGER;
        c2.schedule = std::vector<ScheduleItem>{4};
        c2.index = 1;

        ScheduleItem c2_first{};
        c2_first.arrival = 400;
        c2_first.departure = 400;
        c2_first.original_activity = Activity::STOP;
        c2_first.original_track = 0;
        c2_first.node = node_1.index;
        c2.schedule[1] = c2_first;

        ScheduleItem c2_next{};
        c2.schedule[2] = c2_next;

        graph::Abstractions abstractions{3, 3, 2};

        auto &si0 = abstractions.emplaceAbstraction<graph::MultiTrackSingleAbstraction>(node_1, c1, 1);
        auto &si1 = abstractions.emplaceAbstraction<graph::MultiTrackSingleAbstraction>(node_2, c1, 2);

        si0.materialize(graph);
        si1.materialize(graph);

        auto &si2 = abstractions.emplaceAbstraction<graph::MultiTrackSingleAbstraction>(node_1, c2, 1);
        auto &si3 = abstractions.emplaceAbstraction<graph::MultiTrackSingleAbstraction>(node_2, c2, 2);


        si2.materialize(graph);
        si3.materialize(graph);


        abstractions.registerSuccessor(si0, si1, link);
        abstractions.registerSuccessor(si2, si3, link);
        std::vector<index_t> next_abstractions = {1, invalid<index_t>(), 3, invalid<index_t>()};


        auto start_node = graph.createNode();
        graph.createFixedEdge(start_node, si0.getArrivalTimeInNode(), si0.getOriginalArrival());
        graph.createFixedEdge(start_node, si2.getArrivalTimeInNode(), si2.getOriginalArrival());


        LongestPathCalculator calc{graph, start_node, {}};
        auto initial_distances = calc.calculateInitialPaths(graph::Selection(graph)).distance_from_start;

        SECTION("At Departure") {


            auto same_headway_modolo = GENERATE(16, 8, 4, 2, 1);
            link.headways = Headways{};
            for (int i = 0; i < link.headways.headways.size(); i++) {
                link.headways.headways[i] = (i % same_headway_modolo) + 10;
            }

            std::string hw_string = "[";
            for (auto i: link.headways.headways) {
                hw_string += std::to_string(i);
                hw_string += " ";
            }
            hw_string += "]";

            DYNAMIC_SECTION("with Headways" << hw_string) {
                link.minimumRuntimes = MinimumRuntimes{20, 20, 20, 20};


                MinHeadways min_headways{link.headways.headways};
                LineHeadwayBuilder lhb{graph, abstractions, initial_distances};

                auto hw_res = lhb.build(si0, si2, link);
                REQUIRE(hw_res.valid);
                graph::choice_index_t choice = hw_res.choice_index;
                REQUIRE(isValid(choice));

                auto start = graph.createNode();
                auto end1 = graph.createNode();
                auto end2 = graph.createNode();

                graph.createFixedEdge(start, si0.getArrivalTimeInNode(), si0.getOriginalArrival());
                graph.createFixedEdge(start, si2.getArrivalTimeInNode(), si2.getOriginalArrival());

                graph.createFixedEdge(si0.getDepartureTimeOutNode(), end1, 0);

                graph.createFixedEdge(si2.getDepartureTimeOutNode(), end2, 0);

                std::vector ends = {end1, end2};

                graph::Selection sel{graph};
                LongestPathCalculator calc{graph, start, ends};
                OnlineTopologicalSorter sorter{graph};

                auto sorter_state = sorter.calculateInitialTopologicalOrder(sel);
                auto state = calc.calculateInitialPaths(sel, sorter_state);

                // base conditions without any headway
                REQUIRE(state.distance_from_start[end1] == c1_first.arrival);
                REQUIRE(state.distance_from_start[end2] == c2_first.arrival);

                // c1 before c2
                auto accumulated_result = sel.makeDecision({choice, graph::AlternativeIndex::FIRST});

                sorter.updateTopologicalOrder(sorter_state, sel, accumulated_result.new_edges);
                calc.updateBasedOnNewEdges(state, sel, sorter_state, accumulated_result.new_edges);

                REQUIRE(state.distance_from_start[end1] == c1_first.arrival);
                REQUIRE(state.distance_from_start[end2] == c1_first.arrival + min_headways.min());

                // all possible stop pass choices

                std::optional<Activity> act0 = GENERATE(std::optional<Activity>{}, std::make_optional(Activity::STOP),
                                                        std::make_optional(Activity::PASS));
                auto act1 = GENERATE(std::optional<Activity>{}, std::make_optional(Activity::STOP),
                                     std::make_optional(Activity::PASS));
                auto act2 = GENERATE(std::optional<Activity>{}, std::make_optional(Activity::STOP),
                                     std::make_optional(Activity::PASS));
                auto act3 = GENERATE(std::optional<Activity>{}, std::make_optional(Activity::STOP),
                                     std::make_optional(Activity::PASS));

                std::vector<std::pair<graph::AbstractTrainAtStationAbstraction *, std::optional<Activity>>> pairs =
                        {{&si0, act0}, {&si1, act1}, {&si2, act2}, {&si3, act3}};


                DYNAMIC_SECTION("with activities " << act0 << " " << act1 << " " << act2 << " " << act3) {
                    accumulated_result = {};
                    for (auto const [si, act]: pairs) {
                        if (act) {
                            auto decision_result = sel.makeDecision(
                                    {si->getStopPassChoice().value(), graph::activityToAlternative(act.value())});
                            accumulated_result.append(decision_result);
                        }
                    }

                    sorter.updateTopologicalOrder(sorter_state, sel, accumulated_result.new_edges);
                    calc.updateBasedOnNewEdges(state, sel, sorter_state, accumulated_result.new_edges);
                    REQUIRE(!sorter_state.cycle);
                    CHECK(state.distance_from_start[end2] ==
                          c1_first.arrival + min_headways.min(act0, act1, act2, act3));

                    // no impact on first train
                    CHECK(state.distance_from_start[end1] == c1_first.arrival);
                }
            }
        }

        SECTION("At Arrival") {
            Link link{};
            link.headways = Headways{};


            MinHeadways min_headways{link.headways.headways};
            LineHeadwayBuilder lhb{graph, abstractions, initial_distances};

            auto hw_res = lhb.build(si0, si2, link);

            REQUIRE(hw_res.valid);
            graph::choice_index_t choice = hw_res.choice_index;
            REQUIRE(isValid(choice));

            auto start = graph.createNode();
            auto end1 = graph.createNode();
            auto end2 = graph.createNode();

            graph.createFixedEdge(start, si0.getArrivalTimeInNode(), 3000);// some constraint blocking c1

            graph.createFixedEdge(start, si0.getArrivalTimeInNode(), si0.getOriginalArrival());
            graph.createFixedEdge(start, si2.getArrivalTimeInNode(), si2.getOriginalArrival());

            graph.createFixedEdge(si0.getDepartureTimeOutNode(), end1, 0);

            graph.createFixedEdge(si2.getDepartureTimeOutNode(), end2, 0);

            std::vector ends = {end1, end2};

            graph::Selection sel{graph};
            LongestPathCalculator calc{graph, start, ends};
            OnlineTopologicalSorter sorter{graph};

            auto sorter_state = sorter.calculateInitialTopologicalOrder(sel);
            auto state = calc.calculateInitialPaths(sel, sorter_state);

            // base conditions without any headway
            REQUIRE(state.distance_from_start[end1] == 3000);
            REQUIRE(state.distance_from_start[end2] == c2_first.arrival);

            // c1 before c2
            auto accumulated_result = sel.makeDecision({choice, graph::AlternativeIndex::FIRST});

            sorter.updateTopologicalOrder(sorter_state, sel, accumulated_result.new_edges);
            calc.updateBasedOnNewEdges(state, sel, sorter_state, accumulated_result.new_edges);

            REQUIRE(state.distance_from_start[end1] == 3000);
            REQUIRE(state.distance_from_start[end2] == 3000);// blocked by delayed arrival of c1
        }
    }


}// namespace fb
