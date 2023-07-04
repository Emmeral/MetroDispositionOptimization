
#pragma once

#include "DirectionMappedHeadways.h"
#include "HeadwaysCollection.h"
#include "model/graph/AlternativeGraph.h"
#include "model/graph/GraphWithMeta.h"
#include "options/GraphBuilderOptions.h"
#include <vector>
namespace fb {

    class StaticImplicationBuilder {


    public:
        StaticImplicationBuilder(Model const &model, graph::AlternativeGraph &graph,
                                 const graph::Abstractions &abstractions, HeadwayCollection const &headways,
                                 GraphBuilderOptions const &options = GraphBuilderOptions());
        /**
         * Adds static implication with the semantic : "Trains entering first must leave first"
         * @param node_id
         * @param dir
         */
        void addStaticImplicationsAtNodeInDirection(node_index_t node_id, Direction dir);
        /**
         * Adds static implications with the semantic:
         * "If a common from the other direction is first at node, it also was first at the next node"
         * @param node_id
         */
        void addMixedDirStaticImplications(node_index_t node_id);


    private:
        Model const &model_;
        graph::AlternativeGraph &graph_;
        graph::Abstractions const &abstractions_;

        HeadwayCollection const &headways_;

        GraphBuilderOptions const &options_;

        void addImplicationToPriorLine(node_index_t node_id, graph::choice_index_t station_choice_index,
                                       const graph::TrainAtStationAbstraction &abs1,
                                       const graph::TrainAtStationAbstraction &abs2);
        void addImplicationToNextLine(node_index_t node_id, graph::choice_index_t station_choice_index,
                                      const graph::TrainAtStationAbstraction &abs1,
                                      const graph::TrainAtStationAbstraction &abs2);
    };

}// namespace fb
