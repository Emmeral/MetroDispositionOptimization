
#include "ScheduleValidator.h"
#include "algo/path/definitions.h"
#include "fmt/format.h"
#include <span>

namespace fb {

    template<typename T>
    void append(std::vector<T> &v1, std::vector<T> const &v2) {
        v1.insert(v1.end(), v2.begin(), v2.end());
    }

    ScheduleValidator::ScheduleValidator(const Model &model, const ProblemInstance &problem)
        : model_(model), problem_(problem) {}


    std::vector<std::string> ScheduleValidator::validate(const TimetableAmendment &amendment) {

        std::vector<std::string> violations{};

        append(violations, validateRespectRealizedSchedule(amendment));
        append(violations, validateDwellTimes(amendment));
        append(violations, validateRuntimes(amendment));
        append(violations, validateLineHeadways(amendment));
        append(violations, validateTrackHeadways(amendment));
        append(violations, validateEarlyDepartureBound(amendment));
        append(violations, validateLateDepartures(amendment));
        append(violations, validateDutyElementConsistency(amendment));
        append(violations, validateChangeEndTimes(amendment));
        //append(violations, validateUsedInfrastructure(amendment));

        return violations;
    }
    std::vector<std::string> ScheduleValidator::validateRespectRealizedSchedule(const TimetableAmendment &amendment) {
        std::vector<std::string> violations{};


        for (auto cid = 0; cid < problem_.realized_schedule.size(); ++cid) {
            auto const &sched = problem_.realized_schedule[cid];
            auto &amended = amendment.courses[cid];

            for (auto const &realized: sched) {
                auto item = amended.schedule[realized.index];

                bool same_track = item.original_track == realized.original_track;
                bool same_node = item.original_track == realized.original_track;
                bool same_arrival = item.arrival == realized.arrival;
                bool same_dep = isInvalid(realized.departure) || realized.departure == item.departure;


                if (same_track && same_node && same_arrival && same_dep) {
                    continue;
                }

                violations.push_back(fmt::format("Realized Schedule Mismatch: Course: {} ({}) Index: {}",
                                                 model_.course_code[cid], cid, item.index));
            }
        }


        return violations;
    }
    std::vector<std::string> ScheduleValidator::validateDwellTimes(const TimetableAmendment &amendment) {


        std::vector<std::string> violations{};

        for (auto const &c: amendment.courses) {

            auto const &original_c = model_.courses[c.index];

            for (auto const &si: std::span(c.schedule.begin() + 1, c.schedule.end() - 1)) {
                auto dwell = si.departure - si.arrival;


                if (problem_.hasRealizedDeparture(c, si, model_)) {
                    continue;// don't validate things already in the realized schedule
                }


                distance_t original_dwell = 0;
                if (c.index < model_.courses.size()) {
                    auto const &original_si = original_c.schedule[si.index];

                    if (si.original_activity == Activity::PASS) {
                        continue;
                    }

                    original_dwell = original_si.departure - original_si.arrival;

                    if (dwell < original_dwell) {
                        violations.push_back(
                                fmt::format("Dwell time violation: Course: {} ({}) Index: {} Original: {} Amended: {}",
                                            model_.course_code[c.index], c.index, si.index, original_dwell, dwell));
                    }
                }


                for (auto const &esd: problem_.extended_dwelltimes) {
                    if (esd.node != si.node || si.arrival < esd.start || esd.end + original_dwell <= si.departure ||
                        dwell >= esd.extended_dwell) {
                        continue;
                    }
                    violations.push_back(
                            fmt::format("Extended Dwell violation: Course: {} ({}) Index: {} Required: {} Actual: {}",
                                        model_.course_code[c.index], c.index, si.index, esd.extended_dwell, dwell));
                }
                for (auto const &ecd: problem_.extended_train_dwelltimes) {
                    if (ecd.course != c.index || ecd.node != si.node || dwell >= ecd.extended_dwell) {
                        continue;
                    }
                    violations.push_back(
                            fmt::format("Extended Dwell violation: Course: {} ({}) Index: {} Required: {} Actual: {}",
                                        model_.course_code[c.index], c.index, si.index, ecd.extended_dwell, dwell));
                }
            }
        }

        return violations;
    }
    std::vector<std::string> ScheduleValidator::validateRuntimes(const TimetableAmendment &amendment) {

        std::vector<std::string> violations{};

        for (auto const &c: amendment.courses) {

            for (auto i = 0; i < c.schedule.size() - 1; ++i) {

                auto const &si_i = c.schedule[i];
                auto const &si_i1 = c.schedule[i + 1];


                if (problem_.hasRealizedArrival(c, si_i1, model_)) {
                    continue;// already realized
                }


                auto rt = si_i1.arrival - si_i.departure;
                Link const &link = *model_.getLinkFromTo(si_i.node, si_i1.node);
                auto minimum = link.minimumRuntimes.of(si_i.original_activity, si_i1.original_activity);

                if (rt < minimum) {
                    violations.push_back(
                            fmt::format("Runtime Violation: Course: {} ({}) {}->{} Required: {} Actual: {}",
                                        model_.course_code[c.index], c.index, i, i + 1, minimum, rt));
                }


                for (auto const &ert: problem_.extended_runtimes) {
                    if (ert.link != link.id || si_i.departure < ert.start || ert.end + minimum <= si_i1.arrival ||
                        rt >= ert.extended_runtime) {
                        continue;
                    }
                    violations.push_back(
                            fmt::format("Extended Runtime violation: Course: {} ({}) {}->{} Required: {} Actual: {}",
                                        model_.course_code[c.index], c.index, i, i + 1, ert.extended_runtime, rt));
                }
            }
        }

        return violations;
    }
    std::vector<std::string> ScheduleValidator::validateLineHeadways(const TimetableAmendment &amendment) {
        std::vector<std::string> violations{};


        std::vector<std::vector<PairIndex>> link_sorted{};
        link_sorted.resize(model_.links.size());


        for (auto const &c: amendment.courses) {
            for (auto i = 0u; i + 1 < c.schedule.size(); ++i) {
                auto const &first = c.schedule[i];
                auto const &second = c.schedule[i + 1];

                auto const &link = *model_.getLinkFromTo(first.node, second.node);

                link_sorted[link.id].push_back({c.index, i, i + 1});
            }
        }


        for (auto const &link: model_.links) {
            std::vector<PairIndex> const &sis = link_sorted[link.id];
            validateLinkHeadwaysSameDir(amendment, violations, link, sis);

            if (link.direction == Direction::BOTH) {
                auto const &rev = model_.links[model_.reverse_links[link.id]];
                if (rev.id < link.id) {
                    continue;
                }
                validateLinkHeadwaysBothDir(amendment, violations, link, rev, sis, link_sorted[rev.id]);
            }
        }


        return violations;
    }
    void ScheduleValidator::validateLinkHeadwaysSameDir(const TimetableAmendment &amendment,
                                                        std::vector<std::string> &violations, const Link &link,
                                                        const std::vector<PairIndex> &sis) {
        node_index_t start = link.link_start_node;
        node_index_t end = link.link_end_node;

        auto const &headways = link.headways;

        for (auto i = 0; i < sis.size(); ++i) {

            Course const &first_course = amendment.courses[sis[i].course];
            ScheduleItem const &first_start = first_course.schedule[sis[i].first];
            ScheduleItem const &first_end = first_course.schedule[sis[i].second];

            for (auto j = i + 1; j < sis.size(); ++j) {

                Course const &second_course = amendment.courses[sis[j].course];
                ScheduleItem const &second_start = second_course.schedule[sis[j].first];
                ScheduleItem const &second_end = second_course.schedule[sis[j].second];


                if (problem_.hasRealizedArrival(first_course, first_end, model_) &&
                    problem_.hasRealizedArrival(second_course, second_end, model_)) {
                    continue;// happened before snapshot time
                }

                bool first_first = first_start.departure < second_start.departure;
                bool second_first = second_start.departure < first_start.departure;
                if (first_first && second_end.arrival < first_end.arrival ||
                    second_first && first_end.arrival < second_end.arrival) {
                    violations.push_back(fmt::format("Illegal Overtake: {}({})->{}({}) Course: {}({}) & {}({})", start,
                                                     model_.node_metadata[start].code, end,
                                                     model_.node_metadata[end].code,
                                                     model_.course_code[first_course.index], first_course.index,
                                                     model_.course_code[second_course.index], second_course.index));
                }


                if (problem_.hasRealizedDeparture(first_course, first_start, model_) &&
                    problem_.hasRealizedDeparture(second_course, second_start, model_)) {
                    continue;// already realized -> no check
                }

                long diff = 0;
                seconds_t expected{0};
                if (first_first) {
                    diff = second_start.departure - (long) first_start.departure;

                    expected = headways.of(first_start.original_activity, first_end.original_activity,
                                           second_start.original_activity, second_end.original_activity);
                } else {
                    diff = first_start.departure - (long) second_start.departure;
                    expected = headways.of(second_start.original_activity, second_end.original_activity,
                                           first_start.original_activity, first_end.original_activity);
                }
                if (diff < expected) {

                    violations.push_back(fmt::format(
                            "Line Headway Violation: {}({})->{}({}) Course: {}({}) & {}({}) Required: {} Actual: "
                            "{}",
                            start, model_.node_metadata[start].code, end, model_.node_metadata[end].code,
                            model_.course_code[first_course.index], first_course.index,
                            model_.course_code[second_course.index], second_course.index, expected, diff));
                }
            }
        }
    }
    std::vector<std::string> ScheduleValidator::validateTrackHeadways(const TimetableAmendment &amendment) {
        std::vector<std::string> violations{};


        struct FullIndex {
            course_index_t course;
            index_t item;
        };

        std::vector<std::vector<FullIndex>> node_sorted{};
        node_sorted.resize(model_.nodes.size());


        for (auto const &c: amendment.courses) {
            for (auto const &si: c.schedule) {
                node_sorted[si.node].push_back({c.index, si.index});
            }
        }

        for (Node const &node: model_.nodes) {

            auto const &at_node = node_sorted[node.index];
            for (auto i = 0; i < at_node.size(); ++i) {

                auto const &first_course = amendment.courses[at_node[i].course];
                auto const &first = first_course.schedule[at_node[i].item];

                for (auto j = i + 1; j < at_node.size(); ++j) {


                    auto const &second_course = amendment.courses[at_node[j].course];
                    auto const &second = second_course.schedule[at_node[j].item];


                    if ((problem_.hasRealizedDeparture(first_course, first, model_) &&
                         problem_.hasRealizedArrival(second_course, second, model_)) ||
                        (problem_.hasRealizedDeparture(second_course, second, model_) &&
                         problem_.hasRealizedArrival(first_course, first, model_))) {
                        continue;// already realized
                    }

                    if (first.original_track != second.original_track) {
                        continue;
                    }
                    if (first_course.duty == second_course.duty) {
                        continue;
                    }

                    auto first_arrival = realArrival(first, first_course, amendment);
                    auto second_arrival = realArrival(second, second_course, amendment);

                    auto first_departure = realDeparture(first, first_course, amendment);
                    auto second_departure = realDeparture(second, second_course, amendment);

                    long diff = 0;
                    if (first_arrival < second_arrival) {
                        diff = (long) second_arrival - first_departure;
                    } else {
                        diff = (long) first_arrival - second_departure;
                    }

                    if (diff < 30) {

                        violations.push_back(fmt::format(
                                "Track Headway Violation: {}({}) Track: {} Course: {}({}) & {}({})  Diff: {}",
                                node.index, model_.node_metadata[node.index].code, first.original_track,
                                model_.course_code[first_course.index], first_course.index,
                                model_.course_code[second_course.index], second_course.index, diff));
                    }
                }
            }
        }


        return violations;
    }
    void ScheduleValidator::validateLinkHeadwaysBothDir(const TimetableAmendment &amendment,
                                                        std::vector<std::string> &violations, const Link &link,
                                                        const Link &rev_link, const std::vector<PairIndex> &link_sis,
                                                        const std::vector<PairIndex> &rev_link_sis) {

        for (auto const &pair: link_sis) {

            Course const &first_course = amendment.courses[pair.course];
            ScheduleItem const &first_start = first_course.schedule[pair.first];
            ScheduleItem const &first_end = first_course.schedule[pair.second];

            for (auto const &rev_pair: rev_link_sis) {
                Course const &second_course = amendment.courses[rev_pair.course];
                ScheduleItem const &second_start = second_course.schedule[rev_pair.first];
                ScheduleItem const &second_end = second_course.schedule[rev_pair.second];

                if (problem_.hasRealizedArrival(first_course, first_end, model_) &&
                    problem_.hasRealizedArrival(second_course, second_end, model_)) {
                    continue;// happened before snapshot time
                }

                bool first_first = first_start.departure < second_start.departure;
                bool second_first = second_start.departure < first_start.departure;


                if (problem_.hasRealizedDeparture(first_course, first_start, model_) &&
                    problem_.hasRealizedDeparture(second_course, second_start, model_)) {
                    continue;// already realized -> no check
                }

                long diff = 0;
                seconds_t expected{30};
                if (first_first) {
                    diff = second_start.departure - (long) first_end.arrival;

                } else {
                    diff = first_start.departure - (long) second_start.arrival;
                }
                if (diff < expected) {

                    violations.push_back(fmt::format(
                            "Link Both Headway Violation: {}({})<->{}({}) Course: {}({}) & {}({}) Required: {} Actual: "
                            "{}",
                            link.link_start_node, model_.node_metadata[link.link_start_node].code, link.link_end_node,
                            model_.node_metadata[link.link_end_node].code, model_.course_code[first_course.index],
                            first_course.index, model_.course_code[second_course.index], second_course.index, expected,
                            diff));
                }
            }
        }
    }
    std::vector<std::string> ScheduleValidator::validateEarlyDepartureBound(const TimetableAmendment &amendment) {
        std::vector<std::string> violations{};

        for (auto const &c: amendment.courses) {
            if (c.index >= model_.courses.size()) {
                continue;
            }
            for (auto const &si: c.schedule | std::views::take(c.schedule.size() - 1)) {

                if (problem_.hasRealizedDeparture(c, si, model_)) {
                    continue;
                }
                if (si.original_activity == Activity::PASS) {
                    continue;
                }

                auto const &original = model_.courses[c.index].schedule[si.index];
                if ((long) original.departure - si.departure > 300) {
                    violations.push_back(fmt::format(
                            "Early Departure Violation: Course: {}({}) Item: {} Earliest Permitted: {} Actual: {}",
                            model_.course_code[c.index], c.index, si.index, original.departure - 300, si.departure));
                }
            }
        }
        return violations;
    }
    std::vector<std::string> ScheduleValidator::validateLateDepartures(const TimetableAmendment &amendment) {
        std::vector<std::string> violations{};


        for (auto const &ld: problem_.late_departures) {

            auto const &amended = amendment.courses[ld.course];
            auto const &original = model_.courses[ld.course];

            if ((long) amended.planned_start - original.planned_start < ld.departure_delay) {
                violations.push_back(fmt::format("Late Departure Violation: Course: {}({})",
                                                 model_.course_code[ld.course], ld.course));
            }
        }

        return violations;
    }
    std::vector<std::string> ScheduleValidator::validateDutyElementConsistency(const TimetableAmendment &amendment) {

        std::vector<std::string> violations{};
        for (auto const &d: amendment.duties) {

            for (auto i = 0; i < d.elements.size() - 1; ++i) {
                if (d.elements[i].planned_end_time != d.elements[i + 1].planned_start_time ||
                    d.elements[i].end_node != d.elements[i + 1].start_node) {
                    violations.push_back(fmt::format("Duty Element Consistency Violation: Duty: {}({}) Element: {}->{}",
                                                     model_.duty_code[d.index], d.index, i, i + 1));
                }
            }
        }
        return violations;
    }
    std::vector<std::string> ScheduleValidator::validateChangeEndTimes(const TimetableAmendment &amendment) {
        std::vector<std::string> violations{};

        for (auto const &d: amendment.duties) {
            const RSDuty &original_duty = model_.duties[d.index];

            for (auto const &ceel: d.elementsWithType(EventType::CHANGE_END)) {

                auto const &prior_course = model_.courses[d.lastTrainElementBefore(ceel.index)->course];
                auto const &next_course = model_.courses[d.nextTrainElementAfter(ceel.index)->course];

                if (!problem_.realized_schedule[next_course.index].empty()) {
                    // we do not verify the change end time if the next course already departed before the snapshot time
                    continue;
                }

                auto const *orginal_ce = original_duty.changeEndElementBetween(prior_course.index, next_course.index);
                auto old_dif = orginal_ce->planned_end_time - orginal_ce->planned_start_time;

                auto required_cet =
                        model_.changeEndTime(prior_course.direction, next_course.direction, ceel.start_node);
                auto required = std::min(old_dif, required_cet);

                auto actual_cet = ceel.planned_end_time - ceel.planned_start_time;
                if (actual_cet < required) {
                    violations.push_back(
                            fmt::format("Change End Time Violation: Duty: {}({}) Element: {} Required: {} Actual: {}",
                                        model_.duty_code[d.index], d.index, ceel.index, required, actual_cet));
                }
            }
        }
        return violations;
    }
    std::vector<std::string> ScheduleValidator::validateUsedInfrastructure(const TimetableAmendment &amendment) {
        std::vector<std::string> violations{};

        for (auto const &c: amendment.courses) {

            for (auto i = 0; i < c.schedule.size() - 1; ++i) {
                auto const &first = c.schedule[i];
                auto const &second = c.schedule[i + 1];

                auto const *link = model_.getLinkFromTo(first.node, second.node);
                if (link == nullptr) {
                    violations.push_back(
                            fmt::format("Used Link is non existent: Course: {}({}) Item: {}->{} Node: {}->{}",
                                        model_.course_code[c.index], c.index, i, i + 1, first.node, second.node));
                } else if (link->direction != Direction::BOTH && link->direction != c.direction) {

                    std::stringstream ss{};
                    ss << link->direction;
                    violations.push_back(fmt::format("Link used in wrong direction: Course: {}({}) Item: {}->{} "
                                                     "Node: {}({})->{}({}) Link Direction: {}",
                                                     model_.course_code[c.index], c.index, i, i + 1,
                                                     model_.node_metadata[first.node].code, first.node,
                                                     model_.node_metadata[second.node].code, second.node, ss.str()));
                }
            }
        }
        return violations;
    }
    seconds_t ScheduleValidator::realArrival(const ScheduleItem &item, const Course &course,
                                             const TimetableAmendment &amendment) const {

        if (isValid(item.arrival) || item.index > 0) {
            return item.arrival;
        }
        auto const *prior_course = model_.priorCourseInDuty(course);

        if (prior_course == nullptr) {
            return item.departure;
        }
        return amendment.courses[prior_course->index].schedule.back().arrival;
    }
    seconds_t ScheduleValidator::realDeparture(const ScheduleItem &item, const Course &course,
                                               const TimetableAmendment &amendment) const {
        if (isValid(item.departure) || item.index < course.schedule.size() - 1) {
            return item.departure;
        }
        auto const *next_course = model_.nextCourseInDuty(course);

        if (next_course == nullptr) {
            return item.arrival;
        }
        return amendment.courses[next_course->index].schedule.front().departure;
    }
}// namespace fb
