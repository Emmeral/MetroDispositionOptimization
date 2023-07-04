#include "algo/SkippedStopsInfo.h"
#include "catch2/catch_test_macros.hpp"


namespace fb {


    penalty_t sumFor(int first, int second, int remaining_sum) {
        return first * SkippedStopInfo::HIGHEST_SKIP_FACTOR + second * SkippedStopInfo::SECOND_HIGHEST_SKIP_FACTOR +
               remaining_sum * SkippedStopInfo::REMAINING_SKIP_FACTOR;
    }

    TEST_CASE("SkippedStopInfo") {


        std::vector<bsv_t> bsvs = {10, 40, 20, 30, 50, 60, 7000};

        SkippedStopInfo info{bsvs};


        SECTION("Does update bound correclty") {
            // no choices -> no penalty
            REQUIRE(info.bound == 0);

            // we skip a stop with value 60
            info.registerDecisionForBsv(60, graph::passAlternative());

            // use the 60 for the sum, but the bound is smaller
            REQUIRE(info.sum == sumFor(60, 0, 0));
            REQUIRE(info.bound == sumFor(10, 20, 60));


            info.registerDecisionForBsv(20, graph::passAlternative());

            REQUIRE(info.sum == sumFor(20, 60, 0));
            REQUIRE(info.bound == sumFor(10, 20, 60));


            info.registerDecisionForBsv(10, graph::stopAlternative());

            REQUIRE(info.bound == sumFor(20, 30, 60));

            info.registerDecisionForBsv(30, graph::stopAlternative());
            info.registerDecisionForBsv(40, graph::passAlternative());
            REQUIRE(info.bound == sumFor(20, 40, 60));

            info.registerDecisionForBsv(50, graph::passAlternative());

            REQUIRE(info.bound == sumFor(20, 40, 110));
        }

        SECTION("Projects next bound correctly") {

            auto bound = info.boundAfterDecision(60, graph::passAlternative());
            REQUIRE(bound == sumFor(10, 20, 60));

            info.registerDecisionForBsv(60, graph::passAlternative());
            bound = info.boundAfterDecision(10, graph::stopAlternative());
            REQUIRE(bound == sumFor(20, 30, 60));


            info.registerDecisionForBsv(10, graph::stopAlternative());
            info.registerDecisionForBsv(20, graph::passAlternative());
            info.registerDecisionForBsv(30, graph::passAlternative());

            bound = info.boundAfterDecision(40, graph::passAlternative());
            REQUIRE(bound == sumFor(20, 30, 60 + 40));
        }
    }
}// namespace fb
