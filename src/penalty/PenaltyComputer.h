
#pragma once
#include "DelayPenaltyComputer.h"
#include "algo/path/LongestPathCalculator.h"
#include "model/graph/GraphWithMeta.h"
#include "model/graph/Selection.h"
#include "penalty/definitions.h"
namespace fb {


    class PenaltyComputer {
    public:
        PenaltyComputer(const graph::GraphWithMeta &graph_with_meta, const Model &model, seconds_t snapshot_time);


        penalty_t computeTotalPenalty(graph::Selection const &sel, std::vector<distance_t> const &distances) const;


        penalty_t computeSkippedStopPenalty(const graph::Selection &sel) const;
        penalty_t computeTargetFrequencyPenalty(const graph::Selection &sel,
                                                std::vector<distance_t> const &distances) const;


        penalty_t computeTargetFrequencyPenaltyAtNode(node_index_t node, Direction dir,
                                                      std::vector<distance_t> const &distances) const;


        DelayPenaltyComputer const delay_computer;

    private:
        graph::GraphWithMeta const &graph_with_meta_;
        Model const &model_;
        seconds_t snapshot_time_;
    };
}// namespace fb
