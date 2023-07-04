#pragma once

#include "algo/path/LongestPathCalculator.h"
#include "exceptions/CycleException.h"
#include "model/graph/GraphWithMeta.h"

namespace fb {


    std::vector<std::string> getTopologicalNameHints(const graph::AlternativeGraph &graph,
                                                     LongestPathCalculator::State const &distance_state,
                                                     OnlineTopologicalSorter::State const &sorter_state);


    void logCycleException(const graph::GraphWithMeta &gwm, const CycleException &e);


    std::string nowAsString(std::string const &format = "%Y-%m-%d %H:%M:%S");

    template<typename T>
    static std::string reverseLookup(std::unordered_map<std::string, T> const &map, T const &value) {
        for (auto const &[map_key, map_value]: map) {
            if (map_value == value) {
                return map_key;
            }
        }

        throw std::invalid_argument("Value not in map");
    }

}// namespace fb
