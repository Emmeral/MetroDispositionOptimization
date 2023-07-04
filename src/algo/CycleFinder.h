
#pragma once
#include "model/graph/AlternativeGraph.h"
#include "model/graph/Selection.h"
#include "vector"
namespace fb {

    class CycleFinder {

        struct Cycle {
            std::vector<graph::Node const *> nodes{};
            std::vector<graph::Edge const *> edges{};

            bool empty() const { return nodes.empty(); }
        };


    public:
        Cycle findCycle(const graph::AlternativeGraph &graph, const graph::Selection &selection);

    private:
        Cycle dfs(const graph::AlternativeGraph &graph, const graph::Selection &selection, graph::node_index_t start,
                  std::vector<bool> &visited, std::vector<bool> &finished);

        Cycle buildFromStack(const graph::AlternativeGraph &graph, std::vector<graph::Node const *> nodes,
                             std::vector<int> edge_indices, node_index_t start);
    };

}// namespace fb
