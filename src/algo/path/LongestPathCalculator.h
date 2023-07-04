
#pragma once

#include "algo/OnlineTopologicalSorter.h"
#include "definitions.h"
#include "model/graph/AlternativeGraph.h"
#include "model/graph/Selection.h"
#include "model/graph/definitions.h"


namespace fb {


    class LongestPathCalculator {
    public:
        LongestPathCalculator(const graph::AlternativeGraph &graph, graph::node_index_t start,
                              const std::vector<graph::node_index_t> &end)
            : graph_(graph), start_(start), needs_update_(graph.nodes.size(), false) {}


        struct State {
            std::vector<distance_t> distance_from_start;
            std::vector<graph::node_index_t> predecessor;

            /**
             * List of alternative edges, that are unselected and the source dsf was updated during the last distance update
             */
            std::vector<graph::a_edge_index_t> unselected_edges_with_updated_source;

            State() = default;

            explicit State(graph::AlternativeGraph const &graph)
                : distance_from_start(graph.nodes.size(), MIN_DISTANCE),
                  predecessor(graph.nodes.size(), invalid<graph::node_index_t>()) {}
        };

        /**
         * Shortcut method to calculate the order on the fly
         * @param selection selection of the edges
         * @return a distance state
         */
        State calculateInitialPaths(const graph::Selection &selection) {
            OnlineTopologicalSorter sorter{selection.getGraph()};
            return calculateInitialPaths(selection, sorter.calculateInitialTopologicalOrder(selection));
        }
        /**
         *
         * @param selection the selection of the alternative edges to use
         * @return the initial calculated state
         */
        State calculateInitialPaths(const graph::Selection &selection,
                                    OnlineTopologicalSorter::State const &sorter_state);
        /**
         *
         * @param selection the selection of the alternative edges
         * @param new_edges the new edges added im comparison to the last calculation
         */
        void updateBasedOnNewEdges(State &state, graph::Selection const &selection,
                                   OnlineTopologicalSorter::State const &sorter_state,
                                   std::vector<graph::a_edge_index_t> const &new_edges);

    private:
        bool relaxEdge(const graph::Edge &edge, State &state);

        unsigned int updateNode(const graph::Node &node, State &state, graph::Selection const &selection);

        // given data
        graph::AlternativeGraph const &graph_;
        graph::node_index_t start_;

        std::vector<bool> needs_update_;
    };
}// namespace fb
