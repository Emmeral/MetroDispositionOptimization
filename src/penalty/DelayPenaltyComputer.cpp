
#include "DelayPenaltyComputer.h"
#include "algo/path/BackwardsPathCalculator.h"
#include "model/main/Model.h"

namespace fb {


    DelayPenaltyComputer::DelayPenaltyComputer(const graph::MeasureNodes &measure_nodes)
        : measure_nodes_(measure_nodes) {}

    penalty_t DelayPenaltyComputer::getPenaltyForDelay(distance_t delay) {
        if (delay > DELAY_THRESHOLD) {
            return (delay * DELAY_COST_PER_MINUTE) / 60;// currently only rounding for whole pounds
        } else {
            return 0;
        }
    }
    penalty_t DelayPenaltyComputer::computeDelayPenalty(const std::vector<distance_t> &distances) const {
        penalty_t penalty{0};
        for (auto node: measure_nodes_.getNodes()) {
            penalty += getPenaltyForDelay(distances[node]);
        }
        return penalty;
    }


}// namespace fb
