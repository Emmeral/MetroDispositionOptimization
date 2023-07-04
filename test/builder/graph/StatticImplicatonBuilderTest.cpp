#include "builder/graph/headways/StaticImplicationBuilder.h"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "model/graph/abstractions/SingleAbstractions/predefinedactivity/PredefinedActivityMultiTrackAbstraction.h"
#include <algorithm>

namespace fb {


    TEST_CASE("Static Implication Builder Test") {

        SECTION("Headways in same direction") {

            Direction dir = GENERATE(Direction::EB, Direction::WB);

            bool single_track = GENERATE(true, false);

            DYNAMIC_SECTION("In Direction " << dir << " with single_track=" << single_track) {

                Model model{};

                Node n0{};
                n0.tracks = {Direction::EB, Direction::WB, Direction::BOTH};
                n0.index = 0;

                Node n1{};
                n1.tracks = single_track ? Tracks{dir, opposite(dir)} : Tracks{dir, dir, opposite(dir)};
                n1.index = 1;

                Node n2{};
                n2.tracks = {Direction::EB, Direction::WB};
                n2.index = 2;


                Link l0{};
                l0.link_start_node = n0.index;
                l0.link_end_node = n1.index;
                l0.id = 0;

                Link l1{};
                l1.link_start_node = n1.index;
                l1.link_end_node = n2.index;
                l1.id = 1;

                model.links = {l0, l1};

                n0.outgoing_links = {l0.id};
                n1.outgoing_links = {l1.id};

                model.nodes = {n0, n1, n2};

                std::vector<Node> nodes = {n0, n1};

                ScheduleItem s0{};
                s0.index = 0;
                s0.node = n0.index;
                s0.arrival = 10;
                s0.departure = 10;

                ScheduleItem s1{};
                s0.index = 1;
                s0.node = n1.index;
                s0.arrival = 20;
                s0.departure = 20;

                ScheduleItem s2{};
                s0.index = 2;
                s0.node = n2.index;
                s0.arrival = 30;
                s0.departure = 30;


                Course c0{};
                c0.index = 0;
                c0.direction = dir;
                c0.schedule = {s0, s1, s2};

                Course c1{};
                c1.index = 1;
                c1.direction = dir;
                c1.schedule = {s0, s1, s2};

                model.courses = {c0, c1};

                graph::AlternativeGraph graph{};
                auto tc = graph.createChoice(graph::ChoiceType::TRACK_HEADWAY, graph::AlternativeIndex::FIRST);
                auto lc0 = graph.createChoice(graph::ChoiceType::LINE_HEADWAY, graph::AlternativeIndex::FIRST);
                auto lc1 = graph.createChoice(graph::ChoiceType::LINE_HEADWAY, graph::AlternativeIndex::FIRST);


                std::vector<DirectionMappedHeadways> line_headways{2};
                std::vector<DirectionMappedHeadways> station_headways{2};


                graph::Abstractions abstractions{3, 2, 2};

                auto &abs0 =
                        abstractions.emplaceAbstraction<graph::PredefinedActivityMultiTrackAbstraction>(n0, c0, s0);
                auto &abs1 =
                        abstractions.emplaceAbstraction<graph::PredefinedActivityMultiTrackAbstraction>(n1, c0, s1);
                auto &abs2 =
                        abstractions.emplaceAbstraction<graph::PredefinedActivityMultiTrackAbstraction>(n0, c1, s0);
                auto &abs3 =
                        abstractions.emplaceAbstraction<graph::PredefinedActivityMultiTrackAbstraction>(n1, c1, s1);

                // dummy last abs
                auto &abs4 =
                        abstractions.emplaceAbstraction<graph::PredefinedActivityMultiTrackAbstraction>(n2, c0, s2);
                auto &abs5 =
                        abstractions.emplaceAbstraction<graph::PredefinedActivityMultiTrackAbstraction>(n2, c1, s2);


                abstractions.registerSuccessor(abs0, abs1, l0);
                abstractions.registerSuccessor(abs2, abs3, l0);

                abstractions.registerSuccessor(abs1, abs4, l1);
                abstractions.registerSuccessor(abs3, abs5, l1);

                HeadwayCollection headway_collection{model.nodes.size(), model.links.size()};

                headway_collection.line_headways[l0.id][{abs0.getIndex(), abs2.index}] = lc0;
                headway_collection.line_headways[l1.id][{abs1.index, abs3.getIndex()}] = lc1;

                headway_collection.station_headways[n1.index].ofDir(dir)[{abs1.getIndex(), abs3.getIndex()}] = tc;


                StaticImplicationBuilder sib{model, graph, abstractions, headway_collection};

                sib.addStaticImplicationsAtNodeInDirection(n1.index, dir);

                for (auto ai: {graph::AlternativeIndex::FIRST, graph::AlternativeIndex::SECOND}) {
                    std::vector<graph::FullAlternativeIndex> &implications =
                            graph.choices[tc].alternative(ai).static_implications;


                    auto implied_choices = single_track ? std::vector<graph::choice_index_t>{lc0, lc1}
                                                        : std::vector<graph::choice_index_t>{lc0};

                    for (auto c: implied_choices) {
                        graph::FullAlternativeIndex other = {c, ai};
                        std::vector<graph::FullAlternativeIndex> &other_implications =
                                graph.choices[c].alternative(ai).static_implications;
                        REQUIRE(std::ranges::find(implications, other) != implications.end());

                        graph::FullAlternativeIndex thiis = {tc, ai};
                        REQUIRE(std::ranges::find(other_implications, thiis) != other_implications.end());
                        REQUIRE(other_implications.size() == 1);// only one implication added
                    }

                    REQUIRE(implications.size() == (1 + single_track));// no more than the two
                }
            }
        }

        SECTION("Headways in different directions") {

            Model model{};

            Node n0{};
            n0.tracks = {Direction::EB, Direction::WB, Direction::BOTH};
            n0.index = 0;

            // middle node
            Node n1{};
            n1.tracks = {Direction::EB, Direction::WB};
            n1.index = 1;

            Node n2{};
            n2.tracks = {Direction::EB, Direction::WB, Direction::BOTH};
            n2.index = 2;

            model.nodes = {n0, n1, n2};

            Link l0{};
            l0.link_start_node = n0.index;
            l0.link_end_node = n1.index;
            l0.id = 0;

            Link l1{};
            l1.link_start_node = n1.index;
            l1.link_end_node = n2.index;
            l1.id = 1;

            Link l2{};
            l2.link_start_node = n2.index;
            l2.link_end_node = n1.index;
            l2.id = 2;

            Link l3{};
            l3.link_start_node = n1.index;
            l3.link_end_node = n0.index;
            l3.id = 3;

            model.links = {l0, l1, l2, l3};

            std::vector<Node> nodes = {n0, n1, n2};

            ScheduleItem s00{};
            s00.index = 0;
            s00.node = n0.index;
            s00.arrival = 10;
            s00.departure = 10;

            ScheduleItem s01{};
            s01.index = 1;
            s01.node = n1.index;
            s01.arrival = 20;
            s01.departure = 20;

            ScheduleItem s02{};
            s01.index = 2;
            s01.node = n2.index;
            s01.arrival = 30;
            s01.departure = 30;


            Course c0{};
            c0.index = 0;
            c0.direction = Direction::EB;
            c0.schedule = {s00, s01, s02};


            ScheduleItem s10{};
            s10.index = 0;
            s10.node = n2.index;
            s10.arrival = 10;
            s10.departure = 10;

            ScheduleItem s11{};
            s10.index = 1;
            s10.node = n1.index;
            s10.arrival = 20;
            s10.departure = 20;

            ScheduleItem s12{};
            s10.index = 2;
            s10.node = n0.index;
            s10.arrival = 30;
            s10.departure = 30;


            Course c1{};
            c1.index = 1;
            c1.direction = Direction::WB;
            c1.schedule = {s10, s11, s12};

            model.courses = {c0, c1};

            graph::AlternativeGraph graph{};
            auto tc0 = graph.createChoice(graph::ChoiceType::TRACK_HEADWAY, graph::AlternativeIndex::FIRST);
            auto tc1 = graph.createChoice(graph::ChoiceType::TRACK_HEADWAY, graph::AlternativeIndex::SECOND);


            HeadwayCollection headway_collection{model.nodes.size(), model.links.size()};

            graph::Abstractions abstractions{3, 2, 4};
            auto &abs0 = abstractions.emplaceAbstraction<graph::PredefinedActivityMultiTrackAbstraction>(n0, c0, s00);
            auto &abs1 = abstractions.emplaceAbstraction<graph::PredefinedActivityMultiTrackAbstraction>(n1, c0, s01);
            auto &abs2 = abstractions.emplaceAbstraction<graph::PredefinedActivityMultiTrackAbstraction>(n2, c0, s02);


            auto &abs3 = abstractions.emplaceAbstraction<graph::PredefinedActivityMultiTrackAbstraction>(n2, c1, s10);
            auto &abs4 = abstractions.emplaceAbstraction<graph::PredefinedActivityMultiTrackAbstraction>(n1, c1, s11);
            auto &abs5 = abstractions.emplaceAbstraction<graph::PredefinedActivityMultiTrackAbstraction>(n0, c1, s12);

            abstractions.registerSuccessor(abs0, abs1, l0);
            abstractions.registerSuccessor(abs1, abs2, l1);

            abstractions.registerSuccessor(abs3, abs4, l2);
            abstractions.registerSuccessor(abs4, abs5, l3);


            headway_collection.station_headways[n0.index].mixed[{abs0.getIndex(), abs5.getIndex()}] = tc0;
            headway_collection.station_headways[n2.index].mixed[{abs2.getIndex(), abs3.getIndex()}] = tc1;

            StaticImplicationBuilder sib{model, graph, abstractions, headway_collection};

            sib.addMixedDirStaticImplications(n2.index);


            graph::FullAlternativeIndex other = {tc0, graph::AlternativeIndex::FIRST};
            std::vector<graph::FullAlternativeIndex> implications =
                    graph.choices[tc1].alternative(graph::AlternativeIndex::FIRST).static_implications;
            REQUIRE(std::ranges::find(implications, other) != implications.end());
            REQUIRE(implications.size() == 1);

            // no implication in other direction
            implications = graph.choices[tc1].alternative(graph::AlternativeIndex::SECOND).static_implications;
            REQUIRE(implications.empty());

            other = {tc1, graph::AlternativeIndex::SECOND};
            implications = graph.choices[tc0].alternative(graph::AlternativeIndex::SECOND).static_implications;
            REQUIRE(std::ranges::find(implications, other) != implications.end());
            REQUIRE(implications.size() == 1);// only one implication added

            implications = graph.choices[tc0].alternative(graph::AlternativeIndex::FIRST).static_implications;
            REQUIRE(implications.empty());
        }
    }


}// namespace fb
