
#include "statistics/MaxChoiceStatistics.h"
#include "catch2/catch_test_macros.hpp"

namespace fb {


    TEST_CASE("MaxChoiceStatistics") {

        std::vector<graph::TaggedChoiceMetadata> meta{};
        meta.resize(20, graph::TaggedChoiceMetadata{graph::StopPassMetadata()});
        MaxChoicesStatistics<5> stats{meta};

        SECTION("Initializes correct") {
            for (auto c: stats.top_choices) {
                REQUIRE(c.count == 0);
            }
        }

        SECTION("Updates Correct") {

            stats.markConsidered(7, graph::ChoiceType::STOP_PASS);
            stats.markConsidered(7, graph::ChoiceType::STOP_PASS);
            stats.markConsidered(7, graph::ChoiceType::STOP_PASS);
            stats.markConsidered(8, graph::ChoiceType::STOP_PASS);

            REQUIRE(stats.top_choices[0].count == 3);
            REQUIRE(stats.top_choices[0].choice_index == 7);

            REQUIRE(stats.top_choices[1].count == 1);
            REQUIRE(stats.top_choices[1].choice_index == 8);

            stats.markConsidered(10, graph::ChoiceType::STOP_PASS);
            stats.markConsidered(11, graph::ChoiceType::STOP_PASS);
            stats.markConsidered(12, graph::ChoiceType::STOP_PASS);
            stats.markConsidered(13, graph::ChoiceType::STOP_PASS);

            stats.markConsidered(8, graph::ChoiceType::STOP_PASS);
            stats.markConsidered(8, graph::ChoiceType::STOP_PASS);
            stats.markConsidered(8, graph::ChoiceType::STOP_PASS);

            REQUIRE(stats.top_choices[0].count == 4);
            REQUIRE(stats.top_choices[0].choice_index == 8);

            REQUIRE(stats.top_choices[1].count == 3);
            REQUIRE(stats.top_choices[1].choice_index == 7);

            stats.markConsidered(10, graph::ChoiceType::STOP_PASS);

            REQUIRE(stats.top_choices[2].count == 2);
            REQUIRE(stats.top_choices[2].choice_index == 10);
        }
    }

}// namespace fb
