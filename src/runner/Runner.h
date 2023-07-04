
#pragma once

#include "builder/graph/GraphBuilder.h"
#include "model/problems/ProblemInstance.h"
#include "options/Options.h"
#include "output/timetable/TimetableAmendment.h"
#include "statistics/SingleRunStatistics.h"
namespace fb {

    class Runner {

    public:
        Runner(const Options &options, const Model &model);
        virtual TimetableAmendment run(ProblemInstance const &original_problem, SingleRunStatistics &stats) = 0;

    protected:
        bool checkDone(const PartialSchedule &partial_schedule);
        bool checkEndsOnTime(const PartialSchedule &partial_schedule);
        void completeWithPlanned(PartialSchedule &partial_schedule);
        bool problemsInPast(const PartialSchedule &partial_schedule, const ProblemInstance &problem);

        Options const &options_;
        Model const &model_;
        GraphBuilder graph_builder_;
    };

}// namespace fb
