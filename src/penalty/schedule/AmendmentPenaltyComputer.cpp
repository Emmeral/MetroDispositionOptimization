
#include "AmendmentPenaltyComputer.h"
#include "BSVCollection.h"

namespace fb {
    AmendmentPenaltyComputer::AmendmentPenaltyComputer(const Model &model, const ProblemInstance &problem)
        : model_(model), problem_(problem) {}
    DetailedPenalty AmendmentPenaltyComputer::computeAmendmentPenalty(const TimetableAmendment &amendment,
                                                                      bool include_realized) const {
        auto delay = computeDelayPenalty(amendment, include_realized);
        auto skipped_stop = computeSkippedStopPenalty(amendment, include_realized);
        auto frequency = computeFrequencyPenalty(amendment, include_realized);
        return {delay, skipped_stop, frequency};
    }


    penalty_t AmendmentPenaltyComputer::computeDelayPenalty(const TimetableAmendment &amendment,
                                                            bool include_realized) const {

        penalty_t delay_penalty = 0;

        for (auto const &c: amendment.courses) {
            if (c.category != CourseCategory::PASSENGER) {
                continue;
            }

            auto const &amended_si = c.schedule.back();
            auto const &original_si = model_.courses[c.index].schedule[amended_si.index];

            // if si is already realized, skip it
            if (!include_realized && problem_.realized_schedule[c.index].size() > amended_si.index) {
                continue;
            }


            auto dd = (long) amended_si.arrival - (long) original_si.arrival;

            if (dd > DELAY_THRESHOLD) {
                delay_penalty += dd * DELAY_COST_PER_MINUTE / 60;
            }
        }
        return delay_penalty;
    }
    penalty_t AmendmentPenaltyComputer::computeSkippedStopPenalty(const TimetableAmendment &amendment,
                                                                  bool include_realized) const {

        penalty_t skipped_stop_penalty = 0;

        for (auto const &c: amendment.courses) {
            if (c.category != CourseCategory::PASSENGER) {
                continue;
            }
            Course const &original_course = model_.courses[c.index];

            BSVCollection bsvs{};

            for (auto const &si: c.schedule) {// Pending: Calculation does not work for partial cancellation currently
                auto original_si = original_course.schedule[si.index];

                if (original_si.original_activity == Activity::PASS) {
                    continue;
                }
                if (si.original_activity == Activity::STOP) {
                    continue;
                }

                // now have original=stop and amended=pass

                if (!include_realized && problem_.realized_schedule[c.index].size() > si.index) {
                    continue;
                }


                auto bsv = model_.base_station_values[original_si.node].of(original_si.arrival, c.direction);
                bsvs.add(bsv);
            }
            skipped_stop_penalty += bsvs.sum();
        }


        return skipped_stop_penalty;
    }
    penalty_t AmendmentPenaltyComputer::computeFrequencyPenalty(const TimetableAmendment &amendment,
                                                                bool include_realized) const {

        penalty_t frequency_penalty = 0;

        std::array<std::pair<Direction, node_index_t>, 2> measures = {
                std::make_pair(Direction::WB, model_.wb_frequency_measure),
                std::make_pair(Direction::EB, model_.eb_frequency_measure)};

        for (auto const &[d, n]: measures) {
            std::vector<seconds_t> arrivals{};

            for (auto const &c: amendment.courses) {
                if (c.direction != d) {
                    continue;
                }
                for (auto const &si: c.schedule) {
                    if (si.node == n && isValid(si.arrival)) {
                        arrivals.push_back(si.arrival);
                    }
                }
            }

            std::sort(arrivals.begin(), arrivals.end());

            for (auto i = 0; i < arrivals.size() - 1; i++) {
                long first = arrivals[i];
                long second = arrivals[i + 1];
                if (!include_realized && second < problem_.snapshot_time) {
                    continue;
                }

                auto target = std::max(model_.target_frequencies.of(first), model_.target_frequencies.of(second));
                auto actual = second - first;

                if (actual <= target) {
                    continue;
                }
                frequency_penalty += (actual - target) * FREQUENCY_MISS_COST_PER_MINUTE / 60;
            }
        }
        return frequency_penalty;
    }
}// namespace fb
