
#include "OnlineTopologicalSorter.h"
#include <algorithm>
#include <queue>

namespace fb {

    OnlineTopologicalSorter::OnlineTopologicalSorter(const graph::AlternativeGraph &graph)
        : graph_(graph), flag_(graph.nodes.size(), false) {}


    OnlineTopologicalSorter::State
    OnlineTopologicalSorter::calculateInitialTopologicalOrder(const graph::Selection &selection) {

        State state{graph_};
        std::vector<bool> &visited = flag_;
        auto order_index = graph_.nodes.size() - 1;


        for (auto const &start: graph_.nodes) {
            if (visited[start.index]) {
                continue;
            }

            bool success = sortByDfs(selection, start.index, visited, order_index, state);
            if (!success) {
                // cycle;
                resetFlag();
                state.cycle = true;
                return std::move(state);
            }
        }

        // the flag should be all true now -> set it to false
        visited.flip();

        return std::move(state);
    }


    bool OnlineTopologicalSorter::sortByDfs(graph::Selection const &selection, graph::node_index_t start,
                                            std::vector<bool> &visited, size_t &order_index, State &state) {

        visited[start] = true;
        std::vector<NodeAndEdge> stack;
        stack.emplace_back(start, 0);


        while (!stack.empty()) {
            auto &current = stack.back();
            graph::Node const &node = graph_.nodes[current.node];

            if (current.edge_index >= node.totalOutgoingEdgeCount()) {
                state.sorted_nodes[order_index] = current.node;
                state.topological_order[current.node] = order_index;
                --order_index;
                stack.pop_back();
                continue;
            }

            graph::node_index_t next = getNextNode(current, selection);
            ++current.edge_index;

            if (isInvalid(next)) {
                continue;
            }

            if (visited[next] && !isValid(state.topological_order[next])) {
                // cycle;
                return false;
            }

            if (!visited[next]) {
                visited[next] = true;
                stack.emplace_back(next, 0);
            }
        }
        return true;
    }

    graph::node_index_t OnlineTopologicalSorter::getNextNode(const OnlineTopologicalSorter::NodeAndEdge &current,
                                                             const graph::Selection &selection) {

        graph::Node const &node = graph_.nodes[current.node];

        if (current.edge_index < node.outgoing_edges.size()) {
            return graph_.fixed_edges[node.outgoing_edges[current.edge_index]].to;
        }
        // use alternative edges, as most don't cause cycles and make updating the order later easier
        auto fixed_count = node.outgoing_edges.size();
        auto edge = node.outgoing_alternative_edges[current.edge_index - fixed_count];

        const graph::FullAlternativeIndex fai = graph_.alternative_edges_metadata[edge];
        graph::ChoiceType type = graph_.choice_metadata[fai.choice_index].type;

        // only use selected edges if there is a selection, otherwise use subset of alt edges that don't cause cycles
        if (selection.isEmpty()) {
            if (type == graph::ChoiceType::TRACK_HEADWAY || type == graph::ChoiceType::LINE_HEADWAY ||
                type == graph::ChoiceType::TRACK) {
                return invalid<node_index_t>();
            }
        } else {
            if (!selection.isSelectedEdge(edge)) {
                return invalid<node_index_t>();
            }
        }
        return graph_.alternative_edges[edge].to;
    }
    void OnlineTopologicalSorter::updateTopologicalOrder(State &state, const graph::Selection &selection,
                                                         const std::vector<graph::a_edge_index_t> &new_edges) {


        std::vector<FrontierPair> shift_queue{};
        std::vector<graph::Edge const *> invalidating{};

        for (auto const index: new_edges) {
            auto const &edge = graph_.alternative_edges[index];
            if (state.topological_order[edge.from] > state.topological_order[edge.to]) {
                invalidating.push_back(&edge);
            }
        }
        // return if the order is still valid
        if (invalidating.empty()) {
            return;
        }
        // sort descending by tail
        std::ranges::sort(invalidating, std::ranges::greater(),
                          [&](auto const *e) { return state.topological_order[e->from]; });

        auto lower_bound = invalid<order_t>();
        auto region_start = 0;


        for (auto i = 0; i < invalidating.size(); ++i) {

            auto const *edge = invalidating[i];
            if (isValid(lower_bound) && state.topological_order[edge->from] < lower_bound) {

                std::span<graph::Edge const *> region_subspan =
                        std::span(invalidating).subspan(region_start, i - region_start);
                bool cycle = !discover(state, selection, shift_queue, region_subspan);
                if (cycle) {
                    resetFlag();
                    state.cycle = true;
                    return;
                }
                shift(state, shift_queue, lower_bound);
                region_start = i;
            }
            lower_bound = std::min(lower_bound, state.topological_order[edge->to]);
        }

        std::span<graph::Edge const *> span = std::span(invalidating).subspan(region_start);
        bool cycle = !discover(state, selection, shift_queue, span);
        if (cycle) {
            resetFlag();
            state.cycle = true;
            return;
        }
        // discover final region
        shift(state, shift_queue, lower_bound);

        // assert(std::ranges::none_of(flag_, std::identity()));
    }
    /**
     *
     * @param queue
     * @param overlapping_inv_edges
     * @return false if a cycle was found
     */
    bool OnlineTopologicalSorter::discover(State const &state, graph::Selection const &selection,
                                           std::vector<FrontierPair> &queue,
                                           std::span<const graph::Edge *> overlapping_inv_edges) {

        std::vector<bool> &vacant = flag_;

        std::vector<bool> on_stack(graph_.nodes.size(), false);
        std::vector<NodeAndEdge> stack;

        order_t upper_bound;

        for (auto const &inv_edge: overlapping_inv_edges) {

            if (!vacant[inv_edge->to]) {
                on_stack[inv_edge->from] = true;// this line is not in the paper
                stack.emplace_back(inv_edge->to, 0);
                on_stack[inv_edge->to] = true;
                vacant[inv_edge->to] = true;
                upper_bound = state.topological_order[inv_edge->from];
            }

            while (!stack.empty()) {

                NodeAndEdge &current = stack.back();
                graph::Node const &node = graph_.nodes[current.node];

                if (current.edge_index >= node.totalOutgoingEdgeCount()) {
                    on_stack[node.index] = false;

                    queue.push_back({node.index, state.sorted_nodes[upper_bound]});
                    stack.pop_back();

                    continue;
                }

                graph::node_index_t next;

                if (current.edge_index < node.outgoing_edges.size()) {
                    next = graph_.fixed_edges[node.outgoing_edges[current.edge_index]].to;
                } else {
                    auto fixed_count = node.outgoing_edges.size();
                    auto edge = node.outgoing_alternative_edges[current.edge_index - fixed_count];
                    if (selection.isSelectedEdge(edge)) {
                        next = graph_.alternative_edges[edge].to;
                    } else {
                        next = invalid<graph::node_index_t>();
                        // PENDING: maybe allow for stop/pass, track edges as they don't cause cycles anyway and
                        // must be updated later if selected. However, if they will not be selected in the future
                        // we have unecessary work here
                    }
                }

                ++current.edge_index;
                if (isInvalid(next)) {
                    continue;
                }

                if (on_stack[next]) {
                    // cycle
                    return false;
                }
                if (!vacant[next] && state.topological_order[next] < upper_bound) {
                    vacant[next] = true;
                    on_stack[next] = true;
                    stack.emplace_back(next, 0);
                }
            }
            on_stack[inv_edge->from] = false;
        }
        return true;
    }
    void OnlineTopologicalSorter::shift(State &state, std::vector<FrontierPair> &shift_queue, order_t leftmost) {


        auto &vacant = flag_;
        auto n = 0;
        auto i = leftmost;

        while (!shift_queue.empty()) {
            auto w = state.sorted_nodes[i];
            if (vacant[w]) {
                n = n + 1;
                vacant[w] = false;
            } else {
                state.topological_order[w] = i - n;
                state.sorted_nodes[i - n] = w;
            }
            FrontierPair pair{};
            while (!shift_queue.empty() && w == shift_queue.back().destination) {
                pair = shift_queue.back();
                shift_queue.pop_back();
                n = n - 1;

                state.topological_order[pair.node] = i - n;
                state.sorted_nodes[i - n] = pair.node;
            }
            ++i;
        }
    }
    void OnlineTopologicalSorter::resetFlag() {
        for (auto i = 0; i < flag_.size(); ++i) {
            flag_[i] = false;
        }
    }


}// namespace fb
