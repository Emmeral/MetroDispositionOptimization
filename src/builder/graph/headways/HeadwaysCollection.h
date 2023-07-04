#pragma once


namespace fb {

    struct HeadwayCollection {
        HeadwayCollection(size_t node_count, size_t link_count)
            : line_headways(link_count), fixed_line_headways_(link_count), station_headways(node_count),
              fixed_station_headways(node_count) {}

        std::vector<std::map<abs_pair, graph::choice_index_t>> line_headways{};
        std::vector<std::map<abs_pair, graph::AlternativeIndex>> fixed_line_headways_{};
        std::vector<DirectionMappedHeadways> station_headways{};
        std::vector<DirectionMappedFixedHeadways> fixed_station_headways{};
    };
}// namespace fb
