#include "builder/graph/headways/DutyConsistencyStaticImplicationBuilder.h"
#include "catch2/catch_test_macros.hpp"
#include "model/graph/abstractions/Abstractions.h"
#include "model/main/Model.h"

namespace fb {
    TEST_CASE("DutyConsistencyStaticImplBuilderTest") {
        SECTION("Basic implications") {


            Model model{};

            Node n0{};
            n0.index = 0;
            n0.tracks = {Direction::EB, Direction::WB};

            Node n1{};
            n1.index = 1;
            n1.tracks = {Direction::EB, Direction::WB};

            Link l0{};
            l0.id = 0;
            l0.link_start_node = n0.index;
            l0.link_end_node = n1.index;

            n0.outgoing_links.push_back(l0.id);

            model.nodes.push_back(n0);
            model.nodes.push_back(n1);

            model.links.push_back(l0);

            RSDuty d0{};
            d0.index = 0;

            RSDuty d1{};
            d1.index = 1;

            model.duties.push_back(d0);

            Course c0{};
            c0.index = 0;
            c0.index_in_duty = 2;
            c0.duty = d0.index;

            Course c1{};
            c1.index = 1;
            c1.index_in_duty = 3;
            c1.duty = d0.index;

            Course c2{};
            c2.index = 2;
            c2.index_in_duty = 2;
            c2.duty = d1.index;

            Course c3{};
            c3.index = 3;
            c3.index_in_duty = 3;
            c3.duty = d1.index;


            ScheduleItem s0{};
            s0.index = 0;
            s0.node = n0.index;

            ScheduleItem s1{};
            s1.index = 0;
            s1.node = n0.index;

            ScheduleItem s2{};
            s2.index = 0;
            s2.node = n0.index;

            ScheduleItem s3{};
            s3.index = 0;
            s3.node = n0.index;

            c0.schedule.push_back(s0);
            c1.schedule.push_back(s1);
            c2.schedule.push_back(s2);
            c3.schedule.push_back(s3);

            model.courses = {c0, c1, c2, c3};
            model.duties = {d0, d1};

            graph::AlternativeGraph graph{};
            graph::Abstractions abstractions{model.nodes.size(), model.courses.size(), model.links.size()};

            auto const &tas0 = abstractions.emplaceAbstraction<graph::FullyDecidedSingleAbstraction>(n0, c0, s0);
            auto const &tas1 = abstractions.emplaceAbstraction<graph::FullyDecidedSingleAbstraction>(n0, c1, s1);
            auto const &tas2 = abstractions.emplaceAbstraction<graph::FullyDecidedSingleAbstraction>(n0, c2, s2);
            auto const &tas3 = abstractions.emplaceAbstraction<graph::FullyDecidedSingleAbstraction>(n0, c3, s3);

            auto choice_0_2 = graph.createChoice(graph::ChoiceType::TRACK_HEADWAY, graph::AlternativeIndex::FIRST);
            graph.choice_metadata[choice_0_2].data.th.first = tas0.index;
            graph.choice_metadata[choice_0_2].data.th.second = tas2.index;

            abstractions.registerTrackHeadway(tas0, {choice_0_2, graph::AlternativeIndex::FIRST});
            abstractions.registerTrackHeadway(tas2, {choice_0_2, graph::AlternativeIndex::SECOND});

            auto choice_1_2 = graph.createChoice(graph::ChoiceType::TRACK_HEADWAY, graph::AlternativeIndex::FIRST);
            graph.choice_metadata[choice_1_2].data.th.first = tas1.index;
            graph.choice_metadata[choice_1_2].data.th.second = tas3.index;

            abstractions.registerTrackHeadway(tas1, {choice_1_2, graph::AlternativeIndex::FIRST});
            abstractions.registerTrackHeadway(tas2, {choice_1_2, graph::AlternativeIndex::SECOND});

            auto choice_0_3 = graph.createChoice(graph::ChoiceType::TRACK_HEADWAY, graph::AlternativeIndex::FIRST);
            graph.choice_metadata[choice_0_3].data.th.first = tas0.index;
            graph.choice_metadata[choice_0_3].data.th.second = tas3.index;

            abstractions.registerTrackHeadway(tas0, {choice_0_3, graph::AlternativeIndex::FIRST});
            abstractions.registerTrackHeadway(tas3, {choice_0_3, graph::AlternativeIndex::SECOND});

            auto choice_1_3 = graph.createChoice(graph::ChoiceType::TRACK_HEADWAY, graph::AlternativeIndex::FIRST);
            graph.choice_metadata[choice_1_3].data.th.first = tas1.index;
            graph.choice_metadata[choice_1_3].data.th.second = tas2.index;

            abstractions.registerTrackHeadway(tas1, {choice_1_3, graph::AlternativeIndex::FIRST});
            abstractions.registerTrackHeadway(tas3, {choice_1_3, graph::AlternativeIndex::SECOND});

            DutyConsistencyStaticImplicationBuilder builder{model, graph, abstractions};

            builder.addDutyConsistencyStaticImplication(tas0.index);

            // implication before prior -> before latter
            auto impl = graph.choices[choice_0_2].alternative<graph::AlternativeIndex::FIRST>().static_implications;
            REQUIRE(impl.size() == 1);
            REQUIRE(impl[0].choice_index == choice_0_3);
            REQUIRE(impl[0].alternative_index == graph::AlternativeIndex::FIRST);


            // implication after latter -> after prior
            impl = graph.choices[choice_0_3].alternative<graph::AlternativeIndex::SECOND>().static_implications;
            REQUIRE(impl.size() == 1);
            REQUIRE(impl[0].choice_index == choice_0_2);
            REQUIRE(impl[0].alternative_index == graph::AlternativeIndex::SECOND);
        }
    }


}// namespace fb
