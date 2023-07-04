#pragma once

#include "Json.h"
#include "ModelStatistics.h"
#include "ProblemStatistics.h"
#include "statistics/bab/BranchAndBoundStatistics.h"
#include "statistics/graph/GraphStatistics.h"
namespace fb {
    struct SingleRunStatistics {

        SingleRunStatistics() {}
        long total_time{invalid<long>()};

        ProblemStatistics problem_statistics{};
        std::vector<long> time_per_iteration{};
        std::vector<BABStatistics> bab_statistics{};
        std::vector<GraphStatistics> graph_statistics{};
        std::vector<seconds_t> snapshot_times{};
        bool completed_with_planned{false};
    };


    std::ostream &operator<<(std::ostream &os, SingleRunStatistics const &stats);

    template<>
    std::string toJson(SingleRunStatistics const &t);
}// namespace fb
