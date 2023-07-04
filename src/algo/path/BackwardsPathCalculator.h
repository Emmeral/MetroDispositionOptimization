
#pragma once

#include "algo/OnlineTopologicalSorter.h"
#include "algo/bab/states/State.h"
#include "definitions.h"
#include "model/graph/AlternativeGraph.h"
#include "model/graph/Selection.h"
#include "model/graph/definitions.h"
#include <numeric>
#include <vector>
namespace fb {


    struct DistDestPair {
        distance_t dist{MIN_DISTANCE};
        graph::node_index_t dest{invalid<node_index_t>()};

        bool isValid() const { return fb::isValid(dest); }

        bool operator==(DistDestPair const &other) const { return other.dist == dist && other.dest == dest; }
    };
    template<size_t N>
    struct BackwardsPathState {
        static constexpr auto BACKWARDS_DISTANCES_COUNT = N;

        std::vector<std::array<DistDestPair, N>> distances_to_end{};

        std::vector<distance_t> const *distance_to_end_bias{nullptr};
        /**
        * List of alternative edges, that are unselected and the target dsb was updated during the last distance update
        */
        std::vector<graph::a_edge_index_t> unselected_edges_with_updated_target{};
    };


    class BackwardsPathCalculator {

    public:
        BackwardsPathCalculator(const graph::AlternativeGraph &graph, graph::node_index_t start,
                                const std::vector<graph::node_index_t> &end, const std::vector<distance_t> &bias = {})
            : graph_(graph), start_(start), end_(end), needs_update_(graph.nodes.size(), false), bias_(bias) {
            // add bias for all end nodes
            if (bias_.size() < end_.size()) {
                bias_.resize(end_.size(), 0);
            }
        }

        template<size_t N>
        BackwardsPathState<N> calculateInitialPaths(const graph::Selection &selection,
                                                    OnlineTopologicalSorter::State const &order_state,
                                                    std::vector<distance_t> const &dist_from_start = {});
        /**
         *
         * @param selection the selection of the alternative edges
         * @param new_edges the new edges added im comparison to the last calculation
         */
        template<size_t N>
        void updateBasedOnNewEdges(BackwardsPathState<N> &state, graph::Selection const &selection,
                                   OnlineTopologicalSorter::State const &order_state,
                                   std::vector<graph::a_edge_index_t> const &new_edges,
                                   std::vector<distance_t> const &dist_from_start = {});


        /**
         * Merges two sorted arrays into one. Chooses the greatest N elements. Does not use the same distance twice
         * @tparam N size of the arrays
         * @param first first array
         * @param second second array
         * @param destination merged array output parameter
         * @return whether an element from the second array was used
         */
        template<size_t N>
        static bool merge(std::array<DistDestPair, N> const &first, std::array<DistDestPair, N> second,
                          std::array<DistDestPair, N> &destination);

    private:
        template<size_t N>
        bool relaxEdge(const graph::Edge &edge, BackwardsPathState<N> &state,
                       std::vector<distance_t> const &dist_from_start);


        template<size_t N>
        unsigned int updateNode(const graph::Node &node, BackwardsPathState<N> &state,
                                graph::Selection const &selection, std::vector<distance_t> const &dist_from_start);


        // given data
        graph::AlternativeGraph const &graph_;

        graph::node_index_t start_;
        std::vector<graph::node_index_t> end_;
        std::vector<distance_t> bias_;

        std::vector<bool> needs_update_;
    };

    template<>
    bool BackwardsPathCalculator::relaxEdge<1>(const graph::Edge &edge, BackwardsPathState<1> &state,
                                               std::vector<distance_t> const &dist_from_start);


    template<size_t N>
    bool BackwardsPathCalculator::merge(const std::array<DistDestPair, N> &first, std::array<DistDestPair, N> second,
                                        std::array<DistDestPair, N> &destination) {

        bool took_from_second = false;

        std::array<bool, N> first_valid;
        first_valid.fill(true);
        std::array<bool, N> second_valid = first_valid;

        auto first_index = 0;
        auto second_index = 0;
        auto dest_index = 0;

        while (dest_index < N && (first_index < N || second_index < N)) {


            std::array<DistDestPair, N> const *to_prune;
            std::array<bool, N> *to_prune_valid;
            auto *to_prune_index = &first_index;


            if (second_index == N || first[first_index].dist >= second[second_index].dist) {
                destination[dest_index] = first[first_index];
                first_index++;


                to_prune = &second;
                to_prune_valid = &second_valid;
                to_prune_index = &second_index;
            } else {
                destination[dest_index] = second[second_index];
                second_index++;
                took_from_second = true;

                to_prune = &first;
                to_prune_valid = &first_valid;
                to_prune_index = &first_index;
            }
            node_index_t chosen = destination[dest_index].dest;
            ++dest_index;

            // only need to prune elements that can potentially be selected
            auto bound = std::min(N, *to_prune_index + (N - dest_index));
            for (auto i = *to_prune_index; i < bound; ++i) {
                if (to_prune->at(i).dest == chosen) {
                    to_prune_valid->at(i) = false;
                }
            }

            while (first_index < N && !first_valid[first_index]) {
                ++first_index;
            }
            while (second_index < N && !second_valid[second_index]) {
                ++second_index;
            }
        }

        return took_from_second;
    }


    template<size_t N>
    bool BackwardsPathCalculator::relaxEdge(const graph::Edge &edge, BackwardsPathState<N> &state,
                                            std::vector<distance_t> const &dist_from_start) {


        auto &at_node = state.distances_to_end[edge.to];
        auto &at_prior = state.distances_to_end[edge.from];


        auto from_start = 0;
        if (dist_from_start.size() > edge.from) {
            from_start = dist_from_start[edge.from];
        }
        // edge weight is based on distance to start
        auto edge_weight = graph_.getReverseEdgeWeight(edge, from_start);


        std::array<DistDestPair, N> prior_copy = at_prior;
        std::array<DistDestPair, N> updated = at_node;

        for (auto i = 0; i < N; ++i) {

            if (!at_node[i].isValid()) {
                break;
            }
            updated[i].dist = at_node[i].dist + edge_weight;
        }

        // both arrays are sorted -> use adapted merge from merge sort to keep this + uniqueness
        bool relaxed = merge(prior_copy, updated, at_prior);

        return relaxed;
    }


    template<size_t N>
    unsigned int BackwardsPathCalculator::updateNode(const graph::Node &node, BackwardsPathState<N> &state,
                                                     const graph::Selection &selection,
                                                     std::vector<distance_t> const &dist_from_start) {
        auto new_nodes_to_update = 0;

        auto &at_node = state.distances_to_end[node.index];
        if (std::ranges::none_of(at_node, [](auto const &e) { return e.isValid(); })) {
            // there cannot be an improvement from an unreachable node
            needs_update_[node.index] = false;
            return 0;
        }

        for (auto edge_i: node.incoming_edges) {
            graph::Edge edge = graph_.fixed_edges[edge_i];
            bool relaxed = relaxEdge(edge, state, dist_from_start);
            if (relaxed && !needs_update_[edge.from]) {
                needs_update_[edge.from] = true;
                ++new_nodes_to_update;
            }
        }

        for (auto edge_i: node.incoming_alternative_edges) {
            if (selection.isSelectedEdge(edge_i)) {
                graph::Edge edge = graph_.alternative_edges[edge_i];
                bool relaxed = relaxEdge(edge, state, dist_from_start);
                if (relaxed && !needs_update_[edge.from]) {
                    needs_update_[edge.from] = true;
                    ++new_nodes_to_update;
                }
            } else if (!selection.isDenied(graph_.alternative_edges_metadata[edge_i])) {
                state.unselected_edges_with_updated_target.emplace_back(edge_i);
            }
        }
        needs_update_[node.index] = false;
        return new_nodes_to_update;
    }

    template<size_t N>
    BackwardsPathState<N>
    BackwardsPathCalculator::calculateInitialPaths(const graph::Selection &selection,
                                                   const OnlineTopologicalSorter::State &order_state,
                                                   std::vector<distance_t> const &dist_from_start) {

        BackwardsPathState<N> state{};
        state.distance_to_end_bias = &bias_;
        state.distances_to_end.resize(graph_.nodes.size());


        auto max_end_order = 0u;
        for (auto i = 0; i < end_.size(); ++i) {
            auto end = end_[i];
            std::array<DistDestPair, N> &at_node = state.distances_to_end[end];
            at_node[0].dist = bias_[i];
            at_node[0].dest = end;
            needs_update_[end] = true;
            max_end_order = std::max(max_end_order, order_state.topological_order[end]);
        }

        auto dist_to_end = order_state.sorted_nodes.size() - 1 - max_end_order;
        // drop nodes before start, as they can not be reached anyway
        for (auto node_id: order_state.sorted_nodes | std::views::reverse | std::views::drop(dist_to_end)) {
            graph::Node const &node = graph_.nodes[node_id];

            if (!needs_update_[node_id]) {
                continue;
            }
            updateNode(node, state, selection, dist_from_start);
        }
        // assert(std::ranges::none_of(needs_update_, std::identity()));
        return std::move(state);
    }


    template<size_t N>
    void BackwardsPathCalculator::updateBasedOnNewEdges(BackwardsPathState<N> &state, const graph::Selection &selection,
                                                        const OnlineTopologicalSorter::State &order_state,
                                                        const std::vector<graph::a_edge_index_t> &new_edges,
                                                        std::vector<distance_t> const &dist_from_start) {

        if (order_state.cycle) {
            // we have a cycle in the graph, so we abort here
            return;
        }
        auto const &order = order_state.topological_order;

        state.unselected_edges_with_updated_target.clear();

        auto max_order = 0u;
        unsigned int count_to_update = 0;

        for (auto i: new_edges) {
            auto const &edge = graph_.alternative_edges[i];

            auto &at_node = state.distances_to_end[edge.to];
            if (std::ranges::none_of(at_node, [](auto const &e) { return e.isValid(); })) {
                continue;// don't update from nodes with invalid distances;
            }

            relaxEdge(edge, state, dist_from_start);
            if (order[edge.from] > max_order) {
                max_order = order[edge.from];
            }
            if (!needs_update_[edge.from]) {
                needs_update_[edge.from] = true;
                ++count_to_update;
            }
        }

        // start with the highest edge.from potentially affected
        auto number_to_drop = order_state.sorted_nodes.size() - 1 - max_order;
        for (auto const &node_id: order_state.sorted_nodes | std::views::reverse | std::views::drop(number_to_drop)) {
            if (!needs_update_[node_id]) {
                continue;
            }
            auto new_nodes_to_update = updateNode(graph_.nodes[node_id], state, selection, dist_from_start);
            count_to_update += new_nodes_to_update;// add new nodes that now have a increased distance from the start
            --count_to_update;                     // updated the current node

            if (count_to_update == 0) {
                // we don't have to continue the loop if no update will happen anymore
                break;
            }
        }

        // make sure that we don't change the state
        // assert(std::ranges::none_of(needs_update_, std::identity()));
    }

}// namespace fb
