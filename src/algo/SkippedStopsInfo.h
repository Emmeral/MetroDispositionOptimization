#pragma once
#include "algo/bab/StateUpdater.h"
#include "algo/bab/states/State.h"
#include "model/graph/GraphWithMeta.h"
#include "model/graph/abstractions/TrainAtStationAbstraction.h"
#include "model/main/Model.h"
#include "penalty/definitions.h"
#include <set>
#include <vector>

namespace fb {
    class SkippedStopInfo {
    public:
        auto static constexpr HIGHEST_SKIP_FACTOR = BIGGEST_SFF;
        auto static constexpr SECOND_HIGHEST_SKIP_FACTOR = SECOND_BIGGEST_SFF;
        auto static constexpr REMAINING_SKIP_FACTOR = DEFAULT_SFF;

        explicit SkippedStopInfo(std::vector<fb::bsv_t> &bsvs);

        SkippedStopInfo(const SkippedStopInfo &) = default;
        SkippedStopInfo(SkippedStopInfo &&) = default;

        SkippedStopInfo &operator=(const SkippedStopInfo &) = default;
        SkippedStopInfo &operator=(SkippedStopInfo &&) = default;

        /**
     *
     * @param bsv the Base Station Value to consider
     * @param selected whether the bsv shall be considered for the updateSum
     * @return whether the updateSum or bound are updated by this change
     */
        bool registerDecisionForBsv(fb::bsv_t bsv, fb::graph::AlternativeIndex alternative);

        fb::penalty_t boundAfterDecision(fb::bsv_t bsv, fb::graph::AlternativeIndex alternative) const;

        fb::penalty_t bound{0};
        fb::penalty_t sum{0};

    private:
        fb::penalty_t updateSum();
        fb::penalty_t updateBound();

        fb::bsv_t popFirstRemaining();

        void addSelectedBsv(fb::bsv_t bsv);

        // smallest bsvs of the whole course that are not yet selected
        fb::bsv_t potential_smallest{fb::invalid<fb::bsv_t>()};
        fb::bsv_t potential_second_smallest{fb::invalid<fb::bsv_t>()};
        std::vector<fb::bsv_t> remaining_potential_bsvs{};


        // smallest bsvs of the nodes where pass is selected
        fb::bsv_t smallest_selected{0};
        fb::bsv_t second_smallest_selected{0};
        fb::penalty_t remaining_selected_bsv_sum{0};
    };
}// namespace fb
