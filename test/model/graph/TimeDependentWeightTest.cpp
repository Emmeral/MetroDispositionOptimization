#include "model/graph/TimeDependentWeight.h"
#include "catch2/catch_test_macros.hpp"


namespace fb::graph {


    TEST_CASE("TimeDependentWeightTest") {


        TimeDependentWeight w{5, 9, 20, 40};

        SECTION("Before") { REQUIRE(5 == w.getWeightAt(6)); }

        SECTION("In") {
            REQUIRE(9 == w.getWeightAt(30));
            REQUIRE(9 == w.getWeightAt(16));// intersection with intervall
        }

        SECTION("At End") {
            REQUIRE(6 == w.getWeightAt(39));
            REQUIRE(7 == w.getWeightAt(38));
            REQUIRE(9 == w.getWeightAt(36));
        }

        SECTION("After") {
            REQUIRE(5 == w.getWeightAt(50));
            REQUIRE(5 == w.getWeightAt(40));
        }
    }
}// namespace fb::graph
