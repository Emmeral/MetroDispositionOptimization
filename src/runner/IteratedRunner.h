
#pragma once

#include "Runner.h"
#include "builder/graph/GraphBuilder.h"
#include "model/problems/ProblemInstance.h"
#include "options/Options.h"
#include "output/timetable/TimetableAmendment.h"
#include "statistics/SingleRunStatistics.h"
namespace fb {

    /**
     * Runs multiple times until we meet the planned schedule again
     */
    class IteratedRunner : public Runner {

    public:
        IteratedRunner(const Options &options, const Model &model);
        TimetableAmendment run(const ProblemInstance &original_problem, SingleRunStatistics &stats) override;
    };

}// namespace fb
