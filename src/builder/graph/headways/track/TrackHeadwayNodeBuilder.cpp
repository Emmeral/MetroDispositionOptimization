
#include "TrackHeadwayNodeBuilder.h"
#include "model/graph/abstractions/abstract/MultiTrackAbstraction.h"
#include "model/graph/abstractions/abstract/SingleTrackAbstraction.h"
#include "util.h"

namespace fb {
    TrackHeadwayNodeBuilder::TrackHeadwayNodeBuilder(graph::AlternativeGraph &graph) : graph_(graph) {}


    template<TrackHeadwayNodeBuilder::Type TYPE>
    TrackHeadwayNodeBuilder::hw_nodes_t
    TrackHeadwayNodeBuilder::createHeadwayNodes(const graph::TrainAtStationAbstraction &abs) {
        auto nodes = std::map<track_id_t, graph::node_index_t>{};

        auto base = TYPE == Type::OUT ? abs.getDepartureTimeOutNode() : abs.getArrivalTimeOutNode();

        auto track_count = abs.consideredTrackCount();
        if (track_count == 1) {
            auto const &single_track = dynamic_cast<graph::SingleTrackAbstraction const &>(abs);
            auto track = single_track.consideredTrack();
            nodes[track] = base;
            return std::move(nodes);
        }

        auto const &multi_track = dynamic_cast<graph::MultiTrackAbstraction const &>(abs);
        auto const &considered_tracks = multi_track.consideredTracksAsVec();
        std::vector<graph::choice_index_t> const &track_choices = *abs.getTrackChoices();


        return createNodesForMultiTrack<TYPE>(base, considered_tracks, track_choices);
    }


    template<TrackHeadwayNodeBuilder::Type TYPE>
    TrackHeadwayNodeBuilder::hw_nodes_t
    TrackHeadwayNodeBuilder::createNodesForMultiTrack(node_index_t base,
                                                      const std::vector<track_id_t> &considered_tracks,
                                                      const std::vector<graph::choice_index_t> &track_choices) {

        auto nodes = hw_nodes_t{};

        std::vector<graph::node_index_t> created_nodes{};
        auto track_count = considered_tracks.size();
        auto track_guard_amount = std::bit_width(track_count - 1);

        int size_of_bin_tree_with_track_count_leafs = track_count * 2 - 1;
        int size_of_complete_binary_tree = (0b1 << track_guard_amount) * 2 - 1;// 2 * 2^h -1
        created_nodes.resize(size_of_complete_binary_tree, invalid<node_index_t>());
        created_nodes[0] = base;

        auto j = 0u;
        for (auto tree_size = 1; tree_size < size_of_bin_tree_with_track_count_leafs; tree_size += 2) {

            auto i = std::bit_width(j + 1) - 1;           // i = log_2(j +1)
            auto index = reverseLowestNBits(j + 1, i) - 1;// we create only nodes that are necessary
            auto current = created_nodes[index];
            auto one = created_nodes[index * 2 + 1] = graph_.createNode();// child nodes in the tree
            auto two = created_nodes[index * 2 + 2] = graph_.createNode();
            connectNodes<TYPE>(current, one, {track_choices[i], graph::AlternativeIndex::FIRST});
            connectNodes<TYPE>(current, two, {track_choices[i], graph::AlternativeIndex::SECOND});

            ++j;
        }

        for (auto internal_index = 0; internal_index < considered_tracks.size(); internal_index++) {
            auto track_id = considered_tracks[internal_index];

            node_index_t node_index = (0b1 << track_guard_amount) - 1;
            node_index += reverseLowestNBits(internal_index, track_guard_amount);

            if (isInvalid(created_nodes[node_index])) {
                node_index = ((node_index - 1) / 2);// go one level down the tree
            }

            nodes[track_id] = created_nodes[node_index];
        }


        return std::move(nodes);
    }


    template<TrackHeadwayNodeBuilder::Type DIR>
    void TrackHeadwayNodeBuilder::connectNodes(graph::node_index_t inner_node, graph::node_index_t outer_node,
                                               graph::FullAlternativeIndex alt) {

        auto first = DIR == Type::OUT ? inner_node : outer_node;
        auto second = DIR == Type::OUT ? outer_node : inner_node;
        graph_.createAlternativeEdge(first, second, alt);
    }


    template TrackHeadwayNodeBuilder::hw_nodes_t
    TrackHeadwayNodeBuilder::createHeadwayNodes<TrackHeadwayNodeBuilder::Type::OUT>(
            const graph::TrainAtStationAbstraction &abs);
    template TrackHeadwayNodeBuilder::hw_nodes_t
    TrackHeadwayNodeBuilder::createHeadwayNodes<TrackHeadwayNodeBuilder::Type::IN>(
            const graph::TrainAtStationAbstraction &abs);
}// namespace fb
