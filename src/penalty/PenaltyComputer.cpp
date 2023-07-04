
#include "PenaltyComputer.h"

namespace fb {


    PenaltyComputer::PenaltyComputer(const graph::GraphWithMeta &graph_with_meta, const Model &model,
                                     seconds_t snapshot_time)
        : graph_with_meta_(graph_with_meta), model_(model), snapshot_time_{snapshot_time},
          delay_computer(graph_with_meta.measure_nodes) {}


    penalty_t PenaltyComputer::computeTotalPenalty(const graph::Selection &sel,
                                                   const std::vector<distance_t> &distances) const {

        penalty_t delay_penalty = delay_computer.computeDelayPenalty(distances);
        penalty_t skipped_stop_penalty = computeSkippedStopPenalty(sel);
        penalty_t frequency_penalty = computeTargetFrequencyPenalty(sel, distances);

        return delay_penalty + skipped_stop_penalty + frequency_penalty;
    }


    penalty_t PenaltyComputer::computeSkippedStopPenalty(const graph::Selection &sel) const {

        auto const &abstractions = graph_with_meta_.abstractions;

        penalty_t total_penalty = 0;

        bsv_t biggest_bsv;
        bsv_t second_biggest_bsv;
        std::vector<bsv_t> remaining_bsvs{};

        for (auto const &c: model_.courses) {
            if (c.category == CourseCategory::EMPTY) {
                continue;
            }

            graph::TrainAtStationAbstraction const *start_index = abstractions.firstAbsOfCourse(c.index);
            penalty_t course_penalty = 0;

            biggest_bsv = 0;
            second_biggest_bsv = 0;
            remaining_bsvs.clear();// remove elements from last iteration but keep allocated memory


            for (graph::TrainAtStationAbstraction const *abs = start_index; abs != nullptr;
                 abs = abstractions.nextAbsOfCourse(*abs, c.index)) {

                const ScheduleItem *schedule_item = abs->getOriginalInScheduleItem();
                if (schedule_item->original_activity == Activity::PASS) {
                    continue;
                }

                std::optional<graph::choice_index_t> choice = abs->getStopPassChoice();

                // if we don't have a choice, we decided for stop/original activity
                if (!choice.has_value()) {
                    continue;
                }

                auto decision = sel.decisionFor(choice.value());
                if (!decision.made) {
                    // if we don't have a decision yet, we can't assume any penalty
                    continue;
                }
                if (decision.chosen == graph::stopAlternative()) {
                    // if we choose stop, we don't have a penalty
                    continue;
                }

                BaseStationValue bsvs = model_.base_station_values[abs->getNode().index];
                auto bsv = bsvs.of(schedule_item->arrival, c.direction);


                if (bsv > biggest_bsv) {
                    remaining_bsvs.push_back(second_biggest_bsv);
                    second_biggest_bsv = biggest_bsv;
                    biggest_bsv = bsv;
                } else if (bsv > second_biggest_bsv) {
                    remaining_bsvs.push_back(second_biggest_bsv);
                    second_biggest_bsv = bsv;
                } else {
                    remaining_bsvs.push_back(bsv);
                }
            }

            course_penalty += biggest_bsv * 35;
            course_penalty += second_biggest_bsv * 15;
            for (auto bsv: remaining_bsvs) {
                course_penalty += bsv;
            }
            total_penalty += course_penalty;
        }
        return total_penalty;
    }
    penalty_t PenaltyComputer::computeTargetFrequencyPenalty(const graph::Selection &sel,
                                                             const std::vector<distance_t> &distances) const {

        penalty_t penalty{0};
        penalty += computeTargetFrequencyPenaltyAtNode(model_.eb_frequency_measure, Direction::EB, distances);
        penalty += computeTargetFrequencyPenaltyAtNode(model_.wb_frequency_measure, Direction::WB, distances);

        return penalty;
    }
    penalty_t PenaltyComputer::computeTargetFrequencyPenaltyAtNode(node_index_t node, Direction dir,
                                                                   const std::vector<distance_t> &distances) const {
        // TODO: this is no lower bound
        penalty_t penalty{0};


        std::vector<graph::abs_index_t> const &abstractions = graph_with_meta_.abstractions.atNode(node).of(dir);

        std::vector<seconds_t> arrivals{};

        std::ranges::transform(abstractions, std::back_inserter(arrivals), [&](auto index) {
            return distances[graph_with_meta_.abstractions[index].getArrivalTimeOutNode()];
        });

        auto r = abstractions |
                 std::views::transform([&](auto index) { return &graph_with_meta_.abstractions[index]; }) |
                 std::views::filter(
                         [](auto const *abs) { return abs->getInCourse().category == CourseCategory::PASSENGER; }) |
                 std::views::transform([&](auto const *abs) { return distances[abs->getArrivalTimeOutNode()]; });
        std::ranges::copy(r, std::back_inserter(arrivals));


        std::ranges::sort(arrivals);
        auto it = std::ranges::upper_bound(arrivals, snapshot_time_);

        seconds_t last;
        if (it == arrivals.begin()) {
            it++;
        }
        last = *(it - 1);

        while (it != arrivals.end()) {
            seconds_t current = *it;

            auto diff = current - last;

            auto thresh = std::max(model_.target_frequencies.of(current), model_.target_frequencies.of(last));

            if (diff > thresh) {
                penalty += (FREQUENCY_MISS_COST_PER_MINUTE * (diff - thresh)) / 60;// 150L per minute
            }

            last = current;
            ++it;
        }

        return penalty;
    }
}// namespace fb
