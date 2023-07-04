
#pragma once

#include "GraphDotWriter.h"
#include "algo/path/LongestPathCalculator.h"
#include "model/graph/GraphWithMeta.h"
#include "model/graph/Selection.h"
#include <string>
namespace fb {

    class SubgraphWriter {


    public:
        static void writeSubgraphToDot(const graph::GraphWithMeta &graph_meta, std::string const &filename,
                                       const graph::Selection *sel, std::vector<graph::node_index_t> const &nodes) {


            auto node_filter = [&](graph::Node const &node) {
                return std::find(nodes.begin(), nodes.end(), node.index) != nodes.end();
            };

            auto edge_filter = [&](graph::Edge const &edge) {
                return std::find(nodes.begin(), nodes.end(), edge.from) != nodes.end() &&
                       std::find(nodes.begin(), nodes.end(), edge.to) != nodes.end();
            };


            std::vector<std::string> name_hints{};

            GraphDotWriter::writeToDot(graph_meta, name_hints, filename, true, sel, node_filter, edge_filter);
        }

        static void writePathToNodeToDot(const graph::GraphWithMeta &gwm, std::string const &filename,
                                         graph::Selection const &sel, node_index_t target) {

            LongestPathCalculator path_calculator(gwm.graph, gwm.start_node, {target});

            auto pc_state = path_calculator.calculateInitialPaths(sel);

            std::vector<node_index_t> path{};
            path.push_back(target);

            while (pc_state.predecessor[path.back()] != invalid<node_index_t>()) {
                path.push_back(pc_state.predecessor[path.back()]);
            }

            auto node_filter = [&](graph::Node const &node) {
                return std::find(path.begin(), path.end(), node.index) != path.end();
            };

            auto edge_filter = [&](graph::Edge const &edge) {
                return std::find(path.begin(), path.end(), edge.from) != path.end() &&
                       std::find(path.begin(), path.end(), edge.to) != path.end();
            };

            std::vector<std::string> name_hints{};
            name_hints.resize(gwm.graph.nodes.size());
            for (auto const &n: path) {
                name_hints[n] = std::to_string(pc_state.distance_from_start[n]);
            }


            GraphDotWriter::writeToDot(gwm, name_hints, filename, false, &sel, node_filter, edge_filter);
        }
    };

}// namespace fb
