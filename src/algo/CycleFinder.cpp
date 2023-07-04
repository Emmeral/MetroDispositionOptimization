
#include "CycleFinder.h"

namespace fb {
    CycleFinder::Cycle CycleFinder::findCycle(const graph::AlternativeGraph &graph, const graph::Selection &selection) {

        std::vector<bool> visited(graph.nodes.size(), false);
        std::vector<bool> finished(graph.nodes.size(), false);
        for (auto const &node: graph.nodes) {
            if (!visited[node.index]) {
                auto cycle = dfs(graph, selection, node.index, visited, finished);
                if (!cycle.empty()) {
                    return cycle;
                }
            }
        }

        return Cycle{};
    }
    CycleFinder::Cycle CycleFinder::dfs(const graph::AlternativeGraph &graph, const graph::Selection &selection,
                                        graph::node_index_t start, std::vector<bool> &visited,
                                        std::vector<bool> &finished) {

        std::vector<graph::Node const *> nodes{};
        std::vector<int> edge_indexes{};

        visited[start] = true;

        nodes.push_back(&graph.nodes[start]);
        edge_indexes.push_back(-1);


        while (!nodes.empty()) {

            auto &node = *nodes.back();
            edge_indexes.back()++;
            auto const *next = graph.getNthOutgoingEdge(node.index, edge_indexes.back());

            if (next == nullptr) {
                finished[node.index] = true;
                nodes.pop_back();
                edge_indexes.pop_back();
                continue;
            }


            auto const &edge = *next;

            if (!edge.is_fixed && !selection.isSelectedEdge(edge.index)) {
                continue;
            }

            auto const &next_node = graph.nodes[edge.to];

            if (finished[next_node.index]) {
                continue;
            }
            if (visited[next_node.index]) {
                return buildFromStack(graph, nodes, edge_indexes, next_node.index);
            }


            visited[next_node.index] = true;
            nodes.push_back(&next_node);
            edge_indexes.push_back(-1);
        }
        return Cycle{};
    }
    CycleFinder::Cycle CycleFinder::buildFromStack(const graph::AlternativeGraph &graph,
                                                   std::vector<graph::Node const *> nodes,
                                                   std::vector<int> edge_indices, node_index_t start) {

        Cycle cycle{};
        bool started = false;

        for (auto i = 0; i < nodes.size(); i++) {
            auto &node = *nodes[i];
            started = started || node.index == start;
            if (!started) {
                continue;
            }
            auto *edge = graph.getNthOutgoingEdge(node.index, edge_indices[i]);

            cycle.nodes.push_back(&node);
            cycle.edges.push_back(edge);
        }
        return cycle;
    }
}// namespace fb
