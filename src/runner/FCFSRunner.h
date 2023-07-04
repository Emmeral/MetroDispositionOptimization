
#pragma once
#include "Runner.h"

namespace fb {

    class FCFSRunner : Runner {


    public:
        FCFSRunner(const Options &options, const Model &model);
        TimetableAmendment run(const ProblemInstance &original_problem, SingleRunStatistics &stats) override;
    };
}// namespace fb
