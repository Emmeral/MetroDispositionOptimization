
#pragma once
#include "Node.h"
#include "definitions.h"
#include "model/definitions.h"
#include <cstdint>

namespace fb::graph {

    typedef int weight_t;

    union Weight {
        weight_t weight;
        index_t time_dependent_index;
    };

    struct TaggedWeight {

        static TaggedWeight fixed(weight_t weight) {
            TaggedWeight w{};
            w.time_dependent = false;
            w.weight.weight = weight;
            return w;
        }
        static TaggedWeight dependent(index_t time_dependent_index) {
            TaggedWeight w{};
            w.time_dependent = true;
            w.weight.time_dependent_index = time_dependent_index;
            return w;
        }

        bool time_dependent;
        Weight weight;
    };

    struct Edge {
        edge_index_t index;
        node_index_t from;
        node_index_t to;
        Weight weight;
        bool is_fixed;
        bool is_time_dependent;
    };

}// namespace fb::graph
