
#include "TimetableCreator.h"
#include "ScheduleItemExtractor.h"
#include "algo/path/LongestPathCalculator.h"
#include "model/PartialSchedule.h"

namespace fb {
    TimetableCreator::TimetableCreator(const Model &model,
                                       const std::vector<std::vector<ScheduleItem>> &realized_schedule)
        : model_(model), realized_schedule_(realized_schedule) {}

    TimetableAmendment TimetableCreator::createAmendedTimetable(const graph::Selection &selection,
                                                                const graph::GraphWithMeta &gwm) {

        LongestPathCalculator calc{gwm.graph, gwm.start_node, gwm.measure_nodes.getNodes()};
        auto dfs = calc.calculateInitialPaths(selection).distance_from_start;
        return createAmendedTimetable(selection, gwm, dfs);
    }

    PartialSchedule TimetableCreator::createPartialSchedule(const graph::Selection &selection,
                                                            const graph::GraphWithMeta &gwm) {
        LongestPathCalculator calc{gwm.graph, gwm.start_node, gwm.measure_nodes.getNodes()};
        auto dfs = calc.calculateInitialPaths(selection).distance_from_start;
        return createPartialSchedule(selection, gwm, dfs);
    }


    PartialSchedule TimetableCreator::createPartialSchedule(graph::Selection const &selection,
                                                            const graph::GraphWithMeta &gwm,
                                                            const std::vector<distance_t> &dfs) {

        PartialSchedule partial_schedule{model_.courses.size()};
        ScheduleItemExtractor si_extractor{selection, dfs};

        for (auto const &c: model_.courses) {

            std::vector<ScheduleItem> &amended_sched = partial_schedule.schedule[c.index];
            amended_sched.clear();
            amended_sched.reserve(c.schedule.size());
            auto const &realized_sched = realized_schedule_[c.index];

            if (!realized_sched.empty()) {
                amended_sched.insert(amended_sched.begin(), realized_sched.begin(), realized_sched.end());
            }

            auto const *abs = gwm.abstractions.firstAbsOfCourse(c.index);

            while (abs != nullptr) {

                ScheduleItem amended_item = si_extractor.extractItem(*abs, c);

                auto const *successor = gwm.abstractions.next(*abs);
                // if the next abstraction is not modeled, we do not have all constrains for the departure,
                // therefore we set it to invalid. However, if abs is the last item of the entire duty
                // the rolling stock immediately goes to the depot, so we do not set the departure to invalid

                bool to_depot = abs->getOriginalOutScheduleItem()->index == c.schedule.size() - 1 &&
                                c.index_in_duty == model_.duties[c.duty].elements.size() - 1;
                if (successor == nullptr && abs->getRealizedOutScheduleItem() == nullptr && !to_depot) {

                    // we know that the duty stays at node for at least the prognosed departure time
                    partial_schedule.end_of_validity =
                            std::min(partial_schedule.end_of_validity, amended_item.departure);

                    amended_item.departure = invalid<seconds_t>();
                }


                if (amended_item.index < amended_sched.size()) {// in case it was already created by realized
                    amended_sched[amended_item.index] = amended_item;
                } else {
                    amended_sched.emplace_back(amended_item);
                }
                abs = gwm.abstractions.nextAbsOfCourse(*abs, c.index);
            }
        }

        return std::move(partial_schedule);
    }
    TimetableAmendment TimetableCreator::createAmendedTimetable(graph::Selection const &selection,
                                                                const graph::GraphWithMeta &gwm,
                                                                const std::vector<distance_t> &dfs) {


        TimetableAmendment amendment{};

        ScheduleItemExtractor si_extractor{selection, dfs};

        std::vector<Course> &amended_courses = amendment.courses;
        for (auto const &c: model_.courses) {
            auto &amended_course = amended_courses.emplace_back(c);
            std::vector<ScheduleItem> &amended_sched = amended_course.schedule;
            amended_sched.clear();
            amended_sched.reserve(c.schedule.size());
            auto const &realized_sched = realized_schedule_[c.index];

            if (!realized_sched.empty()) {
                amended_sched.insert(amended_sched.begin(), realized_sched.begin(), realized_sched.end());
            }

            auto const *abs = gwm.abstractions.firstAbsOfCourse(c.index);

            while (abs != nullptr) {

                ScheduleItem amended_item = si_extractor.extractItemWithAmendment(*abs, c, amendment);

                if (amended_item.index < amended_sched.size()) {// in case it was already created by realized
                    amended_sched[amended_item.index] = amended_item;
                } else {
                    amended_sched.emplace_back(amended_item);
                }

                abs = gwm.abstractions.nextAbsOfCourse(*abs, c.index);
            }
            assert(amended_sched.size() == c.schedule.size());

            amended_course.planned_start = amended_course.schedule.front().departure;
            amended_course.planned_end = amended_course.schedule.back().arrival;
        }

        amendment.duties = createAmendedDuties(amended_courses);
        return amendment;
    }
    std::vector<RSDuty> TimetableCreator::createAmendedDuties(const std::vector<Course> &amended_courses) const {
        std::vector<RSDuty> amended_duties{};

        for (auto const &r: model_.duties) {
            auto &amended_duty = amended_duties.emplace_back();
            amended_duty.index = r.index;
            auto &amended_elements = amended_duty.elements;
            amended_elements.reserve(r.elements.size());


            Course const *last_course{nullptr};
            Course const *current_course{nullptr};

            for (auto const &te: r.trainElements()) {
                current_course = &amended_courses[te.course];

                if (last_course != nullptr) {
                    // add change end
                    auto &change_end = amended_elements.emplace_back();
                    auto const *old = r.changeEndElementBetween(last_course->index, current_course->index);
                    assert(old != nullptr);
                    auto old_dif = old->planned_end_time - old->planned_start_time;
                    auto diff_dirs = current_course->direction != last_course->direction;
                    auto needed_cet =
                            std::min(old_dif, model_.changeEndTime(last_course->direction, current_course->direction,
                                                                   last_course->end_node));
                    change_end.planned_start_time = last_course->planned_end;
                    change_end.planned_end_time = change_end.planned_start_time + needed_cet;
                    change_end.start_node = last_course->end_node;
                    change_end.end_node = current_course->start_node;
                    change_end.event = EventType::CHANGE_END;
                    change_end.index = amended_elements.size() - 1;

                    // might be the case if both courses are already realized and the cet is not respected
                    if (change_end.planned_end_time > current_course->planned_start) {
                        assert(!realized_schedule_[current_course->index].empty());
                        change_end.planned_end_time = current_course->planned_start;
                    }

                    if (change_end.planned_end_time < current_course->planned_start) {
                        // add spare
                        auto &spare = amended_elements.emplace_back();
                        spare.planned_start_time = change_end.planned_end_time;
                        spare.planned_end_time = current_course->planned_start;
                        spare.start_node = change_end.end_node;
                        spare.end_node = current_course->start_node;
                        spare.event = EventType::SPARE;
                        spare.index = amended_elements.size() - 1;
                    }
                }

                // add train element
                auto &train_element = amended_elements.emplace_back();
                train_element.planned_start_time = current_course->planned_start;
                train_element.planned_end_time = current_course->planned_end;
                train_element.start_node = current_course->start_node;
                train_element.end_node = current_course->end_node;
                train_element.event = EventType::TRAIN;
                train_element.course = current_course->index;
                train_element.index = amended_elements.size() - 1;

                last_course = current_course;
            }

            if (last_course == nullptr) {
                // this is the case if the duty only contained reserve elements
                assert(r.elements.size() == 1 && r.elements.front().event == EventType::RESERVE);
                amended_elements.emplace_back(r.elements.front());
            }
        }
        return std::move(amended_duties);
    }
    TimetableAmendment TimetableCreator::createAmendedTimetable(PartialSchedule const &partial_schedule,
                                                                bool addAmendmentsFromRealized) const {


        TimetableAmendment amendment{};


        for (auto c_id = 0; c_id < partial_schedule.schedule.size(); c_id++) {
            auto original_course = model_.courses[c_id];
            auto real_sched = realized_schedule_[c_id];
            auto modified_sched = partial_schedule.schedule[c_id];

            // add amendments
            for (auto const &si: modified_sched) {
                auto const &original_si = original_course.schedule[si.index];

                // skip already realized items
                if (!addAmendmentsFromRealized && realized_schedule_[c_id].size() > si.index) {
                    continue;
                }

                auto first_with_invalid_arrival = original_si.index == 0 && isInvalid(original_si.arrival);
                if (!first_with_invalid_arrival && si.arrival != original_si.arrival) {
                    amendment.arrival_retimings.emplace_back(c_id, original_si, si.arrival);
                }
                auto last_with_invalid_departure =
                        original_si.index == original_course.schedule.size() - 1 && isInvalid(original_si.departure);
                if (!last_with_invalid_departure && si.departure != original_si.departure) {
                    amendment.departure_retimings.emplace_back(c_id, original_si, si.departure);
                }
                if (si.original_track != original_si.original_track) {
                    amendment.replatformings.emplace_back(c_id, original_si, si.original_track);
                }
                if (si.original_activity != original_si.original_activity) {
                    amendment.skipped_stops.emplace_back(c_id, original_si);
                }
            }

            // build course

            auto &modified_course = amendment.courses.emplace_back(original_course);
            modified_course.schedule = modified_sched;
            modified_course.planned_start = modified_sched.front().departure;
            modified_course.planned_end = modified_sched.back().arrival;
        }

        amendment.duties = createAmendedDuties(amendment.courses);
        return amendment;
    }


}// namespace fb
