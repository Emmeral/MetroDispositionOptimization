
#pragma once

#include "model/graph/AlternativeGraph.h"
#include "model/graph/abstractions/TrainAtStationAbstraction.h"
namespace fb {

    class TrackHeadwayNodeBuilder {
    public:
        typedef std::map<track_id_t, graph::node_index_t> hw_nodes_t;
        enum class Type { IN, OUT };

        explicit TrackHeadwayNodeBuilder(graph::AlternativeGraph &graph);

        template<Type TYPE>
        hw_nodes_t createHeadwayNodes(const graph::TrainAtStationAbstraction &abs);

        template<Type TYPE>
        TrackHeadwayNodeBuilder::hw_nodes_t
        createNodesForMultiTrack(node_index_t base, const std::vector<track_id_t> &considered_tracks,
                                 const std::vector<graph::choice_index_t> &track_choices);

    private:
        graph::AlternativeGraph &graph_;

        template<Type DIR>
        void connectNodes(node_index_t inner_node, node_index_t outer_node, graph::FullAlternativeIndex alt);
    };

}// namespace fb
