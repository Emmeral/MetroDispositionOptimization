
#pragma once
#include "Choice.h"
#include "Edge.h"
#include "definitions.h"
#include "model/definitions.h"
#include <vector>

namespace fb::graph {

    struct Node {
        index_t index;

        std::vector<edge_index_t> outgoing_edges;
        std::vector<a_edge_index_t> outgoing_alternative_edges;

        std::vector<edge_index_t> incoming_edges;
        std::vector<a_edge_index_t> incoming_alternative_edges;


        auto totalOutgoingEdgeCount() const { return outgoing_edges.size() + outgoing_alternative_edges.size(); }
    };

}// namespace fb::graph
