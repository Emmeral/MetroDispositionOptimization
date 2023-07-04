
#pragma once

#include "model/graph/AlternativeGraph.h"
#include "model/graph/GraphWithMeta.h"
#include "model/graph/Selection.h"
#include <functional>
namespace fb {


    class GraphDotWriter {


    public:
        static std::string writeToDot(const graph::GraphWithMeta &graph_meta,
                                      std::vector<std::string> const &node_name_hints, bool ignore_headways = false,
                                      const graph::Selection *sel = nullptr,
                                      std::function<bool(graph::Node const &)> const &node_filter = NO_NODE_FILTER,
                                      std::function<bool(graph::Edge const &)> const &edge_filter = NO_EDGE_FILTER);
        static void writeToDot(const graph::GraphWithMeta &graph_meta, std::vector<std::string> const &node_name_hints,
                               const std::string &filename, bool ignore_headways, const graph::Selection *sel = nullptr,
                               std::function<bool(graph::Node const &)> const &node_filter = NO_NODE_FILTER,
                               std::function<bool(graph::Edge const &)> const &edge_filter = NO_EDGE_FILTER);

        template<typename STREAM>
        static void writeToDot(STREAM &stream, const graph::GraphWithMeta &graph_meta,
                               std::vector<std::string> const &node_name_hints, bool ignore_headways = false,
                               const graph::Selection *sel = nullptr,
                               std::function<bool(graph::Node const &)> const &node_filter = NO_NODE_FILTER,
                               std::function<bool(graph::Edge const &)> const &edge_filter = NO_EDGE_FILTER);


        static std::string getEdgeWeightLabel(const graph::AlternativeGraph &graph, const graph::Edge &edge);


    private:
        static const std::function<bool(graph::Node const &)> NO_NODE_FILTER;
        static const std::function<bool(graph::Edge const &)> NO_EDGE_FILTER;
    };

}// namespace fb
