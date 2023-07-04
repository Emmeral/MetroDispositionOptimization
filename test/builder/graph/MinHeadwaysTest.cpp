
#include "builder/graph/headways/line/MinHeadways.h"
#include "catch2/catch_test_macros.hpp"
#include "model/main/Model.h"

namespace fb {


    TEST_CASE("Calculated Minimum correctly") {


        Headways headways{};
        headways.of(Activity::STOP, Activity::STOP, Activity::STOP, Activity::STOP) = 11;
        headways.of(Activity::STOP, Activity::STOP, Activity::STOP, Activity::PASS) = 12;
        headways.of(Activity::STOP, Activity::STOP, Activity::PASS, Activity::STOP) = 13;
        headways.of(Activity::STOP, Activity::STOP, Activity::PASS, Activity::PASS) = 14;

        headways.of(Activity::STOP, Activity::PASS, Activity::STOP, Activity::STOP) = 15;
        headways.of(Activity::STOP, Activity::PASS, Activity::STOP, Activity::PASS) = 16;
        headways.of(Activity::STOP, Activity::PASS, Activity::PASS, Activity::STOP) = 17;
        headways.of(Activity::STOP, Activity::PASS, Activity::PASS, Activity::PASS) = 18;

        headways.of(Activity::PASS, Activity::STOP, Activity::STOP, Activity::STOP) = 1;
        headways.of(Activity::PASS, Activity::STOP, Activity::STOP, Activity::PASS) = 2;
        headways.of(Activity::PASS, Activity::STOP, Activity::PASS, Activity::STOP) = 3;
        headways.of(Activity::PASS, Activity::STOP, Activity::PASS, Activity::PASS) = 4;

        headways.of(Activity::PASS, Activity::PASS, Activity::STOP, Activity::STOP) = 5;
        headways.of(Activity::PASS, Activity::PASS, Activity::STOP, Activity::PASS) = 6;
        headways.of(Activity::PASS, Activity::PASS, Activity::PASS, Activity::STOP) = 7;
        headways.of(Activity::PASS, Activity::PASS, Activity::PASS, Activity::PASS) = 8;

        MinHeadways min_headways{headways.headways};

        REQUIRE(min_headways.min() == 1);

        REQUIRE(min_headways.min({}, {}, {}, {}) == 1);
        REQUIRE(min_headways.min(Activity::STOP, {}, {}, {}) == 11);
        REQUIRE(min_headways.min({}, Activity::PASS, {}, {}) == 5);
        REQUIRE(min_headways.min({}, {}, Activity::PASS, {}) == 3);

        REQUIRE(min_headways.min({}, {}, Activity::PASS, Activity::PASS) == 4);

        REQUIRE(min_headways.min(Activity::STOP, {}, Activity::PASS, Activity::PASS) == 14);

        REQUIRE(min_headways.min(Activity::STOP, Activity::PASS, Activity::PASS, Activity::PASS) == 18);
    }
}// namespace fb
