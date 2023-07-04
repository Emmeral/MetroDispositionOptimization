
#pragma once


#include "model/graph/AlternativeGraph.h"
#include "model/graph/Selection.h"
#include "model/graph/definitions.h"
#include <queue>
#include <span>
#include <vector>

namespace fb {
    typedef graph::node_index_t order_t;

    // taken from "A Batch Algorithm for Maintaining a Topological Order"
    class OnlineTopologicalSorter {


    public:
        struct State {
            bool cycle;
            /**
            * order of node with id x is topological_order_[x]
            */
            std::vector<order_t> topological_order;
            /**
            * sorted_nodes[i] = node index of node with order i
            */
            std::vector<graph::node_index_t> sorted_nodes;

            State() = default;
            explicit State(graph::AlternativeGraph const &graph)
                : cycle{false}, topological_order(graph.nodes.size(), invalid<order_t>()),
                  sorted_nodes(graph.nodes.size(), invalid<graph::node_index_t>()) {}
        };

        explicit OnlineTopologicalSorter(const graph::AlternativeGraph &graph);
        /**
         *
         * @return true if a valid order could be constructed
         */
        State calculateInitialTopologicalOrder(const graph::Selection &selection);
        /**
         *
         * @param new_edges
         * @param selection
         * @return true if a valid order could be constructed
         */
        void updateTopologicalOrder(State &state, const graph::Selection &selection,
                                    const std::vector<graph::a_edge_index_t> &new_edges);


    private:
        struct NodeAndEdge {

            NodeAndEdge(graph::node_index_t node, size_t edge_index) : node(node), edge_index(edge_index){};
            explicit NodeAndEdge(graph::node_index_t node) : NodeAndEdge(node, 0) {}

            graph::node_index_t node;
            size_t edge_index;
        };

        struct FrontierPair {
            graph::node_index_t node;
            graph::node_index_t destination;
        };

        /**
         *
         * @param start
         * @param visited
         * @return true if the search was a success, false if a cycle was found
         */
        bool sortByDfs(graph::Selection const &selection, graph::node_index_t start, std::vector<bool> &visited,
                       size_t &order_index, State &state);

        graph::node_index_t getNextNode(NodeAndEdge const &current, graph::Selection const &selection);

        bool discover(State const &state, graph::Selection const &selection, std::vector<FrontierPair> &queue,
                      std::span<const graph::Edge *> overlapping_inv_edges);
        void shift(State &state, std::vector<FrontierPair> &shift_queue, order_t leftmost);

        void resetFlag();

        graph::AlternativeGraph const &graph_;
        std::vector<bool> flag_;
    };
}// namespace fb
