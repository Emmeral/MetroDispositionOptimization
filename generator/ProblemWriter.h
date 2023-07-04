
#pragma once

#include "model/problems/ProblemInstance.h"
#include "parser/csv/CSVRow.h"
namespace fb {

    class ProblemWriter {

    public:
        explicit ProblemWriter(const Model &model);
        void writeProblem(ProblemInstance const &problem, std::string const &folder);

    private:
        CSVRow writeExtendedRuntime(ExtendedRunTime const &extended_run_time);
        CSVRow writeExtendedDwellTime(ExtendedDwellTime const &extended_dwell_time);
        CSVRow writeCourseDwellTime(TrainExtendedDwell const &course_dwell_time);
        CSVRow writeLateDeparture(LateDeparture const &late_departure);

        CSVRow writeRealizedScheduleItem(const ScheduleItem &schedule_item, const Course &course,
                                         ProblemInstance const &problem);

        Model const &model_;
    };

}// namespace fb
