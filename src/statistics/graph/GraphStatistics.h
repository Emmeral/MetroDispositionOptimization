
#pragma once

#include "Json.h"
#include "model/graph/GraphWithMeta.h"
#include "model/graph/abstractions/Abstractions.h"
#include "statistics/ChoiceSplitStatistics.h"
namespace fb {

    struct GraphStatistics {

        GraphStatistics(graph::GraphWithMeta gwm, unsigned long build_time_ms);

        unsigned long build_time_ms{0};

        size_t vertex_count{0};
        size_t fixed_edge_count{0};
        size_t alternative_edge_count{0};

        size_t measure_vertex_count{0};

        ChoiceSplitStatistic<size_t> choice_count{};
        ChoiceSplitStatistic<size_t> edge_count_by_choice_type{};


        size_t abstraction_count{0};

        size_t fully_decided_single_count{0};
        size_t multi_track_single_count{0};
        size_t track_decided_single_count{0};
        size_t activity_decided_single_count{0};
        size_t multi_track_link_count{0};
        size_t fully_decided_link_count{0};
        size_t realized_link_count{0};
        size_t realized_single_count{0};
    };

    std::ostream &operator<<(std::ostream &os, GraphStatistics const &stats);

    template<>
    std::string toJson(GraphStatistics const &stats);

}// namespace fb
