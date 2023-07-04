
#include "PartialSchedule.h"
namespace fb {


    seconds_t PartialSchedule::firstUncompletedEnd(std::vector<RSDuty> const &duties,
                                                   std::vector<Course> const &courses) const {

        seconds_t end = std::numeric_limits<seconds_t>::max();
        for (auto const &d: duties) {

            if (!d.trainElements().empty()) {
                // don't consider duties that are already completed
                auto const &last_course = courses[d.trainElements().back().course];
                if (last_course.schedule.size() == schedule[last_course.index].size()) {
                    continue;
                }
            }

            seconds_t duty_end = dutyEnd(d, courses);

            end = std::min(duty_end, end);
        }
        return end;
    }


    seconds_t PartialSchedule::lastEnd(const std::vector<RSDuty> &duties, const std::vector<Course> &courses) const {
        seconds_t end = std::numeric_limits<seconds_t>::min();
        for (auto const &d: duties) {
            end = std::max(dutyEnd(d, courses), end);
        }
        return end;
    }
    seconds_t PartialSchedule::dutyEnd(const RSDuty &duty, const std::vector<Course> &courses) const {

        bool has_train_elements = false;
        for (auto const &elem: duty.trainElements() | std::views::reverse) {
            auto const &cid = elem.course;
            has_train_elements = true;

            if (schedule[cid].empty()) {
                continue;
            }

            auto const &last = schedule[cid].back();

            if (isValid(last.departure)) {
                return last.departure;
            } else {
                return last.arrival;
            }
        }


        if (has_train_elements) {
            return duty.elements.front().planned_start_time;
        } else {
            return duty.elements.back().planned_end_time;
        }
    }
    void PartialSchedule::pruneToSnapshotTime(seconds_t time) {

        for (auto &c_sched: schedule) {

            while (!c_sched.empty()) {
                if (isValid(c_sched.back().departure) && c_sched.back().departure > time) {
                    c_sched.back().departure = invalid<seconds_t>();
                }
                if (isValid(c_sched.back().arrival) && c_sched.back().arrival > time) {
                    c_sched.pop_back();
                } else {
                    break;
                }
            }
        }
    }
}// namespace fb
