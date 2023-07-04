
#include "catch2/catch_test_macros.hpp"
#include "statistics/CharacteristicStatistic.h"
#include <catch2/matchers/catch_matchers_floating_point.hpp>


namespace fb {


    TEST_CASE("CharacteristicsStatistics") {

        CharacteristicStatistic stats{};


        SECTION("Mean") {

            stats.newValue(10);
            stats.newValue(20);
            stats.newValue(30);
            stats.newValue(40);

            REQUIRE(stats.mean() == 25);
        }

        SECTION("Variance 0") {
            stats.newValue(10);
            stats.newValue(10);
            stats.newValue(10);
            stats.newValue(10);

            REQUIRE(stats.sampleVariance() == 0);
        }

        SECTION("Variance != 0") {
            stats.newValue(10);
            stats.newValue(20);
            stats.newValue(20);
            stats.newValue(10);

            // variance =
            REQUIRE_THAT(stats.sampleVariance(), Catch::Matchers::WithinAbs(100.0 / 3, 0.001));
        }
    }


}// namespace fb
