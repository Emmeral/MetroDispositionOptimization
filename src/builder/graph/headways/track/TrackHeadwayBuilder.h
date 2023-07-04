
#pragma once

#include "TrackHeadwayNodeBuilder.h"
#include "builder/graph/headways/HeadwayBuildResult.h"
#include "model/graph/AlternativeGraph.h"
#include "model/graph/GraphWithMeta.h"
#include "model/graph/abstractions/TrainAtStationAbstraction.h"
#include "options/GraphBuilderOptions.h"
namespace fb {

    class TrackHeadwayBuilder {

    public:
        TrackHeadwayBuilder(graph::GraphWithMeta &gwm, const GraphBuilderOptions &options = GraphBuilderOptions())
            : TrackHeadwayBuilder(gwm.graph, gwm.abstractions, gwm.no_constrain_distances, options) {}
        TrackHeadwayBuilder(graph::AlternativeGraph &graph, graph::Abstractions const &abstractions,
                            std::vector<distance_t> const &no_constrain_distances_,
                            const GraphBuilderOptions &options = GraphBuilderOptions())
            : graph_(graph), node_builder_(graph), abstractions_{abstractions}, cached_nodes_(abstractions.size()),
              no_constrain_distances_(no_constrain_distances_), options_(options) {}

        HeadwayBuildResult build(const graph::TrainAtStationAbstraction &abs1,
                                 const graph::TrainAtStationAbstraction &abs2);

    private:
        struct CachedHeadwayNodes {
            bool exists{false};
            std::map<track_id_t, graph::node_index_t> in_nodes_{};
            std::map<track_id_t, graph::node_index_t> out_nodes_{};
        };

        TrackHeadwayNodeBuilder node_builder_;
        graph::AlternativeGraph &graph_;
        graph::Abstractions const &abstractions_;
        std::vector<distance_t> const &no_constrain_distances_;

        std::vector<CachedHeadwayNodes> cached_nodes_;

        GraphBuilderOptions const &options_;

        void createHeadwayEdges(const graph::TrainAtStationAbstraction &abs1,
                                const graph::TrainAtStationAbstraction &abs2, track_id_t id,
                                const graph::FullAlternativeIndex &alternative);

        const CachedHeadwayNodes &getOrCreateHeadwayNodes(const graph::TrainAtStationAbstraction &tas);
        graph::choice_index_t createChoice(const graph::TrainAtStationAbstraction &abs1,
                                           const graph::TrainAtStationAbstraction &abs2, int diff);
    };

}// namespace fb
