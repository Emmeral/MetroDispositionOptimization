
#include "ProblemSanitizer.h"

namespace fb {
    ProblemSanitizer::ProblemSanitizer(const Model &model) : model_(model) {}


    void ProblemSanitizer::sanitizeRealizedSchedule(ProblemInstance &problem) {


        auto &re_sched = problem.realized_schedule;


        for (auto c_id = 0; c_id < re_sched.size(); ++c_id) {
            auto const &course = model_.courses[c_id];
            auto &realized = re_sched[c_id];

            if (realized.empty()) {
                continue;// nothing to do for empty courses
            }

            // in some cases the first item is missing
            if (course.schedule.size() > 1 && course.schedule[1].node == realized[0].node) {
                // interpolate first si based on planned schedule
                ScheduleItem missing_re{};
                missing_re = course.schedule[0];
                auto default_runtime = course.schedule[1].arrival - course.schedule[0].departure;
                missing_re.departure = realized[0].arrival - default_runtime;
                missing_re.arrival = missing_re.departure;

                // insert missing item
                realized.insert(realized.begin(), missing_re);
                // fix indices
                for (auto i = 0; i < realized.size(); ++i) {
                    realized[i].index = i;
                }
                // std::cout << "INFO: Fixed broken realized course " << model_.course_code[c_id] << "\n";
            }
        }
    }
}// namespace fb
