
#include "LongestPathCalculator.h"
#include "algo/bab/states/State.h"
#include "model/main/Model.h"
#include <algorithm>

namespace fb {


    LongestPathCalculator::State
    LongestPathCalculator::calculateInitialPaths(const graph::Selection &selection,
                                                 OnlineTopologicalSorter::State const &sorter_state) {

        State state{graph_};
        if (sorter_state.cycle) {
            return std::move(state);
        }

        state.distance_from_start[start_] = 0;
        needs_update_[start_] = true;

        // drop nodes before start, as they can not be reached anyway
        for (auto node_id: sorter_state.sorted_nodes | std::views::drop(sorter_state.topological_order[start_])) {
            graph::Node const &node = graph_.nodes[node_id];

            if (!needs_update_[node_id]) {
                continue;
            }
            updateNode(node, state, selection);
        }
        // assert(std::ranges::none_of(needs_update_, std::identity()));
        return std::move(state);
    }
    unsigned int LongestPathCalculator::updateNode(const graph::Node &node, State &state,
                                                   graph::Selection const &selection) {

        auto new_nodes_to_update = 0;

        if (state.distance_from_start[node.index] == MIN_DISTANCE) {
            // there cannot be an improvement from an unreachable node
            needs_update_[node.index] = false;
            return 0;
        }

        for (auto edge_i: node.outgoing_edges) {
            graph::Edge edge = graph_.fixed_edges[edge_i];
            bool relaxed = relaxEdge(edge, state);
            if (relaxed && !needs_update_[edge.to]) {
                needs_update_[edge.to] = true;
                ++new_nodes_to_update;
            }
        }

        for (auto edge_i: node.outgoing_alternative_edges) {
            if (selection.isSelectedEdge(edge_i)) {
                graph::Edge edge = graph_.alternative_edges[edge_i];
                bool relaxed = relaxEdge(edge, state);
                if (relaxed && !needs_update_[edge.to]) {
                    needs_update_[edge.to] = true;
                    ++new_nodes_to_update;
                }
            } else if (!selection.isDenied(graph_.alternative_edges_metadata[edge_i])) {
                state.unselected_edges_with_updated_source.emplace_back(edge_i);
            }
        }
        needs_update_[node.index] = false;
        return new_nodes_to_update;
    }

    bool LongestPathCalculator::relaxEdge(const graph::Edge &edge, State &state) {

        auto &current_distance = state.distance_from_start[edge.from];
        auto &next_distance = state.distance_from_start[edge.to];

        auto potential_update = current_distance + graph_.getEdgeWeight(edge, current_distance);
        if (potential_update > next_distance) {
            next_distance = potential_update;
            state.predecessor[edge.to] = edge.from;
            return true;
        }
        return false;
    }
    void LongestPathCalculator::updateBasedOnNewEdges(State &state, graph::Selection const &selection,
                                                      OnlineTopologicalSorter::State const &sorter_state,
                                                      std::vector<graph::a_edge_index_t> const &new_edges) {


        if (sorter_state.cycle) {
            // we have a cycle in the graph, so we abort here
            return;
        }
        auto const &order = sorter_state.topological_order;

        state.unselected_edges_with_updated_source.clear();// at the beginning no source has been updated

        std::vector<const graph::Edge *> edges;
        auto min_order = invalid<order_t>();
        unsigned int count_to_update = 0;

        for (auto i: new_edges) {
            auto const &edge = graph_.alternative_edges[i];

            if (state.distance_from_start[edge.from] == MIN_DISTANCE) {
                continue;// don't update from unreachable
            }

            bool relaxed = relaxEdge(edge, state);
            if (!relaxed) {
                continue;// not relevant as it did not change anything
            }
            if (order[edge.to] < min_order) {
                min_order = order[edge.to];
            }
            if (!needs_update_[edge.to]) {
                needs_update_[edge.to] = true;
                ++count_to_update;
            }
        }

        // start with the lowest edge.to potentially affected
        for (auto const &node_id: sorter_state.sorted_nodes | std::views::drop(min_order)) {
            if (!needs_update_[node_id]) {
                continue;
            }
            auto new_nodes_to_update = updateNode(graph_.nodes[node_id], state, selection);
            count_to_update += new_nodes_to_update;// add new nodes that now have a increased distance from the start
            --count_to_update;                     // updated the current node

            if (count_to_update == 0) {
                // we don't have to continue the loop if no update will happen anymore
                break;
            }
        }

        // make sure that we don't change the state
        //assert(std::ranges::none_of(needs_update_, std::identity()));
    }


}// namespace fb
