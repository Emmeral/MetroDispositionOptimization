
#pragma once

#include "Runner.h"
namespace fb {

    class NonIteratedRunner : public Runner {

    public:
        NonIteratedRunner(const Options &options, const Model &model);
        TimetableAmendment run(const ProblemInstance &original_problem, SingleRunStatistics &stats) override;
    };

}// namespace fb
