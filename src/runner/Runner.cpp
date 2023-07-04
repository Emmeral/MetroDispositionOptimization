
#include "Runner.h"
#include "BranchAndBoundRunner.h"
#include "IteratedRunner.h"
#include "builder/graph/GraphBuilder.h"
#include "exceptions/CycleException.h"
#include "output/timetable/TimetableCreator.h"
#include "statistics/SingleRunStatistics.h"
#include "util.h"


namespace fb {
    Runner::Runner(const Options &options, const Model &model)
        : options_(options), model_(model), graph_builder_(model, options.graph) {}
    bool Runner ::checkDone(const PartialSchedule &partial_schedule) {

        for (auto const &course: model_.courses) {
            if (partial_schedule.schedule[course.index].size() < course.schedule.size()) {
                return false;
            }
        }
        assert(isInvalid(partial_schedule.end_of_validity));
        return true;
    }
    bool Runner::checkEndsOnTime(const PartialSchedule &partial_schedule) {

        for (auto const &d: model_.duties) {


            auto opt_last_course =
                    d.trainElements() | std::views::reverse |
                    std::views::transform([&](auto const &de) { return model_.courses[de.course]; }) |
                    std::views::filter([&](auto const &c) { return !partial_schedule.schedule[c.index].empty(); }) |
                    std::views::take(1);


            if (opt_last_course.empty()) {
                continue;
            }
            Course const &last_course = opt_last_course.front();
            auto const &last_si = partial_schedule.schedule[last_course.index].back();

            if (last_course.index_in_duty == d.elements.size() - 1 &&
                last_si.index == last_course.schedule.size() - 1) {
                continue;// train goes to depot -> if in depot the schedule ends on time
            }

            if (isValid(last_si.departure)) {
                if (last_si.departure > model_.realDepartureTime(last_course, last_course.schedule[last_si.index])) {
                    return false;
                }
            } else if (last_si.arrival > model_.realArrivalTime(last_course, last_course.schedule[last_si.index])) {
                return false;
            }
        }

        return true;
    }
    void Runner::completeWithPlanned(PartialSchedule &partial_schedule) {
        for (auto const &course: model_.courses) {
            auto &partial = partial_schedule.schedule[course.index];

            if (!partial.empty() && isInvalid(partial.back().departure)) {
                partial.back().departure = course.schedule[partial.back().index].departure;
            }

            for (auto i = partial.size(); i < course.schedule.size(); ++i) {
                partial.push_back(course.schedule[i]);
            }
        }
    }
    bool Runner ::problemsInPast(const PartialSchedule &partial_schedule, ProblemInstance const &problem) {
        return partial_schedule.end_of_validity >= problem.last_incident_end;
    }

}// namespace fb
