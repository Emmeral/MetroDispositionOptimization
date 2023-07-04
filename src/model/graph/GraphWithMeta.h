
#pragma once


#include "model/graph/AlternativeGraph.h"
#include "model/graph/abstractions/Abstractions.h"

#include "MeasureNodes.h"
#include "algo/path/definitions.h"
#include "model/main/Model.h"
#include "model/problems/ProblemInstance.h"

namespace fb::graph {

    struct GraphWithMeta {

        GraphWithMeta(size_t node_count, size_t course_count, size_t link_count)
            : abstractions(node_count, course_count, link_count){};

        AlternativeGraph graph{};
        node_index_t start_node{invalid<node_index_t>()};
        MeasureNodes measure_nodes{};
        std::vector<course_index_t> measure_node_courses{};

        // note that this does not contain distances to all nodes;
        std::vector<distance_t> no_constrain_distances{};

        Abstractions abstractions;
    };
}// namespace fb::graph
