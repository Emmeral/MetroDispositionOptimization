
#pragma once

#include "model/graph/AlternativeGraph.h"
#include "model/graph/abstractions/TrainAtStationAbstraction.h"
#include "model/main/Model.h"
#include "model/problems/ProblemInstance.h"
namespace fb {

    class AbstractionLinker {

    public:
        explicit AbstractionLinker(
                graph::AlternativeGraph &graph,
                std::map<link_index_t, std::vector<ExtendedRunTime *>> const &extended_runtimes = {});


        void link(graph::TrainAtStationAbstraction &first, graph::TrainAtStationAbstraction &second, const Link &link);

    private:
        graph::AlternativeGraph &graph_;
        std::map<link_index_t, std::vector<ExtendedRunTime *>> const &extended_runtimes_;
    };

}// namespace fb
