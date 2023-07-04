
#include "builder/graph/PrematureActionPreventer.h"
#include "algo/path/LongestPathCalculator.h"
#include "catch2/catch_test_macros.hpp"
#include "model/graph/abstractions/SingleAbstractions/MultiTrackSingleAbstraction.h"
#include "model/graph/abstractions/SingleAbstractions/predefinedactivity/RealizedSingleAbstraction.h"


namespace fb {


    TEST_CASE("PrematureActionPreventer") {


        graph::AlternativeGraph graph{};
        graph::Abstractions abstractions{1, 1, 1};

        auto start = graph.createNode();


        auto checkArrivalDeparture = [&](auto const &tas, auto arrival, auto depature) {
            auto end = graph.createNode();
            graph.createFixedEdge(start, tas.getArrivalTimeInNode());
            graph.createFixedEdge(tas.getDepartureTimeOutNode(), end);

            LongestPathCalculator calc{graph, start, {end}};

            auto state = calc.calculateInitialPaths(graph::Selection{graph});


            auto const &dist = state.distance_from_start;

            REQUIRE(dist[tas.getArrivalTimeOutNode()] == arrival);
            REQUIRE(dist[tas.getDepartureTimeOutNode()] == depature);
        };


        Node n{};
        n.index = 0;
        n.tracks = {Direction::EB, Direction::WB, Direction::BOTH};

        ScheduleItem psi{};
        psi.index = 0;
        psi.node = n.index;
        psi.arrival = 1100;
        psi.departure = 1600;

        Course c{};
        c.schedule = {psi};
        c.start_node = n.index;

        ProblemInstance problem{};

        SECTION("fixes realized abstraction") {


            ScheduleItem rsi{};
            rsi.index = 0;
            rsi.node = n.index;
            rsi.arrival = 1000;
            rsi.departure = 1500;

            SECTION("before snapshot time") {

                auto &tas = abstractions.emplaceAbstraction<graph::RealizedSingleAbstraction>(n, c, rsi);
                tas.materialize(graph);

                problem.snapshot_time = 3000;
                PrematureActionPreventer pap{graph, abstractions, start, problem, 5 * 60, true};

                pap.preventEarlyActions(tas);

                checkArrivalDeparture(tas, rsi.arrival, rsi.departure);
            }

            SECTION("to snapshot time") {
                rsi.departure = invalid<seconds_t>();
                problem.snapshot_time = 1400;


                auto &tas = abstractions.emplaceAbstraction<graph::RealizedSingleAbstraction>(n, c, rsi);
                tas.materialize(graph);
                PrematureActionPreventer pap{graph, abstractions, start, problem, 5 * 60, true};

                pap.preventEarlyActions(tas);

                checkArrivalDeparture(tas, rsi.arrival, problem.snapshot_time);
            }

            SECTION("after snapshot time with no early departure allowed") {
                rsi.departure = invalid<seconds_t>();
                problem.snapshot_time = 1200;

                auto &tas = abstractions.emplaceAbstraction<graph::RealizedSingleAbstraction>(n, c, rsi);
                tas.materialize(graph);

                PrematureActionPreventer pap{graph, abstractions, start, problem, 0, true};
                pap.preventEarlyActions(tas);

                checkArrivalDeparture(tas, rsi.arrival, psi.departure);
            }
        }

        SECTION("fixes unrealized abstractions") {

            SECTION("if early departure limited") {


                auto &tas = abstractions.emplaceAbstraction<graph::MultiTrackSingleAbstraction>(n, c, psi.index);
                PrematureActionPreventer pap{graph, abstractions, start, problem, 60, true};

                tas.materialize(graph);
                pap.preventEarlyActions(tas);

                checkArrivalDeparture(tas, 0, psi.departure - 60);
            }

            SECTION("if early departure unlimited") {
                auto &tas = abstractions.emplaceAbstraction<graph::MultiTrackSingleAbstraction>(n, c, psi.index);
                tas.materialize(graph);

                PrematureActionPreventer pap{graph, abstractions, start, problem, invalid<seconds_t>(), true};
                pap.preventEarlyActions(tas);

                checkArrivalDeparture(tas, 0, 0);
            }

            SECTION("if early departure limited but early passes allowed") {

                auto &tas = abstractions.emplaceAbstraction<graph::MultiTrackSingleAbstraction>(n, c, psi.index);
                tas.materialize(graph);

                PrematureActionPreventer pap{graph, abstractions, start, problem, 60, false};
                pap.preventEarlyActions(tas);

                checkArrivalDeparture(tas, 0, 0);
            }
        }
    }
}// namespace fb
