
#include "ProblemInstance.h"

namespace fb {
    ScheduleItem const *ProblemInstance::realized(course_index_t c, index_t si) const {

        if (c >= realized_schedule.size()) {
            return nullptr;
        }
        auto const &sched = realized_schedule[c];
        if (si >= sched.size()) {
            return nullptr;
        }
        return &sched[si];
    }
    bool ProblemInstance::hasRealizedDeparture(course_index_t c, index_t si, Model const &model) const {
        auto *rsi = realized(c, si);
        if (rsi == nullptr) {
            return false;
        }
        if (isValid(rsi->departure)) {
            return true;
        }

        if (si != model.courses[c].schedule.size() - 1) {
            return false;
        }

        Course const *next_course = model.nextCourseInDuty(c);
        if (next_course == nullptr) {
            return hasRealizedArrival(c, si, model);
        }

        return hasRealizedArrival(next_course->index, 0, model);
    }
    bool ProblemInstance::hasRealizedArrival(course_index_t c, index_t si, Model const &model) const {
        auto *rsi = realized(c, si);

        if (rsi == nullptr) {
            return false;
        }
        if (isValid(rsi->arrival)) {
            return true;
        }

        if (si != 0) {
            return false;
        }

        Course const *prior_course = model.priorCourseInDuty(c);

        if (prior_course == nullptr) {
            return hasRealizedDeparture(c, si, model);
        }
        return hasRealizedArrival(prior_course->index, prior_course->schedule.size() - 1, model);
    }
}// namespace fb
