
#include "AbstractionCreator.h"

namespace fb {


    graph::TrainAtStationAbstraction *
    AbstractionCreator::createAbstraction(const ProblemInstance &problem, graph::Abstractions &abstractions,
                                          course_index_t next_course_index, const Course &course, index_t si_index,
                                          const graph::TaggedWeight &stop_time) const {

        std::vector<ScheduleItem> const &realized_schedule = problem.realized_schedule[course.index];

        bool realized = realized_schedule.size() > si_index;
        ScheduleItem const &si = course.schedule[si_index];
        ScheduleItem const *rsi = realized ? &realized_schedule[si_index] : nullptr;

        bool first = (si_index == 0);
        bool last = (si_index == course.schedule.size() - 1);
        const Node &node = model_.nodes[si.node];

        auto real_original_arrival = si.arrival;
        if (isInvalid(si.arrival)) {
            real_original_arrival = si.departure;
        }

        bool is_link = last && isValid(next_course_index);
        bool original_passes = si.original_activity == Activity::PASS;
        bool over_stop_pass_threshold = (((long) real_original_arrival) - problem.last_incident_end) >
                                        options_.stop_pass_choice_after_last_incident;

        bool extended_course_dwell = problem.mapped_extended_train_dwelltimes.contains({course.index, node.index});

        bool fixed_activity = first || last || original_passes || over_stop_pass_threshold ||
                              !options_.stop_pass_choices || extended_course_dwell;

        bool over_track_decision_threshold = (((long) real_original_arrival) - problem.last_incident_end) >
                                             options_.track_choice_after_last_incident;


        bool only_one_track_available = node.tracks.countInDirection(course.direction) <= 1;
        if (!only_one_track_available && is_link) {// duties turning need a "Both" track
            auto const &next_course = model_.courses[next_course_index];
            if (next_course.direction != course.direction) {
                only_one_track_available = node.tracks.countInDirection(Direction::BOTH) <= 1;
            }
        }
        bool fixed_track =
                realized || over_track_decision_threshold || only_one_track_available || !options_.track_choices;


        graph::TrainAtStationAbstraction *created = nullptr;

        if (is_link) {
            // the first item of a connected course has to consider the last item of the prior course
            auto const &next_course = model_.courses[next_course_index];

            if (realized) {
                auto const &next_course_realized = problem.realized_schedule[next_course_index];

                created = &abstractions.emplaceAbstraction<graph::RealizedLinkAbstraction>(
                        node, course, next_course, realized_schedule.back(), next_course_realized.front(), stop_time);
            } else if (fixed_track) {
                created = &abstractions.emplaceAbstraction<graph::FullyDecidedLinkAbstraction>(node, course,
                                                                                               next_course, stop_time);
            } else {
                created = &abstractions.emplaceAbstraction<graph::MultiTrackLinkAbstraction>(node, course, next_course,
                                                                                             stop_time);
            }

        } else {

            if (realized) {
                created = &abstractions.emplaceAbstraction<graph::RealizedSingleAbstraction>(node, course, *rsi,
                                                                                             stop_time);
            } else if (fixed_track && fixed_activity) {
                created = &abstractions.emplaceAbstraction<graph::FullyDecidedSingleAbstraction>(node, course, si,
                                                                                                 stop_time);
            } else if (fixed_track) {
                created = &abstractions.emplaceAbstraction<graph::TrackDecidedSingleAbstraction>(node, course, si,
                                                                                                 stop_time);
            } else if (fixed_activity) {
                created = &abstractions.emplaceAbstraction<graph::PredefinedActivityMultiTrackAbstraction>(
                        node, course, si, stop_time);
            } else {
                created = &abstractions.emplaceAbstraction<graph::MultiTrackSingleAbstraction>(node, course, si,
                                                                                               stop_time);
            }
        }


        return created;
    }
}// namespace fb
