
#pragma once

#include "model/definitions.h"
#include "model/graph/AlternativeGraph.h"
#include "model/graph/GraphWithMeta.h"
#include "model/graph/abstractions/TrainAtStationAbstraction.h"
namespace fb {

    class PrematureActionPreventer {

    public:
        PrematureActionPreventer(graph::GraphWithMeta &gwm, ProblemInstance const &problem,
                                 seconds_t allowed_early_departure = invalid<seconds_t>(),
                                 bool limit_early_passes = true);

        PrematureActionPreventer(graph::AlternativeGraph &graph, graph::Abstractions const &abstractions,
                                 graph::node_index_t start_node, ProblemInstance const &problem,
                                 seconds_t allowed_early_departure = invalid<seconds_t>(),
                                 bool limit_early_passes = true);


        void preventEarlyActions(graph::TrainAtStationAbstraction const &tas);

    private:
        graph::AlternativeGraph &graph_;
        graph::Abstractions const &abstractions_;
        graph::node_index_t start_node_;

        ProblemInstance const &problem_;
        seconds_t allowed_early_departure_;
        bool limit_early_passes_;
    };

}// namespace fb
