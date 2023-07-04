
#include "algo/SkippedStopsInfo.h"

namespace fb {


    SkippedStopInfo::SkippedStopInfo(std::vector<bsv_t> &bsvs) {

        if (bsvs.empty()) {
            return;
        }

        std::sort(bsvs.begin(), bsvs.end());
        potential_smallest = bsvs[0];

        if (bsvs.size() > 1) {
            potential_second_smallest = bsvs[1];
        }

        if (bsvs.size() > 2) {
            remaining_potential_bsvs.insert(remaining_potential_bsvs.begin(), bsvs.begin() + 2, bsvs.end());
        }


        updateBound();
    }
    bool SkippedStopInfo::registerDecisionForBsv(bsv_t bsv, graph::AlternativeIndex alternative) {

        bool update = true;

        if (bsv == potential_smallest) {
            potential_smallest = potential_second_smallest;
            potential_second_smallest = popFirstRemaining();
        } else if (bsv == potential_second_smallest) {
            potential_second_smallest = popFirstRemaining();
        } else {
            auto const &it = std::ranges::lower_bound(remaining_potential_bsvs, bsv);
            if (it != remaining_potential_bsvs.end()) {
                remaining_potential_bsvs.erase(it);
            } else {
                assert(false);// this should not happen
            }
            update = false;
        }

        if (alternative == graph::passAlternative()) {
            addSelectedBsv(bsv);
            update = true;
        }

        if (update) {
            updateBound();
        }
        return update;
    }
    penalty_t SkippedStopInfo::updateSum() {
        sum = smallest_selected * HIGHEST_SKIP_FACTOR + second_smallest_selected * SECOND_HIGHEST_SKIP_FACTOR +
              remaining_selected_bsv_sum * REMAINING_SKIP_FACTOR;
        return sum;
    }
    penalty_t SkippedStopInfo::updateBound() {
        bound = updateSum();

        if (isInvalid(potential_smallest) || smallest_selected == 0) {
            return bound;
        }

        penalty_t potentially_better = 0;

        if (smallest_selected > potential_smallest) {
            potentially_better = potential_smallest * HIGHEST_SKIP_FACTOR +
                                 smallest_selected * SECOND_HIGHEST_SKIP_FACTOR +
                                 second_smallest_selected * REMAINING_SKIP_FACTOR +
                                 remaining_selected_bsv_sum * REMAINING_SKIP_FACTOR;
            bound = std::min(bound, potentially_better);
            if (isInvalid(potential_second_smallest)) {
                return bound;
            }

            if (smallest_selected > potential_second_smallest) {
                potentially_better = potential_smallest * HIGHEST_SKIP_FACTOR +
                                     potential_second_smallest * SECOND_HIGHEST_SKIP_FACTOR +
                                     smallest_selected * REMAINING_SKIP_FACTOR +
                                     second_smallest_selected * REMAINING_SKIP_FACTOR +
                                     remaining_selected_bsv_sum * REMAINING_SKIP_FACTOR;
                bound = std::min(bound, potentially_better);
            }
        } else if (second_smallest_selected > potential_smallest) {
            potentially_better = smallest_selected * HIGHEST_SKIP_FACTOR +
                                 potential_smallest * SECOND_HIGHEST_SKIP_FACTOR +
                                 second_smallest_selected * REMAINING_SKIP_FACTOR +
                                 remaining_selected_bsv_sum * REMAINING_SKIP_FACTOR;
            bound = std::min(bound, potentially_better);
        }

        return bound;
    }
    bsv_t SkippedStopInfo::popFirstRemaining() {
        if (remaining_potential_bsvs.empty()) {
            return invalid<bsv_t>();
        } else {
            auto temp = *remaining_potential_bsvs.begin();
            remaining_potential_bsvs.erase(remaining_potential_bsvs.begin());
            return temp;
        }
    }
    void SkippedStopInfo::addSelectedBsv(bsv_t bsv) {
        if (bsv < smallest_selected || smallest_selected == 0) {
            remaining_selected_bsv_sum += second_smallest_selected;
            second_smallest_selected = smallest_selected;
            smallest_selected = bsv;
        } else if (bsv < second_smallest_selected || second_smallest_selected == 0) {
            remaining_selected_bsv_sum += second_smallest_selected;
            second_smallest_selected = bsv;
        } else {
            remaining_selected_bsv_sum += bsv;
        }
    }
    penalty_t SkippedStopInfo::boundAfterDecision(bsv_t bsv, graph::AlternativeIndex alternative) const {

        if (alternative == graph::stopAlternative()) {
            if (isValid(potential_second_smallest) && bsv > potential_second_smallest) {
                // nothing relevant changed
                return bound;
            }
        } else {
            if (isValid(potential_second_smallest) && bsv > potential_second_smallest &&
                second_smallest_selected != 0 && bsv > second_smallest_selected) {
                // nothing relevant changed
                return bound + bsv * REMAINING_SKIP_FACTOR;
            }
        }


        SkippedStopInfo copy(*this);
        copy.registerDecisionForBsv(bsv, alternative);
        return copy.bound;
    }

}// namespace fb
